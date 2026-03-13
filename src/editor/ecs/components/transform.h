//
// Created by killian on 3/11/26.
//

#ifndef CHERRYENGINE_TRANSFORM_H
#define CHERRYENGINE_TRANSFORM_H

#include "utils/maths/Vector3.h"
#include "utils/utils.h"

typedef struct GameObject_t GameObject;

typedef struct Transform_t {
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
} Transform;

/**
 * \brief fonction qui permet d'qfficher un Transform
 */
void Transform_Afficher(Transform t);
void Component_Transform_Update(Transform* transform, GameObject* gameObject);


#endif //CHERRYENGINE_TRANSFORM_H