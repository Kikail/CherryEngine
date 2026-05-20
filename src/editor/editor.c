//
// Created by killian on 5/20/26.
//
#include "editor.h"
#include "components/editorTransform.h"

void Editor_render(Editor* editor, Game* game) {
    // Affichage d'une fenetre Imgui
    igBegin("Components Tab", NULL, 0);

    if (editor->selectedObjet == NULL) {
        igEnd();
        return;
    }

    if (GameObject_HasComponent(editor->selectedObjet, COMPONENT_TRANSFORM)) {
        Editor_Transform_RenderTab(editor, GameObject_GetComponent(editor->selectedObjet, game->componentPool, COMPONENT_TRANSFORM));
    }

    igEnd();
}