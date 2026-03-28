#ifndef CHERRYENGINE_TESTCOLLISION_H
#define CHERRYENGINE_TESTCOLLISION_H

#include "colliders.h"
#include "physicsWorld.h" // Nécessaire pour accéder aux orientations des objets

// Dispatcher principal
CollisionPoints Collisions_testCollisions(PhysicsObject* a, PhysicsObject* b);

// Algorithmes spécifiques
CollisionPoints Collisions_sphereSphere(PhysicsObject* a, PhysicsObject* b);
CollisionPoints Collisions_spherePlane(PhysicsObject* a, PhysicsObject* b);
CollisionPoints Collisions_sphereCube(PhysicsObject* a, PhysicsObject* b);
CollisionPoints Collisions_cubePlane(PhysicsObject* a, PhysicsObject* b);
CollisionPoints Collisions_cubeCube(PhysicsObject* a, PhysicsObject* b);

#endif //CHERRYENGINE_TESTCOLLISION_H