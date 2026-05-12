//
// Created by killian on 3/11/26.
//

#ifndef CHERRYENGINE_PLAYERCONTROLLER_H
#define CHERRYENGINE_PLAYERCONTROLLER_H

#include "render/camera.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

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
SerialObject PlayerController_serialize(PlayerController* playerController);
void PlayerController_deserialize(PlayerController* playerController, SerialObject* serialObject);

#endif //CHERRYENGINE_PLAYERCONTROLLER_H