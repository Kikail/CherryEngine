//
// Created by killian on 5/12/26.
//

#ifndef CHERRYENGINE_METADATA_H
#define CHERRYENGINE_METADATA_H

// PROCHAINE ETAPE
// CREER UN FICHIER DE METADATA POUR CHAQUE RESOURCE
// LE PROGRAMME VA D ABORD SCANNER TOUT LES FICHIERS PUIS CREER DES METADATA SI PAS DEJA FAIT
// ENSUITE IL VA IMPORTER LE FICHIER GRACE AU METADATA DANS LEQUEL IL VA RECUP TOUTES LES INFOS
// CELA VA FACILITER L UTILISATION DE RESOURCE DANS LE MOTEUR COMME PAR EXEMPLE MESHRENDERER QUI STOCKERA
// UNIQUEMENT LA SINATURE DU MESH ET DU SHADER QU IL A BESOIN AU LIEU DU CHEMIN FICHIER

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>

#include "filePicker.h"
#include "serializer.h"

typedef struct MetaData_t{
    char path[512];
    char name[512];
    unsigned int type;
    unsigned int signature;
}MetaData;

MetaData MetaData_create(char* filepath, char* name, FileType filetype);
bool MetaData_save(MetaData* metaData);
MetaData MetaData_load(SerialObject* serial_object);
bool MetaData_doExists(char* filepath);
void MetaData_check(char* filepath, char* name, FileType filetype);
void Path_ReplaceExtension(char* dest, const char* src, const char* newExt);
unsigned int Hash_String(const char* str);
unsigned int Hash_Path(const char* path);

#define afficher_hash(p) printf("%s -> %u\n",p,Hash_Path(p))

#endif //CHERRYENGINE_METADATA_H
