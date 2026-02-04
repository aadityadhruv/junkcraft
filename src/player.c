#include "player.h"
#include "camera.h"
#include "cglm/io.h"
#include "cglm/vec3.h"
#include "chunk.h"
#include "world.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_WALK_VELOCITY 10
// Note: Difference between friction and move scale will essentially give
// you net accel - how fast will you reach top speed
// Higher the numbers for both, the snappier the movment feels. If it were 10 vs 20, 
// it feels slippery like ice (because FRICTION determines how quickly velocity drops) 
// so higher both, the snappier
#define FRICTION 100
#define MOVE_SCALE 110 
#define JUMP_SCALE 500
#define GRAVITY -30.0f

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

void player_move(struct player* player, struct engine* engine, enum DIRECTION move, double dt) {
    vec3 unit_direction = { 0 };
    glm_normalize_to(player->camera->direction, unit_direction);
    if (move == FORWARD) {
        fprintf(stderr, "for\n");
        // Do nothing, we move in unit_direction direction
    } else if (move == BACKWARD) {
        fprintf(stderr, "back\n");
        // Go in the reverse direction
        vec3 neg = { -1.0f, -1.0f, -1.0f };
        glm_vec3_mul(neg, unit_direction, unit_direction);
    } else if (move == LEFT) {
        fprintf(stderr, "left\n");
        // Right hand rule - this will be on the left (negative)
        glm_vec3_crossn(player->camera->up, unit_direction, unit_direction);
    } else if (move == RIGHT) {
        fprintf(stderr, "right\n");
        // Right hand rule - this will be on the righ (positive)
        glm_vec3_crossn(unit_direction, player->camera->up, unit_direction);
        // Jump only if grounded and player is going downwards or still
    } else if (move == JUMP && player->grounded && player->velocity[1] <= 0.0f) {
        fprintf(stderr, "JUMP\n");
        unit_direction[0] = 0.0f;
        unit_direction[1] = 1.0f * JUMP_SCALE;
        unit_direction[2] = 0.0f;
        player->grounded = 0;
    } else {
        glm_vec3_zero(unit_direction);
    }
    // float scale = 0.1f;
    // glm_vec3_scale(unit_direction, scale, unit_direction);
    // Check if can move
    // if (!player_can_move_x(player, engine, unit_direction[0])) {
    //     fprintf(stderr, "Cannot move X\n");
    //     unit_direction[0] = 0.0f;
    // }
    // if (!player_can_move_y(player, engine, unit_direction[1])) {
    //     fprintf(stderr, "Cannot move Y\n");
    //     unit_direction[1] = 0.0f;
    // }
    // if (!player_can_move_z(player, engine, unit_direction[2]) ) {
    //     fprintf(stderr, "Cannot move Z\n");
    //     unit_direction[2] = 0.0f;
    // }
    // if (fabsf(player->velocity[0]) > MAX_WALK_VELOCITY) {
    //     if (player->velocity[0] > 0) {
    //         player->velocity[0] =  MAX_WALK_VELOCITY;
    //     } else {
    //         player->velocity[0] =  -MAX_WALK_VELOCITY;
    //     }
    // }
    // if (fabsf(player->velocity[2]) > MAX_WALK_VELOCITY) {
    //     if (player->velocity[2] > 0) {
    //         player->velocity[2] =  MAX_WALK_VELOCITY;
    //     } else {
    //         player->velocity[2] =  -MAX_WALK_VELOCITY;
    //     }
    // }
    // Only allow with jumps
    if (move != JUMP) {
        unit_direction[1] = 0.0f;
    }
    glm_vec3_scale(unit_direction, MOVE_SCALE, unit_direction);
    glm_vec3_add(player->accel, unit_direction, player->accel);
    // glm_vec3_print(unit_direction, stderr);
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
    if (mov <= 0) h = 0;
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
    if (mov > 0) l = 0;
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

void player_physics(struct player* player, struct engine* engine, double dt) {
    // fprintf(stderr, "Grounded state: %d\n", player->grounded);
    // float w = player->hitbox->dimension[0];
    // float h = player->hitbox->dimension[1];
    // float l = player->hitbox->dimension[2];
    //
    // vec3 lifted_pos = { 0 };
    // glm_vec3_add(player->position, player->hitbox->start, lifted_pos);
    //
    // vec3 pc1 = { lifted_pos[0], lifted_pos[1], lifted_pos[2] };
    // vec3 pc2 = { lifted_pos[0] + w, lifted_pos[1], lifted_pos[2] };
    // vec3 pc3 = { lifted_pos[0], lifted_pos[1], lifted_pos[2] + l };
    // vec3 pc4 = { lifted_pos[0] + w, lifted_pos[1], lifted_pos[2] + l };
    // // fprintf(stderr, "Player Position:\n");
    // // glm_vec3_print(player->position, stderr);
    // // If no block anywhere near feet
    // if (
    //         world_chunk_block_get(engine->world, pc1, NULL) &&
    //         world_chunk_block_get(engine->world, pc2, NULL) &&
    //         world_chunk_block_get(engine->world, pc3, NULL) &&
    //         world_chunk_block_get(engine->world, pc4, NULL)
    //    ) {
    //     player->grounded = 0;
    // } else {
    //     player->grounded  = 1;
    // }


    // Friction calculation 
    // Get the direction of normalized motion (-velocity/speed) and then scale that by the friction constant.
    float speed = glm_vec3_norm(player->velocity) + 0.01f;
    float friction = FRICTION;
    vec3 f2 = { -player->velocity[0] * friction / speed, -player->velocity[1] * 10 / speed, -player->velocity[2] * friction / speed };
    // Apply the friction to acceleration (F = ma), so force/accel applied here
    glm_vec3_add(player->accel, f2, player->accel);


    // Velocity = a * dt
    vec3 velocity = { 0.0f };
    glm_vec3_scale(player->accel, dt, velocity);
    // Add dv/dt (vec3 velocity) caused by the acceleration
    glm_vec3_add(player->velocity, velocity, player->velocity);
    fprintf(stderr, "Accel");
    glm_vec3_print(player->accel, stderr);
    fprintf(stderr, "Vel");
    glm_vec3_print(player->velocity, stderr);
    
    // Reset accel to zeros - this is because there are no more forces on the player. If there are, they will 
    // change acceleration next update, so we don't have to "store" acceleration - that would imply a constantly applying
    // force which is incorrect. 
    player->accel[0] = 0;
    // Set y-axis to gravity
    player->accel[1] = GRAVITY;
    player->accel[2] = 0;

    vec3 displacement = { 0 };
    vec3 unit_velocity = { 0 };
    glm_vec3_normalize_to(player->velocity, unit_velocity);
    // Check if can move
    if (!player_can_move_x(player, engine, unit_velocity[0])) {
        // fprintf(stderr, "Cannot move X\n");
        player->velocity[0] = 0.0f;
    }
    if (!player_can_move_y(player, engine, unit_velocity[1])) {
        // fprintf(stderr, "Cannot move Y\n");
        player->velocity[1] = 0.0f;
        // fprintf(stderr, "GROUNDED: %f\n", unit_velocity[1]);
        if (unit_velocity[1] <= 0.0f) {
            player->grounded = 1;
        }
    } else {
        // fprintf(stderr, "NOT GROUNDED: %f\n", unit_velocity[1]);
        if (unit_velocity[1] <= 0.0f) {
            player->grounded = 0;
        } 
    }
    if (!player_can_move_z(player, engine, unit_velocity[2]) ) {
        // fprintf(stderr, "Cannot move Z\n");
        player->velocity[2] = 0.0f;
    }
    if (fabsf(player->velocity[0]) > MAX_WALK_VELOCITY) {
        if (player->velocity[0] > 0) {
            player->velocity[0] =  MAX_WALK_VELOCITY;
        } else {
            player->velocity[0] =  -MAX_WALK_VELOCITY;
        }
    }
    if (fabsf(player->velocity[2]) > MAX_WALK_VELOCITY) {
        if (player->velocity[2] > 0) {
            player->velocity[2] =  MAX_WALK_VELOCITY;
        } else {
            player->velocity[2] =  -MAX_WALK_VELOCITY;
        }
    }
    glm_vec3_scale(player->velocity, dt, displacement);
    glm_vec3_add(player->position, displacement, player->position);
    player_camera_set_position(player);
}
