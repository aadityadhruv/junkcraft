#include "world.h"
#include <stdlib.h>
#include <string.h>

int world_init(int32_t seed, struct world** world) {
    srand(seed);
    struct world* wld = malloc(sizeof(struct world));
    memset(wld, 0, sizeof(struct world));
    wld->seed = seed;
    //TODO: Improve loading here
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            struct chunk* chunk;
            vec2 coords = { i, j };
            chunk_gen(wld, coords, &chunk);
            wld->chunks[i][j] = chunk;

        }
    }
    *world = wld;
    return 0;
}

int world_get_chunk(struct world* world, vec2 coord, struct chunk** chunk) {
    int x = (int)coord[0] % WORLD_WIDTH;
    int y = (int)coord[1] % WORLD_LENGTH;
    *chunk = world->chunks[x][y];
    return 0;
}
