#include "input.h"
#include "utils/utils.h" // Pour WIDTH et HEIGHT
#include <string.h>
#include <stdlib.h>

// Définitions réelles des variables
bool isDragging = false;
float lastX = 800.0f / 2.0f; // Valeurs par défaut si WIDTH n'est pas dispo
float lastY = 600.0f / 2.0f;
float camYaw = -90.0f;       // -90 orienté vers l'avant en OpenGL
float camPitch = 0.0f;
float camRadius = 5.0f;

Input* Input_create() {
    return (Input*)calloc(1, sizeof(Input));
}

void Input_update(Input* input, GLFWwindow* window) {
    memcpy(input->previous, input->current, sizeof(bool) * MAX_KEYS);
    for (int i = 32; i < MAX_KEYS; i++) { // Commence à 32 (Espace) pour éviter les touches système
        input->current[i] = (glfwGetKey(window, i) == GLFW_PRESS);
    }
    if (Input_isPressed(input, GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

bool Input_isHeld(Input* input, int key)     { return input->current[key]; }
bool Input_isPressed(Input* input, int key)  { return input->current[key] && !input->previous[key]; }
bool Input_isReleased(Input* input, int key) { return !input->current[key] && input->previous[key]; }

// --- Callbacks ---

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        isDragging = (action == GLFW_PRESS);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camRadius -= (float)yoffset * 0.5f;
    if (camRadius < 1.0f) camRadius = 1.0f;
    if (camRadius > 50.0f) camRadius = 50.0f;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = (float)xposIn;
    float ypos = (float)yposIn;

    static bool firstMouse = true;
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    if (isDragging) {
        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // Inversé car l'axe Y va du haut vers le bas

        float sensitivity = 0.1f;
        camYaw   += xoffset * sensitivity;
        camPitch += yoffset * sensitivity;

        if (camPitch > 89.0f)  camPitch = 89.0f;
        if (camPitch < -89.0f) camPitch = -89.0f;
    }

    lastX = xpos;
    lastY = ypos;
}
void cleanup(GLFWwindow* window)
{
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}