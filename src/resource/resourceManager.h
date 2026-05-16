//
// Created by killian on 4/9/26.
//

#ifndef CHERRYENGINE_RESOURCEMANAGER_H
#define CHERRYENGINE_RESOURCEMANAGER_H

#define RESOURCE_MAX_TEXTURES 64
#define RESOURCE_MAX_SHADERS 8
#define RESOURCE_MAX_MODELS 32
#include <stdio.h>

#include "filePicker.h"
#include "metaData.h"
#include "render/model.h"
#include "render/shader.h"
#include "render/texture.h"

#define CHERRY_RESOURCE_SIGNATURE_NULL 4294967295

void GetPath(const char* file, char* path);

typedef enum CherryResourceType_t {
    CHERRY_RESOURCE_TYPE_NONE,
    CHERRY_RESOURCE_TYPE_TEXTURE,
    CHERRY_RESOURCE_TYPE_SHADER,
    CHERRY_RESOURCE_TYPE_MODEL
}CherryResourceType;

typedef struct CherryResource_t {
    char path[CHERRY_MAX_FILEPATH_LENGTH];
    CherryResourceType type;
    unsigned int index;
    unsigned int signature;
}CherryResource;

typedef struct ResourceManager_t {
    CherryTexture textures[RESOURCE_MAX_TEXTURES]; unsigned int numTextures;
    Shader shaders[RESOURCE_MAX_SHADERS]; unsigned int numShaders;
    Model models[RESOURCE_MAX_MODELS]; unsigned int numModels;
    CherryResource resources[RESOURCE_MAX_TEXTURES + RESOURCE_MAX_SHADERS + RESOURCE_MAX_MODELS];
    unsigned int numResources;
}ResourceManager;
ResourceManager* ResourceManager_create();
void ResourceManager_loadAllFilesFromDirectory(ResourceManager* resourceManager, char* directory);
CherryTexture* ResourceManager_loadTexture(ResourceManager* resourceManager, MetaData* metaData, bool absolutePath);
Shader* ResourceManager_loadShader(ResourceManager* resourceManager, MetaData* metaData, bool absolutePath);
Model* ResourceManager_loadModel(ResourceManager* resourceManager, MetaData* metaData, bool absolutePath);
CherryResource* ResourceManager_addResource(ResourceManager* resourceManager, char* path, CherryResourceType type, unsigned int index, unsigned int signature);
void ResourceManager_showResources(ResourceManager* resourceManager);
void ResourceManager_loadResource(ResourceManager* resourceManager, char* path, FileType filetype);

CherryTexture* ResourceManager_getTextureBySignature(ResourceManager* resourceManager, unsigned int signature);
Shader* ResourceManager_getShaderBySignature(ResourceManager* resourceManager, unsigned int signature);
Model* ResourceManager_getModelBySignature(ResourceManager* resourceManager, unsigned int signature);

#endif //CHERRYENGINE_RESOURCEMANAGER_H
