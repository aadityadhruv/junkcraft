#include "world.h"
#include "cglm/io.h"
#include "chunk.h"
#include <junk/queue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "random.h"



struct chunk_thread_data {
    int coord[2];
};

/**
 * The main routine run by the threads in world->chunk_threads.
 * It will gain a lock to wld->terrain_gen_lock, pop a a value from the 
 * queue of chunks to be generated, and create it if needed 
 *
 */
void* _thread_world_chunk_terrain_gen(void* world) {
    struct world* wld = (struct world*) world;
    // Only allow up to chunk_threads threads to generate terrain
    while (1) {
        // Pop the head of the terrain gen queue. This has to be an atomic operation
        // If there is nothing in the queue, continue
        pthread_mutex_lock(&wld->terrain_gen_lock);
        if (junk_queue_length(&wld->chunk_terrain_queue) == 0) {
            pthread_mutex_unlock(&wld->terrain_gen_lock);
            continue;
        }
        struct chunk_thread_data* data = junk_queue_pop(&wld->chunk_terrain_queue);
        pthread_mutex_unlock(&wld->terrain_gen_lock);
        // If we actually have something to gen
        if (data) {
            int w = ((abs(data->coord[0]) / WORLD_WIDTH) + 1) * WORLD_WIDTH;
            int l = ((abs(data->coord[1]) / WORLD_LENGTH) + 1) * WORLD_LENGTH;
            int x = (data->coord[0] + w) % WORLD_WIDTH;
            int y = (data->coord[1] + l) % WORLD_LENGTH;
            vec2 new_coord = { x, y };
            // Lock current chunk for terrain generation
            pthread_mutex_lock(&wld->chunk_locks[x][y]);
            struct chunk* c = wld->chunks[x][y];
            // Chunk already generated
            if (c != NULL) {
                free(data);
                pthread_mutex_unlock(&wld->chunk_locks[x][y]);
                continue;
            } else {
                struct chunk* chunk;
                chunk_terrain_gen(wld, new_coord, &chunk);
                wld->chunks[x][y] = chunk;
            }
            free(data);
            // We are done with chunk generation, unlock mutex
            pthread_mutex_unlock(&wld->chunk_locks[x][y]);
        }
    }
    return NULL;
}

/**
 * The main routine run by the threads in world->structure_threads.
 * It will gain a lock to wld->structure_gen_lock, pop a a value from the 
 * queue of chunks to be generated, and create it if needed. If the 
 * structure for the chunk cannot be created (neighbors not ready, chunk not ready)
 * push it to back of queue and get next
 *
 */
void* _thread_world_chunk_structure_gen(void* world) {
    struct world* wld = (struct world*) world;
    // Only allow up to chunk_threads threads to generate structures 
    while (1) {
        // Pop the head of the structure gen queue. This has to be an atomic operation
        // If there is nothing in the queue, continue
        pthread_mutex_lock(&wld->structure_gen_lock);
        if (junk_queue_length(&wld->chunk_structure_queue) == 0) {
            pthread_mutex_unlock(&wld->structure_gen_lock);
            continue;
        }
        struct chunk_thread_data* data = junk_queue_pop(&wld->chunk_structure_queue);
        pthread_mutex_unlock(&wld->structure_gen_lock);
        // If we actually have something to gen
        if (data) {
            struct chunk* chunk;
            // If current chunk is not generated, or if neighbors are not
            // ready yet, push the data (coord) to back of queue and continue
            world_get_chunk_no_gen(wld, data->coord, &chunk);
            if (chunk == NULL) {
                pthread_mutex_lock(&wld->structure_gen_lock);
                junk_queue_push(&wld->chunk_structure_queue, data);
                pthread_mutex_unlock(&wld->structure_gen_lock);
                continue;
            }
            // Hold chunk's structure lock for structure gen
            pthread_mutex_lock(&wld->structure_locks[(int)chunk->coord[0]][(int)chunk->coord[1]]);
            int ret = chunk_structure_gen(wld, chunk);
            // Structures already generated
            if (ret == 1) {
                free(data);
                pthread_mutex_unlock(&wld->structure_locks[(int)chunk->coord[0]][(int)chunk->coord[1]]);
                continue;
            }
            if (ret == -1) {
                pthread_mutex_lock(&wld->structure_gen_lock);
                junk_queue_push(&wld->chunk_structure_queue, data);
                pthread_mutex_unlock(&wld->structure_gen_lock);
                pthread_mutex_unlock(&wld->structure_locks[(int)chunk->coord[0]][(int)chunk->coord[1]]);
                continue;
            }
            free(data);
            pthread_mutex_unlock(&wld->structure_locks[(int)chunk->coord[0]][(int)chunk->coord[1]]);
        }
    }
    return NULL;
}

// LOAD_DISTANCE determines how many chunks are loaded on world creation
#define LOAD_DISTANCE 1

int32_t seed;
enum biome chunk_biomes[WORLD_WIDTH][WORLD_LENGTH];

int world_init(int32_t seed, struct world** world) {
    struct world* wld = malloc(sizeof(struct world));
    memset(wld, 0, sizeof(struct world));
    junk_queue_init(&wld->chunk_terrain_queue);
    junk_queue_init(&wld->chunk_structure_queue);
    wld->seed = seed;
    // world_all_chunk_gen_biome();
    for (int i = -LOAD_DISTANCE; i <= LOAD_DISTANCE; i++) {
        for (int j = -LOAD_DISTANCE; j <= LOAD_DISTANCE; j++) {
            struct chunk* chunk;
            int coords[2] = { i, j };
            world_get_chunk(wld, coords, &chunk);
        }
    }
    pthread_mutex_init(&wld->terrain_gen_lock, 0);
    pthread_mutex_init(&wld->structure_gen_lock, 0);
    for (int i = 0; i < CHUNK_THREADS; i++) {
        pthread_create(&wld->chunk_threads[i], 0, _thread_world_chunk_terrain_gen, wld);
        pthread_create(&wld->structure_threads[i], 0, _thread_world_chunk_structure_gen, wld);
    }
    // Init all chunk's terrain and structure locks
    for (int i = 0; i < WORLD_WIDTH; i++) {
        for (int j = 0; j < WORLD_LENGTH; j++) {
            pthread_mutex_init(&wld->chunk_locks[i][j], 0);
            pthread_mutex_init(&wld->structure_locks[i][j], 0);
        }
    }
    *world = wld;
    return 0;
}

void world_get_chunk_no_gen(struct world* world, int coord[2], struct chunk** chunk) {
    int w = ((abs(coord[0]) / WORLD_WIDTH) + 1) * WORLD_WIDTH;
    int l = ((abs(coord[1]) / WORLD_LENGTH) + 1) * WORLD_LENGTH;
    int x = (coord[0] + w) % WORLD_WIDTH;
    int y = (coord[1] + l) % WORLD_LENGTH;
    struct chunk* c = world->chunks[x][y];
    *chunk = c;
}
void world_submit_chunk_terrain_gen(struct world* world, int coord[2]) {
    struct chunk_thread_data* data = malloc(sizeof(struct chunk_thread_data)); 
    data->coord[0] = coord[0];
    data->coord[1] = coord[1];
    pthread_mutex_lock(&world->terrain_gen_lock);
    junk_queue_push(&world->chunk_terrain_queue, data);
    pthread_mutex_unlock(&world->terrain_gen_lock);
}
void world_submit_chunk_structure_gen(struct world* world, int coord[2]) {
    struct chunk_thread_data* data = malloc(sizeof(struct chunk_thread_data)); 
    data->coord[0] = coord[0];
    data->coord[1] = coord[1];
    pthread_mutex_lock(&world->structure_gen_lock);
    junk_queue_push(&world->chunk_structure_queue, data);
    pthread_mutex_unlock(&world->structure_gen_lock);
}

int world_get_chunk(struct world* world, int coord[2], struct chunk** chunk) {
    int w = ((abs(coord[0]) / WORLD_WIDTH) + 1) * WORLD_WIDTH;
    int l = ((abs(coord[1]) / WORLD_LENGTH) + 1) * WORLD_LENGTH;
    int x = (coord[0] + w) % WORLD_WIDTH;
    int y = (coord[1] + l) % WORLD_LENGTH;
    vec2 new_coord = { x, y };
    struct chunk* c = world->chunks[x][y];
    if (c != NULL) {
        *chunk = c;
    } else {
        chunk_terrain_gen(world, new_coord, chunk);
        world->chunks[x][y] = *chunk;
    }
    return 0;
}

void world_get_chunk_real_coord(struct world* world, vec2 coord, int out[2]) {
    int x = (int)coord[0] % WORLD_WIDTH;
    int y = (int)coord[1] % WORLD_LENGTH;
    out[0] = x;
    out[1] = y;
}

int world_chunk_block_get(struct world* world, vec3 pos, struct block** block) {
    int x = floorf(pos[0]);
    //Note: OpenGL FLIP
    int y = floorf(-pos[2]);
    int z = floorf(pos[1]);
    int curr_chunk[2] = { floorf(x / (float)CHUNK_WIDTH), floorf(y / (float)CHUNK_LENGTH) };
    struct chunk* c = {0};
    world_get_chunk_no_gen(world, curr_chunk, &c);
    if (c == NULL) return 1;
    // Set coords to chunk-local coords
    x = (abs(curr_chunk[0]) * CHUNK_WIDTH + x) % CHUNK_WIDTH;
    y = (abs(curr_chunk[1]) * CHUNK_LENGTH + y) % CHUNK_LENGTH;
    vec3 new_pos = { x, y, z };
    return chunk_block_get(c, new_pos, block);
}
int world_chunk_block_delete(struct world* world, vec3 pos) {
    int x = floorf(pos[0]);
    //Note: OpenGL FLIP
    int y = floorf(-pos[2]);
    int z = floorf(pos[1]);
    int curr_chunk[2] = { floorf(x / (float)CHUNK_WIDTH), floorf(y / (float)CHUNK_LENGTH) };
    struct chunk* c = {0};
    world_get_chunk_no_gen(world, curr_chunk, &c);
    if (c == NULL) return 1;
    // Set coords to chunk-local coords
    x = (abs(curr_chunk[0]) * CHUNK_WIDTH + x) % CHUNK_WIDTH;
    y = (abs(curr_chunk[1]) * CHUNK_LENGTH + y) % CHUNK_LENGTH;
    vec3 new_pos = { x, y, z };
    return chunk_block_delete(c, new_pos);
}
int world_chunk_block_place(struct world* world, vec3 pos, enum BLOCK_ID block_id) {
    int x = pos[0];
    int y = pos[1];
    int z = pos[2];
    int curr_chunk[2] = { floorf(x / (float)CHUNK_WIDTH), floorf(y / (float)CHUNK_LENGTH) };
    struct chunk* c = {0};
    world_get_chunk_no_gen(world, curr_chunk, &c);
    if (c == NULL) return 1;
    // Set coords to chunk-local coords
    x = (abs(curr_chunk[0]) * CHUNK_WIDTH + x) % CHUNK_WIDTH;
    y = (abs(curr_chunk[1]) * CHUNK_LENGTH + y) % CHUNK_LENGTH;
    vec3 new_pos = { x, y, z };
    return chunk_block_place(c, new_pos, block_id);
}
