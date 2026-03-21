//
// Created by killian on 3/19/26.
// colliders.h

#ifndef CHERRYENGINE_COLLIDERS_H
#define CHERRYENGINE_COLLIDERS_H

#include "../editor/ecs/components/transform.h"

typedef enum ColliderType_t {
    PLANE,
    SPHERE,
    CUBE
}ColliderType;

typedef struct SphereCollider_t {
    vec3s Center;
    float Radius;
}SphereCollider;

typedef struct PlaneCollider_t {
    vec3s Normal;
    float Distance;
}PlaneCollider;

typedef struct BoxCollider_t {
    vec3s HalfSize;
}BoxCollider;

typedef struct Collider_t {
    ColliderType type;
    void* collider;
}Collider;

typedef struct CollisionPoints_t {
    vec3s A;
    vec3s B;
    vec3s Normal;
    float Depth;
    bool HasCollision;
}CollisionPoints;

#endif //CHERRYENGINE_COLLIDERS_H