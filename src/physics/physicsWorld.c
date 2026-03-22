#include "physicsWorld.h"
#include "testCollision.h"
#include <stdlib.h>
#include <math.h>

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

PhysicsWorld PhysicsWorld_create() {
    PhysicsWorld world;
    world.numPhysicsObjects = 0;
    world.numCollisions = 0;
    world.gravity = (vec3s){0, -9.81f, 0};

    float groundSize = 30.0f; // La moitié de 30
    float wallHeight = 1.5f;  // La moitié de 3
    float wallThickness = 0.5f;

    // 1. LE SOL (Centre : 0, -5, 0)
    addWall(&world, (vec3s){0.0f, -5.0f, 0.0f}, (vec3s){groundSize, 0.5f, groundSize});

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
        float rx = ((float)rand() / (float)RAND_MAX) * 28.0f - 14.0f;
        float rz = ((float)rand() / (float)RAND_MAX) * 28.0f - 14.0f;

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

        // --- DANS TA BOUCLE DE COLLISION ---

        // 1. Calculer la vitesse relative
        vec3s rv = glms_vec3_sub(b->Velocity, a->Velocity);

        // 2. Calculer la composante tangente (perpendiculaire à la normale)
        vec3s tangent = glms_vec3_sub(rv, glms_vec3_scale(cp->Normal, glms_vec3_dot(rv, cp->Normal)));

        if (glms_vec3_norm(tangent) > 0.001f) {
            tangent = glms_vec3_normalize(tangent);
        }

        // 3. Calculer la magnitude de l'impulsion de friction (Coulomb's Law)
        float jt = -glms_vec3_dot(rv, tangent);
        jt /= totalInvMass;

        // Friction simple (entre 0.0 et 1.0)
        float mu = 5.0;

        // On limite la friction par l'impulsion normale (j) pour ne pas freiner plus que l'impact
        float frictionImpulseMagnitude = fmaxf(-j * mu, fminf(jt, j * mu));

        vec3s frictionImpulse = glms_vec3_scale(tangent, frictionImpulseMagnitude);

        // 4. Appliquer au joueur et aux objets
        a->Velocity = glms_vec3_sub(a->Velocity, glms_vec3_scale(frictionImpulse, invMassA));
        b->Velocity = glms_vec3_add(b->Velocity, glms_vec3_scale(frictionImpulse, invMassB));
    }
}
void PhysicsWorld_addCollision(PhysicsWorld* world, Collision* collision) {
    if (world->numCollisions < PHYSICS_MAX_OBJECTS * 2) {
        world->collisions[world->numCollisions] = *collision;
        world->numCollisions += 1;
    }
}

void PhysicsWorld_resolveCollisions(PhysicsWorld *world, float deltaTime) {
    world->numCollisions = 0;

    for (int i = 0; i < world->numPhysicsObjects; i++) {
        // CORRECTION 1 : j = i + 1 (pour ne pas tester un objet contre lui-même)
        for (int j = i + 1; j < world->numPhysicsObjects; j++) {

            // CORRECTION 2 : obj.Collider est déjà un pointeur, on enlève le '&'
            CollisionPoints collisionPoints = Collisions_testCollisions(
                world->physicsObjects[i].Collider, &world->physicsObjects[i].Transform,
                world->physicsObjects[j].Collider, &world->physicsObjects[j].Transform
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

void PhysicsWorld_impulse(PhysicsWorld *world, float deltaTime, vec3s position, float intensity, float attenuationRadius) {
    for (int i = 0; i < world->numPhysicsObjects; i++) {
        PhysicsObject* obj = &world->physicsObjects[i];

        // On n'explose pas le sol ou les murs statiques
        if (obj->PhysicsType == STATIC || obj->PhysicsTag == PLAYER) continue;

        // 1. Calculer le vecteur direction (de l'explosion vers l'objet)
        vec3s dir = glms_vec3_sub(obj->Transform.position, position);
        float distance = glms_vec3_norm(dir);

        // 2. Vérifier si l'objet est dans le rayon d'action
        if (distance < attenuationRadius && distance > 0.0001f) {
            // Normaliser le vecteur direction
            dir = glms_vec3_normalize(dir);

            // 3. Calculer l'atténuation (0.0 à 1.0)
            // Plus on est loin, moins c'est fort
            float strength = 1.0f - (distance / attenuationRadius);

            // 4. Calculer la force finale
            float forceMagnitude = intensity * strength;
            vec3s impulse = glms_vec3_scale(dir, forceMagnitude);

            // 5. Appliquer directement à la vitesse (Impulsion instantanée)
            // On divise par la masse : F = ma -> a = F/m
            vec3s velocityChange = glms_vec3_scale(impulse, 1.0f / obj->Mass);
            obj->Velocity = glms_vec3_add(obj->Velocity, velocityChange);
        }
    }
}