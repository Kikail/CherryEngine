//
// Created by killian on 3/28/26.
//
#include "mesh.h"

#include <string.h>

Mesh Mesh_create(Vertex* vertices, unsigned int nbVertices, unsigned int* indices, unsigned int nbIndices, Texture* textures, unsigned int nbTextures) {
    Mesh mesh;
    mesh.vertices = vertices;
    mesh.nbVertices = nbVertices;
    mesh.indices = indices;
    mesh.nbIndices = nbIndices;
    mesh.textures = textures;
    mesh.nbTextures = nbTextures;
    Mesh_setup(&mesh);
    return mesh;
}

void Mesh_setup(Mesh* mesh) {
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);

    glBindVertexArray(mesh->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->nbVertices * sizeof(Vertex), mesh->vertices, GL_STATIC_DRAW);

    // FIX : Utiliser nbIndices ici, pas nbVertices !
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->nbIndices * sizeof(unsigned int), mesh->indices, GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Normale
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // UVs
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0); // Bonne pratique de délier
}

void Mesh_draw(Mesh* mesh, Shader* shader) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;

    for (unsigned int i = 0; i < mesh->nbTextures; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        char number[64] = "";
        char* name = mesh->textures[i].type;
        if (strcmp(name,"texture_diffuse") == 0) {
            sprintf(number, "%d", diffuseNr++);
        }
        else if (strcmp(name,"texture_specular") == 0) {
            sprintf(number, "%d", specularNr++);
        }
        else if (strcmp(name,"texture_normal") == 0) {
            sprintf(number, "%d", normalNr++);
        }
        else if (strcmp(name,"texture_height") == 0) {
            sprintf(number, "%d", heightNr++);
        }
        char completeName[128];
        strcpy(completeName, name);
        strcat(completeName, number);
        glUniform1i(glGetUniformLocation(shader->shaderID, completeName), i);
        glBindTexture(GL_TEXTURE_2D, mesh->textures[i].id);
    }

    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, mesh->nbIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}


































