#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <math.h>
#include <string.h>
#include <time.h>
#include <cglm/struct.h>

#include "cglm/cam.h"
#include "editor/core/shapes/shape.h"
#include "editor/ecs/componentPool.h"
#include "editor/ecs/gameObject.h"
#include "physics/physicsWorld.h"
#include "render/camera.h"
#include "render/shader.h"

static PhysicsWorld physics_world;

static bool firstMouse = true;
static Camera camera;
static float lastX, lastY;

PhysicsObject* physicsObject;

void processInput(Camera* camera, float deltaTime, GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    /*
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        Camera_processKeyboard(camera, FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        Camera_processKeyboard(camera, BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        Camera_processKeyboard(camera, LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        Camera_processKeyboard(camera, RIGHT, deltaTime);
    */
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        PhysicsWorld_addObject(&physics_world);
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
        PhysicsWorld_impulse(&physics_world, deltaTime, physicsObject->Transform.position, 70, 12);
}

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
    srand(time(NULL));

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

    Shape cube = Shape_create(SHAPE_CUBE);

    physics_world = PhysicsWorld_create();

    Shader shader;
    if (!Shader_load(&shader, "/home/killian/Projects/C/CherryEngine/resources/shaders/test.vs", "/home/killian/Projects/C/CherryEngine/resources/shaders/test.fs")) {
        LOG("Erreur de chargement des shaders");
    }

    vec3s lightDirection = {0.403945,0.868481,0.287348};

    physicsObject = PhysicsWorld_addObject(&physics_world);
    physicsObject->PhysicsTag = PLAYER;

    GameObject game_object = GameObject_Create();
    if (!GameObject_AddComponent(&game_object, &component_pool, COMPONENT_PLAYER_CONTROLLER)) {
        LOG("ERROR ADDING PLAYER CONTROLLER");
    }
    PlayerController* player_controller = GameObject_GetComponent(&game_object, &component_pool, COMPONENT_PLAYER_CONTROLLER);
    Component_PlayerController_Init(player_controller, physicsObject, &camera, window);


    float timeCheck = 0.0;
    int nbFrames = 0;
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.6f,0.6f,0.6f,0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (timeCheck >= 1.0) {
            char title[128];
            // Calcul du temps moyen par frame (ms) et des FPS
            double msPerFrame = 1000.0 / (double)nbFrames;
            sprintf(title, "CherryEngine - [FPS: %d | %.2f ms]", nbFrames, msPerFrame);

            // Mise à jour du titre de la fenêtre
            glfwSetWindowTitle(window, title);

            timeCheck = 0.0;
            nbFrames = 0;
        }

        // Update de la physique
        PhysicsWorld_step(&physics_world, deltaTime);

        // Ici on fait ce qu'on veut
        processInput(&camera, deltaTime, window);

        Component_PlayerController_Update(player_controller, &game_object, deltaTime);

        // Commun pour tout les models
        mat4s view = Camera_getViewMatrix(&camera);
        Shader_use(&shader);
        Shader_setVec3(&shader, "lightDirection", lightDirection);
        Shader_setMat4(&shader, "view", view);
        Shader_setMat4(&shader, "projection", perspective);

        // Affichage du model cible
        for (int i = 0; i < physics_world.numPhysicsObjects; i++) {
            mat4s tMatrix = glms_translate(glms_mat4_identity(), physics_world.physicsObjects[i].Transform.position);
            if (physics_world.physicsObjects[i].Collider->type == CUBE) {
                BoxCollider* col = physics_world.physicsObjects[i].Collider->collider;
                tMatrix = glms_scale(tMatrix, glms_vec3_scale(col->HalfSize,2.0));
            }
            Shader_setMat4(&shader, "model", tMatrix);
            if (physics_world.physicsObjects[i].PhysicsTag == PLAYER) {
                Shader_setBool(&shader, "player", true);
            }
            else {
                Shader_setBool(&shader, "player", false);

            }
            Shape_draw(&cube);
        }

        timeCheck += deltaTime;
        nbFrames++;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Ici on clean tout
    glfwDestroyWindow(window);

    return EXIT_SUCCESS;
}
