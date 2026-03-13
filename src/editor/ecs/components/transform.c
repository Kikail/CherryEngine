//
// Created by killian on 3/11/26.
//

#include "stdio.h"
#include "../gameObject.h"
#include "transform.h"


/**
 * \brief fonction qui permet d'qfficher un Transform
 */
void Transform_Afficher(Transform t){
    printf("{\n");
    Vector3_Afficher(t.position);
    Vector3_Afficher(t.rotation);
    Vector3_Afficher(t.scale);
    printf("}\n");
}

void Component_Transform_Update(Transform* transform, GameObject* gameObject) {
    LOG("TRANSFORM UPDATE");
}
