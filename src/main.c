#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <math.h>

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



    // --------- GLFW init ---------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "CherryEngine", NULL, NULL);
    if (window == NULL)
    {
        LOG("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, NULL);
    // --------- GLAD init ---------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LOG("Erreur de Init Glad");
        return false;
    }
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(sin(glfwGetTime()), cos(glfwGetTime()), -cos(glfwGetTime()), 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Ici on fait ce qu'on veut

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Ici on clean tout
    glfwDestroyWindow(window);

    return EXIT_SUCCESS;
}