#include "player.h"
#include "camera.h"
#include "cglm/io.h"
#include "cglm/vec3.h"
#include "chunk.h"
#include "world.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int player_can_move_x(struct player* player, struct engine* engine, float mov);
int player_can_move_y(struct player* player, struct engine* engine, float mov);
int player_can_move_z(struct player* player, struct engine* engine, float mov);

void player_init(vec3 pos, struct player** player) {
    struct player* p = malloc(sizeof(struct player));
    memcpy(p->position, pos, sizeof(vec3));
    struct aabb* box = malloc(sizeof(struct aabb));
    vec3 player_size = { 0.8f, 2.0f, -0.8f };
    // Little offset to use when calculating movement
    vec3 box_start = { 0.0f, 0.1f, 0.0f };
    memcpy(box->dimension, player_size, sizeof(vec3));
    memcpy(box->start, box_start, sizeof(vec3));
    p->hitbox = box;
    // Set camera to height of player
    vec3 cam_pos = { 0.4f, 2.0f, -0.4f };
    glm_vec3_add(cam_pos, pos, cam_pos);
    camera_init(&p->camera);
    camera_set_position(p->camera, cam_pos);
    *player = p;
}

void player_camera_set_position(struct player* player) {
    vec3 cam_pos = { 0.4f, 2.0f, -0.4f };
    glm_vec3_add(cam_pos, player->position, cam_pos);
    camera_set_position(player->camera, cam_pos);
}

void player_rotate(struct player* player, vec2 offset) {
    camera_rotate(player->camera, offset);
}

void player_move(struct player* player, struct engine* engine, enum DIRECTION move) {
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
    float scale = 0.1f;
    glm_vec3_scale(unit_direction, scale, unit_direction);
    // Check if can move
    if (!player_can_move_x(player, engine, unit_direction[0])) {
        // fprintf(stderr, "Cannot move X\n");
        unit_direction[0] = 0.0f;
    }
    // if (!player_can_move_y(player, engine, unit_direction[1])) {
    //     fprintf(stderr, "Cannot move Y\n");
    //     unit_direction[1] = 0.0f;
    // }
    if (!player_can_move_z(player, engine, unit_direction[2])) {
        // fprintf(stderr, "Cannot move Z\n");
        unit_direction[2] = 0.0f;
    }
    // Only allow with jumps
    unit_direction[1] = 0.0f;
    glm_vec3_add(player->position, unit_direction, player->position);
    player_camera_set_position(player);
}

/**
 * Check if player can move on the x-axis. Returns 1 if yes, 0 otherwise.
 */
int player_can_move_x(struct player* player, struct engine* engine, float mov) {
    float w = player->hitbox->dimension[0];
    float h = player->hitbox->dimension[1];
    float l = player->hitbox->dimension[2];
    // Check left plane
    if (mov < 0) w = 0;
    w += mov;
    // This ensures hitbox is slightly above ground
    vec3 lifted_pos = { 0 };
    glm_vec3_add(player->position, player->hitbox->start, lifted_pos);
    vec3 pc1 = { lifted_pos[0] + w, lifted_pos[1], lifted_pos[2] };
    vec3 pc2 = { lifted_pos[0] + w, lifted_pos[1] + h, lifted_pos[2] };
    vec3 pc3 = { lifted_pos[0] + w, lifted_pos[1], lifted_pos[2] + l };
    vec3 pc4 = { lifted_pos[0] + w, lifted_pos[1] + h, lifted_pos[2] + l };
    // Check if block on X-axis
    if (
            world_chunk_block_get(engine->world, pc1, NULL) &&
            world_chunk_block_get(engine->world, pc2, NULL) &&
            world_chunk_block_get(engine->world, pc3, NULL) &&
            world_chunk_block_get(engine->world, pc4, NULL)
       ) {
        return 1;
    } else {
        return 0;
    }
}
/**
 * Check if player can move on the y-axis. Returns 1 if yes, 0 otherwise.
 */
int player_can_move_y(struct player* player, struct engine* engine, float mov) {
    float w = player->hitbox->dimension[0];
    float h = player->hitbox->dimension[1];
    float l = player->hitbox->dimension[2];
    // Check bottom plane
    if (mov < 0) h *= 0;
    h += mov;
    // This ensures hitbox is slightly above ground
    vec3 lifted_pos = { 0 };
    glm_vec3_add(player->position, player->hitbox->start, lifted_pos);
    vec3 pc1 = { lifted_pos[0], lifted_pos[1] + h, lifted_pos[2] };
    vec3 pc2 = { lifted_pos[0] + w, lifted_pos[1] + h, lifted_pos[2] };
    vec3 pc3 = { lifted_pos[0], lifted_pos[1] + h, lifted_pos[2] + l };
    vec3 pc4 = { lifted_pos[0] + w, lifted_pos[1] + h, lifted_pos[2] + l };
    // Check if block on Y-axis
    if (
            world_chunk_block_get(engine->world, pc1, NULL) &&
            world_chunk_block_get(engine->world, pc2, NULL) &&
            world_chunk_block_get(engine->world, pc3, NULL) &&
            world_chunk_block_get(engine->world, pc4, NULL)
       ) {
        return 1;
    } else {
        return 0;
    }
}
/**
 * Check if player can move on the z-axis. Returns 1 if yes, 0 otherwise.
 */
int player_can_move_z(struct player* player, struct engine* engine, float mov) {
    float w = player->hitbox->dimension[0];
    float h = player->hitbox->dimension[1];
    float l = player->hitbox->dimension[2];
    // This ensures hitbox is slightly above ground
    vec3 lifted_pos = { 0 };
    glm_vec3_add(player->position, player->hitbox->start, lifted_pos);
    // Check back plane
    if (mov > 0) l *= 0;
    l += mov;
    vec3 pc1 = { lifted_pos[0], lifted_pos[1], lifted_pos[2] + l };
    vec3 pc2 = { lifted_pos[0] + w, lifted_pos[1], lifted_pos[2] + l };
    vec3 pc3 = { lifted_pos[0], lifted_pos[1] + h, lifted_pos[2] + l };
    vec3 pc4 = { lifted_pos[0] + w, lifted_pos[1] + h, lifted_pos[2] + l };
    // Check if block on Y-axis
    if (
            world_chunk_block_get(engine->world, pc1, NULL) &&
            world_chunk_block_get(engine->world, pc2, NULL) &&
            world_chunk_block_get(engine->world, pc3, NULL) &&
            world_chunk_block_get(engine->world, pc4, NULL)
       ) {
        return 1;
    } else {
        return 0;
    }
}

void player_update(struct player* player, struct shader* shader) {
    camera_update(player->camera, shader);
}
// int player_physics_check_collision() {
//     // Calculate aabb location in world space
//     // Get surface normals 
//     // For each face, take dot product with other box's point
//     // If >= 0, collision
//     struct aabb* box = pb->hitbox;
//     vec3 min = { 0 };
//     memcpy(min, pb->pos, sizeof(vec3));
//     vec3 max = { 0 };
//     glm_vec3_add(pb->pos, box->dimension, max);
//
//     // Each point the static_object bb
//     vec3 points[8] = { 0 };
//     struct aabb* r_box = rb->hitbox;
//     vec3 rp1 = { 0, 0, 0 };
//     vec3 rp2 = { r_box->dimension[0], 0, 0 };
//     vec3 rp3 = { 0, r_box->dimension[1], 0 };
//     vec3 rp4 = { r_box->dimension[0], r_box->dimension[1], 0 };
//     vec3 rp5 = { 0, 0, r_box->dimension[2] };
//     vec3 rp6 = { r_box->dimension[0], 0, r_box->dimension[2] };
//     vec3 rp7 = { 0, r_box->dimension[1], r_box->dimension[2] };
//     vec3 rp8 = { r_box->dimension[0], r_box->dimension[1], r_box->dimension[2] };
//     glm_vec3_add(rb->pos, rp1, points[0]);
//     glm_vec3_add(rb->pos, rp2, points[1]);
//     glm_vec3_add(rb->pos, rp3, points[2]);
//     glm_vec3_add(rb->pos, rp4, points[3]);
//     glm_vec3_add(rb->pos, rp5, points[4]);
//     glm_vec3_add(rb->pos, rp6, points[5]);
//     glm_vec3_add(rb->pos, rp7, points[6]);
//     glm_vec3_add(rb->pos, rp8, points[7]);
//     for (int i = 0; i < 8; i++) {
//         float* point = points[i];
//         // A single point is inside the BB
//         if (
//                 point[0] < max[0] && point[0] > min[0] &&
//                 point[1] < max[1] && point[1] > min[1] &&
//                 point[2] < max[2] && point[2] > min[2]
//            ) {
//             return 1;
//         }
//     }
//
//     return 0;
// }

void player_physics(struct player* player, struct engine* engine) {
    // fprintf(stderr, "Grounded state: %d\n", player->grounded);
    float w = player->hitbox->dimension[0];
    float h = player->hitbox->dimension[1];
    float l = player->hitbox->dimension[2];

    vec3 pc1 = { player->position[0], player->position[1], player->position[2] };
    vec3 pc2 = { player->position[0] + w, player->position[1], player->position[2] };
    vec3 pc3 = { player->position[0], player->position[1], player->position[2] + l };
    vec3 pc4 = { player->position[0] + w, player->position[1], player->position[2] + l };
    // fprintf(stderr, "Player Position:\n");
    // glm_vec3_print(player->position, stderr);
    // If no block anywhere near feet
    if (
            world_chunk_block_get(engine->world, pc1, NULL) &&
            world_chunk_block_get(engine->world, pc2, NULL) &&
            world_chunk_block_get(engine->world, pc3, NULL) &&
            world_chunk_block_get(engine->world, pc4, NULL)
       ) {
        player->grounded = 0;
    } else {
        player->grounded  = 1;
    }

    if (!player->grounded) {
        vec3 gravity = { 0.0f, -0.01f, 0.0f };
        glm_vec3_add(player->position, gravity, player->position);
        player_camera_set_position(player);
    }
}
