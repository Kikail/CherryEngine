//
// Created by killian on 3/28/26.
//

#ifndef CHERRYENGINE_MESH_H
#define CHERRYENGINE_MESH_H

#include "glad/glad.h"
#include <cglm/struct.h>
#include "shader.h"

typedef struct Material_t Material;

typedef struct Vertex_t {
    vec3s position;
    vec3s normal;
    vec2s texCoords;
    vec3s tangent;
    vec3s biTangent;
}Vertex;

typedef struct Texture_t {
    unsigned int id;
    char* type;
    char* path;
}Texture;

typedef struct Mesh_t {
    Vertex* vertices;
    Texture* textures;
    unsigned int* indices;
    unsigned int VAO, VBO, EBO;
    unsigned int nbVertices;
    unsigned int nbTextures;
    unsigned int nbIndices;
}Mesh;

Mesh Mesh_create(Vertex* vertices, unsigned int nbVertices, unsigned int* indices, unsigned int nbIndices, Texture* textures, unsigned int nbTextures);
void Mesh_setup(Mesh* mesh);
void Mesh_draw(Mesh* mesh, Material* material);

#endif //CHERRYENGINE_MESH_H
