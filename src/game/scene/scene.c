//
// Created by killian on 4/9/26.
//
#include "scene.h"

#include "game/ecs/componentPool.h"
#include "utils/utils.h"

Scene* Scene_create(char* name) {
    Scene* scene = malloc(sizeof(Scene));
    #ifdef DEBUG
        if (scene == NULL)
            DEBUG_LOG("SCENE::Scene_create failed to malloc scene");
    #endif

    scene->name = name;
    scene->numGameObjects = 0;
    return scene;
}
GameObject* Scene_addGameObject(Scene* scene, char* objectName) {
    if (scene->numGameObjects >= MAX_COMPONENT_PER_OBJECT) {
        #ifdef DEBUG
            DEBUG_LOG("SCENE::Scene_addGameObject max component reach");
        #endif
        return NULL;
    }
    scene->gameObjects[scene->numGameObjects].name = objectName;
    scene->numGameObjects++;
    return &scene->gameObjects[scene->numGameObjects - 1];
}
void Scene_updateScene(Scene* scene, ComponentPool* componentPoool, float deltaTime) {
    for (uint32 i = 0; i < scene->numGameObjects; i++) {
        GameObject_updateComponents(&scene->gameObjects[i], componentPoool, deltaTime);
    }
}
SerialObject Scene_serialize(Scene* scene, ComponentPool* componentPool) {
    #ifdef DEBUG
    if (scene == NULL || componentPool == NULL) {
        DEBUG_LOG("SCENE::Scene_serialize scene or componentPool null");
        SerialObject_create("invalid");
    }
    #endif

    SerialObject sceneObject = SerialObject_create("Scene");

    SerialObject gameobjectsObject = SerialObject_create("GameObjects");
    SerialObject componentPoolObject = ComponentPool_serialize(componentPool);

    // Sauvegarde de chaque gameObject
    for (int i = 0; i < scene->numGameObjects; i++) {
        printf(" OBJECT %d\n",i);
        GameObject gameObject = scene->gameObjects[i];
        SerialObject gameObjectSerial = SerialObject_create(gameObject.name);
        SerialValue mask = SerialValue_create_uint("componentMask",gameObject.component_mask);
        SerialValue id = SerialValue_create_int("id", i);
        SerialObject_AddSerialValue(&gameObjectSerial, &mask);
        SerialObject_AddSerialValue(&gameObjectSerial, &id);
        SerialObject_AddChild(&gameobjectsObject, &gameObjectSerial);
    }

    SerialObject_AddChild(&sceneObject, &gameobjectsObject);
    SerialObject_AddChild(&sceneObject, &componentPoolObject);

    return sceneObject;
}