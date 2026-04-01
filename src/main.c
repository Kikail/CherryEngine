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

static bool firstMouse = true;
static Camera camera;
static float lastX, lastY;
static bool captureMouse = true;

#define WIDTH  1280
#define HEIGHT 720

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
    vec3s lightDirection = {0.0, 1.0, 0.0};

    // Chargement des ressources
    Shader shader;
    if (!Shader_load(&shader,
                GetPath("shaders/testingModels.vs"),
                GetPath("shaders/testingModels.fs") )) {
        LOG("Erreur de chargement des shaders testingModels");
    }
    Model model = Model_create(
        GetPath("models/test.obj"),
        false
    );

    // Position de base du mesh dans l'espace 3D
    Transform modelTransform = {0};
    Transform* ptr = &modelTransform;
    Transform_setPosition(ptr, (vec3s){ 0.0f, -0.4f, 2.0f });
    Transform_setScale(ptr, (vec3s){ .05f, .05f, .05f });
    if (ptr == NULL) {
        LOG("Erreur avec le transform");
        return 1;
    }

    float timeCheck = 0.0f;
    int nbFrames = 0;

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f); // Légèrement coloré pour bien voir les cubes
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

        mat4s view = Camera_getViewMatrix(&camera);

        processInput(&camera, deltaTime, window);

        // 3. Rendu instancié pour tous les meshes composant le modèle
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