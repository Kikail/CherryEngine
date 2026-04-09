//
// Created by killian on 4/9/26.
//
#include "game.h"

Game* Game_init(Camera* camera, GLFWwindow* window, PhysicsWorld* physicsWorld, ResourceManager* resourceManager) {
    Game* game = malloc(sizeof(Game));
    game->camera = camera;
    game->window = window;
    game->physicsWorld = physicsWorld;
    game->resourceManager = resourceManager;
    return game;
}