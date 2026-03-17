//
// Created by killian on 3/15/26.
//

#ifndef CHERRYENGINE_SHADER_H
#define CHERRYENGINE_SHADER_H

#include <stdbool.h>
#include <cglm/struct.h>

typedef struct Shader_t {
    unsigned int shaderID;
    char vertexPath[128];
    char fragmentPath[128];
}Shader;

bool Shader_load(Shader* shader, char* vs, char* fs);
void Shader_use(Shader* shader);
void Shader_clean(Shader* shader);

void Shader_setBool(Shader* shader, char* name, bool value);
void Shader_setInt(Shader* shader, char* name, int value);
void Shader_setFloat(Shader* shader, char* name, float value);
void Shader_setVec2(Shader* shader, char* name, vec2s value);
void Shader_setVec3(Shader* shader, char* name, vec3s value);
void Shader_setVec4(Shader* shader, char* name, vec4s value);
void Shader_setMat2(Shader* shader, char* name, mat2s value);
void Shader_setMat3(Shader* shader, char* name, mat3s value);
void Shader_setMat4(Shader* shader, char* name, mat4s value);

#endif //CHERRYENGINE_SHADER_H