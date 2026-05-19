//
// Created by killian on 3/11/26.
//

#ifndef CHERRYENGINE_MESHRENDERER_H
#define CHERRYENGINE_MESHRENDERER_H
#include "render/model.h"
#include "../../scene/game.h"
#include "../../../render/material.h"

typedef struct GameObject_t GameObject;
typedef struct Transform_t Transform;

#define MESHRENDERER_MAX_MATERIALS 6
#define MESHRENDERER_SIGNATURE_NULL 0

typedef struct {
    unsigned int modelSignature;
    unsigned int materialSignature;
    Model* model;

    // DANS LA SERIALISATION METTRE LE MATERIAL COUNT POUR CHARGER ET SAUVEGARDER FACILEMENT
    Material* material[MESHRENDERER_MAX_MATERIALS]; unsigned int materialCount; unsigned int materialSignatures[MESHRENDERER_MAX_MATERIALS];
    unsigned int materialsLoaded;
} MeshRenderer;
void Component_MeshRenderer_Init(MeshRenderer* meshRenderer, GameObject* gameObject, Game* game);
void Component_MeshRenderer_Update(MeshRenderer* meshRenderer, GameObject* gameObject, Game* game);
SerialObject MeshRenderer_serialize(MeshRenderer* meshRenderer);
void MeshRenderer_deserialize(MeshRenderer* meshRenderer, SerialObject* serialObject);

#endif //CHERRYENGINE_MESHRENDERER_H