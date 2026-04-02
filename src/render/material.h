//
// Created by killian on 4/2/26.
//

#ifndef CHERRYENGINE_MATERIAL_H
#define CHERRYENGINE_MATERIAL_H

#include <cglm/struct.h>

#include "mesh.h"
#include "shader.h"

typedef struct Material_t{
    bool usingDiffuseTexture;
    bool usingSpecularTexture;
    bool usingNormalTexture;

    unsigned int diffuseTexture;
    unsigned int normalTexture;
    unsigned int specularTexture;

    vec3s ambient;
    vec3s diffuse;
    vec3s specular;
    float shininess;
}Material;
Material Material_create(vec3s a, vec3s d, vec3s s, float shininess, Shader* shader);
void Material_attachDiffuseTexture(Material *material, unsigned int diffuseTexture);
void Material_attachSpecularTexture(Material *material, unsigned int specularTexture);
void Material_attachNormalTexture(Material *material, unsigned int normalTexture);
void Material_sendToShader(Material* mat, Shader* shader);

#endif //CHERRYENGINE_MATERIAL_H
