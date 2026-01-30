#pragma once
#include "chunk.h"
#include <stdint.h>
#define WORLD_LENGTH 6
#define WORLD_WIDTH 6

struct world {
    struct chunk* chunks[WORLD_WIDTH][WORLD_LENGTH];
    int32_t seed;
};

int world_init(int32_t seed, struct world** world);
int world_save(int32_t seed);
int world_get_chunk(struct world* world, int coord[2], struct chunk** chunk);
void world_get_chunk_real_coord(struct world* world, vec2 coord, int out[2]);
