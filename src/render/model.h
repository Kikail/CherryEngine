//
// Created by killian on 3/28/26.
//

#ifndef CHERRYENGINE_MODEL_H
#define CHERRYENGINE_MODEL_H

#include <glad/glad.h>
#include <cglm/struct.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "mesh.h"

typedef struct Model_t {
    Texture* texture_loaded;
    Mesh* meshes;
    char* directory;
    bool gammaCorrection;
    char* filepath;
}Model;

unsigned int TextureFromFile(char* path, bool gamma, vec2s* size);
Model Model_create(char* path, bool gamma);
void Model_Draw(Model* model, Shader* shader);
void Model_load(Model* model, char* path);
void Model_processNode(Model* model, struct aiNode* node, struct aiScene* scene);
Mesh Model_processMesh(Model* model, struct aiMesh* mesh, struct aiScene* scene);
Texture* Model_loadMaterialTextures(Model* model, struct aiMaterial* material, enum aiTextureType type, char* typeName);

#endif //CHERRYENGINE_MODEL_H
