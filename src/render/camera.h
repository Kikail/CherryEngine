//
// Created by killian on 3/14/26.
//

#ifndef CHERRYENGINE_CAMERA_H
#define CHERRYENGINE_CAMERA_H

#include <cglm/vec3.h>
#include <cglm/struct.h>

#define VECTOR_UP {0.0,1.0,0.0}
#define VECTOR_DOWN {0.0,-1.0,0.0}
#define VECTOR_FRONT {0.0,0.0,-1.0}
#define VECTOR_BACK {0.0,0.0,1.0}

#define CAMERA_YAW (-90.0)
#define CAMERA_PITCH (0.0f)
#define CAMERA_SPEED (3.0)
#define CAMERA_SENSIVITY (0.1f)
#define CAMERA_ZOOM (45.0)

typedef enum CameraMovement_t {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
}CameraMovement;

typedef struct Camera_t {
    vec3s position;
    vec3s front;
    vec3s up;
    vec3s right;
    vec3s worldUp;
    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;
    float zoom;
}Camera;

Camera Camera_createCamera(vec3s pos, vec3s up, vec3s front, float yaw, float pitch, float movementSpeed, float mouseSensitivity, float zoom);
mat4s Camera_getViewMatrix(Camera* cam);
void Camera_processKeyboard(Camera* cam ,CameraMovement direction ,float deltaTime);
void Camera_processMouseMovement(Camera* cam, float xoffset, float yoffset);
void Camera_processMouseScroll(Camera* cam, float yoffset);
void Camera_updateCameraVectors(Camera* cam);

#endif //CHERRYENGINE_CAMERA_H