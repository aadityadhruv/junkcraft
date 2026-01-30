#pragma once
#include "block.h"
#include "world.h"
#include <stdint.h>
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
struct chunk {
    struct block* blocks[CHUNK_WIDTH][CHUNK_LENGTH][CHUNK_HEIGHT];
    enum biome biome;
    vec2 coord;
    int loaded;
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
 * @param chunk Chunk to load
 * @param coord coordinates in world space
 */
void chunk_load(struct chunk* chunk, int coord[2]);
/**
 * Unload a chunk. Delete block GPU and memory data, not the chunk data itself
 *
 * @param chunk Chunk to load
 */
void chunk_unload(struct chunk* chunk);
/*
 * Similar to block_draw, this dispatches calls to OpenGL to draw the chunk.
 * Technically this wraps block_draw, so block_draw is the one doing all the work
 * @param chunk Chunk to draw
 * @param shader Shader to pass to block_draw
 */
void chunk_draw(struct chunk* chunk, struct shader* shader);
