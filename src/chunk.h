#pragma once
#include "block.h"
#include "world.h"
#include "junk/vector.h"
#include <stdint.h>
enum biome {
    JUNK_BIOME_PLAINS,
};
#define CHUNK_WIDTH 16
#define CHUNK_LENGTH 16
#define CHUNK_HEIGHT 128

struct chunk {
    struct block* blocks[CHUNK_WIDTH][CHUNK_LENGTH][CHUNK_HEIGHT];
    enum biome biome;
    vec2 coord;
};

int chunk_gen(struct world* world, struct chunk* chunk);
int chunk_gen_structures(void* neighbor_data, struct chunk* chunk);
int chunk_gen_terrain(void* neighbor_data, struct chunk* chunk);
