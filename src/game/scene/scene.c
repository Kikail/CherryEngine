//
// Created by killian on 4/9/26.
//
#include "scene.h"

#include <string.h>

#include "utils/idMaker.h"
#include "utils/utils.h"
#include "game/ecs/componentPool.h"

#define FOR_INF(valeur) for(int i = 0; i < valeur; i++)

Scene* Scene_create(char* name) {
    Scene* scene = malloc(sizeof(Scene));
    #ifdef DEBUG
        if (scene == NULL)
            DEBUG_LOG("SCENE::Scene_create failed to malloc scene");
    #endif

    strcpy(scene->name,name);
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
    strcpy(scene->gameObjects[scene->numGameObjects].name, objectName);
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
void Scene_updateScene(Scene* scene, ComponentPool* componentPoool, float deltaTime, Game* game) {
    for (uint32 i = 0; i < scene->numGameObjects; i++) {
        GameObject_updateComponents(&scene->gameObjects[i], componentPoool, deltaTime,game);
    }
}
GameObject* Scene_getGameObject(Scene* scene, unsigned int id) {
    FOR_INF(scene->numGameObjects) {
        if (scene->gameObjects[i].id == id) {
            return &scene->gameObjects[i];
        }
    }
    return NULL;
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
            SerialValue maskValue = SerialValue_create_uint("type", component.component_type);
            SerialObject_AddSerialValue(&componentObject, &maskValue);
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

Scene* Scene_deserialize(SerialObject* sceneObject, ComponentPool* componentPool) {
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
        gameObject->componentCount = 0;

        // Nous recuperons ensuite chaque donnee du gameObject et le mettons dans la structure creer
        SerialValue gameObjectMaskValue = SerialObject_GetByName(&gameObjectSerialized, "mask");
        unsigned int gameObjectMask = SerialValue_GetUintValue(&gameObjectMaskValue);
        SerialValue gameObjectIdValue = SerialObject_GetByName(&gameObjectSerialized, "id");
        unsigned int gameObjectId = SerialValue_GetUintValue(&gameObjectIdValue);
        gameObject->component_mask = gameObjectMask;
        gameObject->id = gameObjectId;

        #ifdef DEBUG
                printf("(%s, %u, %u) loaded\n", gameObject->name, gameObject->id, gameObject->component_mask);
        #endif
    }

    // Nous allons ensuiter recreer tout les composants et les assigner aux bons objets par la suite
    // Nous recreerons ainsi la hierarchie sauvegardee dans le fichier de scene
    SerialObject* componentPoolSerialObject = SerialObject_GetObjectByName(sceneObject, "ComponentPool");
    if (!gameObjectSerialObject || !componentPoolSerialObject) return NULL;
    FOR_INF(componentPoolSerialObject->num_childrens) {
        // On charge chaque GameObject 1 par 1 et on charge ses donnees dans une structure
        struct SerialObject_t componentSerialized = componentPoolSerialObject->childrens[i];

        // On recupere le type du component
        SerialValue typeValue = SerialObject_GetByName(&componentSerialized, "type");
        ComponentType componentType = SerialValue_GetUintValue(&typeValue);

        // On recupere l'id du parent
        SerialValue parentIdValue = SerialObject_GetByName(&componentSerialized, "parent");
        // On recupere l'id du component
        SerialValue idValue = SerialObject_GetByName(&componentSerialized, "id");

        // On recreer un component
        Component component;
        component.component_type = componentType;
        component.parentId = SerialValue_GetUintValue(&parentIdValue);
        component.id = SerialValue_GetUintValue(&idValue);

        // On ajoute le component dans le gameObject et dans le componentPool
        GameObject* gameObject = Scene_getGameObject(scene, component.parentId);
        gameObject->components[gameObject->componentCount].component_type = component.component_type;
        gameObject->components[gameObject->componentCount].parentId = component.parentId;
        gameObject->components[gameObject->componentCount].id = component.id;
        ComponentPool_CreateComponent(componentPool, component.component_type, &gameObject->components[gameObject->componentCount]);
        ComponentPool_deserialize(componentPool, component.component_type, &componentSerialized);
        // On oublie pas d'augmenter le nombre de components
        gameObject->componentCount += 1;
    }

    return scene;
}
























