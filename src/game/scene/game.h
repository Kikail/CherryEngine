//
// Created by killian on 4/9/26.
//

#ifndef CHERRYENGINE_GAME_H
#define CHERRYENGINE_GAME_H


#include "physics/physicsWorld.h"
#include "render/camera.h"
#include "../../resource/resourceManager.h"
#include "GLFW/glfw3.h"

/**
 * \brief Une structure de donnee regroupant toutes les infos utiles pour les components
 */
typedef struct Game_t {
    Camera* camera;
    GLFWwindow* window;
    PhysicsWorld* physicsWorld;
    ResourceManager* resourceManager;
}Game;
Game* Game_init(Camera* camera, GLFWwindow* window, PhysicsWorld* physicsWorld, ResourceManager* resourceManager);

#endif //CHERRYENGINE_GAME_H
