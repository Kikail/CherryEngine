//
// Created by killian on 3/11/26.
//

#ifndef CHERRYENGINE_MESHRENDERER_H
#define CHERRYENGINE_MESHRENDERER_H

typedef struct GameObject_t GameObject;
typedef struct Transform_t Transform;

typedef struct {
    char* modelPath;
} MeshRenderer;

void Component_MeshRenderer_Update(MeshRenderer* meshRenderer, GameObject* gameObject);
SerialObject MeshRenderer_serialize(MeshRenderer* meshRenderer);
void MeshRenderer_deserialize(MeshRenderer* meshRenderer, SerialObject* serialObject);

#endif //CHERRYENGINE_MESHRENDERER_H