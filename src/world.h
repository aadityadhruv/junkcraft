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
int world_get_chunk(struct world* world, vec2 coord, struct chunk** chunk);
