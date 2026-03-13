#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "editor/ecs/componentPool.h"
#include "editor/ecs/gameObject.h"

int main(int argc, char** argv){
    ComponentPool componentPool = ComponentPool_Create();

    GameObject gameObject = GameObject_Create();
    if(!GameObject_AddComponent(&gameObject, &componentPool, COMPONENT_TRANSFORM)){
        LOG("erreur de AddComponent");
    }
    if(!GameObject_AddComponent(&gameObject, &componentPool, COMPONENT_SPRITE_RENDERER)){
        LOG("erreur de AddComponent");
    }
    Transform* transform = GameObject_GetComponent(&gameObject, &componentPool, COMPONENT_TRANSFORM);
    transform->position.x = 100;
    Transform_Afficher(*transform);

    SpriteRenderer* spriteRenderer = GameObject_GetComponent(&gameObject, &componentPool, COMPONENT_SPRITE_RENDERER);
    spriteRenderer->imagePath = "bonjour monde";

    SpriteRenderer* s = GameObject_GetComponent(&gameObject, &componentPool, COMPONENT_SPRITE_RENDERER);
    LOG(s->imagePath);
    isValid(s);

    for(int i = 0; i < gameObject.componentCount; i++){
        ComponentPool_UpdateComponent(
            &componentPool,
            gameObject.components[i].component_type,
            GameObject_GetComponent(&gameObject, &componentPool, gameObject.components[i].component_type),
            &gameObject
        );
    }

    return EXIT_SUCCESS;
}