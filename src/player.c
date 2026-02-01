#include "player.h"
#include "camera.h"
#include <stdlib.h>
#include <string.h>

void player_init(vec3 pos, struct player** player) {
    struct player* p = malloc(sizeof(struct player));
    memcpy(p->position, pos, sizeof(vec3));
    struct aabb* box = malloc(sizeof(struct aabb));
    vec3 player_size = { 1.0f, 1.0f, 2.0f };
    memcpy(box->dimension, player_size, sizeof(vec3));
    p->hitbox = box;
    // Set camera to height of player
    vec3 cam_pos = { 0.5f, 0.5f, 2.0f };
    glm_vec3_add(cam_pos, pos, cam_pos);
    camera_init(&p->camera);
    camera_set_position(p->camera, cam_pos);
    *player = p;
}

void player_camera_set_position(struct player* player) {
    vec3 cam_pos = { 0.5f, 0.5f, 2.0f };
    glm_vec3_add(cam_pos, player->position, cam_pos);
    camera_set_position(player->camera, cam_pos);
}

void player_rotate(struct player* player, vec2 offset) {
    camera_rotate(player->camera, offset);
}

void player_move(struct player* player, enum DIRECTION move) {
    vec3 unit_direction = { 0 };
    glm_normalize_to(player->camera->direction, unit_direction);
    if (move == FORWARD) {
        // Do nothing, we move in unit_direction direction
    } else if (move == BACKWARD) {
        // Go in the reverse direction
        vec3 neg = { -1.0f, -1.0f, -1.0f };
        glm_vec3_mul(neg, unit_direction, unit_direction);
    } else if (move == LEFT) {
        // Right hand rule - this will be on the left (negative)
        glm_vec3_crossn(player->camera->up, unit_direction, unit_direction);
    } else if (move == RIGHT) {
        // Right hand rule - this will be on the righ (positive)
        glm_vec3_crossn(unit_direction, player->camera->up, unit_direction);
    }
    float scale = 0.8f;
    glm_vec3_scale(unit_direction, scale, unit_direction);
    glm_vec3_add(player->position, unit_direction, player->position);
    player_camera_set_position(player);
}

void player_update(struct player* player, struct shader* shader) {
    camera_update(player->camera, shader);
}
