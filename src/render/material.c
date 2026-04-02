//
// Created by killian on 4/2/26.
//
#include "material.h"

Material Material_create(vec3s a, vec3s d, vec3s s, float shininess) {
    Material mat;
    mat.ambient = a;
    mat.diffuse = d;
    mat.specular = s;
    mat.shininess = shininess;
    return mat;
}
void Material_sendToShader(Material mat, Shader* shader) {
    Shader_use(shader);
    Shader_setVec3(shader, "material.ambient", mat.ambient);
    Shader_setVec3(shader, "material.diffuse", mat.diffuse);
    Shader_setVec3(shader, "material.specular", mat.specular);
    Shader_setFloat(shader, "material.shininess", mat.shininess);
}