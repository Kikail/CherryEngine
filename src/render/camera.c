//
// Created by killian on 3/14/26.
//
#include "camera.h"

Camera Camera_createCamera(vec3s pos, vec3s up, vec3s front, float yaw, float pitch, float movementSpeed, float mouseSensitivity, float zoom) {
    Camera newCamera;
    newCamera.position = pos;
    newCamera.up = up;
    newCamera.front = front;
    newCamera.yaw = yaw;
    newCamera.pitch = pitch;
    newCamera.movementSpeed = movementSpeed;
    newCamera.mouseSensitivity = mouseSensitivity;
    newCamera.zoom = zoom;
    newCamera.worldUp = up;
    Camera_updateCameraVectors(&newCamera);
    return newCamera;
}
mat4s Camera_getViewMatrix(Camera* cam){
    return glms_lookat(cam->position, glms_vec3_add(cam->position, cam->front), cam->up);
}
void Camera_processKeyboard(Camera* cam, CameraMovement direction, float deltaTime){
    float velocity = cam->movementSpeed * deltaTime;
    if (direction == FORWARD)
        cam->position = glms_vec3_add(cam->position, glms_vec3_mul(cam->front, glms_vec3_fill(velocity)));
    if (direction == BACKWARD)
        cam->position = glms_vec3_sub(cam->position, glms_vec3_mul(cam->front, glms_vec3_fill(velocity)));
    if (direction == LEFT)
        cam->position =  glms_vec3_sub(cam->position, glms_vec3_mul(cam->right, glms_vec3_fill(velocity)));;
    if (direction == RIGHT)
        cam->position = glms_vec3_add(cam->position, glms_vec3_mul(cam->right, glms_vec3_fill(velocity)));
}
void Camera_processMouseMovement(Camera* cam, float xoffset, float yoffset){
    xoffset *= cam->mouseSensitivity;
    yoffset *= cam->mouseSensitivity;

    cam->yaw   += xoffset;
    cam->pitch += yoffset;

    // update Front, Right and Up Vectors using the updated Euler angles
    Camera_updateCameraVectors(cam);
}
void Camera_processMouseScroll(Camera* cam, float yoffset){
    cam->zoom -= (float)yoffset;
    if (cam->zoom < 1.0f)
        cam->zoom = 1.0f;
    if (cam->zoom > 45.0f)
        cam->zoom = 45.0f;
}
void Camera_updateCameraVectors(Camera* cam){
    // calculate the new Front vector
    vec3s front;
    front.x = cos(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));
    front.y = sin(glm_rad(cam->pitch));
    front.z = sin(glm_rad(cam->yaw)) * cos(glm_rad(cam->pitch));
    cam->front = glms_normalize(front);
    // also re-calculate the Right and Up vector
    cam->right = glms_normalize(glms_cross(cam->front, cam->worldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    cam->up = glms_normalize(glms_cross(cam->right, cam->front));
}
