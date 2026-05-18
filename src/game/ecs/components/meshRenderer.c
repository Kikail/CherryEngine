//
// Created by killian on 3/11/26.
//
#include "stdio.h"
#include "../../scene/gameObject.h"
#include "transform.h"
#include "meshRenderer.h"

void Component_MeshRenderer_Update(MeshRenderer* meshRenderer, GameObject* gameObject, Game* game) {
    if (meshRenderer->model == NULL) {
        #ifdef DEBUG
            DEBUG_LOG("MESHRENDERER::Component_MeshRenderer_Update model is null trying to load");
        #endif
        Model* model = ResourceManager_getModelBySignature(Game_getResourceManager(game), meshRenderer->modelSignature);
        if (model == NULL) {
            #ifdef DEBUG
                DEBUG_LOG("MESHRENDERER::Component_MeshRenderer_Update model is null failed to load");
            #endif
            return;
        }
        else {
            #ifdef DEBUG
                        DEBUG_LOG("MESHRENDERER::Component_MeshRenderer_Update model loaded successfully");
            #endif
            meshRenderer->model = model;
        }
    }
    if (meshRenderer->materialsLoaded < meshRenderer->materialCount) {
        #ifdef DEBUG
                DEBUG_LOG("MESHRENDERER::Component_MeshRenderer_Update all materials are not loaded");
        #endif
        for (int i = 0;i < meshRenderer->materialCount;i++) {
            Material* material = ResourceManager_getMaterialBySignature(game->resourceManager, meshRenderer->materialSignatures[i]);
            if (material == NULL) {
                #ifdef DEBUG
                    DEBUG_LOG("MESHRENDERER::Component_MeshRenderer_Update failed to load material");
                #endif
                return;
            }
            meshRenderer->material[i] = material;
            meshRenderer->materialsLoaded += 1;
            #ifdef DEBUG
                DEBUG_LOG("MESHRENDERER::Component_MeshRenderer_Update materials loaded successfully");
            #endif
        }
    }
    if (meshRenderer->model != NULL && meshRenderer->materialsLoaded == meshRenderer->materialCount) {
        Transform* transform = GameObject_GetComponent(gameObject, game->componentPool, COMPONENT_TRANSFORM);
        if (transform == NULL) {
            DEBUG_LOG("MESHRENDERER::Component_MeshRenderer_Update transform is null");
        }

        mat4s model = Transform_getWorldMatrix(transform);
        mat4s projection = Game_getPerspective(game);
        mat4s view = Game_getView(game);

        Shader_use(meshRenderer->material[0]->shader);
        Shader_setMat4(meshRenderer->material[0]->shader, "projection", projection);
        Shader_setMat4(meshRenderer->material[0]->shader, "view", view);
        Shader_setMat4(meshRenderer->material[0]->shader, "model", model);

        Model_Draw(meshRenderer->model, meshRenderer->material, meshRenderer->materialCount);
    }
    else {
        #ifdef DEBUG
                DEBUG_LOG("MESHRENDERER::Component_MeshRenderer_Update shader is null failed to draw");
        #endif
    }
}

void GetStringNumber(unsigned int number, char* string) {
    char buffer[128] = "materialSignature";
    char numberString[5];
    sprintf(numberString,"%u",number);
    strcat(buffer,numberString);
    strcpy(string, buffer);
}

SerialObject MeshRenderer_serialize(MeshRenderer* meshRenderer) {
    SerialObject obj = SerialObject_create("MeshRenderer");

    SerialValue signatureValue = SerialValue_create_uint("modelSignature", meshRenderer->modelSignature);
    SerialObject_AddSerialValue(&obj, &signatureValue);

    SerialValue materialCountValue = SerialValue_create_uint("materialCount", meshRenderer->materialCount);
    SerialObject_AddSerialValue(&obj, &materialCountValue);

    for (int i = 0;i < meshRenderer->materialCount;i++) {
        char buffer[128];
        GetStringNumber(i, buffer);
        SerialValue materialSignatureValue = SerialValue_create_uint(buffer, meshRenderer->material[i]->signature);
        SerialObject_AddSerialValue(&obj, &materialSignatureValue);
    }

    return obj;
}

void MeshRenderer_deserialize(MeshRenderer* meshRenderer, SerialObject* serialObject) {
    SerialValue signatureValue = SerialObject_GetByName(serialObject,"modelSignature");
    meshRenderer->modelSignature = SerialValue_GetUintValue(&signatureValue);

    SerialValue materialCountValue = SerialObject_GetByName(serialObject,"materialCount");
    meshRenderer->materialCount = SerialValue_GetUintValue(&materialCountValue);

    for (int i = 0;i < meshRenderer->materialCount;i++) {
        char buffer[128];
        GetStringNumber(i, buffer);
        SerialValue materialSignatureValue = SerialObject_GetByName(serialObject,buffer);
        meshRenderer->materialSignatures[i] = SerialValue_GetUintValue(&materialSignatureValue);
    }
}