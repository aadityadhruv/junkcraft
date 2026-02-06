#pragma once
#include "block.h"
#include "cglm/types.h"
#include "shader.h"
#include "texture.h"
#include "world.h"
#include <stdint.h>
#include <junk/vector.h>
#define CHUNK_WIDTH 16
#define CHUNK_LENGTH 16
#define CHUNK_HEIGHT 128

enum biome {
    JUNK_BIOME_PLAINS,
};


//NOTE: Forward declare world here. There is a circular dependency between 
// chunk and world, and the order in which compiler sees them causes issues.
// World.c compiles -> includes world.h -> first line is include chunk.h -> struct world
// is used here, which messes it up. Forward declare to avoid errors
struct world;
/**
 * A chunk is the "basic" rendering unit used - it will allocate buffers for all blocks in a chunk, hide covered faces of blocks, generate a chunk mesh and dispatch that buffer data to the GPU
 *
 */
struct chunk {
    struct block* blocks[CHUNK_WIDTH][CHUNK_LENGTH][CHUNK_HEIGHT];
    GLuint _vao;
    GLuint _vbo;
    GLuint _ebo;
    int vertex_count;
    mat4 model;
    enum biome biome;
    vec2 coord;
    int loaded;
    int staged_for_load;
};

/**
 * Generate a chunk at coords for the given world. Memory allocation for chunk is
 * handled by the function.
 *
 */
int chunk_gen(struct world* wld, vec2 coord, struct chunk** chunk);
/**
 * Load a chunk to the given coordinates. Essentially, a chunk only knows of
 * it's local coordinate system. We want to load this particular chunk to a
 * location in WORLD coordinates, which is what coord is. This vec2 will be
 * used to translate the blocks that constitute the chunk
 *
 * The chunk will allocate a VAO/VBO/EBO buffer to render the chunk mesh. This GPU data is usually not updated in the loop, unless a chunk_update is called
 * @param The world where this chunk belongs. It is a useful object to have, especially for face culling
 * @param chunk Chunk to load
 * @param coord coordinates in world space
 */
void chunk_load(struct world* world, struct chunk* chunk, int coord[2]);
/**
 * Chunk updates are performed on already loaded chunks. It will redraw the
 * chunk mesh as needed based on block updates and whatnot.
 * @param chunk Chunk to load
 * @param coord coordinates in world space
 */
void chunk_update(struct chunk* chunk);
/**
 * Unload a chunk. Delete GPU and memory data, not the chunk data itself
 *
 * @param chunk Chunk to load
 */
void chunk_unload(struct chunk* chunk);
/*
 * This dispatches calls to OpenGL to draw the chunk.
 * @param chunk Chunk to draw
 * @param shader Shader to pass to block_draw
 * @param texture Textures that block_draw will use
 */
void chunk_draw(struct chunk* chunk, struct shader* shader, struct texture* texture);

/**
 * Get a block in a chunk
 * @param target chunk
 * @param pos block coordinates in chunk coordinates
 * @param block value to store resultant value in. if NULL, won't store value
 * @return 0 if there is a block, 1 if there is no block
 */
int chunk_block_get(struct chunk* chunk, vec3 pos, struct block** block);
int* chunk_face_order_add(int* face_order, int size, int idx);
