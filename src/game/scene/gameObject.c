//
// Created by killian on 3/11/26.
//
#include "../scene/gameObject.h"
#include "../ecs/componentPool.h"

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

    // On lie le component du componentPool avec le component du GameObject
    gameObject->components[gameObject->componentCount].component_type = componentType;
    if(!ComponentPool_CreateComponent(componentPool, componentType, &gameObject->components[gameObject->componentCount].component_adress)){
        DEBUG_LOG("Erreur de creation de component dans le ComponentPool !\n");
        return false;
    }

    // On oublie pas d'augmenter le nombre de components
    gameObject->componentCount += 1;

    return true;
}
bool GameObject_HasComponent(GameObject* gameObject, ComponentType componentType){
    return (gameObject->component_mask & componentType) == componentType;
}
void* GameObject_GetComponent(GameObject* gameObject, ComponentPool* componentPool, ComponentType componentType){
    if(!GameObject_HasComponent(gameObject, componentType))return NULL;
    for(int i = 0; i < gameObject->componentCount; i++){
        if(gameObject->components[i].component_type == componentType){
            return ComponentPool_GetComponent(componentPool, componentType, gameObject->components[i].component_adress);
        }
    }
    return NULL;
}
void GameObject_updateComponents(GameObject* gameObject, ComponentPool* componentPool, float deltaTime) {
    for(int i = 0; i < gameObject->componentCount; i++){
        ComponentPool_UpdateComponent(componentPool, gameObject->components[i].component_type, GameObject_GetComponent(gameObject, componentPool, gameObject->components[i].component_type), gameObject, deltaTime);
    }
}