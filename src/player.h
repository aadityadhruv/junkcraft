#pragma once
#include "camera.h"
#include "cglm/cglm.h"

struct aabb {
    vec3 dimension;
};
struct player {
    vec3 position;
    struct camera* camera;
    struct aabb* hitbox;
    float weight;
    vec3 velocity;
    vec3 accel;
};

void player_init(vec3 pos, struct player** player);

void player_camera_set_position(struct player* player);

void player_rotate(struct player* player, vec2 offset);

void player_move(struct player* player, enum DIRECTION move);

void player_update(struct player* player, struct shader* shader);
