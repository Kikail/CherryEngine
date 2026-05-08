//
// Created by killian on 5/8/26.
//

#ifndef CHERRYENGINE_IDMAKER_H
#define CHERRYENGINE_IDMAKER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_ID ((1<<31)-1)

typedef struct idMaker_t {
    unsigned int currentId;
}idMaker;

void show_binary(unsigned int number);
bool Id_isGameObject(unsigned int id);
unsigned int Id_makeGameObject(unsigned int id);
bool Id_createId(unsigned int* id);

#endif //CHERRYENGINE_IDMAKER_H
