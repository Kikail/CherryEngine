#ifndef CHERRYENGINE_INPUT_H
#define CHERRYENGINE_INPUT_H

#include <stdbool.h>
#include "GLFW/glfw3.h"
#include "render/camera.h"

// Déclarations (extern pour éviter les erreurs de linker)
extern bool isDragging;
extern float lastX;
extern float lastY;
extern float camYaw;
extern float camPitch;
extern float camRadius;

// Configuration
#define MAX_KEYS 350

typedef struct Input_t {
    bool current[MAX_KEYS];
    bool previous[MAX_KEYS];
} Input;

// Fonctions Cycle de vie
Input* Input_create();
void Input_update(Input* input, GLFWwindow* window);
void cleanup(GLFWwindow* window);

// États des touches
bool Input_isHeld(Input* input, int key);
bool Input_isPressed(Input* input, int key);
bool Input_isReleased(Input* input, int key);

// Callbacks (à enregistrer dans ton main avec glfwSet...Callback)
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

#endif