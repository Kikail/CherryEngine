//
// Created by killian on 3/11/26.
//
#include "componentPool.h"
#include "component.h"

ComponentPool ComponentPool_Create(){
    ComponentPool c;
    c.currentMeshRendererCount = 0;
    c.currentTransformCount = 0;
    c.currentSpriteRendererCount = 0;
    c.currentPlayerControllerCount = 0;
    return c;
}
bool ComponentPool_CheckSpace(ComponentPool* componentPool, ComponentType componentType){
    switch (componentType)
    {
        case COMPONENT_TRANSFORM:
            if(componentPool->currentTransformCount < MAX_COMPONENTS)return true;
            break;
        case COMPONENT_PLAYER_CONTROLLER:
            if(componentPool->currentPlayerControllerCount < MAX_COMPONENTS)return true;
            break;
        case COMPONENT_SPRITE_RENDERER:
            if(componentPool->currentSpriteRendererCount < MAX_COMPONENTS)return true;
            break;
        case COMPONENT_MESH_RENDERER:
            if(componentPool->currentMeshRendererCount < MAX_COMPONENTS)return true;
            break;
    }
    #ifdef DEBUG
        DEBUG_LOG("ComponentPool_CheckSpace::Component not found");
    #endif
    return false;
}
bool ComponentPool_CreateComponent(ComponentPool* componentPool, ComponentType componentType, uint32* index){
    // On check si il y a encore de la place sinon on skip
    if(!ComponentPool_CheckSpace(componentPool, componentType)){
        #ifdef DEBUG
            DEBUG_LOG("ComponentPool_CreateComponent::No more space available");
        #endif
        return false;
    }
    switch (componentType)
    {
        case COMPONENT_TRANSFORM:
            *index = componentPool->currentTransformCount;
            componentPool->currentTransformCount += 1;
            break;
        case COMPONENT_PLAYER_CONTROLLER:
            *index = componentPool->currentPlayerControllerCount;
            componentPool->currentPlayerControllerCount += 1;
            break;
        case COMPONENT_SPRITE_RENDERER:
            *index = componentPool->currentSpriteRendererCount;
            componentPool->currentSpriteRendererCount += 1;
            break;
        case COMPONENT_MESH_RENDERER:
            *index = componentPool->currentMeshRendererCount;
            componentPool->currentMeshRendererCount += 1;
            break;
    }

    return true;
}
void* ComponentPool_GetComponent(ComponentPool* componentPool, ComponentType componentType, uint32 index){
    if(index >= MAX_COMPONENTS){
        #ifdef DEBUG
            DEBUG_LOG("ComponentPool_GetComponent::Index is over de max value");
        #endif
        return NULL;
    }
    switch (componentType)
    {
        case COMPONENT_TRANSFORM:
            return &componentPool->transforms[index];
            break;
        case COMPONENT_PLAYER_CONTROLLER:
            return &componentPool->playerControllers[index];
            break;
        case COMPONENT_SPRITE_RENDERER:
            return &componentPool->spriteRenderers[index];
            break;
        case COMPONENT_MESH_RENDERER:
            return &componentPool->meshRenderers[index];
            break;
    }

    #ifdef DEBUG
        DEBUG_LOG("ComponentPool_GetComponent::Component not found");
    #endif

    return NULL;
}

void ComponentPool_UpdateComponent(ComponentPool* componentPool, ComponentType componentType, void* component, GameObject* gameObject, float deltaTime){
    if(component == NULL){
#ifdef DEBUG
        DEBUG_LOG("ComponentPool_UpdateComponent::Component is NULL");
#endif
        return;
    }

    switch (componentType)
    {
        case COMPONENT_TRANSFORM:
            // Pas de Update pour Transform
            break;
        case COMPONENT_PLAYER_CONTROLLER:
            Component_PlayerController_Update(component, gameObject, deltaTime);
            break;
        case COMPONENT_SPRITE_RENDERER:
            Component_SpriteRenderer_Update(component, gameObject);
            break;
        case COMPONENT_MESH_RENDERER:
            Component_MeshRenderer_Update(component, gameObject);
            break;
    }
}