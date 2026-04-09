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