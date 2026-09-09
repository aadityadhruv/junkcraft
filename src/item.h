#pragma once
#include "glad/glad.h"
#include "cglm/cglm.h"
enum ITEM_ID {
    ITEM_BLOCK_GRASS,
    ITEM_BLOCK_STONE,
    ITEM_ID_COUNT,
};


struct item_texture {
    vec2 top_right;
    vec2 top_left;
    vec2 bottom_left;
    vec2 bottom_right;
};

struct item_metadata {
    struct item_texture texture_data;
};

struct item_graphics {
    GLuint _vao;
    GLuint _vbo;
    GLuint _ebo;
    int vertex_count;
};

struct item {
    enum ITEM_ID item_id;
};
/**
 * An item is a object that can be in a user's inventory. 
 *
 */
void item_metadata_init();
int item_init(struct item* item, enum ITEM_ID item_id);
void item_draw(enum ITEM_ID id);
void item_load();
