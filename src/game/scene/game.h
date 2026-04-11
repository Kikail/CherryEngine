//
// Created by killian on 4/9/26.
//

#ifndef CHERRYENGINE_GAME_H
#define CHERRYENGINE_GAME_H


#include "physics/physicsWorld.h"
#include "render/camera.h"
#include "../../resource/resourceManager.h"
#include "game/input/input.h"
#include "GLFW/glfw3.h"

#include "render/camera.h"

/**
 * \brief Une structure de donnee regroupant toutes les infos utiles pour les components
 */
typedef struct Game_t {
    Camera camera;
    GLFWwindow* window;
    PhysicsWorld* physicsWorld;
    ResourceManager* resourceManager;
    mat4s perspective;
    Input* input;
}Game;
Game* Game_init();
int Game_initWindow(Game* game);
void Game_update(Game* game, float deltaTime);

Camera* Game_getCamera(Game* game);
GLFWwindow* Game_getWindow(Game* game);
PhysicsWorld* Game_getPhysicsWorld(Game* game);
ResourceManager* Game_getResourceManager(Game* game);
mat4s Game_getPerspective(Game* game);
Input* Game_getInput(Game* game);

#endif //CHERRYENGINE_GAME_H
