//
// Created by killian on 4/9/26.
//
#include "resourceManager.h"

#include <stdlib.h>

#include "filePicker.h"
#include "metaData.h"
#include "serializer.h"
#include "utils/utils.h"

ResourceManager* ResourceManager_create() {
    ResourceManager* resourceManager = malloc(sizeof(ResourceManager));

    resourceManager->numModels = 0;
    resourceManager->numShaders = 0;
    resourceManager->numTextures = 0;

    return resourceManager;
}

CherryTexture* ResourceManager_loadTexture(ResourceManager* resourceManager, MetaData* metaData, bool absolutePath) {
    if (resourceManager->numTextures >= RESOURCE_MAX_TEXTURES) {
    #ifdef DEBUG
            DEBUG_LOG("RESOURCE_MANAGER::ResourceManager_loadTexture max textures reach");
    #endif
        return NULL;
    }
    if (absolutePath) {
        resourceManager->textures[resourceManager->numTextures].id = TextureFromFile(metaData->path,false,NULL);
        resourceManager->textures[resourceManager->numTextures].signature = metaData->signature;
        ResourceManager_addResource(resourceManager, metaData->path, CHERRY_RESOURCE_TYPE_TEXTURE, resourceManager->numTextures, metaData->signature);
    }
    else {
        char path[512];
        GetPath(metaData->path,path);
        resourceManager->textures[resourceManager->numTextures].id = TextureFromFile(path,false,NULL);
        resourceManager->textures[resourceManager->numTextures].signature = metaData->signature;
        ResourceManager_addResource(resourceManager, path, CHERRY_RESOURCE_TYPE_TEXTURE, resourceManager->numTextures, metaData->signature);
    }
    resourceManager->numTextures += 1;
    return &resourceManager->textures[resourceManager->numTextures-1];
}
Shader* ResourceManager_loadShader(ResourceManager* resourceManager, const char* vsPath, const char* fsPath, MetaData* metaData, bool absolutePath) {
    if (resourceManager->numShaders >= RESOURCE_MAX_SHADERS) {
        #ifdef DEBUG
                DEBUG_LOG("RESOURCE_MANAGER::ResourceManager_loadShader max shaders reach");
        #endif
        return NULL;
    }

    if (absolutePath) {
        Shader_load(&resourceManager->shaders[resourceManager->numShaders],vsPath,fsPath);
        ResourceManager_addResource(resourceManager, vsPath, CHERRY_RESOURCE_TYPE_SHADER, resourceManager->numShaders, metaData->signature);
    }
    else {
        char fs_path[512];
        char vs_path[512];
        GetPath(vsPath,vs_path);
        GetPath(fsPath,fs_path);
        Shader_load(&resourceManager->shaders[resourceManager->numShaders],vs_path,fs_path);
        ResourceManager_addResource(resourceManager, vs_path, CHERRY_RESOURCE_TYPE_SHADER, resourceManager->numShaders, metaData->signature);
    }
    resourceManager->numShaders += 1;
    resourceManager->shaders[resourceManager->numShaders-1].signature = metaData->signature;
    return &resourceManager->shaders[resourceManager->numShaders-1];
}
Model* ResourceManager_loadModel(ResourceManager* resourceManager, MetaData* metaData, bool absolutePath) {
    if (resourceManager->numModels >= RESOURCE_MAX_MODELS) {
        #ifdef DEBUG
                DEBUG_LOG("RESOURCE_MANAGER::ResourceManager_loadModel max models reach");
        #endif
        return NULL;
    }
    if (absolutePath) {
        resourceManager->models[resourceManager->numModels] = Model_create(metaData->path,false);
        ResourceManager_addResource(resourceManager, metaData->path, CHERRY_RESOURCE_TYPE_MODEL, resourceManager->numModels, metaData->signature);
    }
    else {
        char path[512];
        GetPath(metaData->path,path);
        resourceManager->models[resourceManager->numModels] = Model_create(path,false);
        ResourceManager_addResource(resourceManager, path, CHERRY_RESOURCE_TYPE_MODEL, resourceManager->numModels, metaData->signature);
    }
    resourceManager->numModels += 1;
    resourceManager->models[resourceManager->numModels-1].signature = metaData->signature;
    return &resourceManager->models[resourceManager->numModels-1];
}
void ResourceManager_loadAllFilesFromDirectory(ResourceManager* resourceManager, char* directory) {
    // Ici on charge tout les fichiers depuis directory
    FilePicker* filePicker = FilePicker_loadFiles(directory);
    for (int i = 0; i < filePicker->numFiles; i++) {
        CherryFile file = filePicker->files[i];

        // Ici on reagit differement selon les differents types de fichier
        // Il faudrait ne pas charger les fichiers directement selon leur type, mais uniqmuement les charger avec leur fichier metadata
        // Avec les fichiers sauf metadata il faut verifier s il existe un fichier metadata

        switch (file.type) {
            case FILETYPE_TXT:      break;
            case FILETYPE_PNG:      MetaData_check(file.path, file.path, file.type);break;
            case FILETYPE_JPG:      MetaData_check(file.path, file.path, file.type);break;
            case FILETYPE_SVG:      MetaData_check(file.path, file.path, file.type);break;
            case FILETYPE_MTL:      break;
            case FILETYPE_FS:       break;
            case FILETYPE_VS:       break;
            case FILETYPE_OBJ:      MetaData_check(file.path, file.path, file.type);break;
            case FILETYPE_METADATA: ResourceManager_loadResource(resourceManager, file.path, file.type);break;
            case FILETYPE_NONE:     break;
        }
    }
#ifdef DEBUG
    ResourceManager_showResources(resourceManager);
#endif
}
void ResourceManager_loadResource(ResourceManager* resourceManager, char* path, FileType filetype) {
    FILE* f = fopen(path, "r");
    SerialObject metaDataSerialObject = SerialObject_DeserializeSingle(f);
    fclose(f);

    MetaData metaData = MetaData_load(&metaDataSerialObject);
    switch (metaData.type) {
        case FILETYPE_TXT:      break;
        case FILETYPE_PNG:      ResourceManager_loadTexture(resourceManager, &metaData, true);break;
        case FILETYPE_JPG:      ResourceManager_loadTexture(resourceManager, &metaData, true);break;
        case FILETYPE_SVG:      ResourceManager_loadTexture(resourceManager, &metaData, true);break;
        case FILETYPE_MTL:      break;
        case FILETYPE_FS:       break;
        case FILETYPE_VS:       break;
        case FILETYPE_OBJ:      ResourceManager_loadModel(resourceManager, &metaData, true);break;
        case FILETYPE_METADATA: break;
        case FILETYPE_NONE:     break;
    }
}
CherryResource* ResourceManager_addResource(ResourceManager* resourceManager, char* path, CherryResourceType type, unsigned int index, unsigned int signature) {
    strcpy(resourceManager->resources[resourceManager->numResources].path, path);
    resourceManager->resources[resourceManager->numResources].type = type;
    resourceManager->resources[resourceManager->numResources].index = index;
    resourceManager->resources[resourceManager->numResources].signature = signature;
    resourceManager->numResources += 1;
    return &resourceManager->resources[resourceManager->numResources-1];
}
void ResourceManager_showResources(ResourceManager* resourceManager) {
    for (int i = 0; i < resourceManager->numResources; i++) {
        CherryResource* resource = &resourceManager->resources[i];
        printf("resource %d: ( %s , ",i, resource->path);
        switch (resource->type) {
            case CHERRY_RESOURCE_TYPE_MODEL:printf("MODEL"); break;
            case CHERRY_RESOURCE_TYPE_SHADER:printf("SHADER"); break;
            case CHERRY_RESOURCE_TYPE_TEXTURE:printf("TEXTURE"); break;
            case CHERRY_RESOURCE_TYPE_NONE:printf("NONE"); break;
        }
        printf(" , index:%u , signature:%u )\n", resource->index, resource->signature);
    }
}
void GetPath(const char* file, char* path) {
    snprintf(path, 512, "%s/%s", RESOURCES_PATH, file);
}
CherryTexture* ResourceManager_getTextureBySignature(ResourceManager* resourceManager, unsigned int signature) {
    for (int i = 0; i < resourceManager->numTextures; i++) {
        if (resourceManager->textures[i].signature == signature) {
            return &resourceManager->textures[i];
        }
    }
    #ifdef DEBUG
        DEBUG_LOG("RESOURCE_MANAGER::ResourceManager_getTextureBySignature failed to find texture");
    #endif
    return NULL;
}
Shader* ResourceManager_getShaderBySignature(ResourceManager* resourceManager, unsigned int signature) {
    for (int i = 0; i < resourceManager->numShaders; i++) {
        if (resourceManager->shaders[i].signature == signature) {
            return &resourceManager->shaders[i];
        }
    }
#ifdef DEBUG
    DEBUG_LOG("RESOURCE_MANAGER::ResourceManager_getShaderBySignature failed to find texture");
#endif
    return NULL;
}
Model* ResourceManager_getModelBySignature(ResourceManager* resourceManager, unsigned int signature) {
    for (int i = 0; i < resourceManager->numModels; i++) {
        if (resourceManager->models[i].signature == signature) {
            return &resourceManager->models[i];
        }
    }
#ifdef DEBUG
    DEBUG_LOG("RESOURCE_MANAGER::ResourceManager_getModelBySignature failed to find texture");
#endif
    return NULL;
}