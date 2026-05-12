//
// Created by killian on 4/9/26.
//
#include "resourceManager.h"

#include <stdlib.h>

#include "filePicker.h"
#include "metaData.h"

ResourceManager* ResourceManager_create() {
    ResourceManager* resourceManager = malloc(sizeof(ResourceManager));

    resourceManager->numModels = 0;
    resourceManager->numShaders = 0;
    resourceManager->numTextures = 0;

    return resourceManager;
}

unsigned int ResourceManager_loadTexture(ResourceManager* resourceManager, const char* texturePath, bool absolutePath) {
    if (resourceManager->numTextures >= RESOURCE_MAX_TEXTURES) {
    #ifdef DEBUG
            DEBUG_LOG("RESOURCE_MANAGER::ResourceManager_loadTexture max textures reach");
    #endif
        return 0;
    }
    if (absolutePath) {
        resourceManager->textures[resourceManager->numTextures] = TextureFromFile(texturePath,false,NULL);
        ResourceManager_addResource(resourceManager, texturePath, CHERRY_RESOURCE_TYPE_TEXTURE, resourceManager->numTextures);
    }
    else {
        resourceManager->textures[resourceManager->numTextures] = TextureFromFile(GetPath(texturePath),false,NULL);
        ResourceManager_addResource(resourceManager, GetPath(texturePath), CHERRY_RESOURCE_TYPE_TEXTURE, resourceManager->numTextures);
    }
    resourceManager->numTextures += 1;
    return resourceManager->textures[resourceManager->numTextures-1];
}
Shader* ResourceManager_loadShader(ResourceManager* resourceManager, const char* vsPath, const char* fsPath, bool absolutePath) {
    if (resourceManager->numShaders >= RESOURCE_MAX_SHADERS) {
        #ifdef DEBUG
                DEBUG_LOG("RESOURCE_MANAGER::ResourceManager_loadShader max shaders reach");
        #endif
        return NULL;
    }
    if (absolutePath) {
        Shader_load(&resourceManager->shaders[resourceManager->numShaders],vsPath,fsPath);
        ResourceManager_addResource(resourceManager, vsPath, CHERRY_RESOURCE_TYPE_SHADER, resourceManager->numShaders);
    }
    else {
        Shader_load(&resourceManager->shaders[resourceManager->numShaders],GetPath(vsPath),GetPath(fsPath));
        ResourceManager_addResource(resourceManager, GetPath(vsPath), CHERRY_RESOURCE_TYPE_SHADER, resourceManager->numShaders);
    }
    resourceManager->numShaders += 1;
    return &resourceManager->shaders[resourceManager->numShaders-1];
}
Model* ResourceManager_loadModel(ResourceManager* resourceManager, const char* modelPath, bool absolutePath) {
    if (resourceManager->numModels >= RESOURCE_MAX_MODELS) {
        #ifdef DEBUG
                DEBUG_LOG("RESOURCE_MANAGER::ResourceManager_loadModel max models reach");
        #endif
        return NULL;
    }
    if (absolutePath) {
        resourceManager->models[resourceManager->numModels] = Model_create(modelPath,false);
        ResourceManager_addResource(resourceManager, modelPath, CHERRY_RESOURCE_TYPE_MODEL, resourceManager->numModels);
    }
    else {
        resourceManager->models[resourceManager->numModels] = Model_create(GetPath(modelPath),false);
        ResourceManager_addResource(resourceManager, GetPath(modelPath), CHERRY_RESOURCE_TYPE_MODEL, resourceManager->numModels);
    }
    resourceManager->numModels += 1;
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
            case FILETYPE_PNG:      ResourceManager_loadTexture(resourceManager, file.path, true); MetaData_check(file.path, file.path, file.type);break;
            case FILETYPE_JPG:      ResourceManager_loadTexture(resourceManager, file.path, true); MetaData_check(file.path, file.path, file.type);break;
            case FILETYPE_SVG:      ResourceManager_loadTexture(resourceManager, file.path, true); MetaData_check(file.path, file.path, file.type);break;
            case FILETYPE_MTL:      break;
            case FILETYPE_FS:       break;
            case FILETYPE_VS:       break;
            case FILETYPE_OBJ:      ResourceManager_loadModel(resourceManager, file.path, true); MetaData_check(file.path, file.path, file.type);break;
            case FILETYPE_METADATA: break;
            case FILETYPE_NONE:     break;
        }
    }
}
CherryResource* ResourceManager_addResource(ResourceManager* resourceManager, char* path, CherryResourceType type, unsigned int index) {
    strcpy(resourceManager->resources[resourceManager->numResources].path, path);
    resourceManager->resources[resourceManager->numResources].type = type;
    resourceManager->resources[resourceManager->numResources].index = index;
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
        printf(" , index:%d )\n", resource->index);
    }
}