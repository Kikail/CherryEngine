// physics_world.h
// Monde physique 3D — CherryEngine
// Inspiré de Box2D (séquentielle impulsion, warm starting, SAP broadphase)

#ifndef CHERRYENGINE_PHYSICSWORLD_H
#define CHERRYENGINE_PHYSICSWORLD_H

#include <stdbool.h>
#include <cglm/struct.h>

#include "physicsObject.h"
#include "contact_manifold.h"

// ─── Constantes ──────────────────────────────────────────────────────────────

#define PHYSICS_MAX_OBJECTS      5000
#define PHYSICS_MAX_PAIRS        (PHYSICS_MAX_OBJECTS * 8)   // Broadphase pairs max

// Itérations du solveur (Box2D par défaut : 10 velocity, 3 position)
#define VELOCITY_SOLVER_ITERATIONS  10
#define POSITION_SOLVER_ITERATIONS  3

// Baumgarte stabilization
#define BAUMGARTE_FACTOR   0.2f    // Part de la pénétration corrigée par frame
#define PENETRATION_SLOP   0.005f  // Tolérance avant activation de la correction

// ─── Résultat de raycast ─────────────────────────────────────────────────────

typedef struct RaycastHit_t {
    bool          hit;
    float         distance;               // Distance le long du rayon
    vec3s         point;                  // Point d'impact monde
    vec3s         normal;                 // Normale à la surface
    PhysicsObject* object;                // Objet touché (NULL si pas de hit)
} RaycastHit;

// ─── Paires broadphase ───────────────────────────────────────────────────────

typedef struct BroadphasePair_t {
    int indexA;
    int indexB;
} BroadphasePair;

// ─── Monde physique ──────────────────────────────────────────────────────────

typedef struct PhysicsWorld_t {
    // Objets physiques (tableau statique, évite les allocations par frame)
    PhysicsObject physicsObjects[PHYSICS_MAX_OBJECTS];
    int           numPhysicsObjects;

    // Cache de manifolds persistants (warm starting)
    ManifoldCache manifoldCache;

    // Broadphase (SAP — Sweep And Prune sur l'axe X)
    BroadphasePair broadphasePairs[PHYSICS_MAX_PAIRS];
    int            numBroadphasePairs;

    // Paramètres globaux
    vec3s gravity;
    bool  paused;
    float timeScale;                      // 1.0 = temps normal, 0.5 = slow-mo
    float accumulatedTime;                // Pour le substep à pas fixe

    // Statistiques de la dernière frame (lecture seule)
    struct {
        int objectsActive;                // Non-dormants
        int broadphasePairs;
        int narrowphasePairs;
        int manifolds;
        float stepTimeMs;
    } stats;
} PhysicsWorld;

// ─── Cycle de vie ────────────────────────────────────────────────────────────

// CORRECTION : Utilisation d'un pointeur pour éviter le Stack Overflow
void          PhysicsWorld_init(PhysicsWorld* world);
void          PhysicsWorld_destroy(PhysicsWorld* world); // Libère la mémoire des colliders

// ─── Gestion des objets ──────────────────────────────────────────────────────

// Ajoute un objet vide (par défaut DYNAMIC, masse=1, box 1m)
// Retourne un pointeur dans le tableau interne (valide jusqu'à removeObject)
PhysicsObject* PhysicsWorld_addObject(PhysicsWorld* world);

// Supprime un objet (swap-with-last, O(1)) — libère la mémoire du collider
bool           PhysicsWorld_removeObject(PhysicsWorld* world, PhysicsObject* obj);

// Ajoute un mur/sol statique avec BoxCollider
PhysicsObject* PhysicsWorld_addStaticBox(PhysicsWorld* world, vec3s position, vec3s halfSize);

// Configure la masse depuis la densité du matériau + géométrie du collider
void           PhysicsObject_computeMassFromMaterial(PhysicsObject* obj);

// ─── Mise à jour ─────────────────────────────────────────────────────────────

// Avance la simulation d'un pas de temps variable (pas conseillé pour la stabilité)
void PhysicsWorld_step(PhysicsWorld* world, float deltaTime);

// Avance avec un pas de temps fixe + interpolation (recommandé)
// fixedDt = 1/60 typiquement ; appelé depuis la boucle principale avec le deltaTime réel
void PhysicsWorld_stepFixed(PhysicsWorld* world, float realDeltaTime, float fixedDt);

// ─── Application de forces ────────────────────────────────────────────────────

void PhysicsWorld_applyForce       (PhysicsObject* obj, vec3s force);
void PhysicsWorld_applyForceAtPoint(PhysicsObject* obj, vec3s force, vec3s worldPoint);
void PhysicsWorld_applyTorque      (PhysicsObject* obj, vec3s torque);
void PhysicsWorld_applyImpulse     (PhysicsObject* obj, vec3s impulse, vec3s worldPoint);

// Explosion radiale — applique une impulsion à tous les objets dans le rayon
void PhysicsWorld_explosion(PhysicsWorld* world, vec3s center, float intensity, float radius);

// ─── Veille ──────────────────────────────────────────────────────────────────

void PhysicsObject_wake (PhysicsObject* obj);
void PhysicsObject_sleep(PhysicsObject* obj);

// ─── Raycasting ──────────────────────────────────────────────────────────────

// Lance un rayon depuis `origin` dans `direction` sur `maxDistance`.
// `layerMask` filtre les objets (COLLISION_LAYER_ALL = tout tester).
RaycastHit PhysicsWorld_raycast(
    PhysicsWorld* world,
    vec3s         origin,
    vec3s         direction,
    float         maxDistance,
    CollisionLayer layerMask
);

// ─── Utilitaires ─────────────────────────────────────────────────────────────

void PhysicsWorld_setGravity(PhysicsWorld* world, vec3s gravity);
void PhysicsWorld_setPaused (PhysicsWorld* world, bool paused);

#endif // CHERRYENGINE_PHYSICSWORLD_H