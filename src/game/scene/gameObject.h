//
// Created by killian on 3/11/26.
//

#ifndef CHERRYENGINE_GAMEOBJECT_H
#define CHERRYENGINE_GAMEOBJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../../utils/types.h"
#include "../ecs/component.h"

typedef struct ComponentPool_t ComponentPool;

// Creation du type GameObject
#define MAX_COMPONENT_PER_OBJECT 16
typedef struct GameObject_t {
    uint32 component_mask;
    Component components[MAX_COMPONENT_PER_OBJECT]; uint32 componentCount;
    char* name;

    // un id fonctionne de la sorte 10000000 00000000 00000000 00000000
    // 1 au debut si gameObject 0 sinon
    unsigned int id;
} GameObject;

GameObject GameObject_Create(char* name);
bool GameObject_AddComponent(GameObject* gameObject, ComponentPool* componentPool, ComponentType componentType);
bool GameObject_HasComponent(GameObject* gameObject, ComponentType componentType);
void* GameObject_GetComponent(GameObject* gameObject, ComponentPool* componentPool, ComponentType componentType);
void GameObject_updateComponents(GameObject* gameObject, ComponentPool* componentPool, float deltaTime);

#endif //CHERRYENGINE_GAMEOBJECT_H