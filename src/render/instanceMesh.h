//
// Created by killian on 4/1/26.
//

#ifndef CHERRYENGINE_INSTANCEMESH_H
#define CHERRYENGINE_INSTANCEMESH_H

#include <cglm/struct.h>
#include "model.h"

#define MAX_INSTANCE_MESHES 10000

typedef struct InstanceMesh_t {
    Model* model;
    unsigned int instanceVBO;
    mat4s modelMatrices[MAX_INSTANCE_MESHES];
    int instanceCount;
}InstanceMesh;
InstanceMesh* InstanceMesh_create(Model* m);
void InstanceMesh_reset(InstanceMesh* m);
bool InstanceMesh_add(InstanceMesh* m, mat4s currentModelMatrix);
void InstanceMesh_updateGPU(InstanceMesh* m);
void InstanceMesh_draw(InstanceMesh* m);

#endif //CHERRYENGINE_INSTANCEMESH_H
