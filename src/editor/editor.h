//
// Created by killian on 5/20/26.
//



#ifndef CHERRYENGINE_EDITOR_H
#define CHERRYENGINE_EDITOR_H

#include "../game/scene/scene.h"
#include "../game/scene/game.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../external/cimgui/cimgui.h"
#include <cimgui_impl.h> // Souvent, cimgui fournit un header englobant pour les backends C
// Déclarations explicites pour lier le C aux backends C++ d'ImGui
extern bool ImGui_ImplGlfw_InitForOpenGL(GLFWwindow* window, bool install_callbacks);
extern void ImGui_ImplGlfw_NewFrame(void);
extern void ImGui_ImplGlfw_Shutdown(void);
extern bool ImGui_ImplOpenGL3_Init(const char* glsl_version);
extern void ImGui_ImplOpenGL3_NewFrame(void);
extern void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data);
extern void ImGui_ImplOpenGL3_Shutdown(void);


typedef struct Editor_t {
    GameObject* selectedObjet;
}Editor;
void Editor_render(Editor* editor, Game* game);


#endif //CHERRYENGINE_EDITOR_H
