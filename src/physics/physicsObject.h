//
// Created by killian on 3/18/26.
// physicsObject.h

#ifndef CHERRYENGINE_PHYSICSOBJECT_H
#define CHERRYENGINE_PHYSICSOBJECT_H

#include <cglm/struct.h>

#include "colliders.h"
#include "../editor/ecs/components/transform.h"

typedef enum PhysicsType_t {
    DYNAMIC,
    STATIC
}PhysicsType;

typedef enum PhysicsTag_t {
    BASE,
    PLAYER
}PhysicsTag;

typedef struct PhysicsObject_t {
    vec3s Velocity;
    vec3s Force;
    float Mass;

    Collider* Collider;
    Transform Transform;

    PhysicsType PhysicsType;
    PhysicsTag PhysicsTag;
}PhysicsObject;


#endif //CHERRYENGINE_PHYSICSOBJECT_H