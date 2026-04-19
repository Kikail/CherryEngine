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
#include "render/model.h"
#include "render/shader.h"

static char* GetPath(const char* file) {
    size_t pathLen = strlen(RESOURCES_PATH) + 1 + strlen(file) + 1;
    char* finalPath = malloc(pathLen);
    if (finalPath == NULL) {
        perror("Erreur d'allocation mémoire");
        return NULL;
    }
    snprintf(finalPath, pathLen, "%s/%s", RESOURCES_PATH, file);
    return finalPath;
}




typedef enum CherryResourceType_t {
    CHERRY_RESOURCE_TYPE_NONE,
    CHERRY_RESOURCE_TYPE_TEXTURE,
    CHERRY_RESOURCE_TYPE_SHADER,
    CHERRY_RESOURCE_TYPE_MODEL
}CherryResourceType_t;

typedef struct CherryResource_t {
    char path[CHERRY_MAX_FILEPATH_LENGTH];
    CherryResourceType_t type;
    unsigned int index;
}CherryResource_t;
// Cette structure va nous permettre de debug les noms des fichiers charger et de garder ume structure propre
// dans notre projet, Nous pouvons acceder au type et au chemin dans l ordinateur de chaque ressource chargee
// A IMPLEMENTER





typedef struct ResourceManager_t {
    unsigned int textures[RESOURCE_MAX_TEXTURES]; unsigned int numTextures;
    Shader shaders[RESOURCE_MAX_SHADERS]; unsigned int numShaders;
    Model models[RESOURCE_MAX_MODELS]; unsigned int numModels;
}ResourceManager;
ResourceManager* ResourceManager_create();
void ResourceManager_loadAllFilesFromDirectory(ResourceManager* resourceManager, char* directory);
unsigned int ResourceManager_loadTexture(ResourceManager* resourceManager, const char* texturePath, bool absolutePath);
Shader* ResourceManager_loadShader(ResourceManager* resourceManager, const char* vsPath, const char* fsPath, bool absolutePath);
Model* ResourceManager_loadModel(ResourceManager* resourceManager, const char* modelPath, bool absolutePath);

#endif //CHERRYENGINE_RESOURCEMANAGER_H
