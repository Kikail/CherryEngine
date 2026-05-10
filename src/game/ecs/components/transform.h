//
// Created by killian on 3/11/26.
//

#ifndef CHERRYENGINE_TRANSFORM_H
#define CHERRYENGINE_TRANSFORM_H

#include "utils/utils.h"
#include "cglm/struct.h"
#include "resource/serializer.h"

#define TRANSFORM_MAX_CHILDREN 16
#define TRANSFORM_PARENT_NULL 4294967295

typedef struct GameObject_t GameObject;

typedef enum TransformKeep_t {
    KEEP_WORLD,
    KEEP_POSITION,
    KEEP_SCALE,
    KEEP_ROTATION,
    KEEP_POSITION_ROTATION,
    KEEP_POSITION_SCALE,
    KEEP_ROTATION_SCALE,
    KEEP_NOTHING
}TransformKeep;

typedef struct Transform_t {
    vec3s position;
    versors rotation;
    vec3s scale;
    mat4s worldMatrix;
    bool isDirty;

    unsigned int id;
    unsigned int parentId;

    struct Transform_t* parent;
    struct Transform_t* childs[TRANSFORM_MAX_CHILDREN];
    unsigned int childCount;
} Transform;

void Transform_Afficher(Transform t);
void Transform_setParent(Transform* transform, Transform* parent, TransformKeep transformKeep);
Transform* Transform_getParent(Transform* transform);
Transform** Transform_getChilds(Transform* transform);
void Transform_addChild(Transform* transform, Transform* child, TransformKeep transformKeep);
bool Transform_hasParent(Transform* transform);
bool Transform_hasChild(Transform* transform);
bool Transform_isChildOf(Transform* transform, Transform* potentialParent);
int Transform_getChildCount(Transform* transform);
void Transform_setPosition(Transform* transform, vec3s position);
void Transform_setRotation(Transform* transform, versors rotation);
void Transform_setScale(Transform* transform, vec3s scale);
vec3s Transform_getPosition(Transform* transform);
versors Transform_getRotation(Transform* transform);
vec3s Transform_getScale(Transform* transform);
void Transform_translate(Transform* transform, vec3s translation, bool local);
void Transform_rotate(Transform* transform, versors rotation, bool local);
void Transform_scale(Transform* transform, vec3s scale, bool local);
vec3s Transform_getForward(Transform* transform);
vec3s Transform_getRight(Transform* transform);
vec3s Transform_getUp(Transform* transform);
void Transform_lookAt(Transform* transform, vec3s target, vec3s up);
void Transform_updateWorldTransform(Transform* transform);
void Transform_updateLocalTransform(Transform* transform);
mat4s Transform_getWorldMatrix(Transform* transform);
bool Transform_isDirty(Transform* transform);

SerialObject Transform_serialize(Transform* transform);

#endif //CHERRYENGINE_TRANSFORM_H