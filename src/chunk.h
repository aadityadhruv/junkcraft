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
};

int chunk_gen(struct world* wld, vec2 coord, struct chunk** chunk);
int chunk_gen_structures(void* neighbor_data, struct chunk* chunk);
int chunk_gen_terrain(void* neighbor_data, struct chunk* chunk);
