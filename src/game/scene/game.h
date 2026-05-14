//
// Created by killian on 4/9/26.
//

#ifndef CHERRYENGINE_GAME_H
#define CHERRYENGINE_GAME_H


#include "physics/physicsWorld.h"
#include "render/camera.h"
#include "../../resource/resourceManager.h"
#include "game/input/input.h"
#include "render/camera.h"

typedef struct ComponentPool_t ComponentPool;

/**
 * \brief Une structure de donnee regroupant toutes les infos utiles pour les components
 */
typedef struct Game_t {
    Camera camera;
    GLFWwindow* window;
    PhysicsWorld* physicsWorld;
    ResourceManager* resourceManager;
    ComponentPool* componentPool;
    mat4s perspective;
    mat4s view;
    Input* input;
    float currentFrame;
}Game;
Game* Game_init();
int Game_initWindow(Game* game);
void Game_update(Game* game, float deltaTime);

Camera* Game_getCamera(Game* game);
GLFWwindow* Game_getWindow(Game* game);
PhysicsWorld* Game_getPhysicsWorld(Game* game);
ResourceManager* Game_getResourceManager(Game* game);
mat4s Game_getPerspective(Game* game);
mat4s Game_getView(Game* game);
Input* Game_getInput(Game* game);

#endif //CHERRYENGINE_GAME_H
