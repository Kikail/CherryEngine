// physics_object.h
// Représente un corps rigide dans le moteur physique.
// Supporte STATIC, DYNAMIC et KINEMATIC (animé manuellement, repousse les DYNAMIC).

#ifndef CHERRYENGINE_PHYSICSOBJECT_H
#define CHERRYENGINE_PHYSICSOBJECT_H

#include <cglm/struct.h>
#include <stdbool.h>

#include "colliders.h"
#include "physics_material.h"
#include "../editor/ecs/components/transform.h"

// ─── Énumérations ───────────────────────────────────────────────────────────

typedef enum PhysicsTag_t {
    PHYS_TAG_ENVIRONMENT = 0,
    PHYS_TAG_PLAYER,
    PHYS_TAG_ENEMY,
    PHYS_TAG_PROJECTILE,
    PHYS_TAG_USER0,
    PHYS_TAG_USER1,
} PhysicsTag;

typedef enum PhysicsType_t {
    PHYS_DYNAMIC   = 0, // Affecté par les forces, collisions, gravité
    PHYS_STATIC    = 1, // Immobile (masse infinie, InertiaTensorInv = 0)
    PHYS_KINEMATIC = 2, // Déplacé manuellement ; repousse les DYNAMIC sans être affecté
} PhysicsType;

// ─── Seuils de mise en veille ───────────────────────────────────────────────

#define SLEEP_LINEAR_THRESHOLD   0.008f  // m/s   — vitesse sous laquelle on considère l'objet immobile
#define SLEEP_ANGULAR_THRESHOLD  0.008f  // rad/s
#define SLEEP_TIME_THRESHOLD     0.5f    // secondes immobiles avant mise en veille

// ─── Corps rigide ────────────────────────────────────────────────────────────

typedef struct PhysicsObject_t {
    // ── Spatial ──────────────────────────────────────────────────────────────
    Transform       Transform;
    Collider*       Collider;

    // ── Linéaire ─────────────────────────────────────────────────────────────
    vec3s           Velocity;       // m/s
    vec3s           Force;          // N  (accumulateur de frame)
    float           Mass;           // kg (0 = statique)
    float           InverseMass;    // 1/Mass (0 si STATIC/KINEMATIC)

    // ── Angulaire ────────────────────────────────────────────────────────────
    versors         Orientation;          // Quaternion d'orientation (normalisé)
    vec3s           AngularVelocity;      // rad/s (espace monde)
    vec3s           Torque;               // N·m (accumulateur de frame)
    mat3s           InertiaTensorInv;     // Inverse du tenseur d'inertie (espace monde)
    mat3s           InertiaTensorInvLocal;// Inverse du tenseur d'inertie (espace local, constant)

    // ── Matériau ─────────────────────────────────────────────────────────────
    PhysicsMaterial Material;

    // ── Damping ──────────────────────────────────────────────────────────────
    float           LinearDamping;   // [0..1] par seconde  (défaut : 0.05)
    float           AngularDamping;  // [0..1] par seconde  (défaut : 0.05)

    // ── Veille (sleep) ───────────────────────────────────────────────────────
    float           SleepTimer;     // Temps cumulé sous le seuil d'énergie
    bool            IsSleeping;     // Corps en veille (skip intégration)
    bool            CanSleep;       // Permet de désactiver le sleep

    // ── Méta ─────────────────────────────────────────────────────────────────
    PhysicsType     PhysicsType;
    PhysicsTag      PhysicsTag;
    void*           UserData;       // Pointeur libre pour l'utilisateur (ECS entity, etc.)
    int             _index;         // Index dans PhysicsWorld.physicsObjects (géré par le monde)
} PhysicsObject;

// ─── Helpers inertie ─────────────────────────────────────────────────────────

// Retourne l'inverse du tenseur d'inertie LOCAL d'une boîte (pavé solide)
static inline mat3s PhysicsObject_boxInertiaInvLocal(float mass, vec3s halfSize) {
    if (mass <= 0.0f) return (mat3s){0};
    float dx = halfSize.x * 2.0f, dy = halfSize.y * 2.0f, dz = halfSize.z * 2.0f;
    float k  = mass / 12.0f;
    mat3s inertia = (mat3s){0};
    inertia.m00 = k * (dy*dy + dz*dz);
    inertia.m11 = k * (dx*dx + dz*dz);
    inertia.m22 = k * (dx*dx + dy*dy);
    return glms_mat3_inv(inertia);
}

// Retourne l'inverse du tenseur d'inertie LOCAL d'une sphère solide
static inline mat3s PhysicsObject_sphereInertiaInvLocal(float mass, float radius) {
    if (mass <= 0.0f) return (mat3s){0};
    float i   = (2.0f / 5.0f) * mass * radius * radius;
    float inv = 1.0f / i;
    mat3s out = (mat3s){0};
    out.m00 = inv; out.m11 = inv; out.m22 = inv;
    return out;
}

// Retourne l'inverse du tenseur LOCAL d'une capsule (cylindre + 2 hémisphères)
// axis = 1 (Y par défaut) : axe principal de la capsule
static inline mat3s PhysicsObject_capsuleInertiaInvLocal(float mass, float radius, float halfHeight) {
    if (mass <= 0.0f) return (mat3s){0};
    // Masses partielles
    float hh = halfHeight; // demi-hauteur du cylindre
    float r  = radius;
    float cylVol  = (float)M_PI * r*r * (2.0f * hh);
    float hemVol  = (2.0f/3.0f) * (float)M_PI * r*r*r;
    float totalVol = cylVol + 2.0f * hemVol;
    float mCyl  = mass * (cylVol  / totalVol);
    float mHem  = mass * (hemVol  / totalVol); // par hémisphère

    // Cylindre : Iy = 0.5*m*r^2,  Ix=Iz = m*(3r^2+h^2)/12  (h = 2*hh)
    float cylIy = 0.5f * mCyl * r*r;
    float cylIx = mCyl * (3.0f*r*r + (2.0f*hh)*(2.0f*hh)) / 12.0f;

    // Hémisphère : Iy = (2/5)*m*r^2,  Ix=Iz = Iy + m*(3π-4)/(8π)*r^2 approx
    float hemIy  = (2.0f/5.0f) * mHem * r*r;
    float offset = hh + (3.0f*(float)M_PI - 4.0f) / ((float)M_PI * 8.0f) * r; // centroïde depuis centre
    float hemIx  = hemIy + mHem * offset * offset; // Steiner

    float Iy = cylIy + 2.0f * hemIy;
    float Ix = cylIx + 2.0f * hemIx;

    float iy = (Iy > 0.0f) ? 1.0f/Iy : 0.0f;
    float ix = (Ix > 0.0f) ? 1.0f/Ix : 0.0f;

    mat3s out = (mat3s){0};
    out.m00 = ix; out.m11 = iy; out.m22 = ix; // Y = axe capsule
    return out;
}

// Met à jour le tenseur d'inertie monde depuis le tenseur local et l'orientation
// Appeler après chaque changement d'orientation.
// I_world_inv = R * I_local_inv * R^T
static inline void PhysicsObject_updateInertiaTensor(PhysicsObject* obj) {
    if (obj->PhysicsType != PHYS_DYNAMIC) return;
    mat3s R = glms_quat_mat3(obj->Orientation);
    mat3s Rt = glms_mat3_transpose(R);
    obj->InertiaTensorInv = glms_mat3_mul(R, glms_mat3_mul(obj->InertiaTensorInvLocal, Rt));
}

#endif // CHERRYENGINE_PHYSICSOBJECT_H
