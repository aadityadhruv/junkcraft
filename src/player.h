#pragma once
#include "engine.h"
#include "camera.h"
#include "cglm/cglm.h"


struct engine;
struct aabb {
    vec3 dimension;
    vec3 start;
};
struct player {
    vec3 position;
    struct camera* camera;
    struct aabb* hitbox;
    float weight;
    vec3 velocity;
    vec3 accel;
    int grounded;
};

void player_init(vec3 pos, struct player** player);

void player_camera_set_position(struct player* player);

void player_rotate(struct player* player, vec2 offset);

void player_move(struct player* player, struct engine* engine, enum DIRECTION move, double dt);

void player_update(struct player* player, struct shader* shader);

void player_physics(struct player* player, struct engine* engine, double dt);
