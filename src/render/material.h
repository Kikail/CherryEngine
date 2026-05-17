//
// Created by killian on 4/2/26.
//

#ifndef CHERRYENGINE_MATERIAL_H
#define CHERRYENGINE_MATERIAL_H

#include "glad/glad.h"
#include <cglm/struct.h>
#include "mesh.h"
#include "shader.h"
#include "resource/metaData.h"


typedef struct Material_t{
    bool usingDiffuseTexture;
    bool usingSpecularTexture;
    bool usingNormalTexture;
    bool usingAOTexture;
    bool usingDisplacementTexture;

    unsigned int diffuseTexture; unsigned int diffuseTextureSignature;
    unsigned int normalTexture; unsigned int normalTextureSignature;
    unsigned int specularTexture; unsigned int specularTextureSignature;
    unsigned int aoTexture; unsigned int aoTextureSignature;
    unsigned int displacementTexture; unsigned int displacementTextureSignature;

    vec3s ambient;
    vec3s diffuse;
    vec3s specular;
    float aoIntensity;
    float shininess;
    float displacementIntensity;
    float reflectionIntensity;

    unsigned int signature;
}Material;
Material Material_create(vec3s a, vec3s d, vec3s s, float shininess, float aoIntensity, float displacementIntensity, float reflectionIntensity, Shader* shader);
void Material_attachDiffuseTexture(Material *material, unsigned int diffuseTexture);
void Material_attachSpecularTexture(Material *material, unsigned int specularTexture);
void Material_attachNormalTexture(Material *material, unsigned int normalTexture);
void Material_attachAoTexture(Material *material, unsigned int aoTexture);
void Material_attachDisplacementTexture(Material *material, unsigned int displacementTexture);
void Material_sendToShader(Material* mat, Shader* shader);

//bool Material_loadTextures(Material* material, ResourceManager* res);
Material Material_loadFromFile(char* path);
SerialObject Material_serialize(Material* material);
Material Material_deserialize(SerialObject* serialObject);

#endif //CHERRYENGINE_MATERIAL_H
