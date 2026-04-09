//
// Created by killian on 4/9/26.
//
#include "resourceManager.h"

#include <stdlib.h>

ResourceManager* ResourceManager_create() {
    ResourceManager* resourceManager = malloc(sizeof(ResourceManager));

    resourceManager->numModels = 0;
    resourceManager->numShaders = 0;
    resourceManager->numTextures = 0;

    return resourceManager;
}

unsigned int ResourceManager_loadTexture(ResourceManager* resourceManager, const char* texturePath) {
    if (resourceManager->numTextures >= RESOURCE_MAX_TEXTURES) return 0;
    resourceManager->textures[resourceManager->numTextures] = TextureFromFile(GetPath(texturePath),false,NULL);
    resourceManager->numTextures += 1;
    return resourceManager->textures[resourceManager->numTextures-1];
}
Shader* ResourceManager_loadShader(ResourceManager* resourceManager, const char* vsPath, const char* fsPath) {
    if (resourceManager->numShaders >= RESOURCE_MAX_SHADERS) return NULL;
    Shader_load(&resourceManager->shaders[resourceManager->numShaders],GetPath(vsPath),GetPath(fsPath));
    resourceManager->numShaders += 1;
    return &resourceManager->shaders[resourceManager->numShaders-1];
}
Model* ResourceManager_loadModel(ResourceManager* resourceManager, const char* modelPath) {
    if (resourceManager->numModels >= RESOURCE_MAX_MODELS) return NULL;
    resourceManager->models[resourceManager->numModels] = Model_create(GetPath(modelPath),false);
    resourceManager->numModels += 1;
    return &resourceManager->models[resourceManager->numModels-1];
}