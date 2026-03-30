// colliders.h
// Définit tous les types de colliders supportés par CherryEngine :
//   PLANE, SPHERE, CUBE (OBB), CAPSULE
// + système de layers/masks pour filtrer les collisions
// + flag TRIGGER (volume fantôme sans réponse physique)

#ifndef CHERRYENGINE_COLLIDERS_H
#define CHERRYENGINE_COLLIDERS_H

#include <stdint.h>
#include <stdbool.h>
#include <cglm/struct.h>

// ─── Layers de collision (bitmask 16 bits) ──────────────────────────────────
// Un objet avec category=LAYER_PLAYER et mask=LAYER_WORLD | LAYER_ENEMY
// ne réagit qu'aux objets de ces catégories.

typedef uint16_t CollisionLayer;

#define COLLISION_LAYER_ALL      0xFFFFu
#define COLLISION_LAYER_NONE     0x0000u
#define COLLISION_LAYER_DEFAULT  (1u << 0)
#define COLLISION_LAYER_PLAYER   (1u << 1)
#define COLLISION_LAYER_ENEMY    (1u << 2)
#define COLLISION_LAYER_WORLD    (1u << 3)
#define COLLISION_LAYER_TRIGGER  (1u << 4)
#define COLLISION_LAYER_RAYCAST  (1u << 5)
// bits 6-15 libres pour l'utilisateur

// ─── Type de collider ───────────────────────────────────────────────────────

typedef enum ColliderType_t {
    COLLIDER_PLANE   = 0,
    COLLIDER_SPHERE  = 1,
    COLLIDER_CUBE    = 2,   // OBB — Oriented Bounding Box
    COLLIDER_CAPSULE = 3,
} ColliderType;

// ─── Géométries ─────────────────────────────────────────────────────────────

typedef struct SphereCollider_t {
    vec3s Center;   // Offset local par rapport à l'objet
    float Radius;
} SphereCollider;

typedef struct PlaneCollider_t {
    vec3s Normal;   // Normale du plan (doit être normalisée)
    float Distance; // Distance signée depuis l'origine du monde
} PlaneCollider;

typedef struct BoxCollider_t {
    vec3s HalfSize; // Demi-dimensions (x,y,z) en espace local
} BoxCollider;

// Capsule = cylindre + deux hémisphères aux extrémités.
// LocalBase et LocalTip définissent les centres des deux hémisphères
// en espace local de l'objet (avant application de l'orientation).
typedef struct CapsuleCollider_t {
    vec3s LocalBase;   // Centre hémisphère bas  (espace local)
    vec3s LocalTip;    // Centre hémisphère haut (espace local)
    float Radius;
} CapsuleCollider;

// ─── Collider générique ─────────────────────────────────────────────────────

typedef struct Collider_t {
    ColliderType    type;
    void*           collider;   // Pointeur vers la géométrie ci-dessus
    bool            isTrigger;  // true = détection seule, pas de réponse physique
    CollisionLayer  category;   // Ce que cet objet EST
    CollisionLayer  mask;       // Ce avec quoi il PEUT entrer en contact
} Collider;

// ─── Points de contact ──────────────────────────────────────────────────────

typedef struct CollisionPoints_t {
    vec3s A;            // Point de contact sur la surface de A
    vec3s B;            // Point de contact sur la surface de B
    vec3s Normal;       // Normale de collision (de A vers B, normalisée)
    float Depth;        // Profondeur de pénétration (toujours >= 0)
    bool  HasCollision;
} CollisionPoints;

// ─── Constructeurs utilitaires ───────────────────────────────────────────────

static inline Collider* Collider_makeSphere(float radius, vec3s center) {
    Collider*       c = (Collider*)      malloc(sizeof(Collider));
    SphereCollider* s = (SphereCollider*)malloc(sizeof(SphereCollider));
    if (!c || !s) { free(c); free(s); return NULL; }
    s->Center = center;
    s->Radius = radius;
    *c = (Collider){ COLLIDER_SPHERE, s, false, COLLISION_LAYER_DEFAULT, COLLISION_LAYER_ALL };
    return c;
}

static inline Collider* Collider_makeBox(vec3s halfSize) {
    Collider*    c = (Collider*)   malloc(sizeof(Collider));
    BoxCollider* b = (BoxCollider*)malloc(sizeof(BoxCollider));
    if (!c || !b) { free(c); free(b); return NULL; }
    b->HalfSize = halfSize;
    *c = (Collider){ COLLIDER_CUBE, b, false, COLLISION_LAYER_DEFAULT, COLLISION_LAYER_ALL };
    return c;
}

static inline Collider* Collider_makeCapsule(vec3s localBase, vec3s localTip, float radius) {
    Collider*        c = (Collider*)        malloc(sizeof(Collider));
    CapsuleCollider* k = (CapsuleCollider*) malloc(sizeof(CapsuleCollider));
    if (!c || !k) { free(c); free(k); return NULL; }
    k->LocalBase = localBase;
    k->LocalTip  = localTip;
    k->Radius    = radius;
    *c = (Collider){ COLLIDER_CAPSULE, k, false, COLLISION_LAYER_DEFAULT, COLLISION_LAYER_ALL };
    return c;
}

static inline Collider* Collider_makePlane(vec3s normal, float distance) {
    Collider*      c = (Collider*)     malloc(sizeof(Collider));
    PlaneCollider* p = (PlaneCollider*)malloc(sizeof(PlaneCollider));
    if (!c || !p) { free(c); free(p); return NULL; }
    p->Normal   = normal;
    p->Distance = distance;
    *c = (Collider){ COLLIDER_PLANE, p, false, COLLISION_LAYER_DEFAULT, COLLISION_LAYER_ALL };
    return c;
}

static inline void Collider_free(Collider* c) {
    if (!c) return;
    free(c->collider);
    free(c);
}

// Teste si deux objets peuvent entrer en collision (layer filter)
static inline bool Collider_canCollide(const Collider* a, const Collider* b) {
    return (a->category & b->mask) && (b->category & a->mask);
}

#endif // CHERRYENGINE_COLLIDERS_H
