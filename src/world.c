#include "world.h"
#include "cglm/io.h"
#include "chunk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// LOAD_DISTANCE determines how many chunks are loaded on world creation
#define LOAD_DISTANCE 1

int world_init(int32_t seed, struct world** world) {
    srand(seed);
    struct world* wld = malloc(sizeof(struct world));
    memset(wld, 0, sizeof(struct world));
    wld->seed = seed;
    //TODO: Improve loading here
    for (int i = -LOAD_DISTANCE; i <= LOAD_DISTANCE; i++) {
        for (int j = -LOAD_DISTANCE; j <= LOAD_DISTANCE; j++) {
            struct chunk* chunk;
            int coords[2] = { i, j };
            world_get_chunk(wld, coords, &chunk);
        }
    }
    *world = wld;
    return 0;
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
        chunk_gen(world, new_coord, chunk);
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
    world_get_chunk(world, curr_chunk, &c);
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
    world_get_chunk(world, curr_chunk, &c);
    // Set coords to chunk-local coords
    x = (abs(curr_chunk[0]) * CHUNK_WIDTH + x) % CHUNK_WIDTH;
    y = (abs(curr_chunk[1]) * CHUNK_LENGTH + y) % CHUNK_LENGTH;
    vec3 new_pos = { x, y, z };
    return chunk_block_delete(c, new_pos);
}
int world_chunk_block_place(struct world* world, vec3 pos) {
    int x = floorf(pos[0]);
    //Note: OpenGL FLIP
    int y = floorf(-pos[2]);
    int z = floorf(pos[1]);
    int curr_chunk[2] = { floorf(x / (float)CHUNK_WIDTH), floorf(y / (float)CHUNK_LENGTH) };
    struct chunk* c = {0};
    world_get_chunk(world, curr_chunk, &c);
    // Set coords to chunk-local coords
    x = (abs(curr_chunk[0]) * CHUNK_WIDTH + x) % CHUNK_WIDTH;
    y = (abs(curr_chunk[1]) * CHUNK_LENGTH + y) % CHUNK_LENGTH;
    vec3 new_pos = { x, y, z };
    return chunk_block_place(c, new_pos);
}
