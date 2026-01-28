#pragma once
#include "cglm/types.h"
#include "shader.h"


enum DIRECTION {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};
struct camera {
    vec3 position;
    vec3 direction;
    vec3 up;
    mat4 view;
    mat4 perspective;
    float fov;
};

void camera_init(struct camera** camera);
void camera_update(struct camera* camera, struct shader* shader);
void camera_set_position(struct camera* camera, vec3 pos);
void camera_move(struct camera* camera, enum DIRECTION move);
void camera_rotate(struct camera* camera, vec2 offset);
