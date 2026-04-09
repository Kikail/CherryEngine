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
#include "resource/shapes/shape.h"
#include "game/ecs/componentPool.h"
#include "game/scene/gameObject.h"
#include "physics/physicsWorld.h"
#include "render/camera.h"
#include "render/instanceMesh.h"
#include "render/material.h"
#include "render/model.h"
#include "render/shader.h"
#include "game/scene/scene.h"

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

vec3s lightPos = {1.5, 1.5, 1.5};

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

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        lightPos.x -= 1 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        lightPos.x += 1 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        lightPos.z -= 1 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        lightPos.z += 1 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        lightPos.y += 1 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        lightPos.y -= 1 * deltaTime;

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
    // Testing des scenes
    ComponentPool component_pool = ComponentPool_Create();
    DEBUG_LOG("Creating the component pool");

    Scene* scene = Scene_create("testScene");
    DEBUG_showName(scene);
    GameObject* gameObject = Scene_addGameObject(scene, "GameObject01");
    DEBUG_showName(gameObject);
    DEBUG_AddGameObjectComponent(gameObject, &component_pool, COMPONENT_MESH_RENDERER)
    DEBUG_AddGameObjectComponent(gameObject, &component_pool, COMPONENT_SPRITE_RENDERER)
    MeshRenderer* mesh_renderer = GameObject_GetComponent(gameObject, &component_pool, COMPONENT_MESH_RENDERER);
    MeshRenderer* spriteRenderer = GameObject_GetComponent(gameObject, &component_pool, COMPONENT_SPRITE_RENDERER);
    DEBUG_isValid(mesh_renderer);
    DEBUG_isValid(spriteRenderer);

    GameObject* gameObject2 = Scene_addGameObject(scene, "GameObject02");
    DEBUG_AddGameObjectComponent(gameObject2, &component_pool, COMPONENT_MESH_RENDERER)

    GameObject* gameObject3 = Scene_addGameObject(scene, "GameObject03");
    DEBUG_AddGameObjectComponent(gameObject3, &component_pool, COMPONENT_MESH_RENDERER)

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
        DEBUG_LOG("Failed to initialize GLFW");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "CherryEngine", NULL, NULL);
    if (window == NULL) {
        DEBUG_LOG("Failed to create GLFW window");
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
        DEBUG_LOG("Erreur de Init Glad");
        cleanup(window);
        return EXIT_FAILURE;
    }

    glEnable(GL_DEPTH_TEST);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float currentFrame = 0.0f;

    // Chargement des ressources
    Shader shader;
    if (!Shader_load(&shader,
                GetPath("shaders/testingModels.vs"),
                GetPath("shaders/testingModels.fs") )) {
        DEBUG_LOG("Erreur de chargement des shaders testingModels");
    }

    Shader shaderSkybox;
    if (!Shader_load(&shaderSkybox,
                GetPath("shaders/skybox.vs"),
                GetPath("shaders/skybox.fs") )) {
        DEBUG_LOG("Erreur de chargement des shaders skybox");
    }

    Model modelCube = Model_create(
        GetPath("models/cube.obj"),
        false
    );

    Model model = Model_create(
        GetPath("models/test.obj"),
        false
    );
    vec2s textureSize;
    //unsigned int diffuseId = TextureFromFile(GetPath("images/testingMaterial/Rocks001_1K-PNG_Color.png"), false, &textureSize);
    //unsigned int normalId = TextureFromFile(GetPath("images/testingMaterial/Rocks001_1K-PNG_NormalGL.png"), false, &textureSize);
    //unsigned int aoId = TextureFromFile(GetPath("images/testingMaterial/Rocks001_1K-PNG_AmbientOcclusion.png"), false, &textureSize);
    //unsigned int displacementId = TextureFromFile(GetPath("images/testingMaterial/Rocks001_1K-PNG_Displacement.png"), false, &textureSize);
    Material material = Material_create(
        (vec3s){1.0,0.0,1.0},
        (vec3s){1.0,1.0,1.0},
        (vec3s){0.5,0.5,0.5},
        16.00,
        1.0,
        0.35,
        0.75,
        &shader
    );

    char* faces[6] = {
        GetPath("images/skybox/right.jpg"),
        GetPath("images/skybox/left.jpg"),
        GetPath("images/skybox/top.jpg"),
        GetPath("images/skybox/bottom.jpg"),
        GetPath("images/skybox/front.jpg"),
        GetPath("images/skybox/back.jpg")};
    unsigned int cubemapTexture = loadCubemap(faces, 6);

    // Initialisation sécurisée du Transform
    Transform modelTransform = {0};
    Transform* ptr = &modelTransform;

    ptr->rotation = glms_quat_identity(); // IMPORTANT : Initialise le quaternion
    ptr->isDirty = true;

    // On centre l'objet à 0,0,0 pour que la caméra orbite parfaitement autour de lui
    Transform_setPosition(ptr, (vec3s){ 0.0f, 0.0, 0.0f });
    float scale = 0.1f;
    Transform_setScale(ptr, (vec3s){scale, scale, scale});

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

        mat4s viewSkybox = view;
        viewSkybox.raw[3][0] = 0.0f; // Annule X
        viewSkybox.raw[3][1] = 0.0f; // Annule Y
        viewSkybox.raw[3][2] = 0.0f; // Annule Z
        glDepthMask(GL_FALSE);
        Shader_use(&shaderSkybox);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        Shader_setMat4(&shaderSkybox, "projection", perspective);
        Shader_setMat4(&shaderSkybox, "view", viewSkybox);
        Model_Draw(&modelCube, &shaderSkybox);
        glDepthMask(GL_TRUE);

        // Rendu
        Shader_use(&shader);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        Shader_setInt(&shader, "skybox", 5);
        Shader_setMat4(&shader, "projection", perspective);
        Shader_setMat4(&shader, "view", view);
        Shader_setMat4(&shader, "model", Transform_getWorldMatrix(&modelTransform));
        Shader_setVec3(&shader, "lightPos", lightPos);
        Shader_setVec3(&shader, "viewPos", camPos);
        Material_sendToShader(&material, &shader);
        Model_Draw(&model, &shader);


        // Rendu
        mat4s modelView = glms_translate(glms_mat4_identity(), lightPos);
        modelView = glms_scale(modelView, (vec3s){0.1,0.1,0.1});
        Shader_use(&shader);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        Shader_setInt(&shader, "skybox", 5);
        Shader_setMat4(&shader, "projection", perspective);
        Shader_setMat4(&shader, "view", view);
        Shader_setMat4(&shader, "model", modelView);
        Shader_setVec3(&shader, "lightPos", lightPos);
        Shader_setVec3(&shader, "viewPos", camPos);
        Material_sendToShader(&material, &shader);

        Model_Draw(&modelCube, &shader);

        Scene_updateScene(scene, &component_pool, deltaTime);

        timeCheck += deltaTime;
        nbFrames++;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanup(window);
    return EXIT_SUCCESS;
}