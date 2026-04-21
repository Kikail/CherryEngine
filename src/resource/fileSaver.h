//
// Created by killian on 4/21/26.
//

#ifndef CHERRYENGINE_FILESAVER_H
#define CHERRYENGINE_FILESAVER_H

#include "stdbool.h"
#include "string.h"

#define FILE_SAVER_BUFFER_SIZE 5096

typedef struct FileSaver_t {
    char* filepath;
    char buffer[FILE_SAVER_BUFFER_SIZE];
}FileSaver;

FileSaver* FileSaver_create(char* fileName, char* content);
bool FileSaver_addContent(FileSaver* fileSaver, char* content);
void FileSaver_save(FileSaver* fileSaver);

#endif //CHERRYENGINE_FILESAVER_H
