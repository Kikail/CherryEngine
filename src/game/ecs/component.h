//
// Created by killian on 3/11/26.
//

#ifndef CHERRYENGINE_COMPONENT_H
#define CHERRYENGINE_COMPONENT_H

#include "utils/types.h"

// On creer le bitmask pour chaque component
// 32 Components maximum !! car uint32
typedef enum ComponentType_t{
    COMPONENT_TRANSFORM             = 1 << 0,
    COMPONENT_MESH_RENDERER         = 1 << 1,
    COMPONENT_SPRITE_RENDERER       = 1 << 2,
    COMPONENT_PLAYER_CONTROLLER     = 1 << 3
}ComponentType;

// Creation du component type
typedef struct{
    uint32 component_adress;
    ComponentType component_type;
} Component;

#endif //CHERRYENGINE_COMPONENT_H