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
    if (meshRenderer->shader == NULL) {
        #ifdef DEBUG
                DEBUG_LOG("MESHRENDERER::Component_MeshRenderer_Update shader is null trying to load");
        #endif
        Shader* shader = ResourceManager_getShaderBySignature(Game_getResourceManager(game), meshRenderer->shaderSignature);
        if (shader == NULL) {
            #ifdef DEBUG
                        DEBUG_LOG("MESHRENDERER::Component_MeshRenderer_Update shader is null failed to load");
            #endif
            return;
        }
        else {
            #ifdef DEBUG
                        DEBUG_LOG("MESHRENDERER::Component_MeshRenderer_Update shader loaded successfully");
            #endif
            meshRenderer->shader = shader;
        }
    }
    if (meshRenderer->model != NULL && meshRenderer->shader != NULL) {
        Transform* transform = GameObject_GetComponent(gameObject, game->componentPool, COMPONENT_TRANSFORM);
        if (transform == NULL) {
            DEBUG_LOG("MESHRENDERER::Component_MeshRenderer_Update transform is null");
        }

        mat4s model = Transform_getWorldMatrix(transform);
        mat4s projection = Game_getPerspective(game);
        mat4s view = Game_getView(game);

        Shader_use(meshRenderer->shader);
        Shader_setMat4(meshRenderer->shader, "projection", projection);
        Shader_setMat4(meshRenderer->shader, "view", view);
        Shader_setMat4(meshRenderer->shader, "model", model);

        Model_Draw(meshRenderer->model, meshRenderer->shader);
    }
    else {
        #ifdef DEBUG
                DEBUG_LOG("MESHRENDERER::Component_MeshRenderer_Update shader is null failed to draw");
        #endif
    }
}
SerialObject MeshRenderer_serialize(MeshRenderer* meshRenderer) {
    SerialObject obj = SerialObject_create("MeshRenderer");

    SerialValue signatureValue = SerialValue_create_uint("modelSignature", meshRenderer->modelSignature);
    SerialObject_AddSerialValue(&obj, &signatureValue);

    SerialValue shaderValue = SerialValue_create_uint("shaderSignature", meshRenderer->shaderSignature);
    SerialObject_AddSerialValue(&obj, &shaderValue);

    return obj;
}

void MeshRenderer_deserialize(MeshRenderer* meshRenderer, SerialObject* serialObject) {
    SerialValue signatureValue = SerialObject_GetByName(serialObject,"modelSignature");
    meshRenderer->modelSignature = SerialValue_GetUintValue(&signatureValue);

    SerialValue shaderValue = SerialObject_GetByName(serialObject,"shaderSignature");
    meshRenderer->shaderSignature = SerialValue_GetUintValue(&shaderValue);
}