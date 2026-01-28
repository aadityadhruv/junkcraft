#pragma once
#include "camera.h"
#include "cglm/cglm.h"

struct player {
    vec3 position;
    struct camera* camera;
};
