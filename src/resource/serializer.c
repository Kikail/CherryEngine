#include "serializer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ==========================================
// UTILITAIRES DE CHAINES
// ==========================================

char* TrimString(const char* str) {
    if (!str) return NULL;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) {
        char* empty = malloc(1);
        empty[0] = '\0';
        return empty;
    }
    const char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    int len = end - str + 1;
    char* res = malloc(len + 1);
    strncpy(res, str, len);
    res[len] = '\0';
    return res;
}

char* RemoveBrackets(const char* input) {
    char* trimmed = TrimString(input);
    int len = strlen(trimmed);
    if (len >= 2 && trimmed[0] == '{' && trimmed[len - 1] == '}') {
        char* res = malloc(len - 1);
        strncpy(res, trimmed + 1, len - 2);
        res[len - 2] = '\0';
        free(trimmed);
        return res;
    }
    return trimmed;
}

// ==========================================
// SERIAL VALUE
// ==========================================

SerialValue SerialValue_create_int(const char* name, int value) {
    SerialValue sv;
    sv.name = strdup(name);
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", value);
    sv.value = strdup(buffer);
    return sv;
}

SerialValue SerialValue_create_uint(const char* name, unsigned int value) {
    SerialValue sv;
    sv.name = strdup(name);
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%u", value);
    sv.value = strdup(buffer);
    return sv;
}

SerialValue SerialValue_create_double(const char* name, double value) {
    SerialValue sv;
    sv.name = strdup(name);
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%f", value);
    sv.value = strdup(buffer);
    return sv;
}

SerialValue SerialValue_create_str(const char* name, const char* value) {
    SerialValue sv;
    sv.name = strdup(name);
    sv.value = strdup(value);
    return sv;
}

SerialValue SerialValue_parse(const char* serialized) {
    SerialValue sv;
    char* clean = RemoveBrackets(serialized);
    char* comma = strchr(clean, ',');
    
    if (comma) {
        *comma = '\0'; // Coupe la chaîne en deux
        sv.name = TrimString(clean);
        sv.value = TrimString(comma + 1);
    } else {
        sv.name = strdup("invalid");
        sv.value = strdup("-1");
    }
    free(clean);
    return sv;
}

char* SerialValue_Serialize(const SerialValue* sv) {
    int len = strlen(sv->name) + strlen(sv->value) + 4; // "{name,value}\0"
    char* res = malloc(len);
    snprintf(res, len, "{%s,%s}", sv->name, sv->value);
    return res;
}

const char* SerialValue_GetName(const SerialValue* sv) { return sv->name; }
const char* SerialValue_GetStringValue(const SerialValue* sv) { return sv->value; }
int SerialValue_GetIntValue(const SerialValue* sv) { return atoi(sv->value); }
unsigned int SerialValue_GetUintValue(const SerialValue* sv) { return atoi(sv->value); }
double SerialValue_GetDoubleValue(const SerialValue* sv) { return atof(sv->value); }
float SerialValue_GetFloatValue(const SerialValue* sv) { return (float)atof(sv->value); }
bool SerialValue_IsValid(const SerialValue* sv) { return strcmp(sv->value, "-1") != 0; }

// ==========================================
// SERIAL OBJECT
// ==========================================

SerialObject SerialObject_create(const char* name) {
    SerialObject so;
    so.num_svalues = 0;
    so.cap_svalues = 4;
    so.svalues = malloc(so.cap_svalues * sizeof(SerialValue));
    
    so.num_childrens = 0;
    so.cap_childrens = 4;
    so.childrens = malloc(so.cap_childrens * sizeof(SerialObject));
    
    so.pos = 0;

    if (name && strlen(name) > 0) {
        SerialObject_AddSerialValue(&so, &((SerialValue){strdup("name"), strdup(name)}));
    }
    return so;
}

void SerialObject_AddSerialValue(SerialObject* so, const SerialValue* sv) {
    if (so->num_svalues >= so->cap_svalues) {
        so->cap_svalues *= 2;
        so->svalues = realloc(so->svalues, so->cap_svalues * sizeof(SerialValue));
    }
    // Copie profonde (deep copy)
    so->svalues[so->num_svalues].name = strdup(sv->name);
    so->svalues[so->num_svalues].value = strdup(sv->value);
    so->num_svalues++;
}

void SerialObject_AddChild(SerialObject* so, const SerialObject* child) {
    if (so->num_childrens >= so->cap_childrens) {
        so->cap_childrens *= 2;
        so->childrens = realloc(so->childrens, so->cap_childrens * sizeof(SerialObject));
    }
    // On copie la structure entière (attention, en C c'est une copie de surface des pointeurs, 
    // pour une vraie copie profonde, il faudrait une fonction dédiée).
    so->childrens[so->num_childrens] = *child;
    so->childrens[so->num_childrens].pos = so->pos + 1;
    so->num_childrens++;
}

SerialValue SerialObject_GetByName(const SerialObject* so, const char* name) {
    for (int i = 0; i < so->num_svalues; i++) {
        if (strcmp(so->svalues[i].name, name) == 0) {
            return so->svalues[i];
        }
    }
    return (SerialValue){strdup("null"), strdup("-1")};
}

SerialObject* SerialObject_GetObjectByName(SerialObject* so, const char* name) {
    for (int i = 0; i < so->num_childrens; i++) {
        SerialValue nameVal = SerialObject_GetByName(&so->childrens[i], "name");
        if (strcmp(nameVal.value, name) == 0) {
            // Attention : fuite mémoire potentielle ici si nameVal n'est pas free
            return &so->childrens[i];
        }
    }
    return so->num_childrens > 0 ? &so->childrens[0] : NULL;
}

// Fonction utilitaire pour concaténer dynamiquement
static void AppendString(char** dest, const char* src) {
    int old_len = *dest ? strlen(*dest) : 0;
    int src_len = strlen(src);
    *dest = realloc(*dest, old_len + src_len + 1);
    strcpy(*dest + old_len, src);
}

char* SerialObject_Serialize(const SerialObject* so) {
    char* result = calloc(1, 1);
    char tabs[64] = {0};
    for (unsigned int i = 0; i < so->pos && i < 63; i++) tabs[i] = '\t';

    AppendString(&result, tabs);
    AppendString(&result, "<\n");

    for (int i = 0; i < so->num_svalues; i++) {
        char* sv_str = SerialValue_Serialize(&so->svalues[i]);
        AppendString(&result, tabs);
        AppendString(&result, "\t");
        AppendString(&result, sv_str);
        AppendString(&result, "\n");
        free(sv_str);
    }

    for (int i = 0; i < so->num_childrens; i++) {
        char* child_str = SerialObject_Serialize(&so->childrens[i]);
        AppendString(&result, child_str);
        free(child_str);
    }

    AppendString(&result, tabs);
    AppendString(&result, ">\n");
    return result;
}

SerialObject SerialObject_DeserializeSingle(FILE* file) {
    SerialObject root = {0};
    SerialObject stack[64]; // Limite arbitraire de profondeur (au lieu de vector)
    int stack_size = 0;
    
    char line[512];
    while (fgets(line, sizeof(line), file)) {
        char* trimmed = TrimString(line);
        if (strlen(trimmed) == 0) { free(trimmed); continue; }

        if (strcmp(trimmed, "<") == 0) {
            stack[stack_size++] = SerialObject_create("");
        } 
        else if (strcmp(trimmed, ">") == 0) {
            if (stack_size > 0) {
                SerialObject finished = stack[--stack_size];
                if (stack_size > 0) {
                    SerialObject_AddChild(&stack[stack_size - 1], &finished);
                } else {
                    root = finished;
                    free(trimmed);
                    break;
                }
            }
        } 
        else if (trimmed[0] == '{') {
            SerialValue sv = SerialValue_parse(trimmed);
            if (stack_size > 0) {
                SerialObject_AddSerialValue(&stack[stack_size - 1], &sv);
            }
        }
        free(trimmed);
    }
    return root;
}

// ==========================================
// GESTION DE LA MÉMOIRE (FREE)
// ==========================================

void SerialValue_Free(SerialValue* sv) {
    if (sv == NULL) return;

    // On libère les chaînes de caractères allouées avec strdup() ou malloc()
    if (sv->name) free(sv->name);
    if (sv->value) free(sv->value);

    // Sécurité : on remet à NULL pour éviter un "double free" si appelé deux fois
    sv->name = NULL;
    sv->value = NULL;
}

void SerialObject_Free(SerialObject* so) {
    if (so == NULL) return;

    // 1. Libérer toutes les valeurs (SerialValue) du tableau
    if (so->svalues) {
        for (int i = 0; i < so->num_svalues; i++) {
            SerialValue_Free(&so->svalues[i]);
        }
        // Libérer le tableau lui-même
        free(so->svalues);
        so->svalues = NULL;
    }

    // 2. Libérer tous les objets enfants (RÉCURSIVITÉ)
    if (so->childrens) {
        for (int i = 0; i < so->num_childrens; i++) {
            SerialObject_Free(&so->childrens[i]);
        }
        // Libérer le tableau d'enfants lui-même
        free(so->childrens);
        so->childrens = NULL;
    }

    // Réinitialiser les compteurs
    so->num_svalues = 0;
    so->cap_svalues = 0;
    so->num_childrens = 0;
    so->cap_childrens = 0;
}

void SerialObject_Print(SerialObject* so) {
    if (so == NULL) return;

    // On crée une chaîne d'indentation basée sur la position (pos)
    char tabs[64] = {0};
    for (unsigned int i = 0; i < so->pos && i < 63; i++) {
        tabs[i] = '\t';
    }

    // Début de l'objet
    printf("%s<\n", tabs);

    // 1. Affichage des valeurs (SerialValues)
    for (int i = 0; i < so->num_svalues; i++) {
        printf("%s\t- %s : %s\n", tabs, so->svalues[i].name, so->svalues[i].value);
    }

    // 2. Affichage des enfants (Récursivité)
    for (int i = 0; i < so->num_childrens; i++) {
        SerialObject_Print(&so->childrens[i]);
    }

    // Fin de l'objet
    printf("%s>\n", tabs);
}