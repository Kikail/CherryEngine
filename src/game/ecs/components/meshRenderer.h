//
// Created by killian on 3/11/26.
//

#ifndef CHERRYENGINE_MESHRENDERER_H
#define CHERRYENGINE_MESHRENDERER_H
#include "render/model.h"
#include "../../scene/game.h"

typedef struct GameObject_t GameObject;
typedef struct Transform_t Transform;

typedef struct {
    unsigned int signature;
    Model* model;
    Shader* shader;
} MeshRenderer;

void Component_MeshRenderer_Update(MeshRenderer* meshRenderer, GameObject* gameObject, Game* game);
SerialObject MeshRenderer_serialize(MeshRenderer* meshRenderer);
void MeshRenderer_deserialize(MeshRenderer* meshRenderer, SerialObject* serialObject);

#endif //CHERRYENGINE_MESHRENDERER_H