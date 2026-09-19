#pragma once
#include "block.h"
#include "world.h"
#include "camera.h"
#include "item.h"
#include "cglm/cglm.h"


struct engine;
struct aabb {
    vec3 dimension;
    vec3 start;
};
struct player_data {
    vec3 position;
    int chunk_coords[2];
    struct aabb hitbox;
    enum ITEM_ID items[40];
    int curr;
    float weight;
    vec3 direction;
    vec3 up;
    vec3 velocity;
    vec3 accel;
    int grounded;
};
struct player_graphics {
    struct camera camera;
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
    GLuint _vao_inventory;
    GLuint _vbo_inventory;
    GLuint _ebo_inventory;
    int inventory_vertex_count;
};
struct player {
    struct player_data data;
    struct player_graphics graphics;
};

void player_data_init(vec3 pos, struct player_data* player);

void player_camera_set_position(struct player* player);

void player_rotate(struct player_data* player, vec2 offset);

void player_move(struct player_data* player, enum DIRECTION move, double dt);

void player_update(struct player* player, struct shader* shader);

void player_physics(struct player_data* player, struct world* world, double dt);

void player_draw(struct player* player, struct world* world, struct shader* shader);

int player_block_delete(struct player_data* player, struct world* world);

int player_block_place(struct player_data* player, struct world* world, enum BLOCK_ID blk_id);
void player_use(struct player_data* player, struct world* world);

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
void player_move_hotbar(struct player_data* player, int direction);
void player_load(struct player* player);
