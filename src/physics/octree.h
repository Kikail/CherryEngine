//
// Created by killian on 3/30/26.
//

#ifndef CHERRYENGINE_OCTREE_H
#define CHERRYENGINE_OCTREE_H

#include <stdbool.h>
#include "physicsObject.h"
#include "render/shader.h"

typedef struct AABB_t {
    vec3s min;
    vec3s max;
}AABB;
bool AABB_Collision(AABB* a, AABB* b);
bool AABB_Point(AABB* a, vec3s p);

typedef struct OctreeNode_t {
    AABB aabb;

    unsigned int maxElements;
    PhysicsObject* objects;
    unsigned int numObjects;

    struct OctreeNode_t** nodes;
    unsigned int maxDepth;
    unsigned int currentDepth;

    bool isLeaf;
}OctreeNode;

OctreeNode* Octree_create(AABB aabb, unsigned int mE, unsigned int mD);
void Octree_subdivide(OctreeNode* node);
void Octree_clean(OctreeNode* node);
bool Octree_addElement(OctreeNode* node, PhysicsObject* object);
void Octree_draw(OctreeNode* node, Shader* shader);

#endif //CHERRYENGINE_OCTREE_H
