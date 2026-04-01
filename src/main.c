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
#include "render/instanceMesh.h"
#include "render/model.h"
#include "render/shader.h"

#define WIDTH  1280
#define HEIGHT 720

// ==========================================
// VARIABLES GLOBALES (Caméra Orbitale)
// ==========================================
static Camera camera; // Conservé pour compatibilité avec tes fonctions existantes

static bool isDragging = false;
static float lastX = WIDTH / 2.0f;
static float lastY = HEIGHT / 2.0f;

static float camYaw = 90.0f;
static float camPitch = 0.0f;
static float camRadius = 5.0f; // Distance (Zoom)

// ==========================================
// FONCTIONS UTILITAIRES
// ==========================================
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
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// ==========================================
// CALLBACKS SOURIS & MOLETTE
// ==========================================
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void)mods;
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            isDragging = true;
        } else if (action == GLFW_RELEASE) {
            isDragging = false;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void)xoffset;
    camRadius -= (float)yoffset * 0.5f; // Ajuste la vitesse du zoom ici

    // Limites du zoom
    if (camRadius < 1.0f) camRadius = 1.0f;
    if (camRadius > 50.0f) camRadius = 50.0f;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = (float)xposIn;
    float ypos = (float)yposIn;

    static bool firstDrag = true;
    if (isDragging && firstDrag) {
        lastX = xpos;
        lastY = ypos;
        firstDrag = false;
    } else if (!isDragging) {
        firstDrag = true;
        return; // Ne rien faire si on ne maintient pas le clic
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Inversé
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.2f;
    camYaw += xoffset * sensitivity;
    camPitch += yoffset * sensitivity;

    // Bloquer le pitch pour éviter de faire des loopings avec la caméra
    if (camPitch > 89.0f) camPitch = 89.0f;
    if (camPitch < -89.0f) camPitch = -89.0f;
}

// ==========================================
// MAIN
// ==========================================
int main(int argc, char** argv)
{
    srand((unsigned int)time(NULL));

    // Initialisation Factice de ta structure Camera (si processInput en a besoin)
    camera = Camera_createCamera(
        glms_vec3_zero(), (vec3s)VECTOR_UP, (vec3s)VECTOR_FRONT,
        CAMERA_YAW, CAMERA_PITCH, CAMERA_SPEED, CAMERA_SENSIVITY, CAMERA_ZOOM
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
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "CherryEngine", NULL, NULL);
    if (window == NULL) {
        LOG("Failed to create GLFW window");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);

    // NOUVEAU : Curseur normal et ajout des callbacks
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG("Erreur de Init Glad");
        cleanup(window);
        return EXIT_FAILURE;
    }

    glEnable(GL_DEPTH_TEST);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float currentFrame = 0.0f;
    vec3s lightDirection = {0.45, 1.0, 0.45};

    // Chargement des ressources
    Shader shader;
    if (!Shader_load(&shader,
                GetPath("shaders/testingModels.vs"),
                GetPath("shaders/testingModels.fs") )) {
        LOG("Erreur de chargement des shaders testingModels");
    }

    Model model = Model_create(
        GetPath("models/teapot.obj"),
        false
    );

    // Initialisation sécurisée du Transform
    Transform modelTransform = {0};
    Transform* ptr = &modelTransform;

    ptr->rotation = glms_quat_identity(); // IMPORTANT : Initialise le quaternion
    ptr->isDirty = true;

    // On centre l'objet à 0,0,0 pour que la caméra orbite parfaitement autour de lui
    Transform_setPosition(ptr, (vec3s){ 0.0f, -0.4f, 0.0f });
    Transform_setScale(ptr, (vec3s){ .2f, .2f, .2f });

    float timeCheck = 0.0f;
    int nbFrames = 0;

    // ==========================================
    // BOUCLE DE RENDU
    // ==========================================
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
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

        processInput(&camera, deltaTime, window);

        // NOUVEAU : Calcul mathématique de la Caméra Orbitale
        vec3s camPos;
        camPos.x = camRadius * cosf(glm_rad(camYaw)) * cosf(glm_rad(camPitch));
        camPos.y = camRadius * sinf(glm_rad(camPitch));
        camPos.z = camRadius * sinf(glm_rad(camYaw)) * cosf(glm_rad(camPitch));

        vec3s target = {0.0f, 0.0f, 0.0f}; // L'origine du monde
        vec3s up = {0.0f, 1.0f, 0.0f};     // Le vecteur haut
        mat4s view = glms_lookat(camPos, target, up);

        // Rendu
        Shader_use(&shader);
        Shader_setMat4(&shader, "projection", perspective);
        Shader_setMat4(&shader, "view", view);
        Shader_setMat4(&shader, "model", Transform_getWorldMatrix(&modelTransform));
        Shader_setVec3(&shader, "lightDirection", lightDirection);

        Model_Draw(&model, &shader);

        timeCheck += deltaTime;
        nbFrames++;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanup(window);
    return EXIT_SUCCESS;
}