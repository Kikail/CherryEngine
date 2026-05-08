//
// Created by killian on 5/8/26.
//
#include "idMaker.h"

#define MAX_ID ((1<<31)-1)

static unsigned int currentId = 0;

void show_binary(unsigned int number) {
    printf("%u -> ", number);
    for (int i = 31; i >= 0; i--) {
        int bit = (number >> i) & 1;
        printf("%d", bit);
        if (i % 8 == 0 && i != 0) printf(" ");
    }
    printf("\n");
}

bool Id_isGameObject(unsigned int id){
    return ((id & (1<<31)) == 1<<31);
}

unsigned int Id_makeGameObject(unsigned int id){
    return id | 1<<31;
}

bool Id_createId(unsigned int* id){
    if(currentId < (1<<31)){
        *id = currentId;
        currentId += 1;
        return true;
    }
    else{
        return false;
    }
}