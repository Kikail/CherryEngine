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
#include "resource/filePicker.h"
#include "resource/fileSaver.h"
#include "resource/serializer.h"


vec3s lightPos = {1.5, 1.5, 1.5};




// PAR LA SUITE :
// S'OCCUPER DU CHARGEMENT D'UNE SCENE


// ==========================================
// MAIN
// ==========================================
int main(int argc, char** argv)
{
    ////////////////////////////////////////////////////
    ///     Testing des scenes
    ////////////////////////////////////////////////////
    ComponentPool component_pool = ComponentPool_Create();
    DEBUG_LOG("Creating the compone"
              "5nt pool");

    Scene* scene = Scene_create("testScene");
    DEBUG_showName(scene);
    GameObject* gameObject = Scene_addGameObject(scene, "GameObject01");
    DEBUG_showName(gameObject);
    DEBUG_AddGameObjectComponent(gameObject, &component_pool, COMPONENT_MESH_RENDERER)
    DEBUG_AddGameObjectComponent(gameObject, &component_pool, COMPONENT_TRANSFORM)
    DEBUG_AddGameObjectComponent(gameObject, &component_pool, COMPONENT_SPRITE_RENDERER)
    MeshRenderer* mesh_renderer = GameObject_GetComponent(gameObject, &component_pool, COMPONENT_MESH_RENDERER);
    MeshRenderer* spriteRenderer = GameObject_GetComponent(gameObject, &component_pool, COMPONENT_SPRITE_RENDERER);
    Transform* tranformComp1 = GameObject_GetComponent(gameObject, &component_pool, COMPONENT_TRANSFORM);
    Transform_translate(tranformComp1, (vec3s){3.0,10.0,0.0}, false);
    DEBUG_isValid(mesh_renderer);
    DEBUG_isValid(spriteRenderer);
    GameObject* gameObject2 = Scene_addGameObject(scene, "GameObject02");
    DEBUG_AddGameObjectComponent(gameObject2, &component_pool, COMPONENT_MESH_RENDERER)
    DEBUG_AddGameObjectComponent(gameObject2, &component_pool, COMPONENT_TRANSFORM)
    Transform* tranformComp2 = GameObject_GetComponent(gameObject2, &component_pool, COMPONENT_TRANSFORM);
    Transform_setParent(tranformComp2, tranformComp1, KEEP_WORLD);

    GameObject* gameObject3 = Scene_addGameObject(scene, "GameObject03");
    DEBUG_AddGameObjectComponent(gameObject3, &component_pool, COMPONENT_MESH_RENDERER)

    srand((unsigned int)time(NULL));


    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float currentFrame = 0.0f;



    FILE* f = fopen("/home/killian/Projects/C/CherryEngine/resources/scenes/test.csn", "r");
    SerialObject serialObject = SerialObject_DeserializeSingle(f);
    fclose(f);
    SerialObject_Print(&serialObject);
    Scene* testScene = Scene_deserialize(&serialObject);

    SerialObject sceneObj = Scene_serialize(scene, &component_pool);
    SerialObject_Print(&sceneObj);
    FileSaver* fileSaver = FileSaver_create("/home/killian/Projects/C/CherryEngine/resources/scenes/test.csn", SerialObject_Serialize(&sceneObj));
    FileSaver_save(fileSaver);

    Game* game = Game_init();

    glEnable(GL_DEPTH_TEST);


    ////////////////////////////////////////////////////
    ///     Chargement des donnees
    ////////////////////////////////////////////////////
    ResourceManager* resourceManager = Game_getResourceManager(game);
    Shader* shader = ResourceManager_loadShader(resourceManager,"shaders/testingModels.vs","shaders/testingModels.fs", false);
    Shader* shaderSkybox = ResourceManager_loadShader(resourceManager,"shaders/skybox.vs","shaders/skybox.fs", false);
    char* faces[6] = {
        GetPath("images/skybox/right.jpg"),
        GetPath("images/skybox/left.jpg"),
        GetPath("images/skybox/top.jpg"),
        GetPath("images/skybox/bottom.jpg"),
        GetPath("images/skybox/front.jpg"),
        GetPath("images/skybox/back.jpg")};
    unsigned int cubemapTexture = loadCubemap(faces, 6);

    ResourceManager_loadAllFilesFromDirectory(resourceManager, "/home/killian/Projects/C/CherryEngine/resources");
    ResourceManager_showResources(resourceManager);

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
        Model_Draw(&resourceManager->models[0], shaderSkybox);
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
        Model_Draw(&resourceManager->models[1], shader);


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

        Model_Draw(&resourceManager->models[0], shader);

        //Scene_updateScene(scene, &component_pool, deltaTime);

        timeCheck += deltaTime;
        nbFrames++;

        glfwSwapBuffers(Game_getWindow(game));
        glfwPollEvents();
    }

    cleanup(Game_getWindow(game));
    return EXIT_SUCCESS;
}