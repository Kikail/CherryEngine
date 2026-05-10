//
// Created by killian on 4/9/26.
//

#ifndef CHERRYENGINE_SCENE_H
#define CHERRYENGINE_SCENE_H

#include "gameObject.h"
#include "resource/serializer.h"
#include "utils/idMaker.h"

#define SCENE_MAX_GAMEOBJECTS 1000

typedef struct Scene_t {
    char* name;
    GameObject gameObjects[SCENE_MAX_GAMEOBJECTS];
    unsigned int numGameObjects;
}Scene;
Scene* Scene_create(char* name);
Scene* Scene_deserialize(SerialObject* sceneObject);
GameObject* Scene_addGameObject(Scene* scene, char* objectName);
void Scene_updateScene(Scene* scene, ComponentPool* componentPoool, float deltaTime);
SerialObject Scene_serialize(Scene* scene, ComponentPool* componentPool);

#endif //CHERRYENGINE_SCENE_H
