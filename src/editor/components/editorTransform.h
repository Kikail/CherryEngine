//
// Created by killian on 5/20/26.
//

#ifndef CHERRYENGINE_EDITORTRANSFORM_H
#define CHERRYENGINE_EDITORTRANSFORM_H

#include "../../game/ecs/components/transform.h"

typedef struct Editor_t Editor;

void Editor_Transform_RenderTab(Editor* editor, Transform* transform);

#endif //CHERRYENGINE_EDITORTRANSFORM_H
