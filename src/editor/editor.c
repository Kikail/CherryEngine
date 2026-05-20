//
// Created by killian on 5/20/26.
//
#include "editor.h"
#include "components/editorTransform.h"

void Editor_RenderHierarchy(Editor* editor, Scene* scene) {
    igBegin("Hierarchy", NULL, 0);

    for (int i = 0; i < scene->numGameObjects; i++) {
        GameObject* obj = &scene->gameObjects[i];
        char label[128];
        sprintf(label, "%s (ID: %u)", obj->name, obj->id);

        bool isSelected = (editor->selectedObjet == obj);

        // Si l'objet est sélectionné, on pousse la couleur
        if (isSelected) {
            igPushStyleColor_Vec4(ImGuiCol_Button, (ImVec4){0.2f, 0.4f, 0.8f, 1.0f});
        }

        // Le bouton est dessiné
        if (igButton(label, (ImVec2){-1, 0})) {
            editor->selectedObjet = obj;
        }

        // Si on a poussé une couleur, on DOIT la dépouiller,
        // peu importe si le bouton a été cliqué ou non.
        if (isSelected) {
            igPopStyleColor(1);
        }
    }

    igEnd();
}

void Editor_render(Editor* editor, Game* game) {
    Editor_renderComponentsTab(editor, game);
    Editor_RenderHierarchy(editor, game->currentScene);
}
void Editor_renderComponentsTab(Editor* editor, Game* game) {
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