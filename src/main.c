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
#include "render/model.h"
#include "render/shader.h"

static PhysicsWorld physics_world;

static bool firstMouse = true;
static Camera camera;
static float lastX, lastY;

static PhysicsObject* physicsObject = NULL;
int nbPhysicsObjects = 0;

static Shader shader;
static bool onceKey = true;
static bool onceMouse = true;
static bool captureMouse = true;

#define WIDTH  1280
#define HEIGHT 720

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char* GetPath(const char* file) {
    // Calculer la taille nécessaire pour le chemin final
    size_t pathLen = strlen(RESOURCES_PATH) + 1 + strlen(file) + 1;
    char* finalPath = malloc(pathLen);
    if (finalPath == NULL) {
        perror("Erreur d'allocation mémoire");
        return NULL;
    }
    snprintf(finalPath, pathLen, "%s/%s", RESOURCES_PATH, file);
    return finalPath;
}

static void cleanup(GLFWwindow* window)
{
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

void processInput(Camera* camera, float deltaTime, GLFWwindow* window)
{
    (void)camera;
    (void)deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        PhysicsObject* obj = PhysicsWorld_addObject(&physics_world);
        nbPhysicsObjects += 1;
        printf("%d\n", nbPhysicsObjects);
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        PhysicsWorld_explosion(&physics_world, physicsObject->Transform.position, 10, 10);
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        if (!onceMouse) return;
        onceMouse = false;

        captureMouse = !captureMouse;
        glfwSetInputMode(window, GLFW_CURSOR,
            captureMouse ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

        LOG(captureMouse ? "capture true" : "capture false");
    } else {
        onceMouse = true;
    }

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        if (!onceKey) return;
        onceKey = false;

        Shader loaded;
        LOG("CHARGEMENT DES SHADERS...");
        if (!Shader_load(&loaded,
                GetPath("shaders/test.vs"),
                GetPath("shaders/test.fs") )) {
            LOG("Erreur de chargement des shaders");
        } else {
            shader = loaded;
        }
    } else {
        onceKey = true;
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    (void)window;

    float xpos = (float)xposIn;
    float ypos = (float)yposIn;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    if (captureMouse) {
        Camera_processMouseMovement(&camera, xoffset, yoffset);
    }
}

int main(int argc, char** argv)
{
    srand((unsigned int)time(NULL));

    ComponentPool component_pool = ComponentPool_Create();

    // Caméra
    camera = Camera_createCamera(
        glms_vec3_zero(),
        (vec3s)VECTOR_UP,
        (vec3s)VECTOR_FRONT,
        CAMERA_YAW,
        CAMERA_PITCH,
        CAMERA_SPEED,
        CAMERA_SENSIVITY,
        CAMERA_ZOOM
    );

    float aspect = (float)WIDTH / (float)HEIGHT;
    float fov = glm_rad(45.0f);
    mat4s perspective = glms_perspective(fov, aspect, 0.1f, 100.0f);

    // GLFW init
    if (!glfwInit()) {
        LOG("Failed to initialize GLFW");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "CherryEngine", NULL, NULL);
    if (window == NULL) {
        LOG("Failed to create GLFW window");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG("Erreur de Init Glad");
        cleanup(window);
        return EXIT_FAILURE;
    }

    glEnable(GL_DEPTH_TEST);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float currentFrame = 0.0f;

    Shape cube = Shape_create(SHAPE_CUBE);

    // CORRECTION : Initialisation par pointeur sur la variable globale
    physics_world = PhysicsWorld_create();

    printf("%s\n",GetPath("shaders/model.vs"));

    if (!Shader_load(&shader,
                GetPath("shaders/model.vs"),
                GetPath("shaders/model.fs") )) {
        LOG("Erreur de chargement des shaders");
    }

    Shader cubeShader;
    if (!Shader_load(&cubeShader,
                GetPath("shaders/test.vs"),
                GetPath("shaders/test.fs") )) {
        LOG("Erreur de chargement des shaders");
    }

    vec3s lightDirection = {0.403945f, 0.868481f, -0.287348f};

    physicsObject = PhysicsWorld_addObject(&physics_world);
    if (!physicsObject) {
        LOG("Impossible de creer l'objet physique joueur");
        cleanup(window);
        return EXIT_FAILURE;
    }
    physicsObject->PhysicsTag = PLAYER;

    GameObject game_object = GameObject_Create();
    if (!GameObject_AddComponent(&game_object, &component_pool, COMPONENT_PLAYER_CONTROLLER)) {
        LOG("ERROR ADDING PLAYER CONTROLLER");
        cleanup(window);
        return EXIT_FAILURE;
    }

    PlayerController* player_controller =
        GameObject_GetComponent(&game_object, &component_pool, COMPONENT_PLAYER_CONTROLLER);

    if (!player_controller) {
        LOG("PlayerController introuvable");
        cleanup(window);
        return EXIT_FAILURE;
    }

    Component_PlayerController_Init(player_controller, physicsObject, &camera, window);

    Model model = Model_create(
        GetPath("models/test.obj"),
        false
    );

    float timeCheck = 0.0f;
    int nbFrames = 0;

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.6f, 0.6f, 0.6f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (timeCheck >= 1.0f) {
            char title[128];
            double msPerFrame = (nbFrames > 0) ? (1000.0 / (double)nbFrames) : 0.0;
            sprintf(title, "CherryEngine - [FPS: %d | %.2f ms]", nbFrames, msPerFrame);
            glfwSetWindowTitle(window, title);

            timeCheck = 0.0f;
            nbFrames = 0;
        }

        PhysicsWorld_step(&physics_world, deltaTime);
        processInput(&camera, deltaTime, window);

        if (player_controller) {
            Component_PlayerController_Update(player_controller, &game_object, deltaTime);
        }

        mat4s view = Camera_getViewMatrix(&camera);

        for (int i = 0; i < physics_world.numPhysicsObjects; i++) {
            PhysicsObject* obj = &physics_world.physicsObjects[i];
            if (!obj->Collider) continue;

            mat4s translation = glms_translate(glms_mat4_identity(), obj->Transform.position);


            mat4s scale = glms_mat4_identity();
            if (obj->Collider->type == CUBE) {
                BoxCollider* col = (BoxCollider*)obj->Collider->collider;
                if (col) {
                    float scaleFactor = (obj->PhysicsType == STATIC) ? 2.0f : 0.1f;
                    scale = glms_scale(glms_mat4_identity(),
                        glms_vec3_scale(col->HalfSize, scaleFactor));
                }
            }

            mat4s modelMatrix = glms_translate(glms_mat4_identity(), physics_world.physicsObjects[i].Transform.position);

            BoxCollider* col = (BoxCollider*)obj->Collider->collider;

            if (obj->PhysicsType == STATIC) {
                modelMatrix = glms_scale(modelMatrix, glms_vec3_scale(col->HalfSize, 2.0));
                Shader_use(&cubeShader);
                Shader_setMat4(&cubeShader, "projection", perspective);
                Shader_setMat4(&cubeShader, "view", view);
                Shader_setMat4(&cubeShader, "model", modelMatrix);
                Shader_setVec3(&cubeShader, "lightDirection", lightDirection);
                Shape_draw(&cube);
            } else {
                /*
                if (obj->PhysicsTag == PLAYER)continue;
                float scale = 0.05f;
                modelMatrix = glms_scale(modelMatrix, (vec3s){scale,scale,scale});
                Shader_use(&shader);
                Shader_setMat4(&shader, "projection", perspective);
                Shader_setMat4(&shader, "view", view);
                Shader_setMat4(&shader, "model", modelMatrix);
                Shader_setVec3(&shader, "lightDirection", lightDirection);
                Model_Draw(&model, &shader);
                */
            }
        }

        timeCheck += deltaTime;
        nbFrames++;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    cleanup(window);
    return EXIT_SUCCESS;
}