//
// Created by killian on 3/11/26.
//

#ifndef CHERRYENGINE_COMPONENTPOOL_H
#define CHERRYENGINE_COMPONENTPOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../../utils/types.h"

#include "component.h"
#include "components/transform.h"
#include "components/meshRenderer.h"
#include "components/spriteRenderer.h"
#include "components/playerController.h"
#include "resource/serializer.h"

typedef struct GameObject_t GameObject;
typedef enum ComponentType_t ComponentType;

// Creation du stock de components
// Ici nous stockons tout les composants de la scene
#define MAX_COMPONENTS 1000
typedef struct ComponentPool_t {
    Transform transforms[MAX_COMPONENTS]; uint32 currentTransformCount;
    MeshRenderer meshRenderers[MAX_COMPONENTS]; uint32 currentMeshRendererCount;
    SpriteRenderer spriteRenderers[MAX_COMPONENTS]; uint32 currentSpriteRendererCount;
    PlayerController playerControllers[MAX_COMPONENTS]; uint32 currentPlayerControllerCount;
} ComponentPool;

ComponentPool ComponentPool_Create();
bool ComponentPool_CheckSpace(ComponentPool* componentPool, ComponentType componentType);
bool ComponentPool_CreateComponent(ComponentPool* componentPool, ComponentType componentType, Component* component);
void* ComponentPool_GetComponent(ComponentPool* componentPool, ComponentType componentType, uint32 index);
void ComponentPool_UpdateComponent(ComponentPool* componentPool, ComponentType componentType, void* component, GameObject* gameObject, float deltaTime);

SerialObject ComponentPool_serializeComponent(ComponentPool* componentPool, ComponentType componentType, uint32 index);

#endif //CHERRYENGINE_COMPONENTPOOL_H