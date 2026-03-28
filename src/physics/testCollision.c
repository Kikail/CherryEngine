#include "testCollision.h"

#include <math.h>
#include <float.h>

// -----------------------------------------------------------------------------
// Utilitaires
// -----------------------------------------------------------------------------

static vec3s NormalizeSafe(vec3s v, vec3s fallback) {
    float n2 = glms_vec3_norm2(v);
    if (n2 < 0.000001f) return fallback;
    return glms_vec3_scale(v, 1.0f / sqrtf(n2));
}

static float PlaneSignedDistance(PlaneCollider* plane, PhysicsObject* planeObj, vec3s point) {
    vec3s n = NormalizeSafe(plane->Normal, (vec3s){0.0f, 1.0f, 0.0f});
    float planeDist = plane->Distance + glms_vec3_dot(planeObj->Transform.position, n);
    return glms_vec3_dot(point, n) - planeDist;
}

// Projette une boîte sur un axe donné (SAT)
float GetBoxProjectionRadius(BoxCollider* box, vec3s axis, mat3s rotation) {
    axis = NormalizeSafe(axis, (vec3s){1.0f, 0.0f, 0.0f});

    return box->HalfSize.x * fabsf(glms_vec3_dot(axis, rotation.col[0])) +
           box->HalfSize.y * fabsf(glms_vec3_dot(axis, rotation.col[1])) +
           box->HalfSize.z * fabsf(glms_vec3_dot(axis, rotation.col[2]));
}

// Trouve le point le plus proche sur un cube orienté
vec3s GetClosestPointOBB(vec3s point, PhysicsObject* obj) {
    BoxCollider* box = (BoxCollider*)obj->Collider->collider;
    vec3s relPos = glms_vec3_sub(point, obj->Transform.position);
    mat3s rot = glms_quat_mat3(obj->Orientation);

    vec3s closest = obj->Transform.position;

    for (int i = 0; i < 3; i++) {
        vec3s axis = rot.col[i];
        float dist = glms_vec3_dot(relPos, axis);
        float maxDist = (i == 0) ? box->HalfSize.x : (i == 1) ? box->HalfSize.y : box->HalfSize.z;

        if (dist > maxDist) dist = maxDist;
        if (dist < -maxDist) dist = -maxDist;

        closest = glms_vec3_add(closest, glms_vec3_scale(axis, dist));
    }

    return closest;
}

// -----------------------------------------------------------------------------
// Collisions
// -----------------------------------------------------------------------------

CollisionPoints Collisions_sphereSphere(PhysicsObject* a, PhysicsObject* b) {
    SphereCollider* sA = (SphereCollider*)a->Collider->collider;
    SphereCollider* sB = (SphereCollider*)b->Collider->collider;
    CollisionPoints cp = {0};

    vec3s posA = glms_vec3_add(a->Transform.position, sA->Center);
    vec3s posB = glms_vec3_add(b->Transform.position, sB->Center);
    vec3s delta = glms_vec3_sub(posB, posA);
    float dist = glms_vec3_norm(delta);
    float radiusSum = sA->Radius + sB->Radius;

    if (dist < radiusSum) {
        cp.HasCollision = true;
        cp.Normal = (dist > 0.0001f) ? glms_vec3_scale(delta, 1.0f / dist) : (vec3s){0.0f, 1.0f, 0.0f};
        cp.Depth = radiusSum - dist;
        cp.A = glms_vec3_add(posA, glms_vec3_scale(cp.Normal, sA->Radius));
        cp.B = glms_vec3_sub(posB, glms_vec3_scale(cp.Normal, sB->Radius));
    }

    return cp;
}

CollisionPoints Collisions_sphereCube(PhysicsObject* a, PhysicsObject* b) {
    SphereCollider* sphere = (SphereCollider*)a->Collider->collider;
    CollisionPoints cp = {0};

    vec3s center = glms_vec3_add(a->Transform.position, sphere->Center);
    vec3s closest = GetClosestPointOBB(center, b);

    vec3s v = glms_vec3_sub(closest, center);
    float distance = glms_vec3_norm(v);

    if (distance < sphere->Radius) {
        cp.HasCollision = true;
        cp.Normal = (distance > 0.0001f) ? glms_vec3_normalize(v) : (vec3s){0.0f, -1.0f, 0.0f};
        cp.Depth = sphere->Radius - distance;
        cp.A = glms_vec3_add(center, glms_vec3_scale(cp.Normal, sphere->Radius));
        cp.B = closest;
    }

    return cp;
}

CollisionPoints Collisions_cubeCube(PhysicsObject* a, PhysicsObject* b) {
    BoxCollider* boxA = (BoxCollider*)a->Collider->collider;
    BoxCollider* boxB = (BoxCollider*)b->Collider->collider;
    CollisionPoints cp = {0};

    mat3s rotA = glms_quat_mat3(a->Orientation);
    mat3s rotB = glms_quat_mat3(b->Orientation);
    vec3s delta = glms_vec3_sub(b->Transform.position, a->Transform.position);

    float minOverlap = FLT_MAX;
    vec3s smallestAxis = {0.0f, 0.0f, 0.0f};

    vec3s aAxes[3] = { rotA.col[0], rotA.col[1], rotA.col[2] };
    vec3s bAxes[3] = { rotB.col[0], rotB.col[1], rotB.col[2] };

    // 15 axes SAT : 3 de A, 3 de B, 9 produits croisés
    vec3s axes[15] = {
        aAxes[0], aAxes[1], aAxes[2],
        bAxes[0], bAxes[1], bAxes[2],
        glms_vec3_cross(aAxes[0], bAxes[0]),
        glms_vec3_cross(aAxes[0], bAxes[1]),
        glms_vec3_cross(aAxes[0], bAxes[2]),
        glms_vec3_cross(aAxes[1], bAxes[0]),
        glms_vec3_cross(aAxes[1], bAxes[1]),
        glms_vec3_cross(aAxes[1], bAxes[2]),
        glms_vec3_cross(aAxes[2], bAxes[0]),
        glms_vec3_cross(aAxes[2], bAxes[1]),
        glms_vec3_cross(aAxes[2], bAxes[2])
    };

    for (int i = 0; i < 15; i++) {
        vec3s axis = axes[i];

        if (glms_vec3_norm2(axis) < 0.000001f) {
            continue;
        }

        axis = glms_vec3_normalize(axis);

        float projA = GetBoxProjectionRadius(boxA, axis, rotA);
        float projB = GetBoxProjectionRadius(boxB, axis, rotB);
        float dist = fabsf(glms_vec3_dot(delta, axis));

        float overlap = (projA + projB) - dist;
        if (overlap <= 0.0f) {
            return cp;
        }

        if (overlap < minOverlap) {
            minOverlap = overlap;
            smallestAxis = axis;
        }
    }

    if (minOverlap == FLT_MAX) {
        return cp;
    }

    cp.HasCollision = true;
    cp.Normal = (glms_vec3_dot(delta, smallestAxis) < 0.0f)
        ? glms_vec3_scale(smallestAxis, -1.0f)
        : smallestAxis;

    cp.Depth = minOverlap;

    // Approximation propre des points de contact
    cp.A = glms_vec3_add(a->Transform.position, glms_vec3_scale(cp.Normal, minOverlap * 0.5f));
    cp.B = glms_vec3_sub(b->Transform.position, glms_vec3_scale(cp.Normal, minOverlap * 0.5f));

    return cp;
}

CollisionPoints Collisions_cubePlane(PhysicsObject* a, PhysicsObject* b) {
    BoxCollider* box = (BoxCollider*)a->Collider->collider;
    PlaneCollider* plane = (PlaneCollider*)b->Collider->collider;
    CollisionPoints cp = {0};

    mat3s rotA = glms_quat_mat3(a->Orientation);
    vec3s n = NormalizeSafe(plane->Normal, (vec3s){0.0f, 1.0f, 0.0f});

    float r = GetBoxProjectionRadius(box, n, rotA);
    float signedDist = PlaneSignedDistance(plane, b, a->Transform.position);

    if (fabsf(signedDist) <= r) {
        cp.HasCollision = true;

        float side = (signedDist >= 0.0f) ? 1.0f : -1.0f;
        cp.Normal = glms_vec3_scale(n, -side); // de A vers B
        cp.Depth = r - fabsf(signedDist);

        vec3s planePoint = glms_vec3_sub(a->Transform.position, glms_vec3_scale(n, signedDist));
        cp.A = glms_vec3_add(a->Transform.position, glms_vec3_scale(cp.Normal, r));
        cp.B = planePoint;
    }

    return cp;
}

CollisionPoints Collisions_spherePlane(PhysicsObject* a, PhysicsObject* b) {
    SphereCollider* sphere = (SphereCollider*)a->Collider->collider;
    PlaneCollider* plane = (PlaneCollider*)b->Collider->collider;
    CollisionPoints cp = {0};

    vec3s center = glms_vec3_add(a->Transform.position, sphere->Center);
    vec3s n = NormalizeSafe(plane->Normal, (vec3s){0.0f, 1.0f, 0.0f});
    float signedDist = PlaneSignedDistance(plane, b, center);

    if (fabsf(signedDist) < sphere->Radius) {
        cp.HasCollision = true;

        float side = (signedDist >= 0.0f) ? 1.0f : -1.0f;
        cp.Normal = glms_vec3_scale(n, -side); // de A vers B
        cp.Depth = sphere->Radius - fabsf(signedDist);

        vec3s planePoint = glms_vec3_sub(center, glms_vec3_scale(n, signedDist));
        cp.A = glms_vec3_add(center, glms_vec3_scale(cp.Normal, sphere->Radius));
        cp.B = planePoint;
    }

    return cp;
}

// -----------------------------------------------------------------------------
// DISPATCHER
// -----------------------------------------------------------------------------

CollisionPoints Collisions_testCollisions(PhysicsObject* a, PhysicsObject* b) {
    if (!a || !b || !a->Collider || !b->Collider) {
        return (CollisionPoints){0};
    }

    if (a->Collider->type == SPHERE) {
        if (b->Collider->type == SPHERE) return Collisions_sphereSphere(a, b);
        if (b->Collider->type == PLANE)  return Collisions_spherePlane(a, b);
        if (b->Collider->type == CUBE)   return Collisions_sphereCube(a, b);
    }

    if (a->Collider->type == CUBE) {
        if (b->Collider->type == SPHERE) {
            CollisionPoints cp = Collisions_sphereCube(b, a);
            cp.Normal = glms_vec3_scale(cp.Normal, -1.0f);

            vec3s tmp = cp.A;
            cp.A = cp.B;
            cp.B = tmp;

            return cp;
        }

        if (b->Collider->type == PLANE) return Collisions_cubePlane(a, b);
        if (b->Collider->type == CUBE)  return Collisions_cubeCube(a, b);
    }

    if (a->Collider->type == PLANE) {
        if (b->Collider->type == SPHERE) {
            CollisionPoints cp = Collisions_spherePlane(b, a);
            cp.Normal = glms_vec3_scale(cp.Normal, -1.0f);

            vec3s tmp = cp.A;
            cp.A = cp.B;
            cp.B = tmp;

            return cp;
        }

        if (b->Collider->type == CUBE) {
            CollisionPoints cp = Collisions_cubePlane(b, a);
            cp.Normal = glms_vec3_scale(cp.Normal, -1.0f);

            vec3s tmp = cp.A;
            cp.A = cp.B;
            cp.B = tmp;

            return cp;
        }
    }

    return (CollisionPoints){0};
}