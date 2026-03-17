//
// Created by killian on 3/17/26.
//

#ifndef CHERRYENGINE_SHAPE_H
#define CHERRYENGINE_SHAPE_H

typedef enum ShapeType_t {
    CUBE,
    PYRAMID
}ShapeType;

typedef struct Shape_t {
    float* vertices;
    unsigned int nbVertices;
    unsigned int VBO;
    unsigned int VAO;
}Shape;

void Shape_generateCube(Shape* shape);
void Shape_generatePyramid(Shape* shape);

Shape Shape_create(ShapeType type);
void Shape_init(Shape* shape);
void Shape_clean(Shape* shape);
void Shape_draw(Shape* shape);

#endif //CHERRYENGINE_SHAPE_H