#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <math.h>
#include <cglm/struct.h>

#include "cglm/cam.h"
#include "editor/core/shapes/shape.h"
#include "editor/ecs/componentPool.h"
#include "editor/ecs/gameObject.h"
#include "physics/physicsWorld.h"
#include "render/camera.h"
#include "render/model.h"
#include "render/shader.h"

static PhysicsWorld* physics_world;

static bool firstMouse = true;
static Camera camera;
static float lastX, lastY;

static PhysicsObject* physicsObject = NULL;
int nbPhysicsObjects = 0;

static Shader shader;
static bool onceKey = true;
static bool onceMouse = true;
static bool captureMouse = true;

#define WIDTH  1920
#define HEIGHT 1080

// VBO pour stocker les matrices d'instanciation
static GLuint instanceVBO;

static char* GetPath(const char* file) {
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

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        PhysicsWorld_explosion(physics_world, physicsObject->Transform.position, 30, 30);
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
    mat4s perspective = glms_perspective(fov, aspect, 0.1f, 300.0f);

    // GLFW init
    if (!glfwInit()) {
        LOG("Failed to initialize GLFW");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // CORE_PROFILE est préférable ici

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

    physics_world = PhysicsWorld_create();

    if (!Shader_load(&shader,
                GetPath("shaders/model.vs"),
                GetPath("shaders/model.fs") )) {
        LOG("Erreur de chargement des shaders");
    }

    vec3s lightDirection = {0.403945f, 0.868481f, -0.287348f};

    physicsObject = PhysicsWorld_addObject(physics_world);
    if (!physicsObject) {
        LOG("Impossible de creer l'objet physique joueur");
        cleanup(window);
        return EXIT_FAILURE;
    }
    physicsObject->PhysicsTag = PLAYER;

    Shader debugShader;
    if (!Shader_load(&debugShader,
                GetPath("shaders/debug.vs"),
                GetPath("shaders/debug.fs") )) {
        LOG("Erreur de chargement des shaders");
    }

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

    // =========================================================================
    // CONFIGURATION DE L'INSTANCIATION POUR LE MODELE
    // =========================================================================
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    // Pré-allocation pour 10000 matrices
    glBufferData(GL_ARRAY_BUFFER, PHYSICS_MAX_OBJECTS * sizeof(mat4s), NULL, GL_DYNAMIC_DRAW);

    // On configure le VAO de CHAQUE Mesh du modèle
    for (unsigned int i = 0; i < model.numMeshes; i++) {
        glBindVertexArray(model.meshes[i].VAO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

        // Les attributs 0 à 4 sont déjà pris par ton Vertex struct
        // La matrice mat4 utilisera les locations 5, 6, 7 et 8
        int startLocation = 5;
        for (int j = 0; j < 4; j++) {
            glEnableVertexAttribArray(startLocation + j);
            glVertexAttribPointer(startLocation + j, 4, GL_FLOAT, GL_FALSE, sizeof(mat4s), (void*)(j * sizeof(vec4s)));
            glVertexAttribDivisor(startLocation + j, 1); // IMPORTANT : Avance par instance
        }
        glBindVertexArray(0);
    }
    // =========================================================================

    float timeCheck = 0.0f;
    int nbFrames = 0;

    for (int i = 0; i < PHYSICS_MAX_OBJECTS; i++) { // J'ai baissé à 2000 pour la demo, ajustable.
        PhysicsObject* obj = PhysicsWorld_addObject(physics_world);
        nbPhysicsObjects += 1;
    }

    mat4s* modelMatrices = malloc(PHYSICS_MAX_OBJECTS * sizeof(mat4s)); // Allocation sécurisée sur la Heap
    int instanceCount = 0;

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f); // Légèrement coloré pour bien voir les cubes
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (timeCheck >= 1.0f) {
            char title[128];
            double msPerFrame = (nbFrames > 0) ? (1000.0 / (double)nbFrames) : 0.0;
            sprintf(title, "CherryEngine - [FPS: %d | %.2f ms | Objects: %d]", nbFrames, msPerFrame, instanceCount);
            glfwSetWindowTitle(window, title);

            timeCheck = 0.0f;
            nbFrames = 0;
        }

        mat4s view = Camera_getViewMatrix(&camera);

        // Ce bloc calcule la physique
        PhysicsWorld_step(physics_world, deltaTime);

        processInput(&camera, deltaTime, window);

        if (player_controller) {
            Component_PlayerController_Update(player_controller, &game_object, deltaTime);
        }

        instanceCount = 0;
        for (int i = 0; i < physics_world->numPhysicsObjects; i++) {
            PhysicsObject* obj = &physics_world->physicsObjects[i];

            if (!obj->Collider) continue;
            if (obj->PhysicsTag == PLAYER) continue;

            if (obj->Collider->type == CUBE && obj->PhysicsType == DYNAMIC) {

                // --- CALCUL CORRECT DE LA MATRICE (T * R * S) ---
                mat4s matrice = glms_mat4_identity();

                // 1. Translation
                matrice = glms_translate(matrice, obj->Transform.position);

                // 3. Scale (Echelle du modèle)
                matrice = glms_scale(matrice, (vec3s){0.05f, 0.05f, 0.05f});

                modelMatrices[instanceCount] = matrice;
                instanceCount++;
                if (instanceCount >= PHYSICS_MAX_OBJECTS) break;
            }
        }

        // 2. Mise à jour du buffer sur le GPU
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, instanceCount * sizeof(mat4s), modelMatrices);

        // 3. Rendu instancié pour tous les meshes composant le modèle
        Shader_use(&shader);
        Shader_setMat4(&shader, "projection", perspective);
        Shader_setMat4(&shader, "view", view);

        // CORRECTION : Envoi de la lumière
        Shader_setVec3(&shader, "lightDirection", lightDirection);
        Shader_setInt(&shader, "texture_diffuse1", 0); // Spécifier l'unité de texture

        for (unsigned int i = 0; i < model.numMeshes; i++) {

            // Lier la texture du modèle s'il en a une
            if(model.meshes[i].nbTextures > 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, model.meshes[i].textures[0].id);
            }

            glBindVertexArray(model.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, model.meshes[i].nbIndices, GL_UNSIGNED_INT, 0, instanceCount);
        }
        glBindVertexArray(0);

        timeCheck += deltaTime;
        nbFrames++;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free(modelMatrices); // Ne pas oublier de free !
    cleanup(window);
    return EXIT_SUCCESS;
}