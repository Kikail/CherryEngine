//
// Created by killian on 4/9/26.
//

#ifndef CHERRYENGINE_SCENE_H
#define CHERRYENGINE_SCENE_H

#include "gameObject.h"
#include "resource/serializer.h"
#include "utils/idMaker.h"

#define SCENE_MAX_GAMEOBJECTS 100

typedef struct Scene_t {
    char name[128];
    GameObject gameObjects[SCENE_MAX_GAMEOBJECTS];
    unsigned int numGameObjects;
}Scene;
Scene* Scene_create(char* name);
Scene* Scene_deserialize(SerialObject* sceneObject, ComponentPool* componentPool);
GameObject* Scene_addGameObject(Scene* scene, char* objectName);
GameObject* Scene_getGameObject(Scene* scene, unsigned int id);
void Scene_updateScene(Scene* scene, ComponentPool* componentPoool, float deltaTime, Game* game);
SerialObject Scene_serialize(Scene* scene, ComponentPool* componentPool);

#endif //CHERRYENGINE_SCENE_H
