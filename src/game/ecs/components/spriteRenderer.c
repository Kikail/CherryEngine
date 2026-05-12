//
// Created by killian on 3/11/26.
//
#include "stdio.h"
#include "../../scene/gameObject.h"
#include "transform.h"
#include "spriteRenderer.h"


void Component_SpriteRenderer_Update(SpriteRenderer* transform, GameObject* gameObject) {
    printf("%s MESHRENDERER UPDATE\n", gameObject->name);
}

SerialObject SpriteRenderer_serialize(SpriteRenderer* spriteRenderer) {
    SerialObject obj = SerialObject_create("SpriteRenderer");
    return obj;
}

void SpriteRenderer_deserialize(SpriteRenderer* spriteRenderer, SerialObject* serialObject) {

}