#include "player.h"
#include "camera.h"
#include "cglm/affine.h"
#include "config.h"
#include "cglm/cglm.h"
#include "cglm/mat4.h"
#include "cglm/vec4.h"
#include "chunk.h"
#include "shader.h"
#include "util.h"
#include "world.h"
#include <SDL2/SDL_messagebox.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MIN(x, y) (x < y) ? x : y
#define MAX(x, y) (x > y) ? x : y
#define SQUARE(x) x*x
#define MAX_WALK_VELOCITY 10
#define MAX_JUMP_VELOCIY 10
// Note: Difference between friction and move scale will essentially give
// you net accel - how fast will you reach top speed
// Higher the numbers for both, the snappier the movment feels. If it were 10 vs 20, 
// it feels slippery like ice (because FRICTION determines how quickly velocity drops) 
// so higher both, the snappier
#define FRICTION 100
#define MOVE_SCALE 110 
#define JUMP_SCALE 50000
#define GRAVITY -30.0f
#define BLOCK_RANGE 7

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

int player_can_move_x(struct player* player, struct engine* engine, float mov);
int player_can_move_y(struct player* player, struct engine* engine, float mov);
int player_can_move_z(struct player* player, struct engine* engine, float mov);
void player_load_debug(struct player* player);


void player_init(vec3 pos, struct player** player) {
    struct player* p = malloc(sizeof(struct player));
    // TODO: Prevents some form of memory corruption? Why...???
    memset(p, 0, sizeof(struct player));
    memcpy(p->position, pos, sizeof(vec3));
    struct aabb* box = malloc(sizeof(struct aabb));
    vec3 player_size = { 0.6f, 1.8f, -0.6f };
    // Little offset to use when calculating movement
    vec3 box_start = { 0.2f, 0.1f, -0.2f };
    memcpy(box->dimension, player_size, sizeof(vec3));
    memcpy(box->start, box_start, sizeof(vec3));
    p->hitbox = box;
    // Set camera to height of player
    vec3 cam_pos = { player_size[0] / 2.0f, 1.8f, player_size[2] / 2.0f };
    glm_vec3_add(cam_pos, pos, cam_pos);
    camera_init(&p->camera);
    camera_set_position(p->camera, cam_pos);

    // Load debug stuff
    player_load_debug(p);
    // Load UI data
    player_load_ui(p);
    *player = p;
}

void player_camera_set_position(struct player* player) {
    vec3 cam_pos = { 0.5, 1.8f, -0.5 };
    glm_vec3_add(cam_pos, player->position, cam_pos);
    camera_set_position(player->camera, cam_pos);
}

void player_rotate(struct player* player, vec2 offset) {
    camera_rotate(player->camera, offset);
}

void player_move(struct player* player, struct engine* engine, enum DIRECTION move, double dt) {
    vec3 unit_direction = { 0 };
    glm_normalize_to(player->camera->direction, unit_direction);
    // Remove any "up" axis part
    unit_direction[1] = 0.0f;
    glm_vec3_normalize(unit_direction);
    if (move == FORWARD) {
        // Do nothing, we move in unit_direction direction
    } else if (move == BACKWARD) {
        // Go in the reverse direction
        vec3 neg = { -1.0f, 0.0f, -1.0f };
        glm_vec3_mul(neg, unit_direction, unit_direction);
    } else if (move == LEFT) {
        // Right hand rule - this will be on the left (negative)
        glm_vec3_crossn(player->camera->up, unit_direction, unit_direction);
    } else if (move == RIGHT) {
        // Right hand rule - this will be on the righ (positive)
        glm_vec3_crossn(unit_direction, player->camera->up, unit_direction);
        // Jump only if grounded and player is going downwards or still
    } else if (move == JUMP && player->grounded && player->velocity[1] <= 0.0f) {
        unit_direction[0] = 0.0f;
        unit_direction[1] = 1.0f * JUMP_SCALE;
        unit_direction[2] = 0.0f;
        player->grounded = 0;
    } else {
        glm_vec3_zero(unit_direction);
    }
    // Only allow with jumps
    if (move != JUMP) {
        unit_direction[1] = 0.0f;
    }
    glm_vec3_scale(unit_direction, MOVE_SCALE, unit_direction);
    glm_vec3_add(player->accel, unit_direction, player->accel);
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
    // Friction calculation 
    // Get the direction of normalized motion (-velocity/speed) and then scale that by the friction constant.
    float speed = glm_vec3_norm(player->velocity) + 0.01f;
    float friction = FRICTION;
    vec3 f2 = { -player->velocity[0] * friction / speed, 0.0f, -player->velocity[2] * friction / speed };
    // Apply the friction to acceleration (F = ma), so force/accel applied here
    glm_vec3_add(player->accel, f2, player->accel);


    // Velocity = a * dt
    vec3 velocity = { 0.0f };
    glm_vec3_scale(player->accel, dt, velocity);
    // Add dv/dt (vec3 velocity) caused by the acceleration
    glm_vec3_add(player->velocity, velocity, player->velocity);
    // fprintf(stderr, "Accel");
    // glm_vec3_print(player->accel, stderr);
    // fprintf(stderr, "Vel");
    // glm_vec3_print(player->velocity, stderr);
    
    // Reset accel to zeros - this is because there are no more forces on the player. If there are, they will 
    // change acceleration next update, so we don't have to "store" acceleration - that would imply a constantly applying
    // force which is incorrect. 
    player->accel[0] = 0;
    // Set y-axis to gravity
    player->accel[1] = GRAVITY;
    player->accel[2] = 0;

    vec3 displacement = { 0 };
    vec3 dt_velocity = { 0 };
    glm_vec3_scale(player->velocity, dt, dt_velocity);
    // Check if can move
    if (!player_can_move_x(player, engine, dt_velocity[0])) {
        player->velocity[0] = 0.0f;
    }
    if (!player_can_move_y(player, engine, dt_velocity[1])) {
        player->velocity[1] = 0.0f;
        if (dt_velocity[1] <= 0.0f) {
            player->grounded = 1;
        }
    } else {
        if (dt_velocity[1] <= 0.0f) {
            player->grounded = 0;
        } 
    }
    if (!player_can_move_z(player, engine, dt_velocity[2]) ) {
        player->velocity[2] = 0.0f;
    }
    if (fabsf(player->velocity[0]) > MAX_WALK_VELOCITY) {
        if (player->velocity[0] > 0) {
            player->velocity[0] =  MAX_WALK_VELOCITY;
        } else {
            player->velocity[0] =  -MAX_WALK_VELOCITY;
        }
    }
    if (fabsf(player->velocity[1]) > MAX_JUMP_VELOCIY) {
        if (player->velocity[1] > 0) {
            player->velocity[1] =  MAX_JUMP_VELOCIY;
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

// See: https://en.wikipedia.org/wiki/Slab_method
float player_ray_block_intersect(struct player* player, struct world* world, vec3 coords) {
    vec3 step = { 0 };
    glm_vec3_normalize_to(player->camera->direction, step);
    float t_close = -INFINITY;
    for (int i = 0; i < 3; i++) {
        if (player->camera->direction[i] != 0) {
            float high = (i != 2) ? 1.0 : -1.0f;
            float t_i_low = (coords[i] - player->camera->position[i]) / step[i];
            float t_i_high = ((coords[i] + high) - player->camera->position[i] ) / step[i];
            float t_i_close = MIN(t_i_low, t_i_high);
            t_close = MAX(t_i_close, t_close);
        }
    }
    return t_close;
}

void player_draw(struct player* player, struct world* world, struct shader* shader) {
    glBindVertexArray(player->_vao_debug);
    // Get projection from camera direction to block
    // Allow highlighting "range" blocks around player
    // Get "step" vector for raycasting
    vec3 step = { 0 };
    glm_normalize_to(player->camera->direction, step);
    float scale = 0.1f;
    glm_vec3_scale(step,scale, step);
    float magnitude = glm_vec3_norm(step);
    vec3 ray_position = { 0 };
    glm_vec3_add(ray_position, player->camera->position, ray_position);
    //Found a target block
    int found = 0;
    while (1) {
        if (magnitude >= BLOCK_RANGE) {
            break;
        }
        // is_block == 0 if there is block
        int is_block = world_chunk_block_get(world, ray_position, NULL);
        if (is_block == 0) {
            found = 1;
            break;
        }
        glm_vec3_add(ray_position, step, ray_position);
        magnitude += glm_vec3_norm(step);
    }
    if (!found) {
        return;
    }
    //So ray_position is at block coordinates
    float x = floorf(ray_position[0]);
    float y = floorf(ray_position[1]);
    // Ceil cause negative
    float z = ceilf(ray_position[2]);
    //DONE: Raycasting sometimes points to block underneath because it player_physics_check_collision
    //right through the center of 3 blocks.... need some way to fix
    // ABOVE is fixed with smaller scale step
    mat4 translate;
    glm_mat4_identity(translate);
    glm_mat4_scale(translate, 1.2f);
    vec3 t = { x, y, z };
    glm_translate(translate, t);
    set_uniform_mat4("model", shader, translate);
    glLineWidth(10.0f);
    glDrawElements(GL_LINES, player->debug_vertex_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
void player_draw_ui(struct player* player, struct shader* shader) {
    mat4 ortho;
    glm_ortho(0.0f, SCREEN_WIDTH, 0.0f, SCREEN_HEIGHT, 0.0f, 1.0f, ortho);
    set_uniform_mat4("projection", shader, ortho);
    // =========== DRAW UI ================
    glBindVertexArray(player->_vao_ui);
    mat4 translate;
    vec3 color = { 0, 0, 0 };
    glm_mat4_identity(translate);
    // Scale down crosshair
    set_uniform_mat4("model", shader, translate);
    set_uniform_vec3("color", shader, color);
    glLineWidth(5.0f);
    glDrawElements(GL_LINES, player->ui_vertex_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    // =========== DRAW HOTBAR ================
    glBindVertexArray(player->inventory._vao_inventory);
    float x_unit = 1 / 12.0f;
    for (int i = 0; i < 10; i++) {
        vec3 hb_color = { 1.0f, 1.0f, 1.0f };
        set_uniform_vec3("color", shader, hb_color);
        glm_mat4_identity(translate);
        vec4 pos = { (i + 1) * x_unit * SCREEN_WIDTH, 0.0f, -0.1f, 0.0f };
        glm_translate(translate, pos);
        set_uniform_mat4("model", shader, translate);
        glDrawElements(GL_TRIANGLES, player->inventory.inventory_vertex_count, GL_UNSIGNED_INT, 0);

        
        enum BLOCK_ID hotbar_block = player->inventory.blocks[i];

        glm_mat4_identity(translate);
        float scale = 0.9f;
        // (1 - scale)/2.0f * "box" width
        float offset = (1 - scale)/2.0f * (1/12.0f) * SCREEN_WIDTH;
        vec4 inbox_pos = { (i + 1) * x_unit * SCREEN_WIDTH + offset, offset, 0.0f, 0.0f };
        glm_translate(translate, inbox_pos);
        glm_scale_uni(translate, scale);
        set_uniform_mat4("model", shader, translate);


        vec3 new_hb_color = { 206 / 255.0f, 206 / 255.0f, 206 / 255.0f };
        set_uniform_vec3("color", shader, new_hb_color);
        glDrawElements(GL_TRIANGLES, player->inventory.inventory_vertex_count, GL_UNSIGNED_INT, 0);
    }
    glBindVertexArray(0);
}

void player_load_ui(struct player* player) {
    // =============== Crosshair Data ===================
    vec2 center = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    float scaler = SCREEN_WIDTH * (1 / 48.0f);
    float crosshair[] = {
        center[0] + scaler, center[1], 
        center[0] - scaler, center[1], 
        center[0], center[1] + scaler, 
        center[0], center[1] - scaler, 
    };
    int crosshair_draw_order[] = {
        0, 1, 2, 3
    };
    // =============== Hotbar Data ===================
    float box_scaler = SCREEN_WIDTH * (1 / 12.0f);
    float hotbar_item[] = {
        box_scaler, box_scaler, // top-right
        1.0f, 1.0f,
        0.0f, box_scaler, // top-left
        0.0f, 1.0f,
        0.0f, 0.0f, // bottom-left
        0.0f, 0.0f,
        box_scaler, 0.0f, // bottom-right
        1.0f, 0.0f,

    };
    int hotbar_draw_order[] = {
        1, 2, 3,   3, 0, 1, // CCW 2-triangles (quad)
    };

    player->ui_vertex_count = ARRAY_SIZE(crosshair_draw_order);
    glGenVertexArrays(1, &player->_vao_ui);
    glBindVertexArray(player->_vao_ui);
    create_vbo(&player->_vbo_ui, (void*)crosshair, sizeof(crosshair));
    create_ebo(&player->_ebo_ui, (void*)crosshair_draw_order, sizeof(crosshair_draw_order));
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, player->_vbo_ui);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, player->_ebo_ui);
    glBindVertexArray(0);

    // === Send hotbar data
    player->inventory.inventory_vertex_count = ARRAY_SIZE(hotbar_item);
    glGenVertexArrays(1, &player->inventory._vao_inventory);
    glBindVertexArray(player->inventory._vao_inventory);
    create_vbo(&player->inventory._vbo_inventory, (void*)hotbar_item, sizeof(hotbar_item));
    create_ebo(&player->inventory._ebo_inventory, (void*)hotbar_draw_order, sizeof(hotbar_draw_order));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, player->inventory._vbo_inventory);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*) (2 * sizeof(float)));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, player->inventory._ebo_inventory);
    glBindVertexArray(0);
}
void player_load_debug(struct player* player) {
    // =============== Face Data ===================
    float front_face[] = {
        1.0f, 1.0f, 0.0f, // top-right
        0.0f, 0.0f, 1.0f, // Front normal
        1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, // top-left
        0.0f, 0.0f, 1.0f, // Front normal
        0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, // bottom-left
        0.0f, 0.0f, 1.0f, // Front normal
        0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, // bottom-right
        0.0f, 0.0f, 1.0f, // Front normal
        1.0f, 0.0f,
    };
    float back_face[] = {
        0.0f, 1.0f, -1.0f, // top-left (back plane)
        0.0f, 0.0f, -1.0f, // Back normal
        0.0f, 1.0f,
        1.0f, 1.0f, -1.0f, // top-right (back plane)
        0.0f, 0.0f, -1.0f, // Back normal
        1.0f, 1.0f,
        1.0f, 0.0f, -1.0f, // bottom-right (back plane)
        0.0f, 0.0f, -1.0f, // Back normal
        1.0f, 0.0f,
        0.0f, 0.0f, -1.0f, // bottom-left (back plane)
        0.0f, 0.0f, -1.0f, // Back normal
        0.0f, 0.0f,
    };
    float right_face[] = {
        1.0f, 1.0f, -1.0f, // top-right (back plane)
        1.0f, 0.0f, 0.0f, // Right normal
        1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, // top-right
        1.0f, 0.0f, 0.0f, // Right normal
        0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, // bottom-right
        1.0f, 0.0f, 0.0f, // Right normal
        0.0f, 0.0f,
        1.0f, 0.0f, -1.0f, // bottom-right (back plane)
        1.0f, 0.0f, 0.0f, // Right normal
        1.0f, 0.0f,
    };
    float left_face[] = {
        0.0f, 1.0f, 0.0f, // top-left
        -1.0f, 0.0f, 0.0f, // Left normal
        1.0f, 1.0f,
        0.0f, 1.0f, -1.0f, // top-left (back plane)
        -1.0f, 0.0f, 0.0f, // Left normal
        0.0f, 1.0f,
        0.0f, 0.0f, -1.0f, // bottom-left (back plane)
        -1.0f, 0.0f, 0.0f, // Left normal
        0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, // bottom-left
        -1.0f, 0.0f, 0.0f, // Left normal
        1.0f, 0.0f,
    };
    float top_face[] = {
        1.0f, 1.0f, -1.0f, // top-right (back plane)
        0.0f, 1.0f, 0.0f, // Top normal
        1.0f, 1.0f,
        0.0f, 1.0f, -1.0f, // top-left (back plane)
        0.0f, 1.0f, 0.0f, // Top normal
        0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, // top-left
        0.0f, 1.0f, 0.0f, // Top normal
        0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, // top-right
        0.0f, 1.0f, 0.0f, // Top normal
        1.0f, 0.0f,
    };
    float bottom_face[] = {
        0.0f, 0.0f, -1.0f, // bottom-left (back plane)
        0.0f, -1.0f, 0.0f, // Bottom normal
        0.0f, 1.0f,
        1.0f, 0.0f, -1.0f, // bottom-right (back plane)
        0.0f, -1.0f, 0.0f, // Bottom normal
        1.0f, 1.0f,
        1.0f, 0.0f, 0.0f, // bottom-right
        0.0f, -1.0f, 0.0f, // Bottom normal
        1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, // bottom-left
        0.0f, -1.0f, 0.0f, // Bottom normal
        0.0f, 0.0f,
    };

    int vertex_draw_order[] = {
        0, 1, 1, 2, 2, 3, 3, 0,
        // 1, 2, 3,   3, 0, 1, // CCW 2-triangles (quad)
    };

    int face_size = ARRAY_SIZE(vertex_draw_order);
    int num_faces = 6;
    int tmp_order[face_size * num_faces];
    float tmp_vertex[ARRAY_SIZE(front_face) * num_faces];
    int size = ARRAY_SIZE(front_face);
    memcpy(tmp_vertex, front_face, sizeof(front_face));
    memcpy(tmp_vertex + size, back_face, sizeof(front_face));
    memcpy(tmp_vertex + 2 * size, right_face, sizeof(front_face));
    memcpy(tmp_vertex + 3 * size, left_face, sizeof(front_face));
    memcpy(tmp_vertex + 4 * size, top_face, sizeof(front_face));
    memcpy(tmp_vertex + 5 * size, bottom_face, sizeof(front_face));
    for (int i = 0; i < num_faces; i++) {
        int* data = chunk_face_order_add(vertex_draw_order, sizeof(vertex_draw_order), i*4);
        memcpy(tmp_order + i*face_size, data, face_size * sizeof(int));
        free(data);
    }

    player->debug_vertex_count = ARRAY_SIZE(tmp_order);
    glGenVertexArrays(1, &player->_vao_debug);
    glBindVertexArray(player->_vao_debug);
    // Do the same for debug VBOs and EBOs
    create_vbo(&player->_vbo_debug, (void*)tmp_vertex, sizeof(tmp_vertex));
    create_ebo(&player->_ebo_debug, (void*)tmp_order, sizeof(tmp_order));
    // Enable 3 attribs - position normals texture
    glEnableVertexAttribArray(0);
    // glEnableVertexAttribArray(1);
    // glEnableVertexAttribArray(2);
    // set vao_buffer to pos buffer obj
    glBindBuffer(GL_ARRAY_BUFFER, player->_vbo_debug);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    // set vao_buffer to normals buffer obj
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(3*sizeof(float)));
    // // set vao_buffer to texture buffer obj
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(6*sizeof(float)));
    // Set EBO to the vertex_order
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, player->_ebo_debug);
    //NOTE: This is important, otherwise with multiple chunk_load calls, it
    //creates a segfault since the bindings get all messed up. Why it gets
    //messed up? Let's say we make 2 chunks. Chunk 1 creates VBOs, then VAO,
    //then binds everything. Now VAO is still bound. Chunk 2 init starts. First
    //call is create_vbo. Since VAO is already bound, it gets bound to the OLD
    //VAO!! Always clear before use. 
    glBindVertexArray(0);
}

void player_block_delete(struct player* player, struct world* world) {
    vec3 step = { 0 };
    glm_normalize_to(player->camera->direction, step);
    float scale = 0.1f;
    glm_vec3_scale(step,scale, step);
    float magnitude = glm_vec3_norm(step);
    vec3 ray_position = { 0 };
    glm_vec3_add(ray_position, player->camera->position, ray_position);
    //Found a target block
    int found = 0;
    while (1) {
        if (magnitude >= BLOCK_RANGE) {
            fprintf(stderr, "out of range\n");
            break;
        }
        // is_block == 0 if there is block
        int is_block = world_chunk_block_get(world, ray_position, NULL);
        if (is_block == 0) {
            found = 1;
            break;
        }
        glm_vec3_add(ray_position, step, ray_position);
        magnitude += glm_vec3_norm(step);
    }
    if (!found) {
        return;
    }
    //So ray_position is at block coordinates
    float x = floorf(ray_position[0]);
    float y = floorf(ray_position[1]);
    // Ceil cause negative
    float z = ceilf(ray_position[2]);
    vec3 block_pos = { x, y, z };
    glm_vec3_print(block_pos, stderr);
    world_chunk_block_delete(world, block_pos);
}

void player_block_place(struct player* player, struct world* world) {
    vec3 step = { 0 };
    glm_normalize_to(player->camera->direction, step);
    float scale = 0.1f;
    glm_vec3_scale(step,scale, step);
    float magnitude = glm_vec3_norm(step);
    vec3 ray_position = { 0 };
    glm_vec3_add(ray_position, player->camera->position, ray_position);
    //Found a target block
    int found = 0;
    while (1) {
        if (magnitude >= BLOCK_RANGE) {
            break;
        }
        // is_block == 0 if there is block
        int is_block = world_chunk_block_get(world, ray_position, NULL);
        if (is_block == 0) {
            found = 1;
            break;
        }
        glm_vec3_add(ray_position, step, ray_position);
        magnitude += glm_vec3_norm(step);
    }
    if (!found) {
        return;
    }
    //So ray_position is at block coordinates
    int x = floorf(ray_position[0]);
    int y = floorf(ray_position[1]);
    // Ceil cause negative
    int z = ceilf(ray_position[2]);

    vec3 block_coords = { x, y, z };

    fprintf(stderr, "BLOCK");
    glm_vec3_print(block_coords, stderr);

    float t = player_ray_block_intersect(player, world, block_coords);
    vec3 point_of_contact = { 0 };
    glm_vec3_add(point_of_contact, player->camera->position, point_of_contact);
    // Reset step value to camera direction normal
    // glm_normalize_to(player->camera->direction, step);
    glm_normalize_to(player->camera->direction, step);
    // glm_vec3_scale(step,scale, step);
    // p(t) = t * step + origin
    glm_vec3_scale(step, t, step);
    glm_vec3_add(point_of_contact, step, point_of_contact);
    // Store the value of the offset from block coords, this means
    // offset will have 1 coord with value "1"
    vec3 offset = { 0 };
    glm_vec3_sub(point_of_contact, block_coords, offset);
    glm_vec3_print(offset, stderr);
    if (offset[0] == 0.0f) {
        block_coords[0] -= 1;
    }
    if (offset[0] == 1.0f) {
        block_coords[0] += 1;
    }
    if (offset[1] == 0.0f) {
        block_coords[1] -= 1;
    }
    if (offset[1] == 1.0f) {
        block_coords[1] += 1;
    }
    if (offset[2] == 0.0f) {
        block_coords[2] += 1;
    }
    if (offset[2] == -1.0f) {
        block_coords[2] -= 1;
    }
    glm_vec3_print(block_coords, stderr);
    int nx = floorf(block_coords[0]);
    //Note: OpenGL FLIP
    int ny = floorf(-block_coords[2]);
    int nz = floorf(block_coords[1]);
    // In World coords, not opengl coords
    vec3 world_block_coords = { nx, ny, nz };
    world_chunk_block_place(world, world_block_coords, BLOCK_STONE);
}

// Return 1 if intersection, 0 if not
int _aabb_edge_projection_check(vec3* aabb, int aabb_v_count, vec3 point, vec3 normal) {
    int positive_count = 0;
    int negative_count = 0;
    for (int i = 0; i < aabb_v_count; i++) {
        vec3 target = { 0 };
        glm_vec3_sub(aabb[i], point, target);
        float dot = glm_vec3_dot(normal, target);
        if (dot > 0) {
            positive_count++;
        } else if (dot < 0) {
            negative_count++;
        }
        if (positive_count && negative_count) {
            // Not a separating axis
            return 1;
        }
    }
    return 0;
}

int player_is_point_in_frustum(struct player* player, vec2 chunk_coord) {
    mat4 clip_space;
    glm_mat4_mul(player->camera->perspective, player->camera->view, clip_space);

    // Switch to row-major to extract planes from the view-perspective matrix
    glm_mat4_transpose(clip_space);
    vec4 p1 = { 0 };
    glm_vec3_add(clip_space[3], clip_space[0], p1);
    vec4 p2 = { 0 };
    glm_vec3_sub(clip_space[3], clip_space[0], p2);
    vec4 p3 = { 0 };
    glm_vec3_add(clip_space[3], clip_space[1], p3);
    vec4 p4 = { 0 };
    glm_vec3_sub(clip_space[3], clip_space[1], p4);
    vec4 p5 = { 0 };
    glm_vec3_add(clip_space[3], clip_space[2], p5);
    vec4 p6 = { 0 };
    glm_vec3_sub(clip_space[3], clip_space[2], p6);
    vec4* faces[6] = { &p1, &p2, &p3, &p4, &p5, &p6 };

    vec4 chunk_coords[8] = {
        { chunk_coord[0] * CHUNK_WIDTH,  -INFINITY, -chunk_coord[1] * CHUNK_LENGTH, 1.0f },
        { chunk_coord[0] * CHUNK_WIDTH + CHUNK_WIDTH, -INFINITY, -chunk_coord[1] * CHUNK_LENGTH, 1.0f },
        { chunk_coord[0] * CHUNK_WIDTH, -INFINITY, -chunk_coord[1] * CHUNK_LENGTH - CHUNK_LENGTH, 1.0f },
        { chunk_coord[0] * CHUNK_WIDTH + CHUNK_WIDTH, -INFINITY, -chunk_coord[1] * CHUNK_LENGTH - CHUNK_LENGTH, 1.0f },

        { chunk_coord[0] * CHUNK_WIDTH,  INFINITY, -chunk_coord[1] * CHUNK_LENGTH, 1.0f },
        { chunk_coord[0] * CHUNK_WIDTH + CHUNK_WIDTH, INFINITY, -chunk_coord[1] * CHUNK_LENGTH, 1.0f },
        { chunk_coord[0] * CHUNK_WIDTH, INFINITY, -chunk_coord[1] * CHUNK_LENGTH - CHUNK_LENGTH, 1.0f },
        { chunk_coord[0] * CHUNK_WIDTH + CHUNK_WIDTH, INFINITY, -chunk_coord[1] * CHUNK_LENGTH - CHUNK_LENGTH, 1.0f },
    };


    for (int i = 0; i < 6; i++) {
        int count = 0;
        for (int j = 0; j < 8; j++) {
            float dot = glm_vec4_dot(*faces[i], chunk_coords[j]);
            int sign = (dot < 0) ? 1 : 0;
            count += sign;
        }
        // If all coords are outside a frustum plane, exit early
        if (count == 8) {

    fprintf(stderr, "cull");
    glm_vec2_print(chunk_coord, stderr);
        return 0;
        }
    }

    mat4 vp_inv;
    mat4 p_inv;
    mat4 v_inv;
    glm_mat4_inv_fast(player->camera->perspective, p_inv);
    glm_mat4_inv_fast(player->camera->view, v_inv);
    glm_mat4_mul(v_inv, p_inv, vp_inv);
    // Normalized clip space frustum coords, we'll translate them to world-space
    vec4 nf1 = { -1.0f, -1.0f, -1.0f, 1.0f };
    vec4 nf2 = { -1.0f, 1.0f, -1.0f, 1.0f };
    vec4 nf3 = { 1.0f, -1.0f, -1.0f, 1.0f };
    vec4 nf4 = { 1.0f, 1.0f, -1.0f, 1.0f };

    vec4 nf5 = { -1.0f, -1.0f, 1.0f, 1.0f };
    vec4 nf6 = { -1.0f, 1.0f, 1.0f, 1.0f };
    vec4 nf7 = { 1.0f, -1.0f, 1.0f, 1.0f };
    vec4 nf8 = { 1.0f, 1.0f, 1.0f, 1.0f };

    vec4 f1;
    vec4 f2;
    vec4 f3;
    vec4 f4;
    vec4 f5;
    vec4 f6;
    vec4 f7;
    vec4 f8;
    glm_mat4_mulv(vp_inv, nf1, f1);
    glm_mat4_mulv(vp_inv, nf2, f2);
    glm_mat4_mulv(vp_inv, nf3, f3);
    glm_mat4_mulv(vp_inv, nf3, f3);
    glm_mat4_mulv(vp_inv, nf4, f4);
    glm_mat4_mulv(vp_inv, nf5, f5);
    glm_mat4_mulv(vp_inv, nf6, f6);
    glm_mat4_mulv(vp_inv, nf7, f7);
    glm_mat4_mulv(vp_inv, nf8, f8);
    glm_vec4_divs(f1, f1[3], f1);
    glm_vec4_divs(f2, f2[3], f2);
    glm_vec4_divs(f3, f3[3], f3);
    glm_vec4_divs(f4, f4[3], f4);
    glm_vec4_divs(f5, f5[3], f5);
    glm_vec4_divs(f6, f6[3], f6);
    glm_vec4_divs(f7, f7[3], f7);
    glm_vec4_divs(f8, f8[3], f8);

    vec4* frustum_coords[8];
    frustum_coords[0] = &f1;
    frustum_coords[1] = &f2;
    frustum_coords[2] = &f3;
    frustum_coords[3] = &f4;
    frustum_coords[4] = &f5;
    frustum_coords[5] = &f6;
    frustum_coords[6] = &f7;
    frustum_coords[7] = &f8;

    vec4 chunk_faces[6] = { 
        { 1.0f, 0.0f, 0.0f, -(chunk_coord[0] * CHUNK_WIDTH) },
        { -1.0f, 0.0f, 0.0f, (chunk_coord[0] * CHUNK_WIDTH + CHUNK_WIDTH) },
        { 0.0f, 0.0f, -1.0f, (-chunk_coord[1] * CHUNK_LENGTH) },
        { 0.0f, 0.0f, 1.0f, -(-chunk_coord[1] * CHUNK_LENGTH - CHUNK_LENGTH) },
        { 0.0f, 1.0f, 0.0f, -(-INFINITY) },
        { 0.0f, -1.0f, 0.0f, INFINITY },
    };
    for (int i = 0; i < 6; i++) {
        int count = 0;
        for (int j = 0; j < 8; j++) {
            float dot = glm_vec4_dot(chunk_faces[i], *frustum_coords[j]);
            int sign = (dot < 0) ? 1 : 0;
            count += sign;
        }
        // If all coords are outside a frustum plane, exit early
        if (count == 8) return 0;
    }

    return 1;
}
