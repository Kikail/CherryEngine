//
// Created by killian on 3/30/26.
//
//
// Created by killian on 3/30/26.
//
#include "octree.h"

#include "glad/glad.h"

bool AABB_Collision(AABB* a, AABB* b) {
    return (a->min.x <= b->max.x && a->max.x >= b->min.x) &&
           (a->min.y <= b->max.y && a->max.y >= b->min.y) &&
           (a->min.z <= b->max.z && a->max.z >= b->min.z);
}
bool AABB_Point(AABB* a, vec3s p) {
    return (p.x >= a->min.x && p.x <= a->max.x) &&
           (p.y >= a->min.y && p.y <= a->max.y) &&
           (p.z >= a->min.z && p.z <= a->max.z);
}

OctreeNode* Octree_create(AABB aabb, unsigned int mE, unsigned int mD) {
    OctreeNode* node= malloc(sizeof(OctreeNode));

    node->aabb = aabb;
    node->maxDepth = mD;
    node->maxElements = mE;
    node->isLeaf = true;
    node->currentDepth = 0;
    node->objects = malloc(sizeof(PhysicsObject) * mE);
    node->numObjects = 0;
    node->nodes = NULL;

    return node;
}

void Octree_subdivide(OctreeNode* node) {
    node->isLeaf = false;
    // On divise en 8 car on coupe en 2 chaque axe
    node->nodes = malloc(sizeof(OctreeNode) * 8);

    // On recupere les infos de base
    vec3s min = node->aabb.min;
    vec3s center = glms_vec3_center(node->aabb.min, node->aabb.max);
    vec3s direction = glms_vec3_sub(center, min);

    // On calcule deja les positions des centre
    vec3s centerBottom =    {center.x, min.y, center.z};
    vec3s centerLeft =      {min.x, center.y, center.z};
    vec3s centerFront =     {center.x, center.y, min.z};

    // Coordonnees necessaires
    vec3s min0 = min;
    vec3s min1 = {centerBottom.x, centerBottom.y, min.z};
    vec3s min2 = {min.x, centerBottom.y, centerBottom.z};
    vec3s min3 = centerBottom;
    vec3s min4 = {min.x, center.y, min.z};
    vec3s min5 = centerFront;
    vec3s min6 = centerLeft;
    vec3s min7 = center;

    // On creer les bons AABB pour chaque node
    AABB aabbs[8];
    aabbs[0] = (AABB){min0,      glms_vec3_add(min0, direction)  };
    aabbs[1] = (AABB){min1,      glms_vec3_add(min1, direction)  };
    aabbs[2] = (AABB){min2,      glms_vec3_add(min2, direction)  };
    aabbs[3] = (AABB){min3,      glms_vec3_add(min3, direction)  };
    aabbs[4] = (AABB){min4,      glms_vec3_add(min4, direction)  };
    aabbs[5] = (AABB){min5,      glms_vec3_add(min5, direction)  };
    aabbs[6] = (AABB){min6,      glms_vec3_add(min6, direction)  };
    aabbs[7] = (AABB){min7,      glms_vec3_add(min7, direction)  };

    // Suffit d'initialiser chaque node
    for (int i = 0; i < 8; i++) {
        node->nodes[i] = Octree_create(aabbs[i], node->maxElements, node->maxDepth);
        node->nodes[i]->currentDepth = node->currentDepth + 1;
    }
}

void Octree_clean(OctreeNode* node) {
    if (node->isLeaf) {
        free(node->objects);
        free(node->nodes);
        free(node);
        return;
    }
    for (int i = 0; i < 8; i++) {
        Octree_clean(node->nodes[i]);
    }
    free(node->nodes);
    free(node);
}

bool Octree_addElement(OctreeNode* node, PhysicsObject* object) {
    // Si on est dans une feuille on peut ajouter
    if (node->isLeaf) {
        // On verifie deja si on depasse pas la taille max
        if (node->numObjects < node->maxElements) {
            node->objects[node->numObjects] = object;
            node->numObjects += 1;
            return true;
        }
        else {
            if (node->currentDepth < node->maxDepth) {
                // On subdivide et on transmet les enfants
                Octree_subdivide(node);

                // On sauvegarde et nettoie
                PhysicsObject** oldObjects = node->objects;
                unsigned int count = node->numObjects;
                node->objects = NULL;
                node->numObjects = 0;

                // On ajoute tout les elements dans les enfants
                for (unsigned int i = 0; i < count; i++) {
                    Octree_addElement(node, oldObjects[i]);
                }
                Octree_addElement(node, object);

                // Enfin on libere le tableau
                free(oldObjects);
                return true;
            }
            else {
                return false;
            }
        }
    }
    else {
        BoxCollider* collider = (BoxCollider*)(object->Collider->collider);
        AABB aabb = { glms_vec3_sub(object->Transform.position, collider->HalfSize), glms_vec3_add(object->Transform.position, collider->HalfSize)  };
        for (int i = 0; i < 8; i++) {
            if (AABB_Collision(&node->nodes[i]->aabb, &aabb )) {
                Octree_addElement(node->nodes[i], object);
            }
        }
        return true;
    }
}

void drawWireCube(vec3s min, vec3s max, vec3s color, Shader* shader) {
    vec3s v[8] = {
        {min.x, min.y, min.z}, {max.x, min.y, min.z},
        {max.x, max.y, min.z}, {min.x, max.y, min.z},
        {min.x, min.y, max.z}, {max.x, min.y, max.z},
        {max.x, max.y, max.z}, {min.x, max.y, max.z}
    };
    glUniform3f(glGetUniformLocation(shader->shaderID, "uColor"), color.x, color.y, color.z);
    unsigned int indices[24] = {
        0,1, 1,2, 2,3, 3,0, // Face arrière
        4,5, 5,6, 6,7, 7,4, // Face avant
        0,4, 1,5, 2,6, 3,7  // Liaisons
    };
    glBegin(GL_LINES);
    for(int i = 0; i < 24; i++) {
        glVertex3f(v[indices[i]].x, v[indices[i]].y, v[indices[i]].z);
    }
    glEnd();
}
void Octree_draw(OctreeNode* node, Shader* shader) {
    if (node == NULL) return;
    vec3s color = node->isLeaf ? (vec3s){0.0f, 1.0f, 0.0f} : (vec3s){1.0f, 1.0f, 1.0f};
    drawWireCube(node->aabb.min, node->aabb.max, color, shader);
    if (!node->isLeaf && node->nodes != NULL) {
        for (int i = 0; i < 8; i++) {
            Octree_draw(node->nodes[i], shader);
        }
    }
}