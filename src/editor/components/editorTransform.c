//
// Created by killian on 5/20/26.
//
#include "editorTransform.h"
#include "../editor.h"

void Editor_Transform_RenderTab(Editor* editor, Transform* transform) {
    if (transform == NULL) {
        #ifdef DEBUG
                DEBUG_LOG("EDITOR::Editor_Transform_RenderTab trying to render a NULL transform");
        #endif
        return;
    }

    igText("Transform");

    // v_speed : la sensibilité du glisser (ex: 0.1f)
    // v_min / v_max : bornes de la valeur (0.0f pour pas de limite)
    if (igDragFloat3("Position", (float*)&transform->position, 0.1f, 0.0f, 0.0f, "%.3f", 0)) {
        transform->isDirty = true;
    }

    if (igDragFloat3("Rotation", (float*)&transform->rotation, 1.0f, 0.0f, 0.0f, "%.3f", 0)) {
        transform->isDirty = true;
    }

    if (igDragFloat3("Scale", (float*)&transform->scale, 0.05f, 0.0f, 0.0f, "%.3f", 0)) {
        transform->isDirty = true;
    }

    igSeparator();
}