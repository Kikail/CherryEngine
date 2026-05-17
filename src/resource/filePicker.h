//
// Created by killian on 4/19/26.
//

#ifndef CHERRYENGINE_FILEPICKER_H
#define CHERRYENGINE_FILEPICKER_H

#define MAX_FILES_LOADED 128
#define MAX_FILENAME_LENGTH 64
#define CHERRY_MAX_FILEPATH_LENGTH 256

typedef enum FileType_e {
    FILETYPE_NONE,
    FILETYPE_TXT,
    FILETYPE_PNG,
    FILETYPE_JPG,
    FILETYPE_SVG,
    FILETYPE_MTL,
    FILETYPE_OBJ,
    FILETYPE_FS,
    FILETYPE_VS,
    FILETYPE_METADATA,
    FILETYPE_MATERIAL
} FileType;

typedef struct CherryFile_t {
    FileType type;
    char path[CHERRY_MAX_FILEPATH_LENGTH];
} CherryFile;

typedef struct FilePicker_t {
    CherryFile files[MAX_FILES_LOADED];
    unsigned int numFiles;
} FilePicker;

// Prototypes de fonctions
void afficherExtension(FileType fileType);
FilePicker* FilePicker_loadFiles(char* path);

#endif //CHERRYENGINE_FILEPICKER_H