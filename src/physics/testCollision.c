// test_collision.c
// Implémentation de tous les algorithmes de collision narrowphase.
//
// Convention : la normale dans CollisionPoints va TOUJOURS de A vers B.
//   cp.A = point de contact sur la surface de A
//   cp.B = point de contact sur la surface de B

#include "testCollision.h"
#include <math.h>
#include <float.h>
#include <string.h>

// ─────────────────────────────────────────────────────────────────────────────
// UTILITAIRES GÉOMÉTRIQUES
// ─────────────────────────────────────────────────────────────────────────────

static inline vec3s _safe_normalize(vec3s v, vec3s fallback) {
    float n2 = glms_vec3_norm2(v);
    if (n2 < 1e-10f) return fallback;
    return glms_vec3_scale(v, 1.0f / sqrtf(n2));
}

static inline float _clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

// Distance signée d'un point au plan (positif = côté de la normale)
static inline float _plane_signed_dist(PlaneCollider* plane, PhysicsObject* planeObj, vec3s point) {
    vec3s n       = _safe_normalize(plane->Normal, (vec3s){0.0f, 1.0f, 0.0f});
    float planeDot = plane->Distance + glms_vec3_dot(planeObj->Transform.position, n);
    return glms_vec3_dot(point, n) - planeDot;
}

// Point le plus proche sur un OBB (dans l'espace monde)
static inline vec3s _closest_point_obb(vec3s point, PhysicsObject* obj) {
    BoxCollider* box    = (BoxCollider*)obj->Collider->collider;
    mat3s        rot    = glms_quat_mat3(obj->Orientation);
    vec3s        relPos = glms_vec3_sub(point, obj->Transform.position);
    vec3s        result = obj->Transform.position;

    const float hs[3] = { box->HalfSize.x, box->HalfSize.y, box->HalfSize.z };
    for (int i = 0; i < 3; i++) {
        float dist = glms_vec3_dot(relPos, rot.col[i]);
        dist = _clampf(dist, -hs[i], hs[i]);
        result = glms_vec3_add(result, glms_vec3_scale(rot.col[i], dist));
    }
    return result;
}

// Projection d'un OBB sur un axe (pour SAT)
static inline float _box_project_radius(BoxCollider* box, vec3s axis, mat3s rot) {
    return box->HalfSize.x * fabsf(glms_vec3_dot(axis, rot.col[0]))
         + box->HalfSize.y * fabsf(glms_vec3_dot(axis, rot.col[1]))
         + box->HalfSize.z * fabsf(glms_vec3_dot(axis, rot.col[2]));
}

// Point le plus proche sur un segment [p0, p1] par rapport au point q.
static inline float _closest_pt_segment(vec3s p0, vec3s p1, vec3s q, vec3s* closest) {
    vec3s d    = glms_vec3_sub(p1, p0);
    float lenSq = glms_vec3_norm2(d);
    if (lenSq < 1e-12f) { *closest = p0; return 0.0f; }
    float t = glms_vec3_dot(glms_vec3_sub(q, p0), d) / lenSq;
    t = _clampf(t, 0.0f, 1.0f);
    *closest = glms_vec3_add(p0, glms_vec3_scale(d, t));
    return t;
}

// Points les plus proches entre deux segments (Ericson, RTCD p.149)
static inline void _closest_segs(
    vec3s p0, vec3s p1,
    vec3s q0, vec3s q1,
    vec3s* closestA, vec3s* closestB)
{
    vec3s d1 = glms_vec3_sub(p1, p0);
    vec3s d2 = glms_vec3_sub(q1, q0);
    vec3s r  = glms_vec3_sub(p0, q0);

    float a = glms_vec3_norm2(d1);
    float e = glms_vec3_norm2(d2);
    float f = glms_vec3_dot(d2, r);

    float s, t;

    if (a < 1e-10f && e < 1e-10f) {
        s = t = 0.0f;
    } else if (a < 1e-10f) {
        s = 0.0f;
        t = _clampf(f / e, 0.0f, 1.0f);
    } else {
        float c = glms_vec3_dot(d1, r);
        if (e < 1e-10f) {
            t = 0.0f;
            s = _clampf(-c / a, 0.0f, 1.0f);
        } else {
            float b     = glms_vec3_dot(d1, d2);
            float denom = a * e - b * b;
            if (fabsf(denom) > 1e-10f)
                s = _clampf((b*f - c*e) / denom, 0.0f, 1.0f);
            else
                s = 0.0f;

            t = (b * s + f) / e;
            if (t < 0.0f) {
                t = 0.0f;
                s = _clampf(-c / a, 0.0f, 1.0f);
            } else if (t > 1.0f) {
                t = 1.0f;
                s = _clampf((b - c) / a, 0.0f, 1.0f);
            }
        }
    }

    *closestA = glms_vec3_add(p0, glms_vec3_scale(d1, s));
    *closestB = glms_vec3_add(q0, glms_vec3_scale(d2, t));
}

// Endpoints monde d'une capsule
static inline void _capsule_world_endpoints(PhysicsObject* obj, vec3s* base, vec3s* tip) {
    CapsuleCollider* cap = (CapsuleCollider*)obj->Collider->collider;
    mat3s rot = glms_quat_mat3(obj->Orientation);
    *base = glms_vec3_add(obj->Transform.position, glms_mat3_mulv(rot, cap->LocalBase));
    *tip  = glms_vec3_add(obj->Transform.position, glms_mat3_mulv(rot, cap->LocalTip));
}

// ─────────────────────────────────────────────────────────────────────────────
// SPHÈRE – SPHÈRE
// ─────────────────────────────────────────────────────────────────────────────

CollisionPoints Collisions_sphereSphere(PhysicsObject* a, PhysicsObject* b) {
    SphereCollider* sA = (SphereCollider*)a->Collider->collider;
    SphereCollider* sB = (SphereCollider*)b->Collider->collider;
    CollisionPoints cp = {0};

    vec3s posA      = glms_vec3_add(a->Transform.position, sA->Center);
    vec3s posB      = glms_vec3_add(b->Transform.position, sB->Center);
    vec3s delta     = glms_vec3_sub(posB, posA);
    float dist      = glms_vec3_norm(delta);
    float radiusSum = sA->Radius + sB->Radius;

    if (dist < radiusSum) {
        cp.HasCollision = true;
        cp.Normal = (dist > 1e-6f)
            ? glms_vec3_scale(delta, 1.0f / dist)
            : (vec3s){0.0f, 1.0f, 0.0f};
        cp.Depth = radiusSum - dist;
        cp.A     = glms_vec3_add(posA, glms_vec3_scale(cp.Normal,  sA->Radius));
        cp.B     = glms_vec3_sub(posB, glms_vec3_scale(cp.Normal,  sB->Radius));
    }
    return cp;
}

// ─────────────────────────────────────────────────────────────────────────────
// SPHÈRE – PLAN
// ─────────────────────────────────────────────────────────────────────────────

CollisionPoints Collisions_spherePlane(PhysicsObject* a, PhysicsObject* b) {
    SphereCollider* sphere = (SphereCollider*)a->Collider->collider;
    PlaneCollider*  plane  = (PlaneCollider*) b->Collider->collider;
    CollisionPoints cp = {0};

    vec3s center     = glms_vec3_add(a->Transform.position, sphere->Center);
    vec3s n          = _safe_normalize(plane->Normal, (vec3s){0.0f, 1.0f, 0.0f});
    float signedDist = _plane_signed_dist(plane, b, center);

    if (fabsf(signedDist) < sphere->Radius) {
        cp.HasCollision = true;
        float side  = (signedDist >= 0.0f) ? 1.0f : -1.0f;
        cp.Normal   = glms_vec3_scale(n, -side);
        cp.Depth    = sphere->Radius - fabsf(signedDist);
        cp.A        = glms_vec3_add(center, glms_vec3_scale(cp.Normal,  sphere->Radius));
        cp.B        = glms_vec3_sub(center, glms_vec3_scale(n, signedDist));
    }
    return cp;
}

// ─────────────────────────────────────────────────────────────────────────────
// SPHÈRE – CUBE (OBB)
// ─────────────────────────────────────────────────────────────────────────────

CollisionPoints Collisions_sphereCube(PhysicsObject* a, PhysicsObject* b) {
    SphereCollider* sphere  = (SphereCollider*)a->Collider->collider;
    CollisionPoints cp = {0};

    vec3s center  = glms_vec3_add(a->Transform.position, sphere->Center);
    vec3s closest = _closest_point_obb(center, b);
    vec3s diff    = glms_vec3_sub(closest, center);
    float dist    = glms_vec3_norm(diff);

    if (dist < sphere->Radius) {
        cp.HasCollision = true;
        cp.Normal = (dist > 1e-6f)
            ? glms_vec3_scale(diff, 1.0f / dist)
            : (vec3s){0.0f, -1.0f, 0.0f};
        cp.Depth = sphere->Radius - dist;
        cp.A     = glms_vec3_add(center,  glms_vec3_scale(cp.Normal, sphere->Radius));
        cp.B     = closest;
    }
    return cp;
}

// ─────────────────────────────────────────────────────────────────────────────
// SPHÈRE – CAPSULE
// ─────────────────────────────────────────────────────────────────────────────

CollisionPoints Collisions_sphereCapsule(PhysicsObject* a, PhysicsObject* b) {
    SphereCollider*  sphere = (SphereCollider*) a->Collider->collider;
    CapsuleCollider* cap    = (CapsuleCollider*)b->Collider->collider;
    CollisionPoints  cp = {0};

    vec3s sphereCenter = glms_vec3_add(a->Transform.position, sphere->Center);
    vec3s capBase, capTip;
    _capsule_world_endpoints(b, &capBase, &capTip);

    vec3s closestOnAxis;
    _closest_pt_segment(capBase, capTip, sphereCenter, &closestOnAxis);

    vec3s diff = glms_vec3_sub(sphereCenter, closestOnAxis);
    float dist = glms_vec3_norm(diff);
    float combinedRadius = sphere->Radius + cap->Radius;

    if (dist < combinedRadius) {
        cp.HasCollision = true;
        vec3s normalAtoB = (dist > 1e-6f)
            ? glms_vec3_scale(diff, -1.0f / dist)
            : (vec3s){0.0f, -1.0f, 0.0f};
        cp.Normal = normalAtoB;
        cp.Depth  = combinedRadius - dist;
        cp.A      = glms_vec3_add(sphereCenter,  glms_vec3_scale(cp.Normal,  sphere->Radius));
        cp.B      = glms_vec3_add(closestOnAxis, glms_vec3_scale(cp.Normal, -cap->Radius));
    }
    return cp;
}

// ─────────────────────────────────────────────────────────────────────────────
// CUBE – PLAN
// ─────────────────────────────────────────────────────────────────────────────

CollisionPoints Collisions_cubePlane(PhysicsObject* a, PhysicsObject* b) {
    BoxCollider*    box   = (BoxCollider*)  a->Collider->collider;
    PlaneCollider*  plane = (PlaneCollider*)b->Collider->collider;
    CollisionPoints cp = {0};

    mat3s  rotA       = glms_quat_mat3(a->Orientation);
    vec3s  n          = _safe_normalize(plane->Normal, (vec3s){0.0f, 1.0f, 0.0f});
    float  r          = _box_project_radius(box, n, rotA);
    float  signedDist = _plane_signed_dist(plane, b, a->Transform.position);

    if (fabsf(signedDist) <= r) {
        cp.HasCollision = true;
        float  side   = (signedDist >= 0.0f) ? 1.0f : -1.0f;
        cp.Normal     = glms_vec3_scale(n, -side);
        cp.Depth      = r - fabsf(signedDist);
        vec3s planePoint = glms_vec3_sub(a->Transform.position, glms_vec3_scale(n, signedDist));
        cp.A = glms_vec3_add(a->Transform.position, glms_vec3_scale(cp.Normal,  r));
        cp.B = planePoint;
    }
    return cp;
}

// ─────────────────────────────────────────────────────────────────────────────
// CUBE – CUBE (SAT 15 axes, early exit)
//
// CORRECTION (v2) :
//   Les points de contact A et B sont maintenant calculés comme les points de
//   support sur la surface de chaque OBB dans la direction de la normale.
//   L'ancienne version utilisait cp.B = b->position (centre) ce qui donnait
//   un bras de levier rB = 0 et annulait toute rotation angulaire pour B.
// ─────────────────────────────────────────────────────────────────────────────

CollisionPoints Collisions_cubeCube(PhysicsObject* a, PhysicsObject* b) {
    BoxCollider* boxA = (BoxCollider*)a->Collider->collider;
    BoxCollider* boxB = (BoxCollider*)b->Collider->collider;
    CollisionPoints cp = {0};

    mat3s rotA  = glms_quat_mat3(a->Orientation);
    mat3s rotB  = glms_quat_mat3(b->Orientation);
    vec3s delta = glms_vec3_sub(b->Transform.position, a->Transform.position);

    float minOverlap  = FLT_MAX;
    vec3s smallestAxis = {0.0f, 0.0f, 0.0f};
    bool  edgeEdge    = false;

    vec3s aAxes[3] = { rotA.col[0], rotA.col[1], rotA.col[2] };
    vec3s bAxes[3] = { rotB.col[0], rotB.col[1], rotB.col[2] };

    // 6 axes des faces
    for (int i = 0; i < 3; i++) {
        // Axe de A
        {
            vec3s axis  = aAxes[i];
            float projA = _box_project_radius(boxA, axis, rotA);
            float projB = _box_project_radius(boxB, axis, rotB);
            float dist  = fabsf(glms_vec3_dot(delta, axis));
            float ov    = (projA + projB) - dist;
            if (ov <= 0.0f) return cp;
            if (ov < minOverlap) { minOverlap = ov; smallestAxis = axis; edgeEdge = false; }
        }
        // Axe de B
        {
            vec3s axis  = bAxes[i];
            float projA = _box_project_radius(boxA, axis, rotA);
            float projB = _box_project_radius(boxB, axis, rotB);
            float dist  = fabsf(glms_vec3_dot(delta, axis));
            float ov    = (projA + projB) - dist;
            if (ov <= 0.0f) return cp;
            if (ov < minOverlap) { minOverlap = ov; smallestAxis = axis; edgeEdge = false; }
        }
    }

    // 9 axes arête-arête (produits croisés)
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            vec3s cross = glms_vec3_cross(aAxes[i], bAxes[j]);
            float len2  = glms_vec3_norm2(cross);
            if (len2 < 1e-6f) continue;

            vec3s axis  = glms_vec3_scale(cross, 1.0f / sqrtf(len2));
            float projA = _box_project_radius(boxA, axis, rotA);
            float projB = _box_project_radius(boxB, axis, rotB);
            float dist  = fabsf(glms_vec3_dot(delta, axis));
            float ov    = (projA + projB) - dist;
            if (ov <= 0.0f) return cp;
            if (ov < minOverlap) { minOverlap = ov; smallestAxis = axis; edgeEdge = true; }
        }
    }

    // Corriger l'orientation de la normale (de A vers B)
    cp.HasCollision = true;
    cp.Normal = (glms_vec3_dot(delta, smallestAxis) < 0.0f)
        ? glms_vec3_scale(smallestAxis, -1.0f)
        : smallestAxis;
    cp.Depth = minOverlap;

    // ── CORRECTION : points de contact sur les surfaces réelles ──────────────
    // cp.A = point le plus avancé de A dans la direction de la normale (sur surface de A)
    // cp.B = point le plus avancé de B dans la direction opposée      (sur surface de B)
    //
    // _box_project_radius donne la distance max de projection du box sur l'axe,
    // ce qui correspond au point de support (extrémité de la box dans cette direction).
    if (!edgeEdge) {
        float rA = _box_project_radius(boxA, cp.Normal, rotA);
        float rB = _box_project_radius(boxB, cp.Normal, rotB);
        cp.A = glms_vec3_add(a->Transform.position, glms_vec3_scale(cp.Normal,  rA));  // surface de A
        cp.B = glms_vec3_sub(b->Transform.position, glms_vec3_scale(cp.Normal,  rB));  // surface de B (CORRECTION: était b->position + 0)
    } else {
        // Contact arête-arête : point médian de la zone de pénétration
        cp.A = glms_vec3_add(a->Transform.position, glms_vec3_scale(cp.Normal, minOverlap * 0.5f));
        cp.B = glms_vec3_sub(b->Transform.position, glms_vec3_scale(cp.Normal, minOverlap * 0.5f));
    }

    return cp;
}

// ─────────────────────────────────────────────────────────────────────────────
// CUBE – CAPSULE
// ─────────────────────────────────────────────────────────────────────────────

CollisionPoints Collisions_cubeCapsule(PhysicsObject* a, PhysicsObject* b) {
    CapsuleCollider* cap = (CapsuleCollider*)b->Collider->collider;
    CollisionPoints  cp  = {0};

    vec3s capBase, capTip;
    _capsule_world_endpoints(b, &capBase, &capTip);

    vec3s segPt  = glms_vec3_lerp(capBase, capTip, 0.5f);
    vec3s obbPt  = _closest_point_obb(segPt, a);

    for (int iter = 0; iter < 8; iter++) {
        obbPt  = _closest_point_obb(segPt, a);
        _closest_pt_segment(capBase, capTip, obbPt, &segPt);
    }
    obbPt = _closest_point_obb(segPt, a);

    vec3s diff = glms_vec3_sub(obbPt, segPt);
    float dist = glms_vec3_norm(diff);

    if (dist < cap->Radius) {
        cp.HasCollision = true;
        vec3s normalAtoB = (dist > 1e-6f)
            ? glms_vec3_scale(diff, -1.0f / dist)
            : (vec3s){0.0f, 1.0f, 0.0f};
        cp.Normal = normalAtoB;
        cp.Depth  = cap->Radius - dist;
        cp.A      = obbPt;
        cp.B      = glms_vec3_sub(segPt, glms_vec3_scale(normalAtoB, cap->Radius));
    }
    return cp;
}

// ─────────────────────────────────────────────────────────────────────────────
// CAPSULE – PLAN
// ─────────────────────────────────────────────────────────────────────────────

CollisionPoints Collisions_capsulePlane(PhysicsObject* a, PhysicsObject* b) {
    CapsuleCollider* cap   = (CapsuleCollider*)a->Collider->collider;
    PlaneCollider*   plane = (PlaneCollider*)  b->Collider->collider;
    CollisionPoints  cp = {0};

    vec3s base, tip;
    _capsule_world_endpoints(a, &base, &tip);
    vec3s n = _safe_normalize(plane->Normal, (vec3s){0.0f, 1.0f, 0.0f});

    float distBase = _plane_signed_dist(plane, b, base);
    float distTip  = _plane_signed_dist(plane, b, tip);

    float minDist = (distBase < distTip) ? distBase : distTip;
    vec3s deepPt  = (distBase < distTip) ? base : tip;

    if (minDist < cap->Radius) {
        cp.HasCollision = true;
        float side    = (minDist >= 0.0f) ? 1.0f : -1.0f;
        cp.Normal     = glms_vec3_scale(n, -side);
        cp.Depth      = cap->Radius - fabsf(minDist);
        cp.A          = glms_vec3_add(deepPt, glms_vec3_scale(cp.Normal, cap->Radius));
        cp.B          = glms_vec3_sub(deepPt, glms_vec3_scale(n, minDist));
    }
    return cp;
}

// ─────────────────────────────────────────────────────────────────────────────
// CAPSULE – CAPSULE
// ─────────────────────────────────────────────────────────────────────────────

CollisionPoints Collisions_capsuleCapsule(PhysicsObject* a, PhysicsObject* b) {
    CapsuleCollider* capA = (CapsuleCollider*)a->Collider->collider;
    CapsuleCollider* capB = (CapsuleCollider*)b->Collider->collider;
    CollisionPoints  cp = {0};

    vec3s baseA, tipA, baseB, tipB;
    _capsule_world_endpoints(a, &baseA, &tipA);
    _capsule_world_endpoints(b, &baseB, &tipB);

    vec3s closestA, closestB;
    _closest_segs(baseA, tipA, baseB, tipB, &closestA, &closestB);

    vec3s diff = glms_vec3_sub(closestB, closestA);
    float dist = glms_vec3_norm(diff);
    float combinedRadius = capA->Radius + capB->Radius;

    if (dist < combinedRadius) {
        cp.HasCollision = true;
        cp.Normal = (dist > 1e-6f)
            ? glms_vec3_scale(diff, 1.0f / dist)
            : (vec3s){0.0f, 1.0f, 0.0f};
        cp.Depth = combinedRadius - dist;
        cp.A     = glms_vec3_add(closestA, glms_vec3_scale(cp.Normal,  capA->Radius));
        cp.B     = glms_vec3_sub(closestB, glms_vec3_scale(cp.Normal,  capB->Radius));
    }
    return cp;
}

// ─────────────────────────────────────────────────────────────────────────────
// RAYCASTING NARROWPHASE
// ─────────────────────────────────────────────────────────────────────────────

float Collisions_raycastSphere(vec3s origin, vec3s dir, float maxDist, PhysicsObject* obj) {
    SphereCollider* s   = (SphereCollider*)obj->Collider->collider;
    vec3s center        = glms_vec3_add(obj->Transform.position, s->Center);
    vec3s oc            = glms_vec3_sub(origin, center);
    float b             = glms_vec3_dot(oc, dir);
    float c             = glms_vec3_norm2(oc) - s->Radius * s->Radius;
    float discriminant  = b*b - c;
    if (discriminant < 0.0f) return -1.0f;
    float t = -b - sqrtf(discriminant);
    if (t < 0.0f) t = -b + sqrtf(discriminant);
    return (t >= 0.0f && t <= maxDist) ? t : -1.0f;
}

float Collisions_raycastBox(vec3s origin, vec3s dir, float maxDist, PhysicsObject* obj) {
    BoxCollider* box = (BoxCollider*)obj->Collider->collider;
    mat3s rot = glms_quat_mat3(obj->Orientation);
    vec3s localOrigin = glms_vec3_sub(origin, obj->Transform.position);
    vec3s lo = (vec3s){
        glms_vec3_dot(localOrigin, rot.col[0]),
        glms_vec3_dot(localOrigin, rot.col[1]),
        glms_vec3_dot(localOrigin, rot.col[2])
    };
    vec3s ld = (vec3s){
        glms_vec3_dot(dir, rot.col[0]),
        glms_vec3_dot(dir, rot.col[1]),
        glms_vec3_dot(dir, rot.col[2])
    };
    const float hs[3] = { box->HalfSize.x, box->HalfSize.y, box->HalfSize.z };
    float tMin = 0.0f, tMax = maxDist;
    for (int i = 0; i < 3; i++) {
        float ldI = ((float*)&ld)[i];
        float loI = ((float*)&lo)[i];
        if (fabsf(ldI) < 1e-8f) {
            if (loI < -hs[i] || loI > hs[i]) return -1.0f;
        } else {
            float t1 = (-hs[i] - loI) / ldI;
            float t2 = ( hs[i] - loI) / ldI;
            if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
            tMin = (t1 > tMin) ? t1 : tMin;
            tMax = (t2 < tMax) ? t2 : tMax;
            if (tMax < tMin) return -1.0f;
        }
    }
    return (tMin >= 0.0f) ? tMin : tMax;
}

float Collisions_raycastCapsule(vec3s origin, vec3s dir, float maxDist, PhysicsObject* obj) {
    CapsuleCollider* cap = (CapsuleCollider*)obj->Collider->collider;
    vec3s base, tip;
    _capsule_world_endpoints(obj, &base, &tip);
    vec3s AB  = glms_vec3_sub(tip, base);
    vec3s AO  = glms_vec3_sub(origin, base);
    vec3s AB_cross_dir = glms_vec3_cross(AB, dir);
    vec3s AB_cross_AO  = glms_vec3_cross(AB, AO);
    float lenAB_sq = glms_vec3_norm2(AB);
    float a = glms_vec3_norm2(AB_cross_dir);
    float b = 2.0f * glms_vec3_dot(AB_cross_dir, AB_cross_AO);
    float c = glms_vec3_norm2(AB_cross_AO) - cap->Radius * cap->Radius * lenAB_sq;
    float disc = b*b - 4.0f*a*c;
    float tBest = maxDist + 1.0f;
    if (disc >= 0.0f && a > 1e-10f) {
        float sqrtDisc = sqrtf(disc);
        float t1 = (-b - sqrtDisc) / (2.0f * a);
        float t2 = (-b + sqrtDisc) / (2.0f * a);
        float t  = (t1 >= 0.0f) ? t1 : t2;
        if (t >= 0.0f && t <= maxDist) {
            vec3s p   = glms_vec3_add(origin, glms_vec3_scale(dir, t));
            vec3s AP  = glms_vec3_sub(p, base);
            float dot = glms_vec3_dot(AP, AB);
            if (dot >= 0.0f && dot <= lenAB_sq) tBest = t;
        }
    }
    // Caps sphériques
    float tSph;
    {
        PhysicsObject dummy = *obj;
        SphereCollider sc = { base, cap->Radius };
        Collider col = { COLLIDER_SPHERE, &sc, false, 0, 0 };
        dummy.Collider = &col;
        tSph = Collisions_raycastSphere(origin, dir, maxDist, &dummy);
        if (tSph >= 0.0f && tSph < tBest) tBest = tSph;
    }
    {
        PhysicsObject dummy = *obj;
        SphereCollider sc = { tip, cap->Radius };
        Collider col = { COLLIDER_SPHERE, &sc, false, 0, 0 };
        dummy.Collider = &col;
        tSph = Collisions_raycastSphere(origin, dir, maxDist, &dummy);
        if (tSph >= 0.0f && tSph < tBest) tBest = tSph;
    }
    return (tBest <= maxDist) ? tBest : -1.0f;
}

float Collisions_raycastPlane(vec3s origin, vec3s dir, float maxDist, PhysicsObject* obj) {
    PlaneCollider* plane = (PlaneCollider*)obj->Collider->collider;
    vec3s n    = _safe_normalize(plane->Normal, (vec3s){0.0f, 1.0f, 0.0f});
    float denom = glms_vec3_dot(dir, n);
    if (fabsf(denom) < 1e-8f) return -1.0f;
    float planeDot = plane->Distance + glms_vec3_dot(obj->Transform.position, n);
    float t = (planeDot - glms_vec3_dot(origin, n)) / denom;
    return (t >= 0.0f && t <= maxDist) ? t : -1.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// DISPATCHER PRINCIPAL
// ─────────────────────────────────────────────────────────────────────────────

static inline CollisionPoints _flip_cp(CollisionPoints cp) {
    if (!cp.HasCollision) return cp;
    cp.Normal = glms_vec3_scale(cp.Normal, -1.0f);
    vec3s tmp = cp.A; cp.A = cp.B; cp.B = tmp;
    return cp;
}

CollisionPoints Collisions_test(PhysicsObject* a, PhysicsObject* b) {
    if (!a || !b || !a->Collider || !b->Collider) return (CollisionPoints){0};
    if (!Collider_canCollide(a->Collider, b->Collider))    return (CollisionPoints){0};

    if (a->PhysicsType == PHYS_STATIC && b->PhysicsType == PHYS_STATIC)
        return (CollisionPoints){0};

    ColliderType tA = a->Collider->type;
    ColliderType tB = b->Collider->type;

    if (tA > tB) return _flip_cp(Collisions_test(b, a));

    if (tA == COLLIDER_SPHERE) {
        if (tB == COLLIDER_SPHERE)  return Collisions_sphereSphere(a, b);
        if (tB == COLLIDER_CUBE)    return Collisions_sphereCube  (a, b);
        if (tB == COLLIDER_CAPSULE) return Collisions_sphereCapsule(a, b);
    }
    if (tA == COLLIDER_PLANE) {
        if (tB == COLLIDER_SPHERE)  return _flip_cp(Collisions_spherePlane(b, a));
        if (tB == COLLIDER_CUBE)    return _flip_cp(Collisions_cubePlane  (b, a));
        if (tB == COLLIDER_CAPSULE) return _flip_cp(Collisions_capsulePlane(b, a));
        if (tB == COLLIDER_PLANE)   return (CollisionPoints){0};
    }
    if (tA == COLLIDER_CUBE) {
        if (tB == COLLIDER_CUBE)    return Collisions_cubeCube   (a, b);
        if (tB == COLLIDER_CAPSULE) return Collisions_cubeCapsule(a, b);
    }
    if (tA == COLLIDER_CAPSULE) {
        if (tB == COLLIDER_CAPSULE) return Collisions_capsuleCapsule(a, b);
    }

    return (CollisionPoints){0};
}