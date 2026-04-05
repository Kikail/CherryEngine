//
// Created by killian on 3/11/26.
//

#ifndef CHERRYENGINE_PLAYERCONTROLLER_H
#define CHERRYENGINE_PLAYERCONTROLLER_H
#include "GLFW/glfw3.h"
#include "render/camera.h"

typedef struct GameObject_t GameObject;
typedef struct Transform_t Transform;

#include "../../../physics/physicsObject.h"

typedef struct {
    float speed;
    PhysicsObject* physicsObject;
    Camera* camera;
    GLFWwindow* window;
} PlayerController;

void Component_PlayerController_Init(PlayerController* playerController, PhysicsObject* physicsObject, Camera* camera, GLFWwindow *window);
void Component_PlayerController_Update(PlayerController* PlayerController, GameObject* gameObject, float deltaTime);

#endif //CHERRYENGINE_PLAYERCONTROLLER_H