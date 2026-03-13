//
// Created by killian on 3/11/26.
//

#ifndef CHERRYENGINE_PLAYERCONTROLLER_H
#define CHERRYENGINE_PLAYERCONTROLLER_H

typedef struct GameObject_t GameObject;
typedef struct Transform_t Transform;

typedef struct {
    float speed;
    float velocity;
} PlayerController;

void Component_PlayerController_Update(PlayerController* PlayerController, GameObject* gameObject);

#endif //CHERRYENGINE_PLAYERCONTROLLER_H