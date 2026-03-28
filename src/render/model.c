//
// Created by killian on 3/28/26.
//
#include "model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

unsigned int TextureFromFile(char* path, bool gamma, vec2s* size) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    stbi_set_flip_vertically_on_load(true);

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

        printf("Texture loaded successfully at path: %s\n", path);

        glBindTexture(GL_TEXTURE_2D, textureID);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Force l'alignement sur 1 octet (indispensable pour le RGB)
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // CORRECTIONS DES PARAMÈTRES :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // <- Corrigé
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // <- Déplacé ici pour utiliser les Mipmaps
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }

    return textureID; // <- L'OUBLI ÉTAIT ICI !
}

Model Model_create(char* path, bool gamma);
void Model_Draw(Model* model, Shader* shader);
void Model_load(Model* model, char* path);
void Model_processNode(Model* model, struct aiNode* node, struct aiScene* scene);
Mesh Model_processMesh(Model* model, struct aiMesh* mesh, struct aiScene* scene);
Texture* Model_loadMaterialTextures(Model* model, struct aiMaterial* material, enum aiTextureType type, char* typeName);