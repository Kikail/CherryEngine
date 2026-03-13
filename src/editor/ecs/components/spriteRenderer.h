//
// Created by killian on 3/11/26.
//

#ifndef CHERRYENGINE_SPRITERENDERER_H
#define CHERRYENGINE_SPRITERENDERER_H

typedef struct GameObject_t GameObject;
typedef struct Transform_t Transform;

typedef struct {
    char* imagePath;
} SpriteRenderer;

void Component_SpriteRenderer_Update(
    SpriteRenderer* transform, GameObject* gameObject);

#endif //CHERRYENGINE_SPRITERENDERER_H