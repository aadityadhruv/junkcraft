#pragma once

#include "cglm/cglm.h"
enum BLOCK_ID {
    BLOCK_GRASS,
    BLOCK_STONE,
    BLOCK_ROCK,
    BLOCK_SAND,
    BLOCK_SNOW,
    BLOCK_WOOD,
    BLOCK_LEAF,
    BLOCK_ID_COUNT,
};
enum block_face {
    BLOCK_FRONT,
    BLOCK_BACK,
    BLOCK_RIGHT,
    BLOCK_LEFT,
    BLOCK_TOP,
    BLOCK_BOTTOM,
    BLOCK_FACE_COUNT,
};


struct block_face_texture {
    vec2 top_right;
    vec2 top_left;
    vec2 bottom_left;
    vec2 bottom_right;
};

struct block_metadata {
    struct block_face_texture texture_data[BLOCK_FACE_COUNT];
};


struct block {
    enum BLOCK_ID block_id;
};
/**
 * A block struct defines what kind of block we will be rendering. It's the metadata of the block array in a chunk
 *
 */
int block_init(struct block* blk, enum BLOCK_ID block_id);
void block_metadata_init();
