//
// Created by killian on 3/19/26.
// testCollision.h

#ifndef CHERRYENGINE_TESTCOLLISION_H
#define CHERRYENGINE_TESTCOLLISION_H

#include "colliders.h"

CollisionPoints Collisions_testCollisions(Collider* a, Transform* ta, Collider* b, Transform* tb);
CollisionPoints Collisions_sphereSphere(Collider* a, Transform* ta, Collider* b, Transform* tb);
CollisionPoints Collisions_spherePlane(Collider* a, Transform* ta, Collider* b, Transform* tb);
CollisionPoints Collisions_sphereCube(Collider* a, Transform* ta, Collider* b, Transform* tb);
CollisionPoints Collisions_cubePlane(Collider* a, Transform* ta, Collider* b, Transform* tb);
CollisionPoints Collisions_cubeCube(Collider* a, Transform* ta, Collider* b, Transform* tb);

#endif //CHERRYENGINE_TESTCOLLISION_H