//
// Created by killian on 3/28/26.
//
#include "mesh.h"

#include <string.h>
#include "material.h"

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

    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, biTangent));

    glBindVertexArray(0); // Bonne pratique de délier
}

void Mesh_draw(Mesh* mesh, Material* material) {
    Material_sendToShader(material, material->shader);
    glBindVertexArray(mesh->VAO);
    glDrawElements(GL_TRIANGLES, mesh->nbIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}


































