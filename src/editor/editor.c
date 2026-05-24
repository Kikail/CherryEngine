//
// Created by killian on 5/20/26.
//
#include "editor.h"
#include "components/editorTransform.h"
#include "game/ecs/components/meshRenderer.h"

void Editor_renderHierarchy(Editor* editor, Scene* scene) {
    igBegin("Hierarchy", NULL, 0);

    for (int i = 0; i < scene->numGameObjects; i++) {
        GameObject* obj = &scene->gameObjects[i];
        char label[128];
        sprintf(label, "%s (ID: %u)", obj->name, obj->id);
        bool isSelected = (editor->selectedObjet == obj);
        if (isSelected) {
            igPushStyleColor_Vec4(ImGuiCol_Button, (ImVec4){0.2f, 0.4f, 0.8f, 1.0f});
        }
        if (igButton(label, (ImVec2){-1, 0})) {
            editor->selectedObjet = obj;
        }
        if (isSelected) {
            igPopStyleColor(1);
        }
    }

    igEnd();
}

void Editor_renderFileExplorer(Editor* editor, Game* game) {
    igBegin("File Explorer", NULL, 0);

    float cellSize = 64.0f;
    float panelWidth = igGetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1) columnCount = 1;

    for (int i = 0; i < game->resourceManager->numResources; i++) {
        CherryResource* resource = &game->resourceManager->resources[i];
        igPushID_Int(i);

        // 1. Récupérer l'ID OpenGL de la texture depuis ton manager
        // Assure-toi que cette fonction existe dans ton manager
        GLuint textureID;
        switch (resource->type) {
            case CHERRY_RESOURCE_TYPE_MATERIAL:
                textureID = ResourceManager_getTextureBySignature(game->resourceManager, 2936160603)->id;
                break;
            case CHERRY_RESOURCE_TYPE_MODEL:
                textureID = ResourceManager_getTextureBySignature(game->resourceManager, 2699999743)->id;
                break;
            case CHERRY_RESOURCE_TYPE_TEXTURE:
                textureID = ResourceManager_getTextureBySignature(game->resourceManager, 3749278416)->id;
                break;
            case CHERRY_RESOURCE_TYPE_SHADER:
                textureID = ResourceManager_getTextureBySignature(game->resourceManager, 938667242)->id;
                break;
            default:
                textureID = ResourceManager_getTextureBySignature(game->resourceManager, 1668917297)->id;
                break;

        }

        // 2. Création du bouton avec image
        // Les coordonnées UV (0,1) sont standards pour OpenGL
        ImVec2 size = {cellSize, cellSize};
        ImVec2 uv0 = {0, 0}; // Haut-gauche
        ImVec2 uv1 = {1, 1}; // Bas-droite (inversé car OpenGL Y est vers le haut)

        // Note : igImageButton accepte un pointeur vers l'ID de texture
        ImTextureRef* ref = ImTextureRef_ImTextureRef_TextureID(textureID);
        if (igImageButton("file_icon", *ref, size, uv0, uv1,
                         (ImVec4){0,0,0,0}, (ImVec4){1,1,1,1})) {
            editor->selectedResource = resource;
                         }

        // 3. Tooltip
        if (igIsItemHovered(0)) {
            igBeginTooltip();
            igText("Signature: %u", resource->signature);
            igText("Path: %s", resource->path);
            igEndTooltip();
        }

        if ((i + 1) % columnCount != 0) {
            igSameLine(0.0f, 10.0f);
        }

        igPopID();
    }
    igEnd();
}

void Editor_renderUtils(Editor* editor, Game* game) {
    igBegin("Utils", NULL, 0);

    if (igButton("New GameObject", (ImVec2){-1, 0})) {
        GameObject* newObject = Scene_addGameObject(game->currentScene, "GameObject");
        printf("EDITOR::Added new GameObject. Count: %d\n",game->currentScene->numGameObjects);
    }

    if (editor->selectedObjet != NULL) {
        igSeparator();

        if (!GameObject_HasComponent(editor->selectedObjet, COMPONENT_TRANSFORM)) {
            if (igButton("Add Transform", (ImVec2){-1, 0})) {
                GameObject_AddComponent(editor->selectedObjet, game->componentPool, COMPONENT_TRANSFORM);
            }
            igSeparator();
        }

        if (!GameObject_HasComponent(editor->selectedObjet, COMPONENT_MESH_RENDERER)) {
            if (igButton("Add MeshRenderer", (ImVec2){-1, 0})) {
                GameObject_AddComponent(editor->selectedObjet, game->componentPool, COMPONENT_MESH_RENDERER);
            }
            igSeparator();
        }
    }

    igEnd();
}

void Editor_render(Editor* editor, Game* game) {
    Editor_renderComponentsTab(editor, game);
    Editor_renderHierarchy(editor, game->currentScene);
    Editor_renderUtils(editor, game);
    Editor_renderFileExplorer(editor, game);
}

void Editor_renderMeshRenderer(Editor* editor, MeshRenderer* renderer, Game* game) {
    igText("MeshRenderer");

    // Définition des pas d'incrémentation pour l'InputScalar (+1 / +10)
    unsigned int step = 1;
    unsigned int step_fast = 10;

    for (int i = 0; i < renderer->materialCount; i++) {
        // 1. Affichage du texte informatif
        char textBuffer[128];
        snprintf(textBuffer, sizeof(textBuffer), "Mat [%d] Sig: %u", i, renderer->material[i]->signature);
        igText(textBuffer);
        igSameLine(0, -1); // Aligne l'input sur la même ligne

        // 2. Génération d'un ID unique masqué (##) pour chaque ligne
        char inputLabel[32];
        snprintf(inputLabel, sizeof(inputLabel), "##signature_%d", i);

        // 3. Saisie directe de l'unsigned int via igInputScalar
        if (igInputScalar(inputLabel, ImGuiDataType_U32, &renderer->material[i]->signature, &step, &step_fast, "%u", 0)) {
            // La signature a été modifiée directement en mémoire. On cherche le matériau associé.
            Material* mat = ResourceManager_getMaterialBySignature(game->resourceManager, renderer->material[i]->signature);
            if (mat != NULL) {
                renderer->material[i] = mat;
                DEBUG_LOG("EDITOR::Trying to assign material");
            } else {
                DEBUG_LOG("EDITOR::Trying to assign NULL material");
            }
        }
    }

    // 4. Boutons d'action (taille fixe de 30 pixels de large pour rester côte à côte)
    if (igButton("-", (ImVec2){30, 0})) {
        if (renderer->materialCount > 0) {
            renderer->materialCount -= 1;
        }
    }

    igSameLine(0, -1);

    if (igButton("+", (ImVec2){30, 0})) {
        if (renderer->materialCount < MESHRENDERER_MAX_MATERIALS) {
            // Optionnel mais recommandé : initialise le pointeur du nouveau slot
            // pour éviter qu'il ne pointe sur de la mémoire résiduelle.
            // renderer->material[renderer->materialCount] = un_materiau_par_defaut;

            renderer->materialCount += 1;
        }
    }

    igSeparator();
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

    if (GameObject_HasComponent(editor->selectedObjet, COMPONENT_MESH_RENDERER)) {
        Editor_renderMeshRenderer(editor, GameObject_GetComponent(editor->selectedObjet, game->componentPool, COMPONENT_MESH_RENDERER), game);
    }

    // ajouter les autres components et un boutton pour ajouter d autres composants

    igEnd();
}