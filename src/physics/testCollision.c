#include "testCollision.h"
#include <math.h>
#include <stdio.h>

// --- UTILITAIRES ---

// Retourne le point du cube le plus proche d'un centre donné (AABB)
vec3s GetClosestPointAABB(vec3s center, vec3s boxPos, vec3s halfSize) {
    vec3s closest;
    vec3s minBound = glms_vec3_sub(boxPos, halfSize);
    vec3s maxBound = glms_vec3_add(boxPos, halfSize);

    closest.x = fmaxf(minBound.x, fminf(center.x, maxBound.x));
    closest.y = fmaxf(minBound.y, fminf(center.y, maxBound.y));
    closest.z = fmaxf(minBound.z, fminf(center.z, maxBound.z));
    return closest;
}

// --- ALGORITHMES DE COLLISION ---

// 1. SPHERE vs SPHERE
CollisionPoints Collisions_sphereSphere(Collider* a, Transform* ta, Collider* b, Transform* tb) {
    SphereCollider* sA = (SphereCollider*)a->collider;
    SphereCollider* sB = (SphereCollider*)b->collider;
    CollisionPoints cp = {0};

    vec3s posA = glms_vec3_add(ta->position, sA->Center);
    vec3s posB = glms_vec3_add(tb->position, sB->Center);

    vec3s delta = glms_vec3_sub(posB, posA);
    float distance = glms_vec3_norm(delta);
    float radiusSum = sA->Radius + sB->Radius;

    if (distance < radiusSum) {
        cp.HasCollision = true;
        cp.Normal = (distance > 0.0001f) ? glms_vec3_scale(delta, 1.0f / distance) : (vec3s){0, 1, 0};
        cp.Depth = radiusSum - distance;
        cp.A = glms_vec3_add(posA, glms_vec3_scale(cp.Normal, sA->Radius));
        cp.B = glms_vec3_sub(posB, glms_vec3_scale(cp.Normal, sB->Radius));
    }
    return cp;
}

// 2. SPHERE vs PLANE
CollisionPoints Collisions_spherePlane(Collider* a, Transform* ta, Collider* b, Transform* tb) {
    SphereCollider* sphere = (SphereCollider*)a->collider;
    PlaneCollider* plane = (PlaneCollider*)b->collider;
    CollisionPoints cp = {0};

    vec3s center = glms_vec3_add(ta->position, sphere->Center);
    float planeDist = plane->Distance + glms_vec3_dot(tb->position, plane->Normal);
    float distToPlane = glms_vec3_dot(center, plane->Normal) - planeDist;

    if (distToPlane < sphere->Radius) {
        cp.HasCollision = true;
        cp.Normal = glms_vec3_scale(plane->Normal, -1.0f); // Normale pointe de A vers B
        cp.Depth = sphere->Radius - distToPlane;
        cp.B = glms_vec3_sub(center, glms_vec3_scale(plane->Normal, distToPlane));
        cp.A = glms_vec3_add(center, glms_vec3_scale(cp.Normal, sphere->Radius));
    }
    return cp;
}

// 3. SPHERE vs CUBE (AABB)
CollisionPoints Collisions_sphereCube(Collider* a, Transform* ta, Collider* b, Transform* tb) {
    SphereCollider* sphere = (SphereCollider*)a->collider;
    BoxCollider* box = (BoxCollider*)b->collider;
    CollisionPoints cp = {0};

    vec3s center = glms_vec3_add(ta->position, sphere->Center);
    vec3s closest = GetClosestPointAABB(center, tb->position, box->HalfSize);

    vec3s v = glms_vec3_sub(closest, center);
    float distance = glms_vec3_norm(v);

    if (distance < sphere->Radius) {
        cp.HasCollision = true;
        cp.Normal = (distance > 0.0001f) ? glms_vec3_normalize(v) : (vec3s){0, -1, 0};
        cp.Depth = sphere->Radius - distance;
        cp.A = glms_vec3_add(center, glms_vec3_scale(cp.Normal, sphere->Radius));
        cp.B = closest;
    }
    return cp;
}

// 4. CUBE vs PLANE
CollisionPoints Collisions_cubePlane(Collider* a, Transform* ta, Collider* b, Transform* tb) {
    BoxCollider* box = (BoxCollider*)a->collider;
    PlaneCollider* plane = (PlaneCollider*)b->collider;
    CollisionPoints cp = {0};

    float planeDist = plane->Distance + glms_vec3_dot(tb->position, plane->Normal);
    float r = box->HalfSize.x * fabsf(plane->Normal.x) +
              box->HalfSize.y * fabsf(plane->Normal.y) +
              box->HalfSize.z * fabsf(plane->Normal.z);

    float s = glms_vec3_dot(ta->position, plane->Normal) - planeDist;

    if (s <= r) {
        cp.HasCollision = true;
        cp.Normal = glms_vec3_scale(plane->Normal, -1.0f);
        cp.Depth = r - s;
    }
    return cp;
}

// 5. CUBE vs CUBE (AABB)
CollisionPoints Collisions_cubeCube(Collider* a, Transform* ta, Collider* b, Transform* tb) {
    BoxCollider* boxA = (BoxCollider*)a->collider;
    BoxCollider* boxB = (BoxCollider*)b->collider;
    CollisionPoints cp = {0};

    vec3s delta = glms_vec3_sub(tb->position, ta->position);
    float overlapX = (boxA->HalfSize.x + boxB->HalfSize.x) - fabsf(delta.x);
    float overlapY = (boxA->HalfSize.y + boxB->HalfSize.y) - fabsf(delta.y);
    float overlapZ = (boxA->HalfSize.z + boxB->HalfSize.z) - fabsf(delta.z);

    if (overlapX > 0 && overlapY > 0 && overlapZ > 0) {
        cp.HasCollision = true;
        if (overlapX < overlapY && overlapX < overlapZ) {
            cp.Depth = overlapX;
            cp.Normal = (vec3s){(delta.x > 0) ? 1.0f : -1.0f, 0, 0};
        } else if (overlapY < overlapZ) {
            cp.Depth = overlapY;
            cp.Normal = (vec3s){0, (delta.y > 0) ? 1.0f : -1.0f, 0};
        } else {
            cp.Depth = overlapZ;
            cp.Normal = (vec3s){0, 0, (delta.z > 0) ? 1.0f : -1.0f};
        }
    }
    return cp;
}

// --- DISPATCHER ---

CollisionPoints Collisions_testCollisions(Collider* a, Transform* ta, Collider* b, Transform* tb) {
    // 1. Sphere vs ...
    if (a->type == SPHERE) {
        if (b->type == SPHERE) return Collisions_sphereSphere(a, ta, b, tb);
        if (b->type == PLANE)  return Collisions_spherePlane(a, ta, b, tb);
        if (b->type == CUBE)   return Collisions_sphereCube(a, ta, b, tb);
    }
    // 2. Cube vs ...
    if (a->type == CUBE) {
        if (b->type == SPHERE) {
            CollisionPoints cp = Collisions_sphereCube(b, tb, a, ta);
            cp.Normal = glms_vec3_scale(cp.Normal, -1.0f);
            return cp;
        }
        if (b->type == PLANE) return Collisions_cubePlane(a, ta, b, tb);
        if (b->type == CUBE)  return Collisions_cubeCube(a, ta, b, tb);
    }
    // 3. Plane vs ...
    if (a->type == PLANE) {
        if (b->type == SPHERE) {
            CollisionPoints cp = Collisions_spherePlane(b, tb, a, ta);
            cp.Normal = glms_vec3_scale(cp.Normal, -1.0f);
            return cp;
        }
        if (b->type == CUBE) {
            CollisionPoints cp = Collisions_cubePlane(b, tb, a, ta);
            cp.Normal = glms_vec3_scale(cp.Normal, -1.0f);
            return cp;
        }
    }

    CollisionPoints empty = {0};
    return empty;
}