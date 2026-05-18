//
// Created by killian on 3/28/26.
//
#include "model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "material.h"
#include "glad/glad.h"
#include "utils/utils.h"

unsigned int TextureFromFile(char* path, bool gamma, vec2s* size) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    stbi_set_flip_vertically_on_load(false);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);

    if (data == NULL) {
        printf("Failed to load texture at path: %s\n", path);
        stbi_image_free(data);
        exit(-1);
    }
    else {
        GLenum format = GL_RGBA; // Valeur par défaut de sécurité
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 2) format = GL_RG;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        //printf("Texture loaded successfully at path: %s\n", path);

        glBindTexture(GL_TEXTURE_2D, textureID);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Force l'alignement sur 1 octet (indispensable pour le RGB)

        // CORRIGÉ : Suppression de la ligne dupliquée glTexImage2D
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }

    return textureID;
}

unsigned int loadCubemap(char** faces, unsigned int nbFaces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < nbFaces; i++)
    {
        unsigned char *data = stbi_load(faces[i], &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            printf("Cubemap tex failed to load at path: %s\n",faces[i]);
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


Model Model_create(char* path, bool gamma) {
    Model model = {0};
    model.gammaCorrection = gamma;
    model.filepath = strdup(path);
    Model_load(&model, path);
    return model;
}

void Model_Draw(Model* model, Material** materials, unsigned int materialCount) {
    for (unsigned int i = 0; i < model->numMeshes; i++) {
        if (i >= materialCount) {
            #ifdef DEBUG
                DEBUG_LOG("MODEL::Model_Draw more meshes than materials");
            #endif
            return;
        }
        Mesh_draw(&model->meshes[i], materials[i]);
    }
}

void Model_load(Model* model, char* path) {
    const struct aiScene* scene = aiImportFile(path,
        aiProcess_Triangulate | aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        printf("ERROR::ASSIMP:: %s\n", aiGetErrorString());
        return;
    }

    char* lastSlash = strrchr(path, '/');
    if (lastSlash) {
        int len = lastSlash - path;
        model->directory = malloc(len + 1);
        strncpy(model->directory, path, len);
        model->directory[len] = '\0';
    } else {
        model->directory = strdup(".");
    }

    Model_processNode(model, scene->mRootNode, scene);

    aiReleaseImport(scene);
}

void Model_processNode(Model* model, struct aiNode* node, struct aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        struct aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        model->meshes = realloc(model->meshes, sizeof(Mesh) * (model->numMeshes + 1));
        model->meshes[model->numMeshes] = Model_processMesh(model, mesh, scene);
        model->numMeshes++;
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        Model_processNode(model, node->mChildren[i], scene);
    }
}

Mesh Model_processMesh(Model* model, struct aiMesh* mesh, struct aiScene* scene) {
    Vertex* vertices = malloc(sizeof(Vertex) * mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.position = (vec3s){mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};

        if (mesh->mNormals != NULL) {
            vertex.normal = (vec3s){mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
        } else {
            vertex.normal = (vec3s){0.0f, 0.0f, 0.0f};
        }

        if (mesh->mTextureCoords[0]) {
            vertex.texCoords = (vec2s){mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};

            // Sécurité supplémentaire : Assimp ne génère pas de tangentes si ça plante ou s'il manque des normales
            if (mesh->mTangents != NULL && mesh->mBitangents != NULL) {
                vertex.tangent = (vec3s){mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
                vertex.biTangent = (vec3s){mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};
            } else {
                vertex.tangent = (vec3s){0.0f, 0.0f, 0.0f};
                vertex.biTangent = (vec3s){0.0f, 0.0f, 0.0f};
            }
        } else {
            vertex.texCoords = (vec2s){0.0f, 0.0f};
            vertex.tangent = (vec3s){0.0f, 0.0f, 0.0f};
            vertex.biTangent = (vec3s){0.0f, 0.0f, 0.0f};
        }
        vertices[i] = vertex;
    }

    unsigned int numIndices = 0;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        numIndices += mesh->mFaces[i].mNumIndices;
    }

    unsigned int* indices = malloc(sizeof(unsigned int) * numIndices);
    unsigned int counter = 0;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        struct aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices[counter++] = face.mIndices[j];
        }
    }

    unsigned int totalMeshTextures = 0;
    Texture* textures = NULL;
    struct aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    // CORRIGÉ : enum aiTextureType au lieu de struct aiTextureType
    enum aiTextureType types[] = { aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_HEIGHT, aiTextureType_AMBIENT };
    char* typeNames[] = { "texture_diffuse", "texture_specular", "texture_normal", "texture_height" };

    for (int t = 0; t < 4; t++) {
        unsigned int count = 0;
        Texture* loadedTexs = Model_loadMaterialTextures(model, material, types[t], typeNames[t], &count);
        if (count > 0) {
            textures = realloc(textures, sizeof(Texture) * (totalMeshTextures + count));
            memcpy(textures + totalMeshTextures, loadedTexs, sizeof(Texture) * count);
            totalMeshTextures += count;
            free(loadedTexs);
        }
    }

    return Mesh_create(vertices, mesh->mNumVertices, indices, numIndices, textures, totalMeshTextures);
}

Texture* Model_loadMaterialTextures(Model* model, struct aiMaterial* mat, enum aiTextureType type, char* typeName, unsigned int* outCount) {
    unsigned int count = aiGetMaterialTextureCount(mat, type);
    if (count == 0) {
        *outCount = 0;
        return NULL;
    }

    Texture* textures = malloc(sizeof(Texture) * count);
    unsigned int loadedCount = 0;

    for (unsigned int i = 0; i < count; i++) {
        struct aiString str;
        aiGetMaterialTexture(mat, type, i, &str, NULL, NULL, NULL, NULL, NULL, NULL);

        bool skip = false;
        for (unsigned int j = 0; j < model->numTexturesLoaded; j++) {
            if (strcmp(model->texture_loaded[j].path, str.data) == 0) {
                textures[loadedCount++] = model->texture_loaded[j];
                skip = true;
                break;
            }
        }

        if (!skip) {
            Texture texture;
            char fullPath[1024];
            snprintf(fullPath, sizeof(fullPath), "%s/%s", model->directory, str.data);

            texture.id = TextureFromFile(fullPath, model->gammaCorrection, NULL);
            texture.type = strdup(typeName);
            texture.path = strdup(str.data);

            textures[loadedCount++] = texture;

            model->texture_loaded = realloc(model->texture_loaded, sizeof(Texture) * (model->numTexturesLoaded + 1));
            model->texture_loaded[model->numTexturesLoaded] = texture;
            model->numTexturesLoaded++;
        }
    }
    *outCount = loadedCount;
    return textures;
}