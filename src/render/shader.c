//
// Created by killian on 3/15/26.
//

#include "shader.h"

#include <string.h>
#include <glad/glad.h>

#define BUFFER_SIZE 2048

char* loadAndFillBuffer(char* path) {
    int i = 0;
    char* buffer = malloc(sizeof(char) * BUFFER_SIZE);
    char ch;
    FILE *fptr = fopen(path, "r");
    if (!fptr) {
        printf("Unable to open file %s\n",path);
        return NULL;
    }
    while ((ch = fgetc(fptr)) != EOF && i < BUFFER_SIZE) {
        buffer[i] = ch;
        i++;
    }
    buffer[i] = '\0';
    fclose(fptr);
    return buffer;
}

Shader Shader_load(char* vs, char* fs) {
    char* vShaderCode = loadAndFillBuffer(vs);
    char* fShaderCode = loadAndFillBuffer(fs);
    if (vShaderCode != NULL && fShaderCode != NULL) {
        Shader shader;
        strcpy(shader.vertexPath, vs);
        strcpy(shader.fragmentPath, fs);

        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vShaderCode, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED : %s\n",infoLog);
        }
        // fragment shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED : %s\n",infoLog);
        }

        // link shaders
        shader.shaderID = glCreateProgram();
        glAttachShader(shader.shaderID, vertexShader);
        glAttachShader(shader.shaderID, fragmentShader);
        glLinkProgram(shader.shaderID);
        // check for linking errors
        glGetProgramiv(shader.shaderID, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader.shaderID, 512, NULL, infoLog);
            printf("ERROR::SHADER::PROGRAM::LINKING_FAILED : %s\n", infoLog);
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    else {
        printf("Unable to load shaders\n");
    }
}
void Shader_use(Shader* shader){
    glUseProgram(shader->shaderID);
}
void Shader_clean(Shader* shader){
    glDeleteProgram(shader->shaderID);
}

void Shader_setBool(Shader* shader, char* name, bool value){
    glUniform1i(glGetUniformLocation(shader->shaderID, name), value);
}

void Shader_setInt(Shader* shader, char* name, int value) {
    glUniform1i(glGetUniformLocation(shader->shaderID, name), value);
}

void Shader_setFloat(Shader* shader, char* name, float value) {
    glUniform1f(glGetUniformLocation(shader->shaderID, name), value);
}

void Shader_setVec2(Shader* shader, char* name, vec2s value) {
    glUniform2fv(glGetUniformLocation(shader->shaderID, name), 1, value.raw);
}

void Shader_setVec3(Shader* shader, char* name, vec3s value) {
    glUniform3fv(glGetUniformLocation(shader->shaderID, name), 1, value.raw);
}

void Shader_setVec4(Shader* shader, char* name, vec4s value) {
    glUniform4fv(glGetUniformLocation(shader->shaderID, name), 1, value.raw);
}

void Shader_setMat2(Shader* shader, char* name, mat2s value) {
    glUniformMatrix2fv(glGetUniformLocation(shader->shaderID, name), 1, GL_FALSE, value.raw[0]);
}

void Shader_setMat3(Shader* shader, char* name, mat3s value) {
    glUniformMatrix3fv(glGetUniformLocation(shader->shaderID, name), 1, GL_FALSE, value.raw[0]);
}

void Shader_setMat4(Shader* shader, char* name, mat4s value) {
    glUniformMatrix4fv(glGetUniformLocation(shader->shaderID, name), 1, GL_FALSE, value.raw[0]);
}