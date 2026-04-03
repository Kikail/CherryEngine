//
// Created by killian on 4/2/26.
//
#include "material.h"

Material Material_create(vec3s a, vec3s d, vec3s s, float shininess, float aoIntensity, float displacementIntensity, Shader* shader) {
    Shader_use(shader);
    Shader_setInt(shader, "material.diffuseTexture", 0);
    Shader_setInt(shader, "material.normalTexture", 1);
    Shader_setInt(shader, "material.specularTexture", 2);
    Shader_setInt(shader, "material.aoTexture", 3);
    Shader_setInt(shader, "material.displacementTexture", 4);

    Material mat;
    mat.ambient = a;
    mat.diffuse = d;
    mat.specular = s;
    mat.aoIntensity = aoIntensity;
    mat.shininess = shininess;
    mat.displacementIntensity = displacementIntensity;
    mat.usingDiffuseTexture = false;
    mat.usingSpecularTexture = false;
    mat.usingNormalTexture = false;
    mat.usingAOTexture = false;
    mat.usingDisplacementTexture = false;

    return mat;
}
void Material_attachDiffuseTexture(Material *material, unsigned int diffuseTexture) {
    material->diffuseTexture = diffuseTexture;
    material->usingDiffuseTexture = true;
}
void Material_attachSpecularTexture(Material *material, unsigned int specularTexture) {
    material->specularTexture = specularTexture;
    material->usingSpecularTexture = true;
}
void Material_attachNormalTexture(Material *material, unsigned int normalTexture) {
    material->normalTexture = normalTexture;
    material->usingNormalTexture = true;
}
void Material_attachAoTexture(Material *material, unsigned int aoTexture) {
    material->aoTexture = aoTexture;
    material->usingAOTexture = true;
}
void Material_attachDisplacementTexture(Material *material, unsigned int displacementTexture) {
    material->displacementTexture = displacementTexture;
    material->usingDisplacementTexture = true;
}
void Material_sendToShader(Material* mat, Shader* shader) {
    Shader_use(shader);
    Shader_setVec3(shader, "material.ambient", mat->ambient);
    Shader_setVec3(shader, "material.diffuse", mat->diffuse);
    Shader_setVec3(shader, "material.specular", mat->specular);
    Shader_setFloat(shader, "material.shininess", mat->shininess);
    Shader_setFloat(shader, "material.aoIntensity", mat->aoIntensity);
    Shader_setFloat(shader, "material.displacementIntensity", mat->displacementIntensity);

    Shader_setBool(shader, "material.usingDiffuseTexture", mat->usingDiffuseTexture);
    if (mat->usingDiffuseTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mat->diffuseTexture);
    }
    Shader_setBool(shader, "material.usingNormalTexture", mat->usingNormalTexture);
    if (mat->usingNormalTexture) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mat->normalTexture);
    }
    Shader_setBool(shader, "material.usingSpecularTexture", mat->usingSpecularTexture);
    if (mat->usingSpecularTexture) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, mat->specularTexture);
    }
    Shader_setBool(shader, "material.usingAOTexture", mat->usingAOTexture);
    if (mat->usingAOTexture) {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, mat->aoTexture);
    }
    Shader_setBool(shader, "material.usingDisplacementTexture", mat->usingDisplacementTexture);
    if (mat->usingDisplacementTexture) {
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, mat->displacementTexture);
    }
}