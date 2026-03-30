// contact_manifold.h
// Manifold de contact persistant — inspiré de Box2D.
// Un manifold est partagé entre deux corps sur plusieurs frames (warm starting).
// Chaque manifold stocke jusqu'à 4 points de contact.
// Les impulsions normales et tangentielles accumulées sont réinjectées
// au début de chaque frame (warm start) pour accélérer la convergence.

#ifndef CHERRYENGINE_CONTACT_MANIFOLD_H
#define CHERRYENGINE_CONTACT_MANIFOLD_H

#include <cglm/struct.h>
#include <stdint.h>
#include <stdbool.h>

#include "physics_material.h"

#define MANIFOLD_MAX_CONTACTS  4       // Max points de contact par paire
#define MANIFOLD_MAX_AGE       2       // Frames de survie sans mise à jour
#define MAX_MANIFOLDS          20000   // Paires simultanées max

// ─── Point de contact individuel ─────────────────────────────────────────────

typedef struct ContactPoint_t {
    vec3s worldPointA;         // Point sur la surface de A (world space)
    vec3s worldPointB;         // Point sur la surface de B (world space)
    vec3s rA;                  // worldPointA - centerA (bras de levier)
    vec3s rB;                  // worldPointB - centerB
    float penetrationDepth;    // > 0 = pénétration

    // ── Warm starting ──────────────────────────────────────────────────────
    float normalImpulse;       // Impulsion normale accumulée (frame N-1)
    float tangentImpulse[2];   // Impulsions tangentielles accumulées

    // ── Masses effectives (précalculées) ──────────────────────────────────
    float normalMass;          // 1 / (invMassA + invMassB + termes angulaires)
    float tangentMass[2];
    float velocityBias;        // Biais Baumgarte pour la pénétration
} ContactPoint;

// ─── Manifold persistant ─────────────────────────────────────────────────────

typedef struct ContactManifold_t {
    // Indices dans PhysicsWorld.physicsObjects (clé de cache)
    int   idxA;
    int   idxB;

    // Géométrie de contact
    vec3s normal;              // Normale (de A vers B, normalisée)
    vec3s tangent[2];          // Repère tangentiel orthogonal

    ContactPoint contacts[MANIFOLD_MAX_CONTACTS];
    int          contactCount;

    // Matériau combiné (calculé une fois par mise à jour)
    PhysicsMaterial material;

    // Gestion du cache
    uint8_t  age;              // Incrémenté chaque frame si non mis à jour
    bool     active;           // Mis à jour cette frame ?
} ContactManifold;

// ─── Cache de manifolds (géré par PhysicsWorld) ──────────────────────────────

typedef struct ManifoldCache_t {
    ContactManifold manifolds[MAX_MANIFOLDS];
    int             count;
    // Table de hachage ouverte pour lookup O(1)
    // Clé : (idxA * 5003 + idxB) % HASH_SIZE   (5003 est premier)
    int             table[MAX_MANIFOLDS * 2];  // -1 = vide
} ManifoldCache;

#define MANIFOLD_HASH_SIZE (MAX_MANIFOLDS * 2)  // doit être > MAX_MANIFOLDS

static inline uint32_t _manifold_hash(int idxA, int idxB) {
    // Cantor pairing function pour garantir unicité
    uint32_t k = (uint32_t)((idxA + idxB) * (idxA + idxB + 1) / 2 + idxB);
    return k % (uint32_t)MANIFOLD_HASH_SIZE;
}

// Trouve ou crée un manifold pour la paire (idxA, idxB).
// idxA DOIT être <= idxB (canonique).
// Retourne NULL si le cache est plein.
static inline ContactManifold* ManifoldCache_get(ManifoldCache* cache, int idxA, int idxB) {
    uint32_t h = _manifold_hash(idxA, idxB);
    uint32_t start = h;

    do {
        int slot = cache->table[h];
        if (slot == -1) {
            // Slot vide → créer
            if (cache->count >= MAX_MANIFOLDS) return NULL;
            int idx = cache->count++;
            ContactManifold* m = &cache->manifolds[idx];
            *m = (ContactManifold){0};
            m->idxA = idxA;
            m->idxB = idxB;
            cache->table[h] = idx;
            return m;
        }
        ContactManifold* m = &cache->manifolds[slot];
        if (m->idxA == idxA && m->idxB == idxB) return m;
        h = (h + 1) % MANIFOLD_HASH_SIZE;
    } while (h != start);

    return NULL; // table pleine
}

static inline void ManifoldCache_init(ManifoldCache* cache) {
    cache->count = 0;
    for (int i = 0; i < MANIFOLD_HASH_SIZE; i++) cache->table[i] = -1;
}

// Supprime les manifolds inactifs (age > seuil).
// Reconstruit la table de hachage proprement (compactage).
static inline void ManifoldCache_purge(ManifoldCache* cache) {
    int newCount = 0;
    for (int i = 0; i < cache->count; i++) {
        ContactManifold* m = &cache->manifolds[i];
        if (m->active) {
            m->age    = 0;
            m->active = false;
            cache->manifolds[newCount++] = *m;
        } else {
            m->age++;
            if (m->age <= MANIFOLD_MAX_AGE) {
                cache->manifolds[newCount++] = *m;
            }
            // else : on le supprime silencieusement
        }
    }
    cache->count = newCount;

    // Reconstruire la table de hachage
    for (int i = 0; i < MANIFOLD_HASH_SIZE; i++) cache->table[i] = -1;
    for (int i = 0; i < cache->count; i++) {
        ContactManifold* m = &cache->manifolds[i];
        uint32_t h = _manifold_hash(m->idxA, m->idxB);
        while (cache->table[h] != -1) h = (h + 1) % MANIFOLD_HASH_SIZE;
        cache->table[h] = i;
    }
}

#endif // CHERRYENGINE_CONTACT_MANIFOLD_H
