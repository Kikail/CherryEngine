//
// Created by killian on 3/18/26.
// physicsWorld.h

#ifndef CHERRYENGINE_PHYSICSWORLD_H
#define CHERRYENGINE_PHYSICSWORLD_H

#include "physicsObject.h"
#include "../editor/ecs/components/transform.h"
#include "collision.h"

#define PHYSICS_MAX_OBJECTS 5000
typedef struct PhysicsWorld_t {
    PhysicsObject physicsObjects[PHYSICS_MAX_OBJECTS]; unsigned int numPhysicsObjects;
    Collision collisions[PHYSICS_MAX_OBJECTS*2]; unsigned int numCollisions;
    vec3s gravity;
}PhysicsWorld;
PhysicsWorld PhysicsWorld_create();
PhysicsObject* PhysicsWorld_addObject(PhysicsWorld *world);
void PhysicsWorld_addCollision(PhysicsWorld* world, Collision* collision);
bool PhysicsWorld_removeObject(PhysicsWorld *world, PhysicsObject* physicsObject);
void PhysicsWorld_step(PhysicsWorld *world, float deltaTime);
void PhysicsWorld_resolveCollisions(PhysicsWorld *world, float deltaTime);
void PhysicsWorld_explosion(PhysicsWorld *world, vec3s pos, float radius, float intensity);

#endif //CHERRYENGINE_PHYSICSWORLD_H