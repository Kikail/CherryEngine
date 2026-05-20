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

#include "editor/editor.h"

vec3s lightPos = {1.5, 1.5, 1.5};

// PROCHAINE ETAPE :
// GROSSE UPDATE A FAIRE : empecher les fuites memoire
// Ajout d'un editeur

// ==========================================
// MAIN
// ==========================================
int main(int argc, char** argv)
{
    ////////////////////////////////////////////////////
    ///     Creation de la structure de jeu
    ////////////////////////////////////////////////////
    Game* game = Game_init();
    glEnable(GL_DEPTH_TEST);

    ////////////////////////////////////////////////////
    ///     Initialisation de ImGui
    ////////////////////////////////////////////////////
    ImGuiContext* ctx = igCreateContext(NULL);
    ImGuiIO* io = igGetIO_ContextPtr(ctx);
    ImGui_ImplGlfw_InitForOpenGL(game->window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    ////////////////////////////////////////////////////
    ///     Testing des scenes
    ////////////////////////////////////////////////////
    ComponentPool componentPool = ComponentPool_Create();
    game->componentPool = &componentPool;
    DEBUG_LOG("Creating the component pool");

    ////////////////////////////////////////////////////
    ///     Chargement des donnees
    ////////////////////////////////////////////////////
    ResourceManager* resourceManager = Game_getResourceManager(game);
    ResourceManager_loadAllFilesFromDirectory(resourceManager, "/home/killian/Projects/C/CherryEngine/resources");

    // Creation des variables permettant de savoir le temps entre chaque frame
    srand((unsigned int)time(NULL));
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float currentFrame = 0.0f;

    ////////////////////////////////////////////////////
    ///     Chargement de la scene
    ////////////////////////////////////////////////////

    FILE* f = fopen("/home/killian/Projects/C/CherryEngine/resources/scenes/test.csn", "r");
    SerialObject serialObject = SerialObject_DeserializeSingle(f);
    fclose(f);
    SerialObject_Print(&serialObject);
    Scene* scene = Scene_deserialize(&serialObject, game->componentPool);
    Scene_initScene(scene, &componentPool, deltaTime, game);
    game->currentScene = scene;

    ////////////////////////////////////////////////////
    ///     Creation de l'editeur
    ////////////////////////////////////////////////////
    Editor editor;
    editor.selectedObjet = &scene->gameObjects[0];

    float timeCheck = 0.0f;
    int nbFrames = 0;

    // ==========================================
    // BOUCLE DE RENDU
    // ==========================================
    while (!glfwWindowShouldClose(Game_getWindow(game))) {
        // Couleur de fond
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calcul du temps entre chaque frame
        currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        game->currentFrame = currentFrame;

        // Raffraichissement du titre avec les fps
        if (timeCheck >= 1.0f) {
            char title[128];
            double msPerFrame = (nbFrames > 0) ? (1000.0 / (double)nbFrames) : 0.0;
            sprintf(title, "CherryEngine - [FPS: %d | %.2f ms]", nbFrames, msPerFrame);
            glfwSetWindowTitle(Game_getWindow(game), title);

            timeCheck = 0.0f;
            nbFrames = 0;
        }

        // Update du jeu comprenenant la physique et les input
        glfwPollEvents();

        // Nouvelle frame Imgui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        igNewFrame();

        Editor_render(&editor, game);

        // Update du jeu
        Game_update(game, deltaTime);

        // NOUVEAU : Calcul mathématique de la Caméra Orbitale
        vec3s camPos;
        camPos.x = camRadius * cosf(glm_rad(camYaw)) * cosf(glm_rad(camPitch));
        camPos.y = camRadius * sinf(glm_rad(camPitch));
        camPos.z = camRadius * sinf(glm_rad(camYaw)) * cosf(glm_rad(camPitch));
        vec3s target = {0.0f, 0.0f, 0.0f}; // L'origine du monde
        vec3s up = {0.0f, 1.0f, 0.0f};     // Le vecteur haut
        mat4s view = glms_lookat(camPos, target, up);
        game->view = view;

        // Update de la scene actuelle, L'affichage de la scene se passe ici
        Scene_updateScene(scene, game->componentPool, deltaTime, game);

        // On augmente le nombre de frame verifiee
        timeCheck += deltaTime;
        nbFrames++;

        // Rendu ImGui sur la fenetre
        igRender();
        ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());

        // Affichage graphique
        glfwSwapBuffers(Game_getWindow(game));
    }

    // Permettant de sauvegarder la scene actuelle a la fermeture de l'application
    SerialObject sceneObj = Scene_serialize(scene, game->componentPool);
    SerialObject_Print(&sceneObj);
    FileSaver* fileSaver = FileSaver_create("/home/killian/Projects/C/CherryEngine/resources/scenes/test.csn", SerialObject_Serialize(&sceneObj));
    FileSaver_save(fileSaver);


    cleanup(Game_getWindow(game));
    return EXIT_SUCCESS;
}