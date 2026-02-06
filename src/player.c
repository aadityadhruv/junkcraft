#include "player.h"
#include "camera.h"
#include "cglm/io.h"
#include "cglm/vec3.h"
#include "chunk.h"
#include "shader.h"
#include "util.h"
#include "world.h"
#include <math.h>
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

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

int player_can_move_x(struct player* player, struct engine* engine, float mov);
int player_can_move_y(struct player* player, struct engine* engine, float mov);
int player_can_move_z(struct player* player, struct engine* engine, float mov);
void player_load_debug(struct player* player);


void player_init(vec3 pos, struct player** player) {
    struct player* p = malloc(sizeof(struct player));
    memcpy(p->position, pos, sizeof(vec3));
    struct aabb* box = malloc(sizeof(struct aabb));
    vec3 player_size = { 0.4f, 2.0f, -0.4f };
    // Little offset to use when calculating movement
    vec3 box_start = { 0.0f, 0.1f, 0.0f };
    memcpy(box->dimension, player_size, sizeof(vec3));
    memcpy(box->start, box_start, sizeof(vec3));
    p->hitbox = box;
    // Set camera to height of player
    vec3 cam_pos = { player_size[0] / 2.0f, 2.0f, player_size[2] / 2.0f };
    glm_vec3_add(cam_pos, pos, cam_pos);
    camera_init(&p->camera);
    camera_set_position(p->camera, cam_pos);

    // Load debug stuff
    player_load_debug(p);
    *player = p;
}

void player_camera_set_position(struct player* player) {
    vec3 cam_pos = { player->hitbox->dimension[0] / 2.0f, 2.0f, player->hitbox->dimension[2] / 2.0f };
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
    vec3 f2 = { -player->velocity[0] * friction / speed, -player->velocity[1] * 10 / speed, -player->velocity[2] * friction / speed };
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
    vec3 unit_velocity = { 0 };
    glm_vec3_normalize_to(player->velocity, unit_velocity);
    // Check if can move
    if (!player_can_move_x(player, engine, unit_velocity[0])) {
        player->velocity[0] = 0.0f;
    }
    if (!player_can_move_y(player, engine, unit_velocity[1])) {
        player->velocity[1] = 0.0f;
        if (unit_velocity[1] <= 0.0f) {
            player->grounded = 1;
        }
    } else {
        if (unit_velocity[1] <= 0.0f) {
            player->grounded = 0;
        } 
    }
    if (!player_can_move_z(player, engine, unit_velocity[2]) ) {
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
void player_draw(struct player* player, struct world* world, struct shader* shader) {
    glBindVertexArray(player->_vao_debug);
    // Get projection from camera direction to block
    // Allow highlighting "range" blocks around player
    float range = 7;
    // Get "step" vector for raycasting
    vec3 step = { 0 };
    glm_normalize_to(player->camera->direction, step);
    float magnitude = glm_vec3_norm(step);
    vec3 ray_position = { 0 };
    glm_vec3_add(ray_position, player->camera->position, ray_position);
    //Found a target block
    int found = 0;
    while (1) {
        if (magnitude >= range) {
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
    //Note: OpenGL FLIP
    int y = floorf(ray_position[1]);
    // Ceil cause negative
    int z = ceilf(ray_position[2]);
    //TODO: Raycasting sometimes points to block underneath because it player_physics_check_collision
    //right through the center of 3 blocks.... need some way to fix
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
    // Do the same for debug VBOs and EBOs
    create_vbo(&player->_vbo_debug, (void*)tmp_vertex, sizeof(tmp_vertex));
    create_ebo(&player->_ebo_debug, (void*)tmp_order, sizeof(tmp_order));

    player->debug_vertex_count = ARRAY_SIZE(tmp_order);
    glGenVertexArrays(1, &player->_vao_debug);
    glBindVertexArray(player->_vao_debug);
    // Enable 3 attribs - position normals texture
    glEnableVertexAttribArray(0);
    // glEnableVertexAttribArray(1);
    // glEnableVertexAttribArray(2);
    // set vao_buffer to pos buffer obj
    glBindBuffer(GL_ARRAY_BUFFER, player->_vbo_debug);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    // set vao_buffer to normals buffer obj
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(3*sizeof(float)));
    // set vao_buffer to texture buffer obj
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(6*sizeof(float)));
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
