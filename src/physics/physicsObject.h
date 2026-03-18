//
// Created by killian on 3/18/26.
//

#ifndef CHERRYENGINE_PHYSICSOBJECT_H
#define CHERRYENGINE_PHYSICSOBJECT_H

#include <cglm/struct.h>

typedef struct PhysicsObject_t {
    vec3s Position;
    vec3s Velocity;
    vec3s Force;
    float Mass;
}PhysicsObject;


#endif //CHERRYENGINE_PHYSICSOBJECT_H