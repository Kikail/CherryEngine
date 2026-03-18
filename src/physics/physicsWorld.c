//
// Created by killian on 3/18/26.
//
#include "physicsWorld.h"

PhysicsWorld PhysicsWorld_create() {
    PhysicsWorld world;
    world.numPhysicsObjects = 0;
    world.gravity = (vec3s){0, -9.81f, 0};
    return world;
}
PhysicsObject* PhysicsWorld_addObject(PhysicsWorld *world) {
    if (world->numPhysicsObjects < PHYSICS_MAX_OBJECTS) {
        world->numPhysicsObjects += 1;
        PhysicsObject* obj = &world->physicsObjects[world->numPhysicsObjects - 1];
        obj->Mass = 1.0;

        float rz = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;

        obj->Position = (vec3s){2.0,0.0,-4.0};
        obj->Velocity = (vec3s){15.0,15.0 + rz * 3,rz * 10};
        return obj;
    }
    else {
        return NULL;
    }
}

void PhysicsWorld_step(PhysicsWorld *world, float deltaTime) {
    // Sécurité pour éviter les divisions par zéro ou les deltas énormes au démarrage
    if (deltaTime <= 0.0f || deltaTime > 0.1f) return;

    for (int i = 0; i < world->numPhysicsObjects; i++) {
        PhysicsObject* obj = &world->physicsObjects[i];

        // 1. Calcul de l'accélération : a = F / m
        // On ajoute la gravité à la force totale (F_tot = F_appliquée + m*g)
        vec3s gravityForce = glms_vec3_scale(world->gravity, obj->Mass);
        vec3s totalForce = glms_vec3_add(obj->Force, gravityForce);

        vec3s acceleration = glms_vec3_scale(totalForce, 1.0f / obj->Mass);

        // 2. Mise à jour de la vitesse : v = v + a * dt
        vec3s deltaV = glms_vec3_scale(acceleration, deltaTime);
        obj->Velocity = glms_vec3_add(obj->Velocity, deltaV);

        // 3. Mise à jour de la position : p = p + v * dt
        vec3s deltaP = glms_vec3_scale(obj->Velocity, deltaTime);
        obj->Position = glms_vec3_add(obj->Position, deltaP);

        // 4. Reset de la force accumulée pour la prochaine frame
        obj->Force = (vec3s){0.0f, 0.0f, 0.0f};
    }
}