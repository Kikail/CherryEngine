//
// Created by killian on 3/11/26.
//

#ifndef CHERRYENGINE_UTILS_H
#define CHERRYENGINE_UTILS_H

#define DEBUG_LOG(x) printf("%s\n",x)
#define DEBUG_isValid(x) ((x==NULL) ? DEBUG_LOG("Not Valid") : DEBUG_LOG("Valid"))
#define DEBUG_LOG_LINE(x) printf("%s",x)
#define DEBUG_loaded(x) do { \
if ((x) == NULL) { \
printf("%s failed to load\n", #x); \
} else { \
printf("%s loaded\n", #x); \
} \
} while(0)

#define DEBUG_showName(x) printf("%s\n",x->name)
#define DEBUG_AddGameObjectComponent(x, y, z) \
if (GameObject_AddComponent(x, y, z)) {\
        DEBUG_LOG("SUCCESS");\
    }\
else {\
    DEBUG_LOG("ERROR");\
}

// Comment this line to disable DEBUG mode
#define DEBUG

#endif //CHERRYENGINE_UTILS_H