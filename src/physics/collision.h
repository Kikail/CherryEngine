//
// Created by killian on 3/19/26.
// collision.h

#ifndef CHERRYENGINE_COLLISION_H
#define CHERRYENGINE_COLLISION_H

#include "physicsObject.h"

typedef struct Collision_t {
    PhysicsObject* objectA;
    PhysicsObject* objectB;
    CollisionPoints Points;
}Collision;

#endif //CHERRYENGINE_COLLISION_H