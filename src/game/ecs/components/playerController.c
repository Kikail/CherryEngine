//
// Created by killian on 3/11/26.
//
#include "stdio.h"
#include "../../scene/gameObject.h"
#include "transform.h"
#include "playerController.h"

void Component_PlayerController_Init(PlayerController* playerController, PhysicsObject* physicsObject, Camera* camera, GLFWwindow *window) {
    playerController->physicsObject = physicsObject;
    playerController->camera = camera;
    playerController->window = window;
    playerController->speed = 10.0f;
}

void Component_PlayerController_Update(PlayerController* pc, GameObject* gameObject, float deltaTime) {
    vec3s front = pc->camera->front;
    vec3s right = pc->camera->right;

    // On force le mouvement à être horizontal pour éviter de "s'envoler" en regardant le ciel
    front.y = 0.0f;
    right.y = 0.0f;
    front = glms_vec3_normalize(front);
    right = glms_vec3_normalize(right);

    vec3s moveDir = glms_vec3_zero();

    // --- 2. DÉPLACEMENTS ZQSD ---
    if (glfwGetKey(pc->window, GLFW_KEY_W) == GLFW_PRESS) moveDir = glms_vec3_add(moveDir, front);
    if (glfwGetKey(pc->window, GLFW_KEY_S) == GLFW_PRESS) moveDir = glms_vec3_sub(moveDir, front);
    if (glfwGetKey(pc->window, GLFW_KEY_A) == GLFW_PRESS) moveDir = glms_vec3_sub(moveDir, right);
    if (glfwGetKey(pc->window, GLFW_KEY_D) == GLFW_PRESS) moveDir = glms_vec3_add(moveDir, right);

    // Application de la vitesse horizontale
    if (glms_vec3_norm(moveDir) > 0.0f) {
        moveDir = glms_vec3_normalize(moveDir);
        pc->physicsObject->Velocity.x = moveDir.x * pc->speed;
        pc->physicsObject->Velocity.z = moveDir.z * pc->speed;
    } else {
        // Freinage horizontal (friction)
        pc->physicsObject->Velocity.x *= 0.1f;
        pc->physicsObject->Velocity.z *= 0.1f;
    }

    // --- 3. LE SAUT (JUMP) ---
    // On détecte l'appui sur Espace
    if (glfwGetKey(pc->window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        // GROUND CHECK : On ne peut sauter que si on est "au sol"
        // On vérifie si la vitesse verticale est proche de 0
        if (fabsf(pc->physicsObject->Velocity.y) < 0.1f) {
            pc->physicsObject->Velocity.y = 6.0f; // Force de saut (ajuste selon ta gravité)
        }
    }

    // --- 4. POSITION DE LA CAMÉRA (VUE FPS) ---
    // On place la caméra exactement au centre du cube du joueur, avec un petit offset vertical
    // On enlève le -10.0 en Z pour être DANS le personnage
    pc->camera->position = glms_vec3_add(pc->physicsObject->Transform.position, (vec3s){0.0f, 1.5f, 0.0f});
}

SerialObject PlayerController_serialize(PlayerController* playerController) {
    SerialObject obj = SerialObject_create("PlayerController");


    return obj;
}