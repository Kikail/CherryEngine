//
// Created by killian on 4/9/26.
//
#include "scene.h"

Scene* Scene_create(char* name) {
    Scene* scene = malloc(sizeof(Scene));
    scene->name = name;
    scene->numGameObjects = 0;
    return scene;
}
GameObject* Scene_addGameObject(Scene* scene, char* objectName) {
    if (scene->numGameObjects >= MAX_COMPONENT_PER_OBJECT) return NULL;
    scene->gameObjects[scene->numGameObjects].name = objectName;
    scene->numGameObjects++;
    return &scene->gameObjects[scene->numGameObjects - 1];
}
void Scene_updateScene(Scene* scene, ComponentPool* componentPoool, float deltaTime) {
    for (uint32 i = 0; i < scene->numGameObjects; i++) {
        GameObject_updateComponents(&scene->gameObjects[i], componentPoool, deltaTime);
    }
}