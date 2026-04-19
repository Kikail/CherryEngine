//
// Created by killian on 4/9/26.
//
#include "resourceManager.h"

#include <stdlib.h>

#include "filePicker.h"

ResourceManager* ResourceManager_create() {
    ResourceManager* resourceManager = malloc(sizeof(ResourceManager));

    resourceManager->numModels = 0;
    resourceManager->numShaders = 0;
    resourceManager->numTextures = 0;

    return resourceManager;
}

unsigned int ResourceManager_loadTexture(ResourceManager* resourceManager, const char* texturePath, bool absolutePath) {
    if (resourceManager->numTextures >= RESOURCE_MAX_TEXTURES) return 0;
    if (absolutePath) {
        resourceManager->textures[resourceManager->numTextures] = TextureFromFile(texturePath,false,NULL);
    }
    else {
        resourceManager->textures[resourceManager->numTextures] = TextureFromFile(GetPath(texturePath),false,NULL);
    }
    resourceManager->numTextures += 1;
    return resourceManager->textures[resourceManager->numTextures-1];
}
Shader* ResourceManager_loadShader(ResourceManager* resourceManager, const char* vsPath, const char* fsPath, bool absolutePath) {
    if (resourceManager->numShaders >= RESOURCE_MAX_SHADERS) return NULL;
    if (absolutePath) {
        Shader_load(&resourceManager->shaders[resourceManager->numShaders],vsPath,GetPath(fsPath));
    }
    else {
        Shader_load(&resourceManager->shaders[resourceManager->numShaders],GetPath(vsPath),GetPath(fsPath));
    }
    resourceManager->numShaders += 1;
    return &resourceManager->shaders[resourceManager->numShaders-1];
}
Model* ResourceManager_loadModel(ResourceManager* resourceManager, const char* modelPath, bool absolutePath) {
    if (resourceManager->numModels >= RESOURCE_MAX_MODELS) return NULL;
    if (absolutePath) {
        resourceManager->models[resourceManager->numModels] = Model_create(modelPath,false);
    }
    else {
        resourceManager->models[resourceManager->numModels] = Model_create(GetPath(modelPath),false);
    }
    resourceManager->numModels += 1;
    return &resourceManager->models[resourceManager->numModels-1];
}
void ResourceManager_loadAllFilesFromDirectory(ResourceManager* resourceManager, char* directory) {
    FilePicker* filePicker = FilePicker_loadFiles(directory);
    for (int i = 0; i < filePicker->numFiles; i++) {
        CherryFile file = filePicker->files[i];
        switch (file.type) {
            case FILETYPE_TXT:      break;
            case FILETYPE_PNG:      ResourceManager_loadTexture(resourceManager, file.path, true);break;
            case FILETYPE_JPG:      ResourceManager_loadTexture(resourceManager, file.path, true);break;
            case FILETYPE_SVG:      ResourceManager_loadTexture(resourceManager, file.path, true);break;
            case FILETYPE_MTL:      break;
            case FILETYPE_FS:       break;
            case FILETYPE_VS:       break;
            case FILETYPE_OBJ:      ResourceManager_loadModel(resourceManager, file.path, true);break;
            case FILETYPE_NONE:     break;
        }
    }
}