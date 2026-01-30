#pragma once
#include "cglm/types.h"
#include "glad/glad.h"
#include "shader.h"

struct block {
    vec3 coords; 
    GLuint _vao;
    GLuint _vbo;
    GLuint _ebo;
    int _vertex_count;
    mat4 model;
    float angle;
};
/**
 * Create a "block" object, which is the building blocks of this world.
 * Blocks belong in chunks, and chunks belong in worlds. vec3 pos here is the coordinates of the block in WORLD space.
 * However, a common method to render these blocks will be that the chunk will set the coordinates in "chunk space", and
 * on a chunk_load, we will translate the blocks to wherever the chunk is loaded
 *
 *
 */
int block_init(vec3 pos, struct block* blk);
int block_draw(struct block* blk, struct shader* shader);
void block_debug(struct block* blk);
void block_update(struct block* blk);

/**
 * Remove GPU related data of a block. This is usually called by chunk_unload
 *
 */
void block_unload(struct block* blk);
/**
 * Load GPU data of a block
 */
void block_load_gpu(struct block* blk);
