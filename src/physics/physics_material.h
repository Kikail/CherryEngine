// physics_material.h
// Matériaux physiques : rebond, friction, densité.
// La combinaison de deux matériaux suit la convention de Box2D :
//   restitution = min(a, b)  — on prend le moins rebondissant
//   friction    = sqrt(a*b)  — moyenne géométrique (Coulomb)

#ifndef CHERRYENGINE_PHYSICS_MATERIAL_H
#define CHERRYENGINE_PHYSICS_MATERIAL_H

#include <math.h>

typedef struct PhysicsMaterial_t {
    float restitution;      // Coefficient de rebond  [0 = mou .. 1 = parfaitement élastique]
    float staticFriction;   // Friction statique (Coulomb)
    float dynamicFriction;  // Friction cinétique (Coulomb, <= staticFriction)
    float density;          // Densité (kg/m³) — utilisé pour calculer la masse automatiquement
} PhysicsMaterial;

// ─── Presets ────────────────────────────────────────────────────────────────

static inline PhysicsMaterial PhysicsMaterial_Default(void) {
    return (PhysicsMaterial){ 0.30f, 0.50f, 0.40f, 1.00f };
}
static inline PhysicsMaterial PhysicsMaterial_Rubber(void) {
    return (PhysicsMaterial){ 0.80f, 0.90f, 0.70f, 1.20f };
}
static inline PhysicsMaterial PhysicsMaterial_Metal(void) {
    return (PhysicsMaterial){ 0.10f, 0.60f, 0.40f, 7.80f };
}
static inline PhysicsMaterial PhysicsMaterial_Ice(void) {
    return (PhysicsMaterial){ 0.05f, 0.05f, 0.02f, 0.90f };
}
static inline PhysicsMaterial PhysicsMaterial_Wood(void) {
    return (PhysicsMaterial){ 0.20f, 0.50f, 0.30f, 0.60f };
}
static inline PhysicsMaterial PhysicsMaterial_Concrete(void) {
    return (PhysicsMaterial){ 0.05f, 0.80f, 0.70f, 2.00f };
}
static inline PhysicsMaterial PhysicsMaterial_Bouncy(void) {
    return (PhysicsMaterial){ 0.95f, 0.30f, 0.20f, 1.00f };
}

// ─── Combinaison (appelée lors de la résolution) ────────────────────────────

static inline PhysicsMaterial PhysicsMaterial_combine(PhysicsMaterial a, PhysicsMaterial b) {
    return (PhysicsMaterial){
        .restitution     = (a.restitution < b.restitution) ? a.restitution : b.restitution,
        .staticFriction  = sqrtf(a.staticFriction  * b.staticFriction),
        .dynamicFriction = sqrtf(a.dynamicFriction * b.dynamicFriction),
        .density         = 1.0f   // non utilisé après combination
    };
}

#endif // CHERRYENGINE_PHYSICS_MATERIAL_H
