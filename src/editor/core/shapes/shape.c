//
// Created by killian on 3/17/26.
//
#include "shape.h"

#include "glad/glad.h"

void Shape_init(Shape* shape) {
    glGenVertexArrays(1, &shape->VAO);
    glGenBuffers(1, &shape->VBO);

    glBindVertexArray(shape->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, shape->VBO);
    glBufferData(GL_ARRAY_BUFFER, shape->nbVertices * 8 * sizeof(float), shape->vertices, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normales
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // UV
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
void Shape_clean(Shape* shape) {
    glDeleteBuffers(1, &shape->VBO);
    glDeleteVertexArrays(1, &shape->VAO);
}
void Shape_draw(Shape* shape) {
    glBindVertexArray(shape->VAO);
    glDrawArrays(GL_TRIANGLES, 0, shape->nbVertices);
}

void Shape_generateCube(Shape* shape) {
    shape->nbVertices = 36;
    static float verts[] = {
        // Face avant (z = 0.5) normale (0,0,1)
        -0.5f,-0.5f, 0.5f, 0,0,1, 0,0,
         0.5f,-0.5f, 0.5f, 0,0,1, 1,0,
         0.5f, 0.5f, 0.5f, 0,0,1, 1,1,
         0.5f, 0.5f, 0.5f, 0,0,1, 1,1,
        -0.5f, 0.5f, 0.5f, 0,0,1, 0,1,
        -0.5f,-0.5f, 0.5f, 0,0,1, 0,0,

        // Face arrière (z = -0.5) normale (0,0,-1)
        -0.5f,-0.5f,-0.5f, 0,0,-1, 0,0,
         0.5f, 0.5f,-0.5f, 0,0,-1, 1,1,
         0.5f,-0.5f,-0.5f, 0,0,-1, 1,0,
         0.5f, 0.5f,-0.5f, 0,0,-1, 1,1,
        -0.5f,-0.5f,-0.5f, 0,0,-1, 0,0,
        -0.5f, 0.5f,-0.5f, 0,0,-1, 0,1,

        // Face gauche (x = -0.5) normale (-1,0,0)
        -0.5f,-0.5f,-0.5f, -1,0,0, 0,0,
        -0.5f, 0.5f, 0.5f, -1,0,0, 1,1,
        -0.5f, 0.5f,-0.5f, -1,0,0, 1,0,
        -0.5f,-0.5f, 0.5f, -1,0,0, 0,1,
        -0.5f, 0.5f, 0.5f, -1,0,0, 1,1,
        -0.5f,-0.5f,-0.5f, -1,0,0, 0,0,

        // Face droite (x = 0.5) normale (1,0,0)
         0.5f,-0.5f,-0.5f, 1,0,0, 0,0,
         0.5f, 0.5f,-0.5f, 1,0,0, 1,0,
         0.5f, 0.5f, 0.5f, 1,0,0, 1,1,
         0.5f,-0.5f, 0.5f, 1,0,0, 0,1,
         0.5f, 0.5f, 0.5f, 1,0,0, 1,1,
         0.5f,-0.5f,-0.5f, 1,0,0, 0,0,

        // Face haut (y = 0.5) normale (0,1,0)
        -0.5f, 0.5f,-0.5f, 0,1,0, 0,0,
         0.5f, 0.5f,-0.5f, 0,1,0, 1,0,
         0.5f, 0.5f, 0.5f, 0,1,0, 1,1,
         0.5f, 0.5f, 0.5f, 0,1,0, 1,1,
        -0.5f, 0.5f, 0.5f, 0,1,0, 0,1,
        -0.5f, 0.5f,-0.5f, 0,1,0, 0,0,

        // Face bas (y = -0.5) normale (0,-1,0)
        -0.5f,-0.5f,-0.5f, 0,-1,0, 0,0,
         0.5f,-0.5f, 0.5f, 0,-1,0, 1,1,
         0.5f,-0.5f,-0.5f, 0,-1,0, 1,0,
         0.5f,-0.5f, 0.5f, 0,-1,0, 1,1,
        -0.5f,-0.5f,-0.5f, 0,-1,0, 0,0,
        -0.5f,-0.5f, 0.5f, 0,-1,0, 0,1
    };
    shape->vertices = verts;
}
void Shape_generatePyramid(Shape* shape) {
    shape->nbVertices = 18;
    static float verts[] = {
        // Base (0,-1,0)
        -0.5f,0, -0.5f, 0,-1,0, 0,0,
         0.5f,0, -0.5f, 0,-1,0, 1,0,
         0.5f,0,  0.5f, 0,-1,0, 1,1,
         0.5f,0,  0.5f, 0,-1,0, 1,1,
        -0.5f,0,  0.5f, 0,-1,0, 0,1,
        -0.5f,0, -0.5f, 0,-1,0, 0,0,

        // Face avant
        -0.5f,0,0.5f, 0,0.894f,0.447f, 0,0,
         0.5f,0,0.5f, 0,0.894f,0.447f, 1,0,
         0.0f,0.8f,0.0f, 0,0.894f,0.447f, 0.5f,1,

        // Face droite
         0.5f,0,0.5f, 0.447f,0.894f,0, 0,0,
         0.5f,0,-0.5f, 0.447f,0.894f,0, 1,0,
         0.0f,0.8f,0.0f, 0.447f,0.894f,0, 0.5f,1,

        // Face arrière
         0.5f,0,-0.5f, 0,0.894f,-0.447f, 0,0,
        -0.5f,0,-0.5f, 0,0.894f,-0.447f, 1,0,
         0.0f,0.8f,0.0f, 0,0.894f,-0.447f, 0.5f,1,

        // Face gauche
        -0.5f,0,-0.5f, -0.447f,0.894f,0, 0,0,
        -0.5f,0,0.5f, -0.447f,0.894f,0, 1,0,
         0.0f,0.8f,0.0f, -0.447f,0.894f,0, 0.5f,1
      };
    shape->vertices = verts;
}
Shape Shape_create(ShapeType type) {
    Shape shape;
    switch (type) {
        case CUBE:
            Shape_generateCube(&shape);
            break;
        case PYRAMID:
            Shape_generatePyramid(&shape);
            break;
    }
    Shape_init(&shape);
    return shape;
}