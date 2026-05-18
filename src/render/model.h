//
// Created by killian on 3/28/26.
//

#ifndef CHERRYENGINE_MODEL_H
#define CHERRYENGINE_MODEL_H

#include <cglm/struct.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "mesh.h"


#define MAX_TEXTURES_PER_MODEL 16
#include "material.h"

typedef struct Model_t {
    Texture* texture_loaded;
    unsigned int numTexturesLoaded;
    Mesh* meshes;
    unsigned int numMeshes;
    char* directory;
    bool gammaCorrection;
    char* filepath;
    unsigned int signature;
}Model;

unsigned int TextureFromFile(char* path, bool gamma, vec2s* size);
unsigned int loadCubemap(char** faces, unsigned int nbFaces);
Model Model_create(char* path, bool gamma);
void Model_Draw(Model* model, Material** materials, unsigned int materialCount);
void Model_load(Model* model, char* path);
void Model_processNode(Model* model, struct aiNode* node, struct aiScene* scene);
Mesh Model_processMesh(Model* model, struct aiMesh* mesh, struct aiScene* scene);
Texture* Model_loadMaterialTextures(Model* model, struct aiMaterial* material, enum aiTextureType type, char* typeName, unsigned int* count);

#endif //CHERRYENGINE_MODEL_H
