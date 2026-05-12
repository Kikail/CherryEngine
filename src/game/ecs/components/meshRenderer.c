//
// Created by killian on 3/11/26.
//
#include "stdio.h"
#include "../../scene/gameObject.h"
#include "transform.h"
#include "meshRenderer.h"

void Component_MeshRenderer_Update(MeshRenderer* meshRenderer, GameObject* gameObject) {
    printf("%s MESHRENDERER UPDATE\n", gameObject->name);
}
SerialObject MeshRenderer_serialize(MeshRenderer* meshRenderer) {
    SerialObject obj = SerialObject_create("MeshRenderer");


    return obj;
}

void MeshRenderer_deserialize(MeshRenderer* meshRenderer, SerialObject* serialObject) {

}