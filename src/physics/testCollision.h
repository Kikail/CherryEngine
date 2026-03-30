// test_collision.h
// Détection de collisions narrowphase pour tous les types de colliders.
// Tous les algorithmes retournent des CollisionPoints avec la normale de A vers B.

#ifndef CHERRYENGINE_TESTCOLLISION_H
#define CHERRYENGINE_TESTCOLLISION_H

#include "colliders.h"
#include "physicsObject.h"

// ─── Dispatcher principal ────────────────────────────────────────────────────
// Choisit automatiquement l'algorithme selon les types de colliders.
// Respecte le filtre layer/mask.
CollisionPoints Collisions_test(PhysicsObject* a, PhysicsObject* b);

// ─── Paires Sphère ───────────────────────────────────────────────────────────
CollisionPoints Collisions_sphereSphere (PhysicsObject* a, PhysicsObject* b);
CollisionPoints Collisions_spherePlane  (PhysicsObject* a, PhysicsObject* b);
CollisionPoints Collisions_sphereCube   (PhysicsObject* a, PhysicsObject* b);  // sphere vs OBB
CollisionPoints Collisions_sphereCapsule(PhysicsObject* a, PhysicsObject* b);

// ─── Paires Cube (OBB) ───────────────────────────────────────────────────────
CollisionPoints Collisions_cubePlane    (PhysicsObject* a, PhysicsObject* b);
CollisionPoints Collisions_cubeCube     (PhysicsObject* a, PhysicsObject* b);  // SAT 15 axes
CollisionPoints Collisions_cubeCapsule  (PhysicsObject* a, PhysicsObject* b);

// ─── Paires Capsule ──────────────────────────────────────────────────────────
CollisionPoints Collisions_capsulePlane  (PhysicsObject* a, PhysicsObject* b);
CollisionPoints Collisions_capsuleCapsule(PhysicsObject* a, PhysicsObject* b);

// ─── Raycasting narrowphase ──────────────────────────────────────────────────
// Retourne la distance le long du rayon jusqu'à l'intersection, -1 si aucune.
float Collisions_raycastSphere (vec3s origin, vec3s dir, float maxDist, PhysicsObject* obj);
float Collisions_raycastBox    (vec3s origin, vec3s dir, float maxDist, PhysicsObject* obj);
float Collisions_raycastCapsule(vec3s origin, vec3s dir, float maxDist, PhysicsObject* obj);
float Collisions_raycastPlane  (vec3s origin, vec3s dir, float maxDist, PhysicsObject* obj);

#endif // CHERRYENGINE_TESTCOLLISION_H
