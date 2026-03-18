#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <math.h>
#include <string.h>
#include <cglm/struct.h>

#include "cglm/cam.h"
#include "editor/core/shapes/shape.h"
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

static bool firstMouse = true;
static Camera camera;
static float lastX, lastY;

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = (float)(xposIn);
    float ypos = (float)(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    Camera_processMouseMovement(&camera, xoffset, yoffset);
}

#define WIDTH 1280.0f
#define HEIGHT 720.0f

int main(int argc, char** argv){
    ComponentPool component_pool = ComponentPool_Create();

    // Creation d'une camera
    camera = Camera_createCamera(glms_vec3_zero(),(vec3s)VECTOR_UP,(vec3s)VECTOR_FRONT,CAMERA_YAW, CAMERA_PITCH,CAMERA_SPEED,CAMERA_SENSIVITY,CAMERA_ZOOM);

    // Chargement du resource path du projet
    //char* path = RESOURCES_PATH;
    //strcat(path,"/");

    // Typiquement dans ton main.c ou ta classe Camera
    float aspect = WIDTH / HEIGHT; // Largeur / Hauteur
    float fov = glm_rad(45.0f);     // Champ de vision de 45 degrés converti en radians
    mat4s perspective = glms_perspective(fov, aspect, 0.1f, 100.0f);

    // --------- GLFW init ---------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "CherryEngine", NULL, NULL);
    if (window == NULL)
    {
        LOG("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowUserPointer(window, NULL);
    glfwSetCursorPosCallback(window, mouse_callback);
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

    Shape cube = Shape_create(CUBE);

    Shader shader;
    if (!Shader_load(&shader, "/home/killian/Projects/C/CherryEngine/resources/shaders/test.vs", "/home/killian/Projects/C/CherryEngine/resources/shaders/test.fs")) {
        LOG("Erreur de chargement des shaders");
    }

    GameObject gameObject = GameObject_Create();
    GameObject_AddComponent(&gameObject, &component_pool, COMPONENT_TRANSFORM);
    Transform* transform = GameObject_GetComponent(&gameObject, &component_pool, COMPONENT_TRANSFORM);
    Transform_setScale(transform, (vec3s){ 1.0f, 1.0f, 1.0f });
    Transform_setPosition(transform, (vec3s){ 0.0f, 0.0f, -5.0f });

    GameObject gameObject2 = GameObject_Create();
    GameObject_AddComponent(&gameObject2, &component_pool, COMPONENT_TRANSFORM);
    Transform* transform2 = GameObject_GetComponent(&gameObject2, &component_pool, COMPONENT_TRANSFORM);
    Transform_setParent(transform2, transform, KEEP_SCALE);
    Transform_setScale(transform2, (vec3s){ 1.0f, 1.0f, 1.0f });
    Transform_setPosition(transform2, (vec3s){ 0.0f, -2.0f, 0.0f });

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.6f,0.6f,0.6f,0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Ici on fait ce qu'on veut
        processInput(&camera, deltaTime, window);

        Transform_setPosition(transform, (vec3s){ 0.0f, sin(currentFrame), -5.0f });
        // Faire pivoter de 90 degrés sur l'axe Y
        float angle = glm_rad(1.0);
        versors rotation = glms_quat(angle, 0.5, 0.0, 1.0);
        // Appliquer la rotation
        Transform_rotate(transform, rotation, true);

        mat4s view = Camera_getViewMatrix(&camera);
        mat4s model = Transform_getWorldMatrix(transform);
        Shader_use(&shader);
        Shader_setMat4(&shader, "model", model);
        Shader_setMat4(&shader, "view", view);
        Shader_setMat4(&shader, "projection", perspective);
        Shape_draw(&cube);

        model = Transform_getWorldMatrix(transform2);
        Shader_use(&shader);
        Shader_setMat4(&shader, "model", model);
        Shader_setMat4(&shader, "view", view);
        Shader_setMat4(&shader, "projection", perspective);
        Shape_draw(&cube);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Ici on clean tout
    glfwDestroyWindow(window);

    return EXIT_SUCCESS;
}
