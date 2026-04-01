//
// Created by killian on 4/1/26.
//
#include "instanceMesh.h"

InstanceMesh* InstanceMesh_create(Model* m) {
    // On sort proprement si il n'y a pas de Model de load
    if (m == NULL) {return NULL;}

    InstanceMesh* instanceMesh = (InstanceMesh*)malloc(sizeof(InstanceMesh));
    instanceMesh->model = m;

    // Initialisation cote GPU
    glGenBuffers(1, &instanceMesh->instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceMesh->instanceVBO);
    // Pré-allocation pour 10000 matrices
    glBufferData(GL_ARRAY_BUFFER, MAX_INSTANCE_MESHES * sizeof(mat4s), NULL, GL_DYNAMIC_DRAW);

    // On configure le VAO de CHAQUE Mesh du modèle
    for (unsigned int i = 0; i < instanceMesh->model->numMeshes; i++) {
        glBindVertexArray(instanceMesh->model->meshes[i].VAO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceMesh->instanceVBO);

        // Les attributs 0 à 4 sont déjà pris par ton Vertex struct
        // La matrice mat4 utilisera les locations 5, 6, 7 et 8
        int startLocation = 5;
        for (int j = 0; j < 4; j++) {
            glEnableVertexAttribArray(startLocation + j);
            glVertexAttribPointer(startLocation + j, 4, GL_FLOAT, GL_FALSE, sizeof(mat4s), (void*)(j * sizeof(vec4s)));
            glVertexAttribDivisor(startLocation + j, 1); // IMPORTANT : Avance par instance
        }
        glBindVertexArray(0);
    }
    instanceMesh->instanceCount = 0;
    return instanceMesh;
}
void InstanceMesh_reset(InstanceMesh* m) {
    if (m == NULL) {return;}
    m->instanceCount = 0;
}
bool InstanceMesh_add(InstanceMesh* m, mat4s currentModelMatrix) {
    // On verifie si c'est possible d'ajouter un model
    if (m == NULL || m->instanceCount >= MAX_INSTANCE_MESHES) {return false;}

    m->modelMatrices[m->instanceCount] = currentModelMatrix;
    m->instanceCount += 1;
    return true;
}
void InstanceMesh_updateGPU(InstanceMesh* m) {
    if (m == NULL) {return;}
    glBindBuffer(GL_ARRAY_BUFFER, m->instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m->instanceCount * sizeof(mat4s), m->modelMatrices);
}
void InstanceMesh_draw(InstanceMesh* m) {
    if (m == NULL) {return;}
    for (unsigned int i = 0; i < m->model->numMeshes; i++) {

        // Lier la texture du modèle s'il en a une
        if(m->model->meshes[i].nbTextures > 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m->model->meshes[i].textures[0].id);
        }

        glBindVertexArray(m->model->meshes[i].VAO);
        glDrawElementsInstanced(GL_TRIANGLES, m->model->meshes[i].nbIndices, GL_UNSIGNED_INT, 0, m->instanceCount);
    }
    glBindVertexArray(0);
}