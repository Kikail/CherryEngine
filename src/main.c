#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <math.h>
#include <cglm/struct.h>

#include "game/scene/game.h"

#include "cglm/cam.h"
#include "resource/shapes/shape.h"
#include "game/ecs/componentPool.h"

#include "game/scene/gameObject.h"
#include "physics/physicsWorld.h"

#include "render/instanceMesh.h"
#include "render/material.h"
#include "render/model.h"
#include "render/shader.h"
#include "game/scene/scene.h"


vec3s lightPos = {1.5, 1.5, 1.5};





// ==========================================
// MAIN
// ==========================================
int main(int argc, char** argv)
{
    ////////////////////////////////////////////////////
    ///     Testing des scenes
    ////////////////////////////////////////////////////
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









    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float currentFrame = 0.0f;



    Game* game = Game_init();

    glEnable(GL_DEPTH_TEST);


    ////////////////////////////////////////////////////
    ///     Chargement des donnees
    ////////////////////////////////////////////////////
    ResourceManager* resourceManager = Game_getResourceManager(game);
    Shader* shader = ResourceManager_loadShader(resourceManager,"shaders/testingModels.vs","shaders/testingModels.fs");
    Shader* shaderSkybox = ResourceManager_loadShader(resourceManager,"shaders/skybox.vs","shaders/skybox.fs");
    Model* modelCube = ResourceManager_loadModel(resourceManager,"models/cube.obj");
    Model* model = ResourceManager_loadModel(resourceManager,"models/test.obj");
    unsigned int diffuseId = ResourceManager_loadTexture(resourceManager,"images/testingMaterial/Rocks001_1K-PNG_Color.png");
    unsigned int normalId = ResourceManager_loadTexture(resourceManager,"images/testingMaterial/Rocks001_1K-PNG_NormalGL.png");
    unsigned int aoId = ResourceManager_loadTexture(resourceManager,"images/testingMaterial/Rocks001_1K-PNG_AmbientOcclusion.png");
    unsigned int displacementId = ResourceManager_loadTexture(resourceManager,"images/testingMaterial/Rocks001_1K-PNG_Displacement.png");
    char* faces[6] = {
        GetPath("images/skybox/right.jpg"),
        GetPath("images/skybox/left.jpg"),
        GetPath("images/skybox/top.jpg"),
        GetPath("images/skybox/bottom.jpg"),
        GetPath("images/skybox/front.jpg"),
        GetPath("images/skybox/back.jpg")};
    unsigned int cubemapTexture = loadCubemap(faces, 6);



    Material material = Material_create(
        (vec3s){1.0,0.0,1.0},
        (vec3s){1.0,1.0,1.0},
        (vec3s){0.5,0.5,0.5},
        16.00,
        1.0,
        0.35,
        0.75,
        shader
    );

    

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
    while (!glfwWindowShouldClose(Game_getWindow(game))) {
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (timeCheck >= 1.0f) {
            char title[128];
            double msPerFrame = (nbFrames > 0) ? (1000.0 / (double)nbFrames) : 0.0;
            sprintf(title, "CherryEngine - [FPS: %d | %.2f ms]", nbFrames, msPerFrame);
            glfwSetWindowTitle(Game_getWindow(game), title);

            timeCheck = 0.0f;
            nbFrames = 0;
        }

        Game_update(game, deltaTime);

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
        Shader_use(shaderSkybox);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        Shader_setMat4(shaderSkybox, "projection", Game_getPerspective(game));
        Shader_setMat4(shaderSkybox, "view", viewSkybox);
        Model_Draw(modelCube, shaderSkybox);
        glDepthMask(GL_TRUE);

        // Rendu
        Shader_use(shader);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        Shader_setInt(shader, "skybox", 5);
        Shader_setMat4(shader, "projection", Game_getPerspective(game));
        Shader_setMat4(shader, "view", view);
        Shader_setMat4(shader, "model", Transform_getWorldMatrix(&modelTransform));
        Shader_setVec3(shader, "lightPos", lightPos);
        Shader_setVec3(shader, "viewPos", camPos);
        Material_sendToShader(&material, shader);
        Model_Draw(model, shader);


        // Rendu
        mat4s modelView = glms_translate(glms_mat4_identity(), lightPos);
        modelView = glms_scale(modelView, (vec3s){0.1,0.1,0.1});
        Shader_use(shader);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        Shader_setInt(shader, "skybox", 5);
        Shader_setMat4(shader, "projection", Game_getPerspective(game));
        Shader_setMat4(shader, "view", view);
        Shader_setMat4(shader, "model", modelView);
        Shader_setVec3(shader, "lightPos", lightPos);
        Shader_setVec3(shader, "viewPos", camPos);
        Material_sendToShader(&material, shader);

        Model_Draw(modelCube, shader);

        //Scene_updateScene(scene, &component_pool, deltaTime);

        timeCheck += deltaTime;
        nbFrames++;

        glfwSwapBuffers(Game_getWindow(game));
        glfwPollEvents();
    }

    cleanup(Game_getWindow(game));
    return EXIT_SUCCESS;
}