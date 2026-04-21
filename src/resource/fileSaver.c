//
// Created by killian on 4/21/26.
//
#include "fileSaver.h"

#include <stdio.h>
#include <stdlib.h>

FileSaver* FileSaver_create(char* fileName, char* content) {
    FileSaver* fileSaver = malloc(sizeof(FileSaver));
    fileSaver->filepath = fileName;
    strcpy(fileSaver->buffer, content);
    return fileSaver;
}
bool FileSaver_addContent(FileSaver* fileSaver, char* content) {
    if (strlen(fileSaver->buffer) + strlen(content) > FILE_SAVER_BUFFER_SIZE - 1) {
        #ifdef DEBUG
            printf("FILESAVER DEPASSE LA CAPACITE DU BUFFER");
        #endif
        return false;
    }
    strcat(fileSaver->buffer, content);
    return true;
}
void FileSaver_save(FileSaver* fileSaver) {
    FILE *fptr;
    fptr = fopen(fileSaver->filepath, "w");
    fprintf(fptr, fileSaver->buffer);
    fclose(fptr);
}