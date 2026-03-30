// physics_world.c
// Cœur du moteur physique CherryEngine.
//
// Architecture inspirée de Box2D :
//   1. Broadphase  — SAP (Sweep And Prune) sur l'axe X → paires candidates
//   2. Narrowphase — Test de collision précis → points de contact
//   3. Warm start  — Ré-applique les impulsions de la frame précédente
//   4. Solveur vitesse — Sequential Impulse × VELOCITY_SOLVER_ITERATIONS
//   5. Intégration — Symplectic Euler (vitesse → position)
//   6. Correction de position directe (évite la traversée pour les grandes pénétrations)
//   7. Sleep       — Met en veille les corps immobiles

#include "physicsWorld.h"
#include "testCollision.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <time.h>

// ─────────────────────────────────────────────────────────────────────────────
// UTILITAIRES INTERNES
// ─────────────────────────────────────────────────────────────────────────────

static inline float _clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static inline vec3s _safe_normalize3(vec3s v) {
    float n2 = glms_vec3_norm2(v);
    if (n2 < 1e-10f) return (vec3s){0.0f, 1.0f, 0.0f};
    return glms_vec3_scale(v, 1.0f / sqrtf(n2));
}

// ─────────────────────────────────────────────────────────────────────────────
// BROADPHASE — AABB + SWEEP AND PRUNE (axe X)
// ─────────────────────────────────────────────────────────────────────────────

typedef struct {
    float minX, maxX;
    vec3s minXYZ;
    vec3s maxXYZ;
    int   objIdx;
} SAP_Entry;

static void _compute_aabb(PhysicsObject* obj, vec3s* outMin, vec3s* outMax) {
    const float PAD = 0.05f;

    if (!obj->Collider) {
        *outMin = *outMax = obj->Transform.position;
        return;
    }

    ColliderType type = obj->Collider->type;

    if (type == COLLIDER_SPHERE) {
        SphereCollider* s = (SphereCollider*)obj->Collider->collider;
        vec3s center = glms_vec3_add(obj->Transform.position, s->Center);
        float r = s->Radius + PAD;
        *outMin = (vec3s){ center.x - r, center.y - r, center.z - r };
        *outMax = (vec3s){ center.x + r, center.y + r, center.z + r };
    }
    else if (type == COLLIDER_CUBE) {
        BoxCollider* box = (BoxCollider*)obj->Collider->collider;
        mat3s rot = glms_quat_mat3(obj->Orientation);
        float rx = fabsf(rot.col[0].x)*box->HalfSize.x + fabsf(rot.col[1].x)*box->HalfSize.y + fabsf(rot.col[2].x)*box->HalfSize.z + PAD;
        float ry = fabsf(rot.col[0].y)*box->HalfSize.x + fabsf(rot.col[1].y)*box->HalfSize.y + fabsf(rot.col[2].y)*box->HalfSize.z + PAD;
        float rz = fabsf(rot.col[0].z)*box->HalfSize.x + fabsf(rot.col[1].z)*box->HalfSize.y + fabsf(rot.col[2].z)*box->HalfSize.z + PAD;
        *outMin = (vec3s){ obj->Transform.position.x - rx, obj->Transform.position.y - ry, obj->Transform.position.z - rz };
        *outMax = (vec3s){ obj->Transform.position.x + rx, obj->Transform.position.y + ry, obj->Transform.position.z + rz };
    }
    else if (type == COLLIDER_CAPSULE) {
        CapsuleCollider* cap = (CapsuleCollider*)obj->Collider->collider;
        mat3s rot = glms_quat_mat3(obj->Orientation);
        vec3s base = glms_vec3_add(obj->Transform.position, glms_mat3_mulv(rot, cap->LocalBase));
        vec3s tip  = glms_vec3_add(obj->Transform.position, glms_mat3_mulv(rot, cap->LocalTip));
        float r    = cap->Radius + PAD;
        *outMin = (vec3s){
            fminf(base.x, tip.x) - r, fminf(base.y, tip.y) - r, fminf(base.z, tip.z) - r
        };
        *outMax = (vec3s){
            fmaxf(base.x, tip.x) + r, fmaxf(base.y, tip.y) + r, fmaxf(base.z, tip.z) + r
        };
    }
    else {  // PLANE — infini
        *outMin = (vec3s){ -FLT_MAX, -FLT_MAX, -FLT_MAX };
        *outMax = (vec3s){  FLT_MAX,  FLT_MAX,  FLT_MAX };
    }
}

static int _sap_cmp(const void* va, const void* vb) {
    const SAP_Entry* a = (const SAP_Entry*)va;
    const SAP_Entry* b = (const SAP_Entry*)vb;
    if (a->minX < b->minX) return -1;
    if (a->minX > b->minX) return  1;
    return 0;
}

static SAP_Entry g_sapEntries[PHYSICS_MAX_OBJECTS];

static void _broadphase_sap(PhysicsWorld* world) {
    int n = world->numPhysicsObjects;
    world->numBroadphasePairs = 0;

    for (int i = 0; i < n; i++) {
        PhysicsObject* obj = &world->physicsObjects[i];
        vec3s mn, mx;
        _compute_aabb(obj, &mn, &mx);
        g_sapEntries[i].minX   = mn.x;
        g_sapEntries[i].maxX   = mx.x;
        g_sapEntries[i].minXYZ = mn;
        g_sapEntries[i].maxXYZ = mx;
        g_sapEntries[i].objIdx = i;
    }

    qsort(g_sapEntries, n, sizeof(SAP_Entry), _sap_cmp);

    for (int i = 0; i < n; i++) {
        SAP_Entry* ei = &g_sapEntries[i];
        PhysicsObject* objA = &world->physicsObjects[ei->objIdx];
        if (!objA->Collider) continue;

        for (int j = i + 1; j < n; j++) {
            SAP_Entry* ej = &g_sapEntries[j];
            if (ej->minX > ei->maxX) break;

            PhysicsObject* objB = &world->physicsObjects[ej->objIdx];
            if (!objB->Collider) continue;

            if (objA->PhysicsType == PHYS_STATIC && objB->PhysicsType == PHYS_STATIC) continue;
            if (objA->IsSleeping && objB->IsSleeping) continue;

            if (ei->minXYZ.y > ej->maxXYZ.y || ej->minXYZ.y > ei->maxXYZ.y) continue;
            if (ei->minXYZ.z > ej->maxXYZ.z || ej->minXYZ.z > ei->maxXYZ.z) continue;

            if (!Collider_canCollide(objA->Collider, objB->Collider)) continue;

            if (world->numBroadphasePairs < PHYSICS_MAX_PAIRS) {
                int idxA = ei->objIdx, idxB = ej->objIdx;
                if (idxA > idxB) { int tmp = idxA; idxA = idxB; idxB = tmp; }
                world->broadphasePairs[world->numBroadphasePairs++] =
                    (BroadphasePair){ idxA, idxB };
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// CALCUL DES MASSES EFFECTIVES
// ─────────────────────────────────────────────────────────────────────────────

static inline float _compute_contact_mass(
    PhysicsObject* a, PhysicsObject* b,
    vec3s rA, vec3s rB, vec3s axis)
{
    float invM = a->InverseMass + b->InverseMass;
    vec3s raCrossN = glms_vec3_cross(rA, axis);
    vec3s rbCrossN = glms_vec3_cross(rB, axis);

    float angA = glms_vec3_dot(glms_mat3_mulv(a->InertiaTensorInv, raCrossN), raCrossN);
    float angB = glms_vec3_dot(glms_mat3_mulv(b->InertiaTensorInv, rbCrossN), rbCrossN);

    float denom = invM + angA + angB;
    return (denom > 1e-10f) ? 1.0f / denom : 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// SOLVEUR DE CONTACT (Sequential Impulse)
// ─────────────────────────────────────────────────────────────────────────────

static void _compute_tangent_frame(vec3s normal, vec3s* t1, vec3s* t2) {
    vec3s up = (fabsf(normal.y) < 0.9f)
        ? (vec3s){0.0f, 1.0f, 0.0f}
        : (vec3s){1.0f, 0.0f, 0.0f};
    *t1 = _safe_normalize3(glms_vec3_cross(up, normal));
    *t2 = glms_vec3_cross(normal, *t1);
}

static inline void _apply_impulse(PhysicsObject* obj, vec3s impulse, vec3s r) {
    if (obj->PhysicsType != PHYS_DYNAMIC || obj->IsSleeping) return;

    obj->Velocity        = glms_vec3_add(obj->Velocity,
                               glms_vec3_scale(impulse, obj->InverseMass));
    obj->AngularVelocity = glms_vec3_add(obj->AngularVelocity,
                               glms_mat3_mulv(obj->InertiaTensorInv,
                                              glms_vec3_cross(r, impulse)));
}

// Prépare un manifold : calcule les masses effectives et le biais Baumgarte.
//
// CORRECTION (v2) :
//   - velocityBias est maintenant POSITIF  (+(beta/dt)*pen)
//     → dans _solve_velocity : jn = -(vn - bias) = (bias - vn)*mass
//       Si vn=0 et bias>0  → jn > 0 → l'impulsion pousse bien à la séparation ✓
//   - terme de restitution : velocityBias -= restitution * vRel
//     (vRel négatif → soustraction d'un négatif = addition positive) ✓
//   - clamp final sur < 0 (biais ne peut être négatif = ne jamais aspirer)
static void _init_manifold(ContactManifold* m, PhysicsObject* a, PhysicsObject* b, float dt) {
    _compute_tangent_frame(m->normal, &m->tangent[0], &m->tangent[1]);

    for (int k = 0; k < m->contactCount; k++) {
        ContactPoint* cp = &m->contacts[k];
        cp->rA = glms_vec3_sub(cp->worldPointA, a->Transform.position);
        cp->rB = glms_vec3_sub(cp->worldPointB, b->Transform.position);

        cp->normalMass = _compute_contact_mass(a, b, cp->rA, cp->rB, m->normal);

        for (int t = 0; t < 2; t++) {
            cp->tangentMass[t] = _compute_contact_mass(a, b, cp->rA, cp->rB, m->tangent[t]);
        }

        // ── Baumgarte : POSITIF pour pousser A et B à se séparer ─────────────
        float pen = fmaxf(cp->penetrationDepth - PENETRATION_SLOP, 0.0f);
        cp->velocityBias = (BAUMGARTE_FACTOR / dt) * pen;  // CORRECTION: signe +

        // ── Vitesse relative AU POINT DE CONTACT (vitesse angulaire incluse) ──
        vec3s vA = glms_vec3_add(a->Velocity, glms_vec3_cross(a->AngularVelocity, cp->rA));
        vec3s vB = glms_vec3_add(b->Velocity, glms_vec3_cross(b->AngularVelocity, cp->rB));
        float vRel = glms_vec3_dot(glms_vec3_sub(vB, vA), m->normal);

        // ── Restitution (rebond) ───────────────────────────────────────────────
        // vRel < 0 : rapprochement. On veut vn_après = restitution * |vRel|
        // → biais += -restitution * vRel  (= +restitution * |vRel| > 0)
        if (vRel < -1.0f) {
            cp->velocityBias -= m->material.restitution * vRel;  // CORRECTION: -= au lieu de +=
        }

        // Le biais ne peut être que positif (pousser, jamais aspirer)
        if (cp->velocityBias < 0.0f) cp->velocityBias = 0.0f;  // CORRECTION: < au lieu de >
    }
}

// Warm start : réapplique les impulsions de la frame précédente
static void _warm_start(ContactManifold* m, PhysicsObject* a, PhysicsObject* b) {
    for (int k = 0; k < m->contactCount; k++) {
        ContactPoint* cp = &m->contacts[k];
        vec3s P = glms_vec3_scale(m->normal, cp->normalImpulse);
        for (int t = 0; t < 2; t++) {
            P = glms_vec3_add(P, glms_vec3_scale(m->tangent[t], cp->tangentImpulse[t]));
        }

        _apply_impulse(a, glms_vec3_scale(P, -1.0f), cp->rA);
        _apply_impulse(b, P,                          cp->rB);
    }
}

// Une itération du solveur d'impulsions pour un manifold
static void _solve_velocity(ContactManifold* m, PhysicsObject* a, PhysicsObject* b) {
    for (int k = 0; k < m->contactCount; k++) {
        ContactPoint* cp = &m->contacts[k];

        // ── Vitesses au point de contact (avec contribution angulaire) ─────────
        // CORRECTION : utiliser les vitesses locales au point de contact, pas les
        //              vitesses linéaires brutes du centre de masse.
        vec3s vA_contact = glms_vec3_add(a->Velocity, glms_vec3_cross(a->AngularVelocity, cp->rA));
        vec3s vB_contact = glms_vec3_add(b->Velocity, glms_vec3_cross(b->AngularVelocity, cp->rB));

        // ── Friction tangentielle ─────────────────────────────────────────────
        {
            // rv = vA - vB (vitesse relative de A par rapport à B au point de contact)
            vec3s rv = glms_vec3_sub(vA_contact, vB_contact);  // CORRECTION: vitesses locales

            float maxFriction = m->material.dynamicFriction * fabsf(cp->normalImpulse);

            for (int t = 0; t < 2; t++) {
                float vt  = glms_vec3_dot(rv, m->tangent[t]);
                float jt  = -vt * cp->tangentMass[t];

                float oldImpulse = cp->tangentImpulse[t];
                cp->tangentImpulse[t] = _clampf(oldImpulse + jt, -maxFriction, maxFriction);
                float delta = cp->tangentImpulse[t] - oldImpulse;

                vec3s P = glms_vec3_scale(m->tangent[t], delta);
                _apply_impulse(a, glms_vec3_scale(P, -1.0f), cp->rA);
                _apply_impulse(b, P,                          cp->rB);
            }
        }

        // ── Impulsion normale ──────────────────────────────────────────────────
        {
            // Recalculer après les impulsions de friction
            vec3s vA2 = glms_vec3_add(a->Velocity, glms_vec3_cross(a->AngularVelocity, cp->rA));
            vec3s vB2 = glms_vec3_add(b->Velocity, glms_vec3_cross(b->AngularVelocity, cp->rB));
            vec3s rv  = glms_vec3_sub(vB2, vA2);

            float vn = glms_vec3_dot(rv, m->normal);
            // jn = (velocityBias - vn) * normalMass
            // avec velocityBias > 0 : pousse à la séparation même en statique
            float jn = -(vn - cp->velocityBias) * cp->normalMass;

            float oldImpulse = cp->normalImpulse;
            cp->normalImpulse = fmaxf(oldImpulse + jn, 0.0f);
            float delta = cp->normalImpulse - oldImpulse;

            vec3s P = glms_vec3_scale(m->normal, delta);
            _apply_impulse(a, glms_vec3_scale(P, -1.0f), cp->rA);
            _apply_impulse(b, P,                          cp->rB);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// MISE À JOUR DU TENSEUR D'INERTIE MONDE
// ─────────────────────────────────────────────────────────────────────────────

static inline void _update_inertia(PhysicsObject* obj) {
    if (obj->PhysicsType != PHYS_DYNAMIC) return;
    mat3s R  = glms_quat_mat3(obj->Orientation);
    mat3s Rt = glms_mat3_transpose(R);
    obj->InertiaTensorInv = glms_mat3_mul(R, glms_mat3_mul(obj->InertiaTensorInvLocal, Rt));
}

// ─────────────────────────────────────────────────────────────────────────────
// INTÉGRATION — Symplectic Euler
// ─────────────────────────────────────────────────────────────────────────────

static void _integrate(PhysicsObject* obj, vec3s gravity, float dt) {
    if (obj->PhysicsType != PHYS_DYNAMIC || obj->IsSleeping) return;

    // ── Linéaire ────────────────────────────────────────────────────────────
    vec3s accel = gravity;
    if (obj->InverseMass > 0.0f) {
        accel = glms_vec3_add(accel, glms_vec3_scale(obj->Force, obj->InverseMass));
    }
    obj->Velocity           = glms_vec3_add(obj->Velocity, glms_vec3_scale(accel, dt));
    obj->Transform.position = glms_vec3_add(obj->Transform.position,
                                             glms_vec3_scale(obj->Velocity, dt));

    // Damping exponentiel
    float ld = expf(-obj->LinearDamping  * dt);
    float ad = expf(-obj->AngularDamping * dt);
    obj->Velocity        = glms_vec3_scale(obj->Velocity,        ld);
    obj->AngularVelocity = glms_vec3_scale(obj->AngularVelocity, ad);

    // ── Angulaire ────────────────────────────────────────────────────────────
    vec3s angAccel = glms_mat3_mulv(obj->InertiaTensorInv, obj->Torque);
    obj->AngularVelocity = glms_vec3_add(obj->AngularVelocity,
                                          glms_vec3_scale(angAccel, dt));

    // Intégration quaternion : q' = q + 0.5 * dt * [omega]_q * q
    {
        versors q = obj->Orientation;
        vec3s   w = obj->AngularVelocity;
        versors wq;
        wq.raw[0] =  w.x*q.raw[3] + w.y*q.raw[2] - w.z*q.raw[1];
        wq.raw[1] = -w.x*q.raw[2] + w.y*q.raw[3] + w.z*q.raw[0];
        wq.raw[2] =  w.x*q.raw[1] - w.y*q.raw[0] + w.z*q.raw[3];
        wq.raw[3] = -w.x*q.raw[0] - w.y*q.raw[1] - w.z*q.raw[2];

        float half_dt = 0.5f * dt;
        obj->Orientation.raw[0] += wq.raw[0] * half_dt;
        obj->Orientation.raw[1] += wq.raw[1] * half_dt;
        obj->Orientation.raw[2] += wq.raw[2] * half_dt;
        obj->Orientation.raw[3] += wq.raw[3] * half_dt;
        obj->Orientation = glms_quat_normalize(obj->Orientation);
    }

    _update_inertia(obj);

    obj->Force  = glms_vec3_zero();
    obj->Torque = glms_vec3_zero();
}

// ─────────────────────────────────────────────────────────────────────────────
// CORRECTION DE POSITION DIRECTE (post-intégration)
// Gère les pénétrations résiduelles que le solveur de vitesse n'a pas pu corriger.
// Particulièrement utile à grande vitesse ou avec de grands objets.
// ─────────────────────────────────────────────────────────────────────────────

#define POSITION_CORRECTION_FACTOR  0.4f   // fraction corrigée par frame [0..1]
#define POSITION_CORRECTION_SLOP    0.01f  // tolérance avant correction

static void _position_correction(ContactManifold* m, PhysicsObject* a, PhysicsObject* b) {
    for (int k = 0; k < m->contactCount; k++) {
        ContactPoint* cp = &m->contacts[k];

        float pen = fmaxf(cp->penetrationDepth - POSITION_CORRECTION_SLOP, 0.0f);
        if (pen <= 0.0f) continue;

        float totalInvMass = a->InverseMass + b->InverseMass;
        if (totalInvMass < 1e-10f) continue;

        // Déplacement proportionnel aux masses inverses
        float corrScale = POSITION_CORRECTION_FACTOR * pen / totalInvMass;
        vec3s correction = glms_vec3_scale(m->normal, corrScale);

        if (a->PhysicsType == PHYS_DYNAMIC)
            a->Transform.position = glms_vec3_sub(a->Transform.position,
                glms_vec3_scale(correction, a->InverseMass));

        if (b->PhysicsType == PHYS_DYNAMIC)
            b->Transform.position = glms_vec3_add(b->Transform.position,
                glms_vec3_scale(correction, b->InverseMass));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// SYSTÈME DE VEILLE (SLEEP)
// ─────────────────────────────────────────────────────────────────────────────

static void _update_sleep(PhysicsObject* obj, float dt) {
    if (!obj->CanSleep || obj->PhysicsType != PHYS_DYNAMIC) return;

    float linearKE  = glms_vec3_norm2(obj->Velocity);
    float angularKE = glms_vec3_norm2(obj->AngularVelocity);

    if (linearKE  < SLEEP_LINEAR_THRESHOLD  * SLEEP_LINEAR_THRESHOLD &&
        angularKE < SLEEP_ANGULAR_THRESHOLD * SLEEP_ANGULAR_THRESHOLD) {
        obj->SleepTimer += dt;
        if (obj->SleepTimer >= SLEEP_TIME_THRESHOLD) {
            PhysicsObject_sleep(obj);
        }
    } else {
        obj->SleepTimer = 0.0f;
        if (obj->IsSleeping) PhysicsObject_wake(obj);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// INITIALISATION D'UN OBJET
// ─────────────────────────────────────────────────────────────────────────────

static void _init_physics_object(PhysicsObject* obj) {
    memset(obj, 0, sizeof(*obj));
    obj->Orientation           = glms_quat_identity();
    obj->InertiaTensorInv      = (mat3s){0};
    obj->InertiaTensorInvLocal = (mat3s){0};
    obj->Material              = PhysicsMaterial_Default();
    obj->LinearDamping         = 0.05f;
    obj->AngularDamping        = 0.05f;
    obj->CanSleep              = true;
    obj->PhysicsType           = PHYS_DYNAMIC;
    obj->PhysicsTag            = PHYS_TAG_ENVIRONMENT;
}

// ─────────────────────────────────────────────────────────────────────────────
// API PUBLIQUE
// ─────────────────────────────────────────────────────────────────────────────

void PhysicsWorld_init(PhysicsWorld* world) {
    if (!world) return;

    memset(world, 0, sizeof(PhysicsWorld));
    world->gravity   = (vec3s){0.0f, -9.81f, 0.0f};
    world->timeScale = 1.0f;
    ManifoldCache_init(&world->manifoldCache);

    // Sol statique par défaut
    PhysicsWorld_addStaticBox(world,
        (vec3s){0.0f, -5.0f, 0.0f},
        (vec3s){30.0f, 0.5f, 30.0f});
}

void PhysicsWorld_destroy(PhysicsWorld* world) {
    if (!world) return;
    for (int i = 0; i < world->numPhysicsObjects; i++) {
        PhysicsObject* obj = &world->physicsObjects[i];
        if (obj->Collider) Collider_free(obj->Collider);
    }
    world->numPhysicsObjects = 0;
}

PhysicsObject* PhysicsWorld_addObject(PhysicsWorld* world) {
    if (!world || world->numPhysicsObjects >= PHYSICS_MAX_OBJECTS) return NULL;

    int idx = world->numPhysicsObjects++;
    PhysicsObject* obj = &world->physicsObjects[idx];
    _init_physics_object(obj);
    obj->_index = idx;

    obj->Collider = Collider_makeBox((vec3s){0.5f, 0.5f, 0.5f});
    obj->Mass     = 1.0f;
    obj->InverseMass = 1.0f;
    obj->InertiaTensorInvLocal = PhysicsObject_boxInertiaInvLocal(1.0f, (vec3s){0.5f,0.5f,0.5f});
    _update_inertia(obj);

    return obj;
}

PhysicsObject* PhysicsWorld_addStaticBox(PhysicsWorld* world, vec3s position, vec3s halfSize) {
    if (!world || world->numPhysicsObjects >= PHYSICS_MAX_OBJECTS) return NULL;

    int idx = world->numPhysicsObjects++;
    PhysicsObject* obj = &world->physicsObjects[idx];
    _init_physics_object(obj);
    obj->_index = idx;

    obj->Collider        = Collider_makeBox(halfSize);
    obj->PhysicsType     = PHYS_STATIC;
    obj->Mass            = 0.0f;
    obj->InverseMass     = 0.0f;
    obj->Transform.position = position;
    obj->Material        = PhysicsMaterial_Concrete();
    return obj;
}

bool PhysicsWorld_removeObject(PhysicsWorld* world, PhysicsObject* obj) {
    if (!world || !obj) return false;
    int idx = (int)(obj - world->physicsObjects);
    if (idx < 0 || idx >= world->numPhysicsObjects) return false;

    if (obj->Collider) Collider_free(obj->Collider);

    world->numPhysicsObjects--;
    if (idx < world->numPhysicsObjects) {
        world->physicsObjects[idx] = world->physicsObjects[world->numPhysicsObjects];
        world->physicsObjects[idx]._index = idx;
    }
    return true;
}

void PhysicsObject_computeMassFromMaterial(PhysicsObject* obj) {
    if (!obj || !obj->Collider) return;

    float density = obj->Material.density;
    float volume  = 1.0f;

    switch (obj->Collider->type) {
    case COLLIDER_SPHERE: {
        SphereCollider* s = (SphereCollider*)obj->Collider->collider;
        float r = s->Radius;
        volume = (4.0f/3.0f) * (float)M_PI * r*r*r;
        break;
    }
    case COLLIDER_CUBE: {
        BoxCollider* b = (BoxCollider*)obj->Collider->collider;
        volume = 8.0f * b->HalfSize.x * b->HalfSize.y * b->HalfSize.z;
        break;
    }
    case COLLIDER_CAPSULE: {
        CapsuleCollider* c = (CapsuleCollider*)obj->Collider->collider;
        float r  = c->Radius;
        float h  = glms_vec3_norm(glms_vec3_sub(c->LocalTip, c->LocalBase));
        float cyl = (float)M_PI * r*r * h;
        float sph = (4.0f/3.0f) * (float)M_PI * r*r*r;
        volume = cyl + sph;
        break;
    }
    default: return;
    }

    float mass = density * volume;
    obj->Mass        = (mass > 0.0f) ? mass : 1.0f;
    obj->InverseMass = 1.0f / obj->Mass;
}

// ─────────────────────────────────────────────────────────────────────────────
// FORCES
// ─────────────────────────────────────────────────────────────────────────────

void PhysicsWorld_applyForce(PhysicsObject* obj, vec3s force) {
    if (!obj || obj->PhysicsType != PHYS_DYNAMIC) return;
    PhysicsObject_wake(obj);
    obj->Force = glms_vec3_add(obj->Force, force);
}

void PhysicsWorld_applyForceAtPoint(PhysicsObject* obj, vec3s force, vec3s worldPoint) {
    if (!obj || obj->PhysicsType != PHYS_DYNAMIC) return;
    PhysicsObject_wake(obj);
    obj->Force  = glms_vec3_add(obj->Force, force);
    vec3s r     = glms_vec3_sub(worldPoint, obj->Transform.position);
    obj->Torque = glms_vec3_add(obj->Torque, glms_vec3_cross(r, force));
}

void PhysicsWorld_applyTorque(PhysicsObject* obj, vec3s torque) {
    if (!obj || obj->PhysicsType != PHYS_DYNAMIC) return;
    PhysicsObject_wake(obj);
    obj->Torque = glms_vec3_add(obj->Torque, torque);
}

void PhysicsWorld_applyImpulse(PhysicsObject* obj, vec3s impulse, vec3s worldPoint) {
    if (!obj || obj->PhysicsType != PHYS_DYNAMIC) return;
    PhysicsObject_wake(obj);
    obj->Velocity = glms_vec3_add(obj->Velocity,
        glms_vec3_scale(impulse, obj->InverseMass));
    vec3s r = glms_vec3_sub(worldPoint, obj->Transform.position);
    obj->AngularVelocity = glms_vec3_add(obj->AngularVelocity,
        glms_mat3_mulv(obj->InertiaTensorInv, glms_vec3_cross(r, impulse)));
}

void PhysicsWorld_explosion(PhysicsWorld* world, vec3s center, float intensity, float radius) {
    if (!world || radius <= 0.0f) return;

    float invRadius = 1.0f / radius;

    for (int i = 0; i < world->numPhysicsObjects; i++) {
        PhysicsObject* obj = &world->physicsObjects[i];
        if (obj->PhysicsType != PHYS_DYNAMIC || obj->Mass <= 0.0f) continue;

        vec3s dir  = glms_vec3_sub(obj->Transform.position, center);
        float dist = glms_vec3_norm(dir);

        if (dist < 1e-4f || dist > radius) continue;

        float strength = (1.0f - dist * invRadius);
        strength *= strength;
        float mag = intensity * strength / obj->Mass;

        dir = glms_vec3_scale(dir, 1.0f / dist);
        PhysicsWorld_applyImpulse(obj, glms_vec3_scale(dir, mag), obj->Transform.position);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// VEILLE
// ─────────────────────────────────────────────────────────────────────────────

void PhysicsObject_wake(PhysicsObject* obj) {
    if (!obj) return;
    obj->IsSleeping = false;
    obj->SleepTimer = 0.0f;
}

void PhysicsObject_sleep(PhysicsObject* obj) {
    if (!obj) return;
    obj->IsSleeping       = true;
    obj->Velocity         = glms_vec3_zero();
    obj->AngularVelocity  = glms_vec3_zero();
    obj->Force            = glms_vec3_zero();
    obj->Torque           = glms_vec3_zero();
}

// ─────────────────────────────────────────────────────────────────────────────
// RAYCASTING
// ─────────────────────────────────────────────────────────────────────────────

RaycastHit PhysicsWorld_raycast(
    PhysicsWorld* world,
    vec3s         origin,
    vec3s         direction,
    float         maxDistance,
    CollisionLayer layerMask)
{
    RaycastHit best = {0};
    best.distance = maxDistance + 1.0f;

    vec3s dir = _safe_normalize3(direction);

    for (int i = 0; i < world->numPhysicsObjects; i++) {
        PhysicsObject* obj = &world->physicsObjects[i];
        if (!obj->Collider) continue;
        if (!(obj->Collider->category & layerMask)) continue;

        float t = -1.0f;
        switch (obj->Collider->type) {
        case COLLIDER_SPHERE:  t = Collisions_raycastSphere (origin, dir, maxDistance, obj); break;
        case COLLIDER_CUBE:    t = Collisions_raycastBox    (origin, dir, maxDistance, obj); break;
        case COLLIDER_CAPSULE: t = Collisions_raycastCapsule(origin, dir, maxDistance, obj); break;
        case COLLIDER_PLANE:   t = Collisions_raycastPlane  (origin, dir, maxDistance, obj); break;
        }

        if (t >= 0.0f && t < best.distance) {
            best.hit      = true;
            best.distance = t;
            best.point    = glms_vec3_add(origin, glms_vec3_scale(dir, t));
            best.object   = obj;
            best.normal   = glms_vec3_scale(dir, -1.0f);
        }
    }
    return best;
}

// ─────────────────────────────────────────────────────────────────────────────
// UTILITAIRES MONDE
// ─────────────────────────────────────────────────────────────────────────────

void PhysicsWorld_setGravity(PhysicsWorld* world, vec3s gravity) {
    if (!world) return;
    world->gravity = gravity;
}

void PhysicsWorld_setPaused(PhysicsWorld* world, bool paused) {
    if (!world) return;
    world->paused = paused;
}

// ─────────────────────────────────────────────────────────────────────────────
// STEP PRINCIPAL
// ─────────────────────────────────────────────────────────────────────────────

void PhysicsWorld_step(PhysicsWorld* world, float deltaTime) {
    if (!world || world->paused || deltaTime <= 0.0f) return;

    float dt = deltaTime * world->timeScale;
    if (dt > 0.05f) dt = 0.05f;

    // ── 1. Broadphase ────────────────────────────────────────────────────────
    _broadphase_sap(world);
    world->stats.broadphasePairs = world->numBroadphasePairs;

    // ── 2. Narrowphase + mise à jour des manifolds ───────────────────────────
    int narrowCount = 0;

    for (int i = 0; i < world->numBroadphasePairs; i++) {
        BroadphasePair* pair = &world->broadphasePairs[i];
        PhysicsObject* a     = &world->physicsObjects[pair->indexA];
        PhysicsObject* b     = &world->physicsObjects[pair->indexB];

        CollisionPoints cp = Collisions_test(a, b);
        if (!cp.HasCollision) continue;

        narrowCount++;

        if (a->IsSleeping) PhysicsObject_wake(a);
        if (b->IsSleeping) PhysicsObject_wake(b);

        if (a->Collider->isTrigger || b->Collider->isTrigger) continue;

        ContactManifold* m = ManifoldCache_get(&world->manifoldCache,
                                                pair->indexA, pair->indexB);
        if (!m) continue;

        // Préserver les impulsions (warm start)
        float prevNormal[MANIFOLD_MAX_CONTACTS]     = {0};
        float prevTangent[MANIFOLD_MAX_CONTACTS][2] = {{0}};
        for (int k = 0; k < m->contactCount && k < MANIFOLD_MAX_CONTACTS; k++) {
            prevNormal[k]     = m->contacts[k].normalImpulse;
            prevTangent[k][0] = m->contacts[k].tangentImpulse[0];
            prevTangent[k][1] = m->contacts[k].tangentImpulse[1];
        }

        m->normal       = cp.Normal;
        m->material     = PhysicsMaterial_combine(a->Material, b->Material);
        m->contactCount = 1;
        m->active       = true;

        ContactPoint* cpt = &m->contacts[0];
        cpt->worldPointA      = cp.A;
        cpt->worldPointB      = cp.B;
        cpt->penetrationDepth = cp.Depth;

        cpt->normalImpulse     = prevNormal[0];
        cpt->tangentImpulse[0] = prevTangent[0][0];
        cpt->tangentImpulse[1] = prevTangent[0][1];
    }
    world->stats.narrowphasePairs = narrowCount;
    world->stats.manifolds        = world->manifoldCache.count;

    // ── 3. Warm start ────────────────────────────────────────────────────────
    for (int i = 0; i < world->manifoldCache.count; i++) {
        ContactManifold* m = &world->manifoldCache.manifolds[i];
        if (!m->active) continue;
        PhysicsObject* a = &world->physicsObjects[m->idxA];
        PhysicsObject* b = &world->physicsObjects[m->idxB];
        _init_manifold(m, a, b, dt);
        _warm_start(m, a, b);
    }

    // ── 4. Solveur d'impulsions (N itérations) ───────────────────────────────
    for (int iter = 0; iter < VELOCITY_SOLVER_ITERATIONS; iter++) {
        for (int i = 0; i < world->manifoldCache.count; i++) {
            ContactManifold* m = &world->manifoldCache.manifolds[i];
            if (!m->active) continue;
            PhysicsObject* a = &world->physicsObjects[m->idxA];
            PhysicsObject* b = &world->physicsObjects[m->idxB];
            _solve_velocity(m, a, b);
        }
    }

    // ── 5. Intégration ───────────────────────────────────────────────────────
    int activeCount = 0;
    for (int i = 0; i < world->numPhysicsObjects; i++) {
        PhysicsObject* obj = &world->physicsObjects[i];
        if (obj->PhysicsType == PHYS_DYNAMIC) {
            _integrate(obj, world->gravity, dt);
            _update_sleep(obj, dt);
            if (!obj->IsSleeping) activeCount++;
        }
    }
    world->stats.objectsActive = activeCount;

    // ── 6. Correction de position directe ────────────────────────────────────
    // Élimine les pénétrations résiduelles (important à grande vitesse / grands objets)
    for (int i = 0; i < world->manifoldCache.count; i++) {
        ContactManifold* m = &world->manifoldCache.manifolds[i];
        if (!m->active) continue;
        PhysicsObject* a = &world->physicsObjects[m->idxA];
        PhysicsObject* b = &world->physicsObjects[m->idxB];
        _position_correction(m, a, b);
    }

    // ── 7. Purge des manifolds périmés ───────────────────────────────────────
    ManifoldCache_purge(&world->manifoldCache);
}

// Pas fixe avec accumulation (recommandé pour la stabilité)
void PhysicsWorld_stepFixed(PhysicsWorld* world, float realDeltaTime, float fixedDt) {
    if (!world || fixedDt <= 0.0f) return;
    world->accumulatedTime += realDeltaTime;
    int maxSubsteps = 8;
    while (world->accumulatedTime >= fixedDt && maxSubsteps-- > 0) {
        PhysicsWorld_step(world, fixedDt);
        world->accumulatedTime -= fixedDt;
    }
    if (world->accumulatedTime > fixedDt * 4.0f) {
        world->accumulatedTime = 0.0f;
    }
}