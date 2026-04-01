#include "physicsWorld.h"
#include "testCollision.h"
#include <stdlib.h>
#include <math.h>

#include "collision.h"
#include "octree.h"

// Fonction utilitaire locale pour éviter de répéter le code des murs
void addWall(PhysicsWorld* world, vec3s pos, vec3s halfSize) {
    if (world->numPhysicsObjects >= PHYSICS_MAX_OBJECTS) return;

    PhysicsObject* wall = &world->physicsObjects[world->numPhysicsObjects++];

    Collider* collider = (Collider*)malloc(sizeof(Collider));
    BoxCollider* box = (BoxCollider*)malloc(sizeof(BoxCollider));

    collider->type = CUBE;
    box->HalfSize = halfSize;
    collider->collider = box;

    wall->Collider = collider;
    wall->PhysicsType = STATIC;
    wall->Transform.position = pos;
    wall->Velocity = glms_vec3_zero();
    wall->Mass = 1.0f;
    wall->Force = glms_vec3_zero();
}

void PhysicsWorld_explosion(PhysicsWorld *world, vec3s pos, float radius, float intensity) {
    for (int i = 0; i < world->numPhysicsObjects; i++) {
        PhysicsObject* obj = &world->physicsObjects[i];
        if (obj->PhysicsType == STATIC) continue;

        vec3s direction = glms_vec3_sub(obj->Transform.position, pos);
        float distance = glms_vec3_norm(direction);

        if (distance < radius && distance > 0.0001f) {
            vec3s dirNorm = glms_vec3_divs(direction, distance);
            float falloff = 1.0f - (distance / radius);
            float forceMagnitude = intensity * falloff;
            vec3s explosionForce = glms_vec3_scale(dirNorm, forceMagnitude);
            vec3s impulse = glms_vec3_scale(explosionForce, 1.0f / obj->Mass);
            obj->Velocity = glms_vec3_add(obj->Velocity, impulse);
        }
    }
}

PhysicsWorld* PhysicsWorld_create() {
    PhysicsWorld* world = malloc(sizeof(PhysicsWorld));
    world->numPhysicsObjects = 0;
    world->numCollisions = 0;
    world->gravity = (vec3s){0, -9.81f, 0};
    world->debug = false;

    float groundSize = WORLD_BOUND; // La moitié de 30
    float wallHeight = 1.5f;  // La moitié de 3
    float wallThickness = 0.5f;

    // 1. LE SOL (Centre : 0, -5, 0)
    addWall(world, (vec3s){0.0f, -5.0f, 0.0f}, (vec3s){groundSize, 0.5f, groundSize});

    // 2. MUR NORD (Z+)
    addWall(world,
        (vec3s){0.0f, -5.0f + wallHeight, groundSize},
        (vec3s){groundSize, wallHeight, wallThickness});

    // 3. MUR SUD (Z-)
    addWall(world,
        (vec3s){0.0f, -5.0f + wallHeight, -groundSize},
        (vec3s){groundSize, wallHeight, wallThickness});

    // 4. MUR EST (X+)
    addWall(world,
        (vec3s){groundSize, -5.0f + wallHeight, 0.0f},
        (vec3s){wallThickness, wallHeight, groundSize});

    // 5. MUR OUEST (X-)
    addWall(world,
        (vec3s){-groundSize, -5.0f + wallHeight, 0.0f},
        (vec3s){wallThickness, wallHeight, groundSize});

    return world;
}

PhysicsObject* PhysicsWorld_addObject(PhysicsWorld *world) {
    if (world->numPhysicsObjects < PHYSICS_MAX_OBJECTS) {
        world->numPhysicsObjects += 1;
        PhysicsObject* obj = &world->physicsObjects[world->numPhysicsObjects - 1];

        // --- CREATION D'UN CUBE DYNAMIQUE ---
        Collider* collider = (Collider*)malloc(sizeof(Collider));
        BoxCollider* boxCollider = (BoxCollider*)malloc(sizeof(BoxCollider));

        collider->type = CUBE;
        // Cube de taille 1x1x1 standard (HalfSize = 0.5)
        boxCollider->HalfSize = (vec3s){0.5f, 0.5f, 0.5f};

        collider->collider = boxCollider;
        obj->Collider = collider;

        obj->PhysicsType = DYNAMIC;
        obj->Mass = 1.0f;

        // Spawn aléatoire sur la surface du plateau (entre -14 et 14 pour pas tomber direct)
        float worldSize = WORLD_BOUND - 5;
        float rx = ((float)rand() / (float)RAND_MAX) * (2*worldSize) - worldSize;
        float rz = ((float)rand() / (float)RAND_MAX) * (2*worldSize) - worldSize;

        obj->Transform.position = (vec3s){rx, 15.0f, rz};

        // Petite impulsion aléatoire
        float vx = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        float vz = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        obj->Velocity = (vec3s){vx, 0.0f, vz};

        obj->Force = (vec3s){0.0f, 0.0f, 0.0f};

        return obj;
    }
    return NULL;
}

// physicsWorld.c

void PhysicsWorld_step(PhysicsWorld *world, float deltaTime) {
    // 1. D'ABORD : Appliquer la gravité et mettre à jour la position
    for (int i = 0; i < world->numPhysicsObjects; i++) {
        PhysicsObject* obj = &world->physicsObjects[i];
        if (obj->PhysicsType == STATIC) continue;

        vec3s gravityForce = glms_vec3_scale(world->gravity, obj->Mass);
        vec3s totalForce = glms_vec3_add(obj->Force, gravityForce);
        vec3s acceleration = glms_vec3_scale(totalForce, 1.0f / obj->Mass);

        // Mise à jour vitesse et position (Euler semi-implicite)
        obj->Velocity = glms_vec3_add(obj->Velocity, glms_vec3_scale(acceleration, deltaTime));
        obj->Transform.position = glms_vec3_add(obj->Transform.position, glms_vec3_scale(obj->Velocity, deltaTime));
        obj->Force = (vec3s){0.0f, 0.0f, 0.0f};
    }

    // 2. ENSUITE : Résoudre les collisions pour corriger la position et la vitesse
    PhysicsWorld_resolveCollisions(world, deltaTime);

    for (int i = 0; i < world->numCollisions; i++) {
        PhysicsObject* a = world->collisions[i].objectA;
        PhysicsObject* b = world->collisions[i].objectB;
        CollisionPoints* cp = &world->collisions[i].Points;

        float invMassA = (a->PhysicsType == STATIC) ? 0.0f : 1.0f / a->Mass;
        float invMassB = (b->PhysicsType == STATIC) ? 0.0f : 1.0f / b->Mass;
        float totalInvMass = invMassA + invMassB;
        if (totalInvMass <= 0.0f) continue;

        // --- SÉPARATION (Anti-tremblement) ---
        float slop = 0.02f; // Marge de tolérance augmentée
        float percent = 0.5f; // On sépare plus doucement
        float correctionMagnitude = (fmaxf(cp->Depth - slop, 0.0f) / totalInvMass) * percent;
        vec3s correctionVector = glms_vec3_scale(cp->Normal, correctionMagnitude);

        a->Transform.position = glms_vec3_sub(a->Transform.position, glms_vec3_scale(correctionVector, invMassA));
        b->Transform.position = glms_vec3_add(b->Transform.position, glms_vec3_scale(correctionVector, invMassB));

        // --- RÉPONSE D'IMPULSION ---
        vec3s relativeVelocity = glms_vec3_sub(b->Velocity, a->Velocity);
        float velAlongNormal = glms_vec3_dot(relativeVelocity, cp->Normal);

        if (velAlongNormal > 0) continue; // S'éloignent déjà

        float e = 0.2f; // Rebond faible pour la stabilité
        float j = -(1.0f + e) * velAlongNormal;
        j /= totalInvMass;

        vec3s impulse = glms_vec3_scale(cp->Normal, j);
        a->Velocity = glms_vec3_sub(a->Velocity, glms_vec3_scale(impulse, invMassA));
        b->Velocity = glms_vec3_add(b->Velocity, glms_vec3_scale(impulse, invMassB));

        relativeVelocity = glms_vec3_sub(b->Velocity, a->Velocity);
        float vDotN = glms_vec3_dot(relativeVelocity, cp->Normal);
        vec3s tangent = glms_vec3_sub(relativeVelocity, glms_vec3_scale(cp->Normal, vDotN));

        if (glms_vec3_norm(tangent) > 0.0001f) {
            tangent = glms_vec3_normalize(tangent);
            float jt = -glms_vec3_dot(relativeVelocity, tangent);
            jt /= totalInvMass;
            float mu = FRICTION_COEF;
            vec3s frictionImpulse = glms_vec3_scale(tangent, jt * mu);
            a->Velocity = glms_vec3_sub(a->Velocity, glms_vec3_scale(frictionImpulse, invMassA));
            b->Velocity = glms_vec3_add(b->Velocity, glms_vec3_scale(frictionImpulse, invMassB));
        }
    }
}
void PhysicsWorld_addCollision(PhysicsWorld* world, Collision* collision) {
    if (world->numCollisions < PHYSICS_MAX_OBJECTS * 2) {
        world->collisions[world->numCollisions] = *collision;
        world->numCollisions += 1;
    }
}

void checkNodeCollisions(PhysicsWorld* world, OctreeNode* node) {
    if (node == NULL) return;

    // Si on est dans une feuille, on teste les objets entre eux
    if (node->isLeaf) {
        for (unsigned int i = 0; i < node->numObjects; i++) {
            for (unsigned int j = i + 1; j < node->numObjects; j++) {
                PhysicsObject* objA = node->objects[i];
                PhysicsObject* objB = node->objects[j];

                // On effectue le test de collision réel (Narrow Phase)
                CollisionPoints collisionPoints = Collisions_testCollisions(
                    objA->Collider, &objA->Transform,
                    objB->Collider, &objB->Transform
                );

                if (collisionPoints.HasCollision) {
                    Collision collision;
                    collision.objectA = objA;
                    collision.objectB = objB;
                    collision.Points = collisionPoints;
                    PhysicsWorld_addCollision(world, &collision);
                }
            }
        }
    }
    // Sinon, on descend dans les enfants
    else{
        for (int i = 0; i < 8; i++) {
            unsigned int position = node->firstChildIndex + i;
            checkNodeCollisions(world, Octree_getNode(position));
        }
    }
}

void PhysicsWorld_resolveCollisions(PhysicsWorld *world, float deltaTime) {
    world->numCollisions = 0;

    AABB worldBounds;
    worldBounds.min = (vec3s){-WORLD_BOUND, -5.0f, -WORLD_BOUND};
    worldBounds.max = (vec3s){WORLD_BOUND, 30.0f, WORLD_BOUND};

    Octree_ResetPool(worldBounds);

    for (int i = 0; i < world->numPhysicsObjects; i++) {
        Octree_addElement(Octree_getNode(0), &world->physicsObjects[i]);
    }

    checkNodeCollisions(world, Octree_getNode(0));

    if (world->debug && world->debugShader!=NULL) {
        Octree_draw(Octree_getNode(0), world->debugShader);
    }

    Octree_clean(Octree_getNode(0));
}

void PhysicsWorld_afficherOctree(PhysicsWorld *world, bool b, Shader* shader) {
    world->debug = b;
    world->debugShader = shader;
}