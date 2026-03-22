//
// Created by killian on 3/11/26.
//
#include "stdio.h"
#include "../gameObject.h"
#include "transform.h"
#include "playerController.h"

void Component_PlayerController_Init(PlayerController* playerController, PhysicsObject* physicsObject, Camera* camera, GLFWwindow *window) {
    playerController->physicsObject = physicsObject;
    playerController->camera = camera;
    playerController->window = window;
    playerController->speed = 10.0f;
}void Component_PlayerController_Update(PlayerController* pc, GameObject* gameObject, float deltaTime) {
    vec3s front = pc->camera->front;
    vec3s right = pc->camera->right;

    // On force le déplacement à rester horizontal (on ignore le Y de la caméra)
    front.y = 0.0f;
    right.y = 0.0f;
    front = glms_vec3_normalize(front);
    right = glms_vec3_normalize(right);

    vec3s moveDir = glms_vec3_zero();

    // Accumulation des directions
    if (glfwGetKey(pc->window, GLFW_KEY_W) == GLFW_PRESS)
        moveDir = glms_vec3_add(moveDir, front);
    if (glfwGetKey(pc->window, GLFW_KEY_S) == GLFW_PRESS)
        moveDir = glms_vec3_sub(moveDir, front);
    if (glfwGetKey(pc->window, GLFW_KEY_A) == GLFW_PRESS)
        moveDir = glms_vec3_sub(moveDir, right);
    if (glfwGetKey(pc->window, GLFW_KEY_D) == GLFW_PRESS)
        moveDir = glms_vec3_add(moveDir, right);

    // --- APPLICATION DE LA VITESSE ---
    if (glms_vec3_norm(moveDir) > 0.0f) {
        moveDir = glms_vec3_normalize(moveDir);
        // On règle X et Z, on garde le Y actuel (gravité)
        pc->physicsObject->Velocity.x = moveDir.x * pc->speed;
        pc->physicsObject->Velocity.z = moveDir.z * pc->speed;
    } else {
        // --- SYSTÈME DE FRICTION / ARRÊT ---
        // Si aucune touche n'est pressée, on freine brutalement sur X et Z
        pc->physicsObject->Velocity.x *= 0.1f * (1.0f - deltaTime);
        pc->physicsObject->Velocity.z *= 0.1f * (1.0f - deltaTime);

        // On stoppe complètement si c'est très bas pour éviter les micro-mouvements
        if (fabsf(pc->physicsObject->Velocity.x) < 0.1f) pc->physicsObject->Velocity.x = 0;
        if (fabsf(pc->physicsObject->Velocity.z) < 0.1f) pc->physicsObject->Velocity.z = 0;
    }

    // Mise à jour de la caméra (offset de 1.8 pour la hauteur des yeux)
    pc->camera->position = glms_vec3_add(pc->physicsObject->Transform.position, (vec3s){0.0f, 8.0f, -10.0});
}