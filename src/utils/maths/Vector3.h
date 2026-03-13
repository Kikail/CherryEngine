//
// Created by killian on 3/11/26.
//

#ifndef CHERRYENGINE_VECTOR3_H
#define CHERRYENGINE_VECTOR3_H

typedef struct {
    float x,y,z;
} Vector3;

/**
 * \brief fonction qui permet d'afficher un vecteur de 3 dimensions
 */
void Vector3_Afficher(Vector3 v);

#endif //CHERRYENGINE_VECTOR3_H