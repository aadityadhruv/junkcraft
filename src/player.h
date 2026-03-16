#pragma once
#include "block.h"
#include "world.h"
#include "engine.h"
#include "camera.h"
#include "cglm/cglm.h"


struct engine;
struct aabb {
    vec3 dimension;
    vec3 start;
};
struct player_inventory {
    // First 10 are hotbar
    enum BLOCK_ID blocks[40];
    GLuint _vao_inventory;
    GLuint _vbo_inventory;
    GLuint _ebo_inventory;
    int inventory_vertex_count;
};
struct player {
    vec3 position;
    struct camera* camera;
    struct aabb* hitbox;
    struct player_inventory inventory;
    float weight;
    vec3 velocity;
    vec3 accel;
    int grounded;
    GLuint _vao_debug;
    GLuint _vbo_debug;
    GLuint _ebo_debug;
    int debug_vertex_count;
    mat4 debug_model;
    GLuint _vao_ui;
    GLuint _vbo_ui;
    GLuint _ebo_ui;
    int ui_vertex_count;
    mat4 ui_model;
};

void player_init(vec3 pos, struct player** player);

void player_camera_set_position(struct player* player);

void player_rotate(struct player* player, vec2 offset);

void player_move(struct player* player, struct engine* engine, enum DIRECTION move, double dt);

void player_update(struct player* player, struct shader* shader);

void player_physics(struct player* player, struct engine* engine, double dt);

void player_draw(struct player* player, struct world* world, struct shader* shader);

void player_block_delete(struct player* player, struct world* world);

void player_block_place(struct player* player, struct world* world);

void player_load_ui(struct player* player);
void player_draw_ui(struct player* player, struct shader* shader);

/**
 * Frustum culling
 *
 * Check for overlap of frustum-chunk and chunk-frustum, and return 1 if there is
 * and overlap, 0 if there is none
 *
 * Used the algorithm from https://iquilezles.org/articles/frustumcorrect/
 * 
 * @param player player struct corresponding to camera to perform frustum culling on
 * @param chunk_coord chunk coordinates in world coords
 *
 */
int player_is_point_in_frustum(struct player* player, vec2 chunk_coord);
