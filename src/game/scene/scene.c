//
// Created by killian on 4/9/26.
//
#include "scene.h"

#include "game/ecs/componentPool.h"
#include "utils/idMaker.h"
#include "utils/utils.h"

#define FOR_INF(valeur) for(int i = 0; i < valeur; i++)

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
            SerialValue parent = SerialValue_create_uint("parent", component.parentId);
            SerialObject_AddSerialValue(&componentObject, &parent);
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

Scene* Scene_deserialize(SerialObject* sceneObject) {
    if (sceneObject == NULL) {
        #ifdef DEBUG
                DEBUG_LOG("SCENE::Scene_deserialize sceneObject null");
        #endif
        return NULL;
    }

    // Ici on recupere le nom initial de la scenes
    SerialValue sceneNameValue = SerialObject_GetByName(sceneObject, "name");
    char* sceneName = SerialValue_GetStringValue(&sceneNameValue);
    Scene* scene = Scene_create(sceneName);

    // Ici on recupere les deux grands conteneurs d'objets qui contiennent tout les objets dans la scene ainsi que tout les composants
    // Notre but va etre de recreer tout les objets, de leur assigner leur mask, id et nom
    SerialObject* gameObjectSerialObject = SerialObject_GetObjectByName(sceneObject, "GameObjects");
    FOR_INF(gameObjectSerialObject->num_childrens) {
        // On charge chaque GameObject 1 par 1 et on charge ses donnees dans une structure
        struct SerialObject_t gameObjectSerialized = gameObjectSerialObject->childrens[i];

        // Nous recuperons le nom de l'objet dans la scene
        SerialValue gameObjectNameValue = SerialObject_GetByName(&gameObjectSerialized, "name");
        char* gameObjectName = SerialValue_GetStringValue(&gameObjectNameValue);

        // Ici on creer un gameObject a partir du nom charger
        GameObject* gameObject = Scene_addGameObject(scene, gameObjectName);

        // Nous recuperons ensuite chaque donnee du gameObject et le mettons dans la structure creer
        SerialValue gameObjectMaskValue = SerialObject_GetByName(&gameObjectSerialized, "mask");
        unsigned int gameObjectMask = SerialValue_GetUintValue(&gameObjectMaskValue);
        SerialValue gameObjectIdValue = SerialObject_GetByName(&gameObjectSerialized, "id");
        unsigned int gameObjectId = SerialValue_GetUintValue(&gameObjectMaskValue);
        gameObject->component_mask = gameObjectMask;
        gameObject->id = gameObjectId;

        #ifdef DEBUG
                printf("(%s, %u, %u) loaded\n", gameObject->name, gameObject->id, gameObject->component_mask);
        #endif
    }

    // Nous allons ensuiter recreer tout les composants et les assigner aux bons objets par la suite
    // Nous recreerons ainsi la hierarchie sauvegardee dans le fichier de scene
    SerialObject* componentPoolSerialObject = SerialObject_GetObjectByName(sceneObject, "ComponentPool");
}
























