//
// Created by killian on 4/9/26.
//
#include "scene.h"

#include "game/ecs/componentPool.h"
#include "utils/idMaker.h"
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
    if (scene->numGameObjects >= SCENE_MAX_GAMEOBJECTS) {
        #ifdef DEBUG
            DEBUG_LOG("SCENE::Scene_addGameObject max component reach");
        #endif
        return NULL;
    }
    scene->gameObjects[scene->numGameObjects].name = objectName;
    if (Id_createId(&scene->gameObjects[scene->numGameObjects].id)) {
        scene->gameObjects[scene->numGameObjects].id = Id_makeGameObject(scene->gameObjects[scene->numGameObjects].id);
    }
    else {
        #ifdef DEBUG
            DEBUG_LOG("SCENE::Scene_addGameObject max id reached");
        #endif
    }
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
    SerialObject componentPoolObject = SerialObject_create("ComponentPool");

    // Sauvegarde de chaque gameObject
    for (int i = 0; i < scene->numGameObjects; i++) {
        GameObject gameObject = scene->gameObjects[i];


        for (int j = 0; j < gameObject.componentCount; j++) {

            Component component = gameObject.components[j];
             // Ici il faudrait avoir un serial object du component mais de maniere abstraire comme le component pool
            // Puis ensuite ajouter l'id qui est dans component
            SerialObject componentObject = ComponentPool_serializeComponent(componentPool, component.component_type, component.component_adress);
            SerialValue idValue = SerialValue_create_uint("id", component.id);
            SerialObject_AddSerialValue(&componentObject, &idValue);

            SerialObject_AddChild(&componentPoolObject, &componentObject);

        }

        // mask id
        SerialObject gameObjectSerialObject = SerialObject_create(gameObject.name);
        SerialValue maskValue = SerialValue_create_uint("mask", gameObject.component_mask);
        SerialObject_AddSerialValue(&gameObjectSerialObject, &maskValue);
        SerialValue idValue = SerialValue_create_uint("id", gameObject.id);
        SerialObject_AddSerialValue(&gameObjectSerialObject, &idValue);
        SerialObject_AddChild(&gameobjectsObject, &gameObjectSerialObject);

    }

    SerialObject_AddChild(&sceneObject, &gameobjectsObject);
    SerialObject_AddChild(&sceneObject, &componentPoolObject);

    return sceneObject;
}