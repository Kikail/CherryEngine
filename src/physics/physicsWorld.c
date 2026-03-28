#include "physicsWorld.h"
#include "testCollision.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

// -----------------------------------------------------------------------------
// Utilitaires locaux
// -----------------------------------------------------------------------------

static void initPhysicsObject(PhysicsObject* obj) {
    if (!obj) return;
    memset(obj, 0, sizeof(*obj));
    obj->Orientation = glms_quat_identity();
    obj->InertiaTensorInv = (mat3s){0};
}

static void addWall(PhysicsWorld* world, vec3s pos, vec3s halfSize) {
    if (!world || world->numPhysicsObjects >= PHYSICS_MAX_OBJECTS) return;

    PhysicsObject* wall = &world->physicsObjects[world->numPhysicsObjects++];
    initPhysicsObject(wall);

    Collider* collider = (Collider*)malloc(sizeof(Collider));
    BoxCollider* box = (BoxCollider*)malloc(sizeof(BoxCollider));
    if (!collider || !box) {
        free(collider);
        free(box);
        world->numPhysicsObjects--;
        return;
    }

    collider->type = CUBE;
    box->HalfSize = halfSize;
    collider->collider = box;

    wall->Collider = collider;
    wall->PhysicsType = STATIC;
    wall->PhysicsTag = ENVIRONMENT;

    wall->Transform.position = pos;
    wall->Velocity = glms_vec3_zero();
    wall->Force = glms_vec3_zero();
    wall->AngularVelocity = glms_vec3_zero();
    wall->Torque = glms_vec3_zero();

    wall->Mass = 0.0f;
    wall->InverseMass = 0.0f;
    wall->InertiaTensorInv = (mat3s){0};
}

static mat3s CalculateBoxInertiaInv(float mass, vec3s halfSize) {
    if (mass <= 0.0f) {
        return (mat3s){0};
    }

    float dx = halfSize.x * 2.0f;
    float dy = halfSize.y * 2.0f;
    float dz = halfSize.z * 2.0f;

    // Inertie d'un pavé autour du centre de masse
    float factor = mass / 12.0f;

    mat3s inertia = (mat3s){0};
    inertia.m00 = factor * (dy * dy + dz * dz);
    inertia.m11 = factor * (dx * dx + dz * dz);
    inertia.m22 = factor * (dx * dx + dy * dy);

    return glms_mat3_inv(inertia);
}

static void ApplyDampingToObject(PhysicsObject* obj, float deltaTime) {
    if (!obj || obj->PhysicsType == STATIC) return;

    // Damping exponentiel, plus stable que des facteurs fixes par frame
    float linearDamping = expf(-1.5f * deltaTime);
    float angularDamping = expf(-4.0f * deltaTime);

    obj->Velocity = glms_vec3_scale(obj->Velocity, linearDamping);
    obj->AngularVelocity = glms_vec3_scale(obj->AngularVelocity, angularDamping);

    // Seuils pour tuer les micro-mouvements
    if (glms_vec3_norm2(obj->Velocity) < 0.000001f) {
        obj->Velocity = glms_vec3_zero();
    }

    if (glms_vec3_norm2(obj->AngularVelocity) < 0.000001f) {
        obj->AngularVelocity = glms_vec3_zero();
    }
}

// -----------------------------------------------------------------------------
// API
// -----------------------------------------------------------------------------

PhysicsWorld PhysicsWorld_create() {
    PhysicsWorld world;
    memset(&world, 0, sizeof(world));

    world.gravity = (vec3s){0.0f, -9.81f, 0.0f};

    float groundSize = 30.0f;

    // Sol
    addWall(&world, (vec3s){0.0f, -5.0f, 0.0f}, (vec3s){groundSize, 0.5f, groundSize});

    return world;
}

PhysicsObject* PhysicsWorld_addObject(PhysicsWorld* world) {
    if (!world || world->numPhysicsObjects >= PHYSICS_MAX_OBJECTS) return NULL;

    PhysicsObject* obj = &world->physicsObjects[world->numPhysicsObjects++];
    initPhysicsObject(obj);

    Collider* collider = (Collider*)malloc(sizeof(Collider));
    BoxCollider* boxCollider = (BoxCollider*)malloc(sizeof(BoxCollider));
    if (!collider || !boxCollider) {
        free(collider);
        free(boxCollider);
        world->numPhysicsObjects--;
        return NULL;
    }

    collider->type = CUBE;
    boxCollider->HalfSize = (vec3s){0.5f, 0.5f, 0.5f};
    collider->collider = boxCollider;

    obj->Collider = collider;
    obj->PhysicsType = DYNAMIC;
    obj->PhysicsTag = ENVIRONMENT;

    obj->Mass = 1.0f;
    obj->InverseMass = 1.0f / obj->Mass;

    float rx = ((float)rand() / (float)RAND_MAX) * 28.0f - 14.0f;
    float rz = ((float)rand() / (float)RAND_MAX) * 28.0f - 14.0f;

    obj->Transform.position = (vec3s){rx, 15.0f, rz};
    obj->Velocity = (vec3s){
        ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f,
        0.0f,
        ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f
    };

    obj->Force = glms_vec3_zero();
    obj->Orientation = glms_quat_identity();
    obj->AngularVelocity = glms_vec3_zero();
    obj->Torque = glms_vec3_zero();

    obj->InertiaTensorInv = CalculateBoxInertiaInv(obj->Mass, boxCollider->HalfSize);

    return obj;
}

void PhysicsWorld_step(PhysicsWorld* world, float deltaTime) {
    if (!world || deltaTime <= 0.0f) return;

    for (int i = 0; i < world->numPhysicsObjects; i++) {
        PhysicsObject* obj = &world->physicsObjects[i];
        if (obj->PhysicsType == STATIC) continue;

        // ---------------------------------------------------------------------
        // Intégration linéaire
        // a = g + F/m
        // ---------------------------------------------------------------------
        vec3s accel = world->gravity;
        if (obj->InverseMass > 0.0f) {
            accel = glms_vec3_add(accel, glms_vec3_scale(obj->Force, obj->InverseMass));
        }

        obj->Velocity = glms_vec3_add(obj->Velocity, glms_vec3_scale(accel, deltaTime));
        obj->Transform.position = glms_vec3_add(obj->Transform.position, glms_vec3_scale(obj->Velocity, deltaTime));

        // ---------------------------------------------------------------------
        // Intégration angulaire
        // ---------------------------------------------------------------------
        vec3s angularAccel = glms_mat3_mulv(obj->InertiaTensorInv, obj->Torque);
        obj->AngularVelocity = glms_vec3_add(obj->AngularVelocity, glms_vec3_scale(angularAccel, deltaTime));

        // q' = 0.5 * omega * q
        versors q = obj->Orientation;
        vec3s w = obj->AngularVelocity;

        versors wq;
        wq.raw[0] =  w.x * q.raw[3] + w.y * q.raw[2] - w.z * q.raw[1];
        wq.raw[1] =  w.y * q.raw[3] + w.z * q.raw[0] - w.x * q.raw[2];
        wq.raw[2] =  w.z * q.raw[3] + w.x * q.raw[1] - w.y * q.raw[0];
        wq.raw[3] = -w.x * q.raw[0] - w.y * q.raw[1] - w.z * q.raw[2];

        obj->Orientation.raw[0] += wq.raw[0] * 0.5f * deltaTime;
        obj->Orientation.raw[1] += wq.raw[1] * 0.5f * deltaTime;
        obj->Orientation.raw[2] += wq.raw[2] * 0.5f * deltaTime;
        obj->Orientation.raw[3] += wq.raw[3] * 0.5f * deltaTime;
        obj->Orientation = glms_quat_normalize(obj->Orientation);

        // Reset des accumulateurs
        obj->Force = glms_vec3_zero();
        obj->Torque = glms_vec3_zero();
    }

    PhysicsWorld_resolveCollisions(world, deltaTime);

    for (int i = 0; i < world->numCollisions; i++) {
        Collision* col = &world->collisions[i];
        PhysicsObject* a = col->objectA;
        PhysicsObject* b = col->objectB;
        CollisionPoints* cp = &col->Points;

        if (!cp->HasCollision) continue;

        float invMassA = (a->PhysicsType == STATIC) ? 0.0f : a->InverseMass;
        float invMassB = (b->PhysicsType == STATIC) ? 0.0f : b->InverseMass;
        float totalInvMass = invMassA + invMassB;

        if (totalInvMass <= 0.0f) continue;

        // ---------------------------------------------------------------------
        // 1) Correction de pénétration
        // ---------------------------------------------------------------------
        float slop = 0.01f;
        float percent = 0.6f;
        float penetration = fmaxf(cp->Depth - slop, 0.0f);

        vec3s correction = glms_vec3_scale(cp->Normal, (penetration / totalInvMass) * percent);

        if (a->PhysicsType != STATIC) {
            a->Transform.position = glms_vec3_sub(a->Transform.position, glms_vec3_scale(correction, invMassA));
        }
        if (b->PhysicsType != STATIC) {
            b->Transform.position = glms_vec3_add(b->Transform.position, glms_vec3_scale(correction, invMassB));
        }

        // Bras de levier
        vec3s rA = glms_vec3_sub(cp->A, a->Transform.position);
        vec3s rB = glms_vec3_sub(cp->B, b->Transform.position);

        // ---------------------------------------------------------------------
        // 2) Impulsion normale
        // ---------------------------------------------------------------------
        vec3s vA = glms_vec3_add(a->Velocity, glms_vec3_cross(a->AngularVelocity, rA));
        vec3s vB = glms_vec3_add(b->Velocity, glms_vec3_cross(b->AngularVelocity, rB));
        vec3s rv = glms_vec3_sub(vB, vA);

        float velAlongNormal = glms_vec3_dot(rv, cp->Normal);
        if (velAlongNormal > 0.0f) continue;

        vec3s raCrossN = glms_vec3_cross(rA, cp->Normal);
        vec3s rbCrossN = glms_vec3_cross(rB, cp->Normal);

        float angularTermA = 0.0f;
        float angularTermB = 0.0f;

        if (a->PhysicsType != STATIC) {
            angularTermA = glms_vec3_dot(glms_mat3_mulv(a->InertiaTensorInv, raCrossN), raCrossN);
        }
        if (b->PhysicsType != STATIC) {
            angularTermB = glms_vec3_dot(glms_mat3_mulv(b->InertiaTensorInv, rbCrossN), rbCrossN);
        }

        float e = 0.08f; // rebond faible pour un comportement plus stable
        float j = -(1.0f + e) * velAlongNormal;
        j /= (totalInvMass + angularTermA + angularTermB);

        vec3s impulse = glms_vec3_scale(cp->Normal, j);

        if (a->PhysicsType != STATIC) {
            a->Velocity = glms_vec3_sub(a->Velocity, glms_vec3_scale(impulse, invMassA));
            a->AngularVelocity = glms_vec3_sub(
                a->AngularVelocity,
                glms_mat3_mulv(a->InertiaTensorInv, glms_vec3_cross(rA, impulse))
            );
        }

        if (b->PhysicsType != STATIC) {
            b->Velocity = glms_vec3_add(b->Velocity, glms_vec3_scale(impulse, invMassB));
            b->AngularVelocity = glms_vec3_add(
                b->AngularVelocity,
                glms_mat3_mulv(b->InertiaTensorInv, glms_vec3_cross(rB, impulse))
            );
        }

        // ---------------------------------------------------------------------
        // 3) Friction tangentielle
        // ---------------------------------------------------------------------
        vA = glms_vec3_add(a->Velocity, glms_vec3_cross(a->AngularVelocity, rA));
        vB = glms_vec3_add(b->Velocity, glms_vec3_cross(b->AngularVelocity, rB));
        rv = glms_vec3_sub(vB, vA);

        vec3s tangent = glms_vec3_sub(rv, glms_vec3_scale(cp->Normal, glms_vec3_dot(rv, cp->Normal)));
        if (glms_vec3_norm2(tangent) > 0.0001f) {
            tangent = glms_vec3_normalize(tangent);

            vec3s raCrossT = glms_vec3_cross(rA, tangent);
            vec3s rbCrossT = glms_vec3_cross(rB, tangent);

            float angularTermTangentA = 0.0f;
            float angularTermTangentB = 0.0f;

            if (a->PhysicsType != STATIC) {
                angularTermTangentA = glms_vec3_dot(glms_mat3_mulv(a->InertiaTensorInv, raCrossT), raCrossT);
            }
            if (b->PhysicsType != STATIC) {
                angularTermTangentB = glms_vec3_dot(glms_mat3_mulv(b->InertiaTensorInv, rbCrossT), rbCrossT);
            }

            float jt = -glms_vec3_dot(rv, tangent);
            jt /= (totalInvMass + angularTermTangentA + angularTermTangentB);

            float mu = 0.45f;
            float maxFriction = fabsf(j) * mu;

            if (jt > maxFriction) jt = maxFriction;
            if (jt < -maxFriction) jt = -maxFriction;

            vec3s frictionImpulse = glms_vec3_scale(tangent, jt);

            if (a->PhysicsType != STATIC) {
                a->Velocity = glms_vec3_sub(a->Velocity, glms_vec3_scale(frictionImpulse, invMassA));
                a->AngularVelocity = glms_vec3_sub(
                    a->AngularVelocity,
                    glms_mat3_mulv(a->InertiaTensorInv, glms_vec3_cross(rA, frictionImpulse))
                );
            }

            if (b->PhysicsType != STATIC) {
                b->Velocity = glms_vec3_add(b->Velocity, glms_vec3_scale(frictionImpulse, invMassB));
                b->AngularVelocity = glms_vec3_add(
                    b->AngularVelocity,
                    glms_mat3_mulv(b->InertiaTensorInv, glms_vec3_cross(rB, frictionImpulse))
                );
            }
        }
    }

    // -------------------------------------------------------------------------
    // Damping final après collisions
    // -------------------------------------------------------------------------
    for (int i = 0; i < world->numPhysicsObjects; i++) {
        ApplyDampingToObject(&world->physicsObjects[i], deltaTime);
    }
}

void PhysicsWorld_addCollision(PhysicsWorld* world, Collision* collision) {
    if (!world || !collision) return;

    if (world->numCollisions < PHYSICS_MAX_OBJECTS * 2) {
        world->collisions[world->numCollisions] = *collision;
        world->numCollisions += 1;
    }
}

void PhysicsWorld_resolveCollisions(PhysicsWorld* world, float deltaTime) {
    (void)deltaTime;

    if (!world) return;

    world->numCollisions = 0;

    for (int i = 0; i < world->numPhysicsObjects; i++) {
        for (int j = i + 1; j < world->numPhysicsObjects; j++) {
            CollisionPoints collisionPoints = Collisions_testCollisions(
                &world->physicsObjects[i],
                &world->physicsObjects[j]
            );

            if (collisionPoints.HasCollision) {
                Collision collision;
                collision.objectA = &world->physicsObjects[i];
                collision.objectB = &world->physicsObjects[j];
                collision.Points = collisionPoints;
                PhysicsWorld_addCollision(world, &collision);
            }
        }
    }
}

void PhysicsWorld_impulse(PhysicsWorld* world, float deltaTime, vec3s position, float intensity, float attenuationRadius) {
    (void)deltaTime;

    if (!world || attenuationRadius <= 0.0f) return;

    for (int i = 0; i < world->numPhysicsObjects; i++) {
        PhysicsObject* obj = &world->physicsObjects[i];

        // On n'explose pas le sol ou les murs statiques
        if (obj->PhysicsType == STATIC || obj->PhysicsTag == PLAYER) continue;
        if (obj->Mass <= 0.0f) continue;

        // 1. Calculer le vecteur direction (de l'explosion vers l'objet)
        vec3s dir = glms_vec3_sub(obj->Transform.position, position);
        float distance = glms_vec3_norm(dir);

        // 2. Vérifier si l'objet est dans le rayon d'action
        if (distance < attenuationRadius && distance > 0.0001f) {
            dir = glms_vec3_normalize(dir);

            // 3. Atténuation
            float strength = 1.0f - (distance / attenuationRadius);

            // 4. Force finale
            float forceMagnitude = intensity * strength;
            vec3s impulse = glms_vec3_scale(dir, forceMagnitude);

            // 5. Application immédiate à la vitesse
            vec3s velocityChange = glms_vec3_scale(impulse, 1.0f / obj->Mass);
            obj->Velocity = glms_vec3_add(obj->Velocity, velocityChange);
        }
    }
}