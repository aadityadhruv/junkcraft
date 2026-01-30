#include "world.h"
#include "cglm/io.h"
#include "chunk.h"
#include <stdlib.h>
#include <string.h>

// LOAD_DISTANCE determines how many chunks are loaded on world creation
#define LOAD_DISTANCE 4

int world_init(int32_t seed, struct world** world) {
    srand(seed);
    struct world* wld = malloc(sizeof(struct world));
    memset(wld, 0, sizeof(struct world));
    wld->seed = seed;
    //TODO: Improve loading here
    for (int i = 0; i < LOAD_DISTANCE; i++) {
        for (int j = 0; j < LOAD_DISTANCE; j++) {
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
