//
// Created by killian on 3/18/26.
//

#ifndef CHERRYENGINE_PHYSICSWORLD_H
#define CHERRYENGINE_PHYSICSWORLD_H

#include "physicsObject.h"

#define PHYSICS_MAX_OBJECTS 1000

typedef struct PhysicsWorld_t {
    PhysicsObject physicsObjects[PHYSICS_MAX_OBJECTS]; unsigned int numPhysicsObjects;
    vec3s gravity;
}PhysicsWorld;
PhysicsWorld PhysicsWorld_create();
PhysicsObject* PhysicsWorld_addObject(PhysicsWorld *world);
bool PhysicsWorld_removeObject(PhysicsWorld *world, PhysicsObject* physicsObject);
void PhysicsWorld_step(PhysicsWorld *world, float deltaTime);

#endif //CHERRYENGINE_PHYSICSWORLD_H