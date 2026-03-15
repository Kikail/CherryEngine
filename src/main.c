#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <math.h>
#include <string.h>
#include <cglm/struct.h>

#include "cglm/cam.h"
#include "editor/ecs/componentPool.h"
#include "editor/ecs/gameObject.h"
#include "render/camera.h"
#include "render/shader.h"

void processInput(Camera* camera, float deltaTime, GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        Camera_processKeyboard(camera, FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        Camera_processKeyboard(camera, BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        Camera_processKeyboard(camera, LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        Camera_processKeyboard(camera, RIGHT, deltaTime);
}

int main(int argc, char** argv){
    ComponentPool component_pool = ComponentPool_Create();

    // Creation d'une camera
    Camera camera = Camera_createCamera(glms_vec3_zero(),(vec3s)VECTOR_UP,(vec3s)VECTOR_FRONT,CAMERA_YAW, CAMERA_PITCH,CAMERA_SPEED,CAMERA_SENSIVITY,CAMERA_ZOOM);

    // Chargement du resource path du projet
    //char* path = RESOURCES_PATH;
    //strcat(path,"/");

    // Typiquement dans ton main.c ou ta classe Camera
    float aspect = 800.0f / 600.0f; // Largeur / Hauteur
    float fov = glm_rad(45.0f);     // Champ de vision de 45 degrés converti en radians
    mat4s perspective = glms_perspective(fov, aspect, 0.1f, 100.0f);

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

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float currentFrame = 0.0f;

    long int taille = sizeof(Transform)*MAX_COMPONENTS + sizeof(SpriteRenderer)*MAX_COMPONENTS + sizeof(MeshRenderer)*MAX_COMPONENTS + sizeof(PlayerController)*MAX_COMPONENTS;
    printf("on alloue : %ld octets\n",taille);

    Shader shader;
    Shader_load("/home/killian/Projects/C/CherryEngine/resources/shaders/test.vs", "/home/killian/Projects/C/CherryEngine/resources/shaders/test.fs");

    while (!glfwWindowShouldClose(window)) {
        glClearColor(sin(glfwGetTime()), cos(glfwGetTime()), -cos(glfwGetTime()), 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Ici on fait ce qu'on veut
        processInput(&camera, deltaTime, window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Ici on clean tout
    glfwDestroyWindow(window);

    return EXIT_SUCCESS;
}
