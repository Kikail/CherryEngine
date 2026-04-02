//
// Created by killian on 4/2/26.
//

#ifndef CHERRYENGINE_MATERIAL_H
#define CHERRYENGINE_MATERIAL_H

#include <cglm/struct.h>

#include "shader.h"

typedef struct Material_t{
    vec3s ambient;
    vec3s diffuse;
    vec3s specular;
    float shininess;
}Material;
Material Material_create(vec3s a, vec3s d, vec3s s, float shininess);
void Material_sendToShader(Material mat, Shader* shader);

#endif //CHERRYENGINE_MATERIAL_H
