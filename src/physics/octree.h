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

#define OCTREE_MAX_CAPACITY 15
#define OCTREE_MAX_DEPTH 6
#define OCTREE_MAX_NODES 20000

typedef struct OctreeNode_t {
    AABB aabb;

    PhysicsObject* objects[OCTREE_MAX_CAPACITY];
    unsigned int numObjects;

    unsigned int firstChildIndex;

    unsigned int currentDepth;

    bool isLeaf;
}OctreeNode;

void Octree_create(OctreeNode* node, AABB aabb);
void Octree_subdivide(OctreeNode* node);
void Octree_clean(OctreeNode* node);
bool Octree_addElement(OctreeNode* node, PhysicsObject* object);
void Octree_draw(OctreeNode* node, Shader* shader);
void Octree_ResetPool(AABB worldBounds);
OctreeNode* Octree_getNode(unsigned int index);

#endif //CHERRYENGINE_OCTREE_H
