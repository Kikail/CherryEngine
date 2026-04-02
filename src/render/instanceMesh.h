//
// Created by killian on 4/1/26.
//

#ifndef CHERRYENGINE_INSTANCEMESH_H
#define CHERRYENGINE_INSTANCEMESH_H

#include <cglm/struct.h>
#include "model.h"

#define MAX_INSTANCE_MESHES 300000

/**
 * Pour utiliser cette structure vous aurez besoin d'un Model
 * Ensuite creez cette structure avec la fonction create
 * Avant chaque frame utilisez reset afin de reinitialiser les donnees
 * Une fois ceci fait ajoutez toutes les matrices de model que vous voulez via add
 * Avant d'utiliser votre shader faites un updateGPU pour mettre toutes les matrices dans la memoir du GPU
 * Et enfin utilisez draw pour afficher tout les models apres utilisation de votre shader
 */
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
