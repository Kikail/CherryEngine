//
// Created by killian on 4/21/26.
//

#ifndef CHERRYENGINE_SERIALIZER_H
#define CHERRYENGINE_SERIALIZER_H

#include <stdbool.h>
#include <stdio.h>

// --- SERIAL VALUE ---
typedef struct SerialValue_t {
    char* name;
    char* value;
} SerialValue;

SerialValue SerialValue_create_int(const char* name, int value);
SerialValue SerialValue_create_double(const char* name, double value);
SerialValue SerialValue_create_str(const char* name, const char* value);
SerialValue SerialValue_create_uint(const char* name, unsigned int value);
SerialValue SerialValue_parse(const char* serialized);

char* SerialValue_Serialize(const SerialValue* sv);
const char* SerialValue_GetName(const SerialValue* sv);
const char* SerialValue_GetStringValue(const SerialValue* sv);
int SerialValue_GetIntValue(const SerialValue* sv);
double SerialValue_GetDoubleValue(const SerialValue* sv);
float SerialValue_GetFloatValue(const SerialValue* sv);
unsigned int SerialValue_GetUintValue(const SerialValue* sv);
bool SerialValue_IsValid(const SerialValue* sv);


// --- SERIAL OBJECT ---
typedef struct SerialObject_t {
    SerialValue* svalues;
    int num_svalues;
    int cap_svalues;

    struct SerialObject_t* childrens;
    int num_childrens;
    int cap_childrens;

    unsigned int pos;
} SerialObject;

SerialObject SerialObject_create(const char* name);
void SerialObject_AddSerialValue(SerialObject* so, const SerialValue* sv);
void SerialObject_AddChild(SerialObject* so, const SerialObject* child);
SerialValue SerialObject_GetByName(const SerialObject* so, const char* name);
SerialObject* SerialObject_GetObjectByName(SerialObject* so, const char* name);
char* SerialObject_Serialize(const SerialObject* so);

// Désérialisation
SerialObject SerialObject_DeserializeSingle(FILE* file);

// Utilitaires de chaîne (internes, mais exposés si besoin)
char* TrimString(const char* str);
char* RemoveBrackets(const char* input);

// Nettoyage de la mémoire
void SerialValue_Free(SerialValue* sv);
void SerialObject_Free(SerialObject* so);
void SerialObject_Print(SerialObject* so);

#endif //CHERRYENGINE_SERIALIZER_H