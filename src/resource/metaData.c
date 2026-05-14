//
// Created by killian on 5/12/26.
//
#include "metaData.h"
#include <string.h>

#include "fileSaver.h"
#include "serializer.h"

MetaData MetaData_create(char* filepath, char* name, FileType filetype) {
    MetaData metaData;

    strcpy(metaData.path, filepath);
    strcpy(metaData.name, name);
    metaData.type = filetype;
    metaData.signature = Hash_Path(filepath);

    return metaData;
}

bool MetaData_save(MetaData* metaData) {
    char filepath[512];
    Path_ReplaceExtension(filepath, metaData->path, ".meta");

    SerialObject serialObject = SerialObject_create("METADATA");
    SerialValue path = SerialValue_create_str("path", metaData->path);
    SerialValue name = SerialValue_create_str("filename", metaData->name);
    SerialValue signature = SerialValue_create_uint("signature", metaData->signature);
    SerialValue type = SerialValue_create_uint("type", metaData->type);
    SerialObject_AddSerialValue(&serialObject, &path);
    SerialObject_AddSerialValue(&serialObject, &name);
    SerialObject_AddSerialValue(&serialObject, &signature);
    SerialObject_AddSerialValue(&serialObject, &type);

    FileSaver* fileSaver = FileSaver_create(filepath, SerialObject_Serialize(&serialObject));
    FileSaver_save(fileSaver);

    return true;
}
MetaData MetaData_load(SerialObject* serial_object) {
    MetaData metaData;

    SerialValue path = SerialObject_GetByName(serial_object, "path");
    SerialValue filename = SerialObject_GetByName(serial_object, "filename");
    SerialValue signature = SerialObject_GetByName(serial_object, "signature");
    SerialValue type = SerialObject_GetByName(serial_object, "type");

    char* p = SerialValue_GetStringValue(&path);
    char* fn = SerialValue_GetStringValue(&filename);
    strcpy(metaData.path, p);
    strcpy(metaData.name, fn);

    metaData.signature = SerialValue_GetUintValue(&signature);
    metaData.type = SerialValue_GetUintValue(&type);

    return metaData;
}

void MetaData_check(char* filepath, char* name, FileType filetype) {
    if (!MetaData_doExists(filepath)) {
        MetaData metaData = MetaData_create(filepath, name, filetype);
        MetaData_save(&metaData);
    }
    else {
        MetaData metaData = MetaData_create(filepath, name, filetype);
        MetaData_save(&metaData);
    }
}

bool MetaData_doExists(char* filepath) {
    char fp[512];
    Path_ReplaceExtension(fp, filepath, ".meta");

    FILE* file = fopen(fp, "r");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}

unsigned int Hash_String(const char* str) {
    unsigned int hash = 0x811C9DC5;
    unsigned int prime = 0x01000193;
    for (int i = 0; str[i] != '\0'; i++) {
        hash ^= (unsigned char)str[i];
        hash *= prime;
    }
    return hash;
}
unsigned int Hash_Path(const char* path) {
    char normalized[512];
    int i = 0;

    while (path[i] != '\0' && i < 511) {
        char c = path[i];
        if (c == '\\') c = '/'; // Windows -> Unix style
        normalized[i] = (char)tolower(c);
        i++;
    }
    normalized[i] = '\0';

    return Hash_String(normalized);
}

void Path_ReplaceExtension(char* dest, const char* src, const char* newExt) {
    // 1. Copier la source dans la destination
    if (dest != src) {
        strcpy(dest, src);
    }

    // 2. Trouver le dernier point '.'
    char* lastDot = strrchr(dest, '.');
    char* lastSlash = strrchr(dest, '/');
#ifdef _WIN32
    char* lastBackslash = strrchr(dest, '\\');
    if (lastBackslash > lastSlash) lastSlash = lastBackslash;
#endif

    // Si le point est avant le dernier slash, ce n'est pas une extension de fichier
    // (c'est un point dans un nom de dossier)
    if (lastDot != NULL && (lastSlash == NULL || lastDot > lastSlash)) {
        *lastDot = '\0'; // On coupe la chaîne au point
    }

    // 3. Ajouter la nouvelle extension
    // On vérifie si l'utilisateur a mis le '.' dans newExt
    if (newExt[0] != '.') {
        strcat(dest, ".");
    }
    strcat(dest, newExt);
}




































