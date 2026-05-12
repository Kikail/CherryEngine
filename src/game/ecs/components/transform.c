//
// Created by killian on 3/11/26.
//

#include "stdio.h"
#include "../../scene/gameObject.h"
#include "transform.h"

#include "transform.h"
#include <stdio.h>
#include <string.h>

#include "game/ecs/componentPool.h"

// ==========================================================
// FONCTIONS INTERNES (Privées)
// ==========================================================

// Propage le flag "dirty" à tous les enfants récursivement
static void Transform_setDirtyInternal(Transform* transform) {
    if (!transform || transform->isDirty) return;

    transform->isDirty = true;
    for (unsigned int i = 0; i < transform->childCount; i++) {
        Transform_setDirtyInternal(transform->childs[i]);
    }
}

// ==========================================================
// DEBUG
// ==========================================================

void Transform_Afficher(Transform t) {
    printf("Transform [Dirty: %s]\n", t.isDirty ? "Oui" : "Non");
    printf("  Pos: (%.2f, %.2f, %.2f)\n", t.position.x, t.position.y, t.position.z);
    printf("  Rot: (%.2f, %.2f, %.2f, %.2f)\n", t.rotation.x, t.rotation.y, t.rotation.z, t.rotation.w);
    printf("  Scl: (%.2f, %.2f, %.2f)\n", t.scale.x, t.scale.y, t.scale.z);
}

// ==========================================================
// HIERARCHIE
// ==========================================================

void Transform_setParent(Transform* transform, Transform* parent, TransformKeep transformKeep) {
    if (!transform || transform == parent) return;

    // Retirer de l'ancien parent si nécessaire
    if (transform->parent) {
        Transform* oldParent = transform->parent;
        for (unsigned int i = 0; i < oldParent->childCount; i++) {
            if (oldParent->childs[i] == transform) {
                // Décaler le reste du tableau
                for (unsigned int j = i; j < oldParent->childCount - 1; j++) {
                    oldParent->childs[j] = oldParent->childs[j + 1];
                }
                oldParent->childCount--;
                break;
            }
        }
    }

    transform->parent = parent;
    if (transform->parent) {
        transform->parentId = parent->id;
    }
    else {
        transform->parentId = TRANSFORM_PARENT_NULL;
    }

    // Ajouter au nouveau parent
    if (parent && parent->childCount < TRANSFORM_MAX_CHILDREN) {
        parent->childs[parent->childCount] = transform;
        parent->childCount++;
    }

    // TODO: Implémenter la logique mathématique des autres TransformKeep
    // Pour l'instant, KEEP_NOTHING garde les valeurs locales telles quelles.
    if (transformKeep == KEEP_WORLD) {
        // Pseudo-logique : Recalculer la position/rotation locale par rapport au nouveau parent
        // pour que l'objet ne bouge pas visuellement dans le monde.
    }

    Transform_setDirtyInternal(transform);
}

void Transform_addChild(Transform* transform, Transform* child, TransformKeep transformKeep) {
    if (!transform || !child) return;
    Transform_setParent(child, transform, transformKeep);
}

Transform* Transform_getParent(Transform* transform) {
    return transform ? transform->parent : NULL;
}

Transform** Transform_getChilds(Transform* transform) {
    return transform ? transform->childs : NULL;
}

bool Transform_hasParent(Transform* transform) {
    return transform && transform->parent != NULL;
}

bool Transform_hasChild(Transform* transform) {
    return transform && transform->childCount > 0;
}

int Transform_getChildCount(Transform* transform) {
    return transform ? (int)transform->childCount : 0;
}

bool Transform_isChildOf(Transform* transform, Transform* potentialParent) {
    if (!transform || !potentialParent) return false;
    Transform* current = transform->parent;
    while (current) {
        if (current == potentialParent) return true;
        current = current->parent;
    }
    return false;
}

// ==========================================================
// GETTERS & SETTERS
// ==========================================================

void Transform_setPosition(Transform* transform, vec3s position) {
    if(!transform) return;
    transform->position = position;
    Transform_setDirtyInternal(transform);
}

void Transform_setRotation(Transform* transform, versors rotation) {
    if(!transform) return;
    transform->rotation = rotation;
    Transform_setDirtyInternal(transform);
}

void Transform_setScale(Transform* transform, vec3s scale) {
    if(!transform) return;
    transform->scale = scale;
    Transform_setDirtyInternal(transform);
}

vec3s Transform_getPosition(Transform* transform) {
    return transform ? transform->position : glms_vec3_zero();
}

versors Transform_getRotation(Transform* transform) {
    return transform ? transform->rotation : glms_quat_identity();
}

vec3s Transform_getScale(Transform* transform) {
    return transform ? transform->scale : glms_vec3_one();
}

// ==========================================================
// MANIPULATION SPATIALE
// ==========================================================

void Transform_translate(Transform* transform, vec3s translation, bool local) {
    if(!transform) return;
    if (local) {
        // Appliquer la rotation locale à la translation
        vec3s localTranslation = glms_quat_rotatev(transform->rotation, translation);
        transform->position = glms_vec3_add(transform->position, localTranslation);
    } else {
        transform->position = glms_vec3_add(transform->position, translation);
    }
    Transform_setDirtyInternal(transform);
}

void Transform_rotate(Transform* transform, versors rotation, bool local) {
    if(!transform) return;
    if (local) {
        transform->rotation = glms_quat_mul(transform->rotation, rotation);
    } else {
        transform->rotation = glms_quat_mul(rotation, transform->rotation);
    }
    transform->rotation = glms_quat_normalize(transform->rotation);
    Transform_setDirtyInternal(transform);
}

void Transform_scale(Transform* transform, vec3s scale, bool local) {
    if(!transform) return;
    // La mise à l'échelle locale vs monde est complexe sans déformation matricielle.
    // Standardement, on multiplie simplement les vecteurs.
    transform->scale = glms_vec3_mul(transform->scale, scale);
    Transform_setDirtyInternal(transform);
}

// ==========================================================
// VECTEURS DE DIRECTION (Basés sur la rotation locale)
// ==========================================================
// Rappel OpenGL : Forward est souvent -Z, Right est +X, Up est +Y

vec3s Transform_getForward(Transform* transform) {
    if(!transform) return (vec3s){0.0f, 0.0f, -1.0f};
    return glms_quat_rotatev(transform->rotation, (vec3s){0.0f, 0.0f, -1.0f});
}

vec3s Transform_getRight(Transform* transform) {
    if(!transform) return (vec3s){1.0f, 0.0f, 0.0f};
    return glms_quat_rotatev(transform->rotation, (vec3s){1.0f, 0.0f, 0.0f});
}

vec3s Transform_getUp(Transform* transform) {
    if(!transform) return (vec3s){0.0f, 1.0f, 0.0f};
    return glms_quat_rotatev(transform->rotation, (vec3s){0.0f, 1.0f, 0.0f});
}

void Transform_lookAt(Transform* transform, vec3s target, vec3s up) {
    if(!transform) return;
    vec3s dir = glms_vec3_normalize(glms_vec3_sub(target, transform->position));

    // CGLM permet de créer un quaternion à partir d'une matrice ou de directions
    mat4s lookMat = glms_lookat(transform->position, target, up);

    // Extrait la rotation de la matrice de vue inversée (matrice modèle)
    mat4s invLook = glms_mat4_inv(lookMat);
    transform->rotation = glms_mat4_quat(invLook);
    Transform_setDirtyInternal(transform);
}

// ==========================================================
// CALCUL DES MATRICES
// ==========================================================

void Transform_updateLocalTransform(Transform* transform) {
    // Cette fonction n'est pas strictement nécessaire si tu calcules tout dans getWorldMatrix,
    // mais elle est utile si tu veux exposer localMatrix plus tard.
}

void Transform_updateWorldTransform(Transform* transform) {
    if (!transform) return;

    // 1. Matrice de Translation
    mat4s tMatrix = glms_translate(glms_mat4_identity(), transform->position);

    // 2. Matrice de Rotation (depuis le quaternion)
    mat4s rMatrix = glms_quat_mat4(transform->rotation);

    // 3. Matrice de Scale
    mat4s sMatrix = glms_scale(glms_mat4_identity(), transform->scale);

    // M_local = T * R * S
    mat4s localMatrix = glms_mat4_mul(tMatrix, glms_mat4_mul(rMatrix, sMatrix));

    if (transform->parent) {
        // S'assurer que le parent est à jour avant de l'utiliser
        if (transform->parent->isDirty) {
            Transform_updateWorldTransform(transform->parent);
        }
        // M_world = M_parent_world * M_local
        transform->worldMatrix = glms_mat4_mul(transform->parent->worldMatrix, localMatrix);
    } else {
        transform->worldMatrix = localMatrix;
    }

    transform->isDirty = false;
}

mat4s Transform_getWorldMatrix(Transform* transform) {
    if (!transform) return glms_mat4_identity();

    // Vérifier récursivement si un parent est dirty
    bool needsUpdate = transform->isDirty;
    Transform* p = transform->parent;
    while (p && !needsUpdate) {
        if (p->isDirty) needsUpdate = true;
        p = p->parent;
    }

    if (needsUpdate) {
        Transform_updateWorldTransform(transform);
    }

    return transform->worldMatrix;
}

bool Transform_isDirty(Transform* transform) {
    return transform ? transform->isDirty : false;
}

// ==========================================================
// SERIALIZATION
// ==========================================================

SerialObject Transform_serialize(Transform* transform) {
    SerialObject transformObject = SerialObject_create("Transform");

    SerialValue posx = SerialValue_create_double("posx", transform->position.x);
    SerialValue posy = SerialValue_create_double("posy", transform->position.y);
    SerialValue posz = SerialValue_create_double("posz", transform->position.z);
    SerialValue rotx = SerialValue_create_double("rotx", transform->rotation.x);
    SerialValue roty = SerialValue_create_double("roty", transform->rotation.y);
    SerialValue rotz = SerialValue_create_double("rotz", transform->rotation.z);
    SerialValue rotw = SerialValue_create_double("rotw", transform->rotation.w);
    SerialValue scalex = SerialValue_create_double("scalex", transform->scale.x);
    SerialValue scaley = SerialValue_create_double("scaley", transform->scale.y);
    SerialValue scalez = SerialValue_create_double("scalez", transform->scale.z);
    SerialValue parentId = SerialValue_create_uint("parentTransformId", transform->parentId);

    SerialObject_AddSerialValue(&transformObject, &posx);
    SerialObject_AddSerialValue(&transformObject, &posy);
    SerialObject_AddSerialValue(&transformObject, &posz);
    SerialObject_AddSerialValue(&transformObject, &rotx);
    SerialObject_AddSerialValue(&transformObject, &roty);
    SerialObject_AddSerialValue(&transformObject, &rotz);
    SerialObject_AddSerialValue(&transformObject, &rotw);
    SerialObject_AddSerialValue(&transformObject, &scalex);
    SerialObject_AddSerialValue(&transformObject, &scaley);
    SerialObject_AddSerialValue(&transformObject, &scalez);
    SerialObject_AddSerialValue(&transformObject, &parentId);

    // IL MANQUE LES INFOS SUR LE PARENT

    return transformObject;
}

void Transform_deserialize(Transform* transform, SerialObject* serialObject, ComponentPool* componentPool) {
    SerialValue posx = SerialObject_GetByName(serialObject,"posx");
    SerialValue posy = SerialObject_GetByName(serialObject,"posy");
    SerialValue posz = SerialObject_GetByName(serialObject,"posz");
    SerialValue rotx = SerialObject_GetByName(serialObject,"rotx");
    SerialValue roty = SerialObject_GetByName(serialObject,"roty");
    SerialValue rotz = SerialObject_GetByName(serialObject,"rotz");
    SerialValue rotw = SerialObject_GetByName(serialObject,"rotw");
    SerialValue scalex = SerialObject_GetByName(serialObject,"scalex");
    SerialValue scaley = SerialObject_GetByName(serialObject,"scaley");
    SerialValue scalez = SerialObject_GetByName(serialObject,"scalez");
    SerialValue parentId = SerialObject_GetByName(serialObject,"parentTransformId");

    transform->position.x = SerialValue_GetDoubleValue(&posx);
    transform->position.y = SerialValue_GetDoubleValue(&posy);
    transform->position.z = SerialValue_GetDoubleValue(&posz);
    transform->rotation.x = SerialValue_GetDoubleValue(&rotx);
    transform->rotation.y = SerialValue_GetDoubleValue(&roty);
    transform->rotation.z = SerialValue_GetDoubleValue(&rotz);
    transform->rotation.w = SerialValue_GetDoubleValue(&rotw);
    transform->scale.x = SerialValue_GetDoubleValue(&scalex);
    transform->scale.y = SerialValue_GetDoubleValue(&scaley);
    transform->scale.z = SerialValue_GetDoubleValue(&scalez);
    transform->parentId = SerialValue_GetUintValue(&parentId);
    transform->isDirty = true;

    if (transform->parentId != TRANSFORM_PARENT_NULL) {
        for (int i = 0; i < componentPool->currentTransformCount; i++) {
            if (componentPool->transforms[i].id == transform->parentId) {
                Transform_setParent(transform, &componentPool->transforms[i], KEEP_NOTHING);
            }
        }
    }
}