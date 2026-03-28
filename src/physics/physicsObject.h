//
// Created by killian on 3/18/26.
// physicsObject.h

#ifndef CHERRYENGINE_PHYSICSOBJECT_H
#define CHERRYENGINE_PHYSICSOBJECT_H

#include <cglm/struct.h>

#include "colliders.h"
#include "../editor/ecs/components/transform.h"

typedef enum PhysicsTag_t {
    ENVIRONMENT,
    PLAYER
}PhysicsTag;

typedef enum PhysicsType_t {
    DYNAMIC,
    STATIC
}PhysicsType;

typedef struct {
    Transform Transform;
    Collider* Collider;

    // Linéaire
    vec3s Velocity;
    vec3s Force;
    float Mass;
    float InverseMass;

    PhysicsType PhysicsType;
    PhysicsTag PhysicsTag;

    // --- NOUVEAU : ANGULAIRE ---
    versors Orientation;      // Quaternion (cglm : versors)
    vec3s AngularVelocity;    // Vitesse de rotation (rad/s)
    vec3s Torque;             // Forces de rotation
    mat3s InertiaTensorInv;   // Résistance à la rotation (Inverse)
} PhysicsObject;


#endif //CHERRYENGINE_PHYSICSOBJECT_H