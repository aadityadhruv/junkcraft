#pragma once
#include "chunk.h"
#include <stdint.h>
#define WORLD_LENGTH 32
#define WORLD_WIDTH 32

struct world {
    struct chunk* chunks[WORLD_WIDTH][WORLD_LENGTH];
    int32_t seed;
};

int world_init(int32_t seed, struct world** world);
int world_save(int32_t seed);
int world_get_chunk(struct world* world, int coord[2], struct chunk** chunk);
void world_get_chunk_real_coord(struct world* world, vec2 coord, int out[2]);
/**
 * Get a block in a chunk from world coordinates. This wraps chunk_block_get
 * and fetches a block based on the chunk it is in
 * @param World
 * @param pos block coordinates in world coordinates
 * @param block value to store resultant value in. if NULL, won't store value
 * @return 0 if there is a block, 1 if there is no block
 */
int world_chunk_block_get(struct world* world, vec3 pos, struct block** block);
