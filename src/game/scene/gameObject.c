//
// Created by killian on 3/11/26.
//
#include "gameObject.h"
#include "../ecs/componentPool.h"
#include "utils/idMaker.h"
#include "game.h"

GameObject GameObject_Create(char* name){
    GameObject gameObject;
    gameObject.component_mask = 0;
    gameObject.componentCount = 0;
    gameObject.name = name;
    return gameObject;
}
bool GameObject_AddComponent(GameObject* gameObject, ComponentPool* componentPool, ComponentType componentType){
    // comparaison bit a bit pour savoir si le gameObject possede deja un component de ce type
    if((gameObject->component_mask & componentType) == componentType){
    #ifdef DEBUG
            DEBUG_LOG("GameObject_AddComponent::GameObject already have a component of this type");
    #endif
        return false;
    }
    // On ajoute le bit correspondant a ce type de component
    gameObject->component_mask |= componentType;

    if (Id_createId(&gameObject->components[gameObject->componentCount].id)) {
        gameObject->components[gameObject->componentCount].parentId = gameObject->id;
    }
    else {
        #ifdef DEBUG
                DEBUG_LOG("GAMEOEBJECT::GameObject_AddComponent max id reached");
        #endif
    }

    // On lie le component du componentPool avec le component du GameObject
    gameObject->components[gameObject->componentCount].component_type = componentType;
    if(!ComponentPool_CreateComponent(componentPool, componentType, &gameObject->components[gameObject->componentCount])){
        DEBUG_LOG("Erreur de creation de component dans le ComponentPool !\n");
        return false;
    }

    // On oublie pas d'augmenter le nombre de components
    gameObject->componentCount += 1;

    return true;
}
bool GameObject_AddComponentWithoutAddingComponentPool(GameObject* gameObject, ComponentPool* componentPool, ComponentType componentType) {
    // comparaison bit a bit pour savoir si le gameObject possede deja un component de ce type
    if((gameObject->component_mask & componentType) == componentType){
#ifdef DEBUG
        DEBUG_LOG("GameObject_AddComponent::GameObject already have a component of this type");
#endif
        return false;
    }
    // On ajoute le bit correspondant a ce type de component
    gameObject->component_mask |= componentType;

    // On lie le component du componentPool avec le component du GameObject
    gameObject->components[gameObject->componentCount].component_type = componentType;

    // On oublie pas d'augmenter le nombre de components
    gameObject->componentCount += 1;

    return true;
}
bool GameObject_HasComponent(GameObject* gameObject, ComponentType componentType){
    #ifdef DEBUG
        if (gameObject == NULL)
            DEBUG_LOG("GAMEOBJECT::GameObject_HasComponent gameObject is NULL");
    #endif
    return (gameObject->component_mask & componentType) == componentType;
}
void* GameObject_GetComponent(GameObject* gameObject, ComponentPool* componentPool, ComponentType componentType){
    #ifdef DEBUG
        if (gameObject == NULL)
            DEBUG_LOG("GAMEOBJECT::GameObject_GetComponent gameObject is NULL");
    #endif
    if(!GameObject_HasComponent(gameObject, componentType))return NULL;
    for(int i = 0; i < gameObject->componentCount; i++){
        if(gameObject->components[i].component_type == componentType){
            return ComponentPool_GetComponent(componentPool, componentType, gameObject->components[i].component_adress);
        }
    }
    #ifdef DEBUG
        DEBUG_LOG("GAMEOBJECT::GameObject_GetComponent no component of this type");
    #endif
        return NULL;
}
void GameObject_updateComponents(GameObject* gameObject, ComponentPool* componentPool, float deltaTime, Game* game) {
    #ifdef DEBUG
        if (gameObject == NULL || componentPool == NULL)
            DEBUG_LOG("GAMEOBJECT::GameObject_updateComponents gameObject or componentPool is NULL");
    #endif
    for(int i = 0; i < gameObject->componentCount; i++){
        ComponentPool_UpdateComponent(componentPool, gameObject->components[i].component_type, GameObject_GetComponent(gameObject, componentPool, gameObject->components[i].component_type), gameObject, deltaTime, game);
    }
}