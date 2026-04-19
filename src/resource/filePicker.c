//
// Created by killian on 4/19/26.
//
#include "filePicker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

FileType getFileType(const char* extension) {
    // On initialise par défaut à NONE pour éviter les valeurs aléatoires
    FileType fileType = FILETYPE_NONE;

    // On compare SANS le point, car get_extension l'a déjà retiré
    if (strcmp(extension, "txt") == 0) {
        fileType = FILETYPE_TXT;
    }
    else if (strcmp(extension, "png") == 0) {
        fileType = FILETYPE_PNG;
    }
    else if (strcmp(extension, "jpg") == 0) {
        fileType = FILETYPE_JPG;
    }
    else if (strcmp(extension, "svg") == 0) {
        fileType = FILETYPE_SVG;
    }
    else if (strcmp(extension, "mtl") == 0) {
        fileType = FILETYPE_MTL;
    }
    else if (strcmp(extension, "obj") == 0) {
        fileType = FILETYPE_OBJ;
    }
    else if (strcmp(extension, "fs") == 0) {
        fileType = FILETYPE_FS;
    }
    else if (strcmp(extension, "vs") == 0) {
        fileType = FILETYPE_VS;
    }

    return fileType;
}

const char *get_extension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "Aucune";
    return dot + 1; // Renvoie juste "txt", "png", etc.
}

void lister_recursif(const char *chemin_base, FilePicker* filePicker) {
    struct dirent *entree;
    DIR *rep = opendir(chemin_base);
    char chemin_complet[PATH_MAX];
    char chemin_absolu[PATH_MAX];
    struct stat etat;

    if (rep == NULL) return;

    while ((entree = readdir(rep)) != NULL) {
        // Arrêt d'urgence si on a atteint la limite de fichiers !
        if (filePicker->numFiles >= MAX_FILES_LOADED) {
            break;
        }

        if (strcmp(entree->d_name, ".") == 0 || strcmp(entree->d_name, "..") == 0) {
            continue;
        }

        snprintf(chemin_complet, sizeof(chemin_complet), "%s/%s", chemin_base, entree->d_name);

        if (stat(chemin_complet, &etat) == 0) {
            if (realpath(chemin_complet, chemin_absolu)) {

                if (S_ISDIR(etat.st_mode)) {
                    lister_recursif(chemin_complet, filePicker);
                } else {
                    // Copie sécurisée du chemin (évite les débordements si le chemin est trop long)
                    strncpy(filePicker->files[filePicker->numFiles].path, chemin_absolu, CHERRY_MAX_FILEPATH_LENGTH - 1);
                    filePicker->files[filePicker->numFiles].path[CHERRY_MAX_FILEPATH_LENGTH - 1] = '\0'; // Garantie null-terminated

                    filePicker->files[filePicker->numFiles].type = getFileType(get_extension(entree->d_name));
                    filePicker->numFiles += 1;
                }
            }
        }
    }
    closedir(rep);
}

FilePicker* FilePicker_loadFiles(char* path) {
    // Allocation uniquement pour la structure principale
    FilePicker* filePicker = malloc(sizeof(FilePicker));
    if (filePicker == NULL) return NULL; // Sécurité

    // Plus besoin du malloc pour "files", il est déjà dans la structure FilePicker
    filePicker->numFiles = 0;
    for (int i = 0; i < MAX_FILES_LOADED; i++) {
        filePicker->files[i].type = FILETYPE_NONE;
    }

    lister_recursif(path, filePicker);

    if (filePicker->numFiles == 0) {
        printf("Aucun fichier trouve dans %s\n", path);
    }

    return filePicker;
}

void afficherExtension(FileType fileType) {
    switch (fileType) {
        case FILETYPE_TXT: printf("TXT\n"); break;
        case FILETYPE_PNG: printf("PNG\n"); break;
        case FILETYPE_JPG: printf("JPG\n"); break;
        case FILETYPE_SVG: printf("SVG\n"); break;
        case FILETYPE_MTL: printf("MTL\n"); break;
        case FILETYPE_FS:  printf("FS\n"); break;
        case FILETYPE_VS:  printf("VS\n"); break;
        case FILETYPE_OBJ: printf("OBJ\n"); break;
        case FILETYPE_NONE: printf("INCONNU\n"); break;
    }
}