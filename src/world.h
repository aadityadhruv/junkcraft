#pragma once
#include "chunk.h"
#include "junk/queue.h"
#include <bits/pthreadtypes.h>
#include <stdint.h>
#define WORLD_LENGTH 128
#define WORLD_WIDTH 128
#define CHUNK_THREADS 16

struct world {
    struct chunk* chunks[WORLD_WIDTH][WORLD_LENGTH];
    struct junk_queue chunk_terrain_queue;
    struct junk_queue chunk_structure_queue;
    pthread_t chunk_threads[CHUNK_THREADS];
    pthread_mutex_t chunk_locks[WORLD_WIDTH][WORLD_LENGTH];
    pthread_mutex_t structure_locks[WORLD_WIDTH][WORLD_LENGTH];
    pthread_mutex_t terrain_gen_lock;
    pthread_mutex_t structure_gen_lock;
    pthread_t structure_threads[CHUNK_THREADS];
    int32_t seed;
};

int world_init(int32_t seed, struct world** world);
int world_save(int32_t seed);
int world_get_chunk(struct world* world, int coord[2], struct chunk** chunk);
void world_get_chunk_real_coord(struct world* world, int coord[2], int out[2]);
/**
 * Get a block in a chunk from world coordinates. This wraps chunk_block_get
 * and fetches a block based on the chunk it is in
 * @param World
 * @param pos block coordinates in world coordinates
 * @param block value to store resultant value in. if NULL, won't store value
 * @return 0 if there is a block, 1 if there is no block
 */
int world_chunk_block_get(struct world* world, vec3 pos, struct block** block);

/**
 * Delete a block in a chunk from world coordinates. This wraps chunk_block_delete 
 * @param World
 * @param pos block coordinates in world coordinates
 * @return 0 if block was deleted, 1 if not
 */
int world_chunk_block_delete(struct world* world, vec3 pos);
/**
 * Place a block in a chunk from world coordinates. This wraps chunk_block_place
 * @param World
 * @param pos block coordinates in world coordinates
 * @param block_id type of block to place
 * @return 0 if block was placed, 1 if not
 */
int world_chunk_block_place(struct world* world, vec3 pos, enum BLOCK_ID block_id);
/**
 * Equivalent to world_get_chunk, but if the chunk doesn't exit, DON'T generate it, set chunk to NULL
 * TODO: Refactor this, combine with world_get_chunk, this repeated code is disgusting
 */
void world_get_chunk_no_gen(struct world* world, int coord[2], struct chunk** chunk);


/**
 * Submit a coordinate coresponding to a chunk for it to be generated
 * If it already exists, nothing will happen. Chunk generation will be handled
 * by the world's chunk_threads array of threads. 
 * @param world The world to generate chunks for
 * @param coordinates of the chunk to generate
 *
 */
void world_submit_chunk_terrain_gen(struct world* world, int coord[2]);
void world_submit_chunk_structure_gen(struct world* world, int coord[2]);
