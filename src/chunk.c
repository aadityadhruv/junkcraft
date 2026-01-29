#include "chunk.h"
#include "block.h"
#include "cglm/io.h"
#include "cglm/util.h"
#include "cglm/vec2.h"
#include "cglm/vec3.h"
#include "world.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define MIN(x, y) (x < y) ? x : y
#define MAX(x, y) (x > y) ? x : y

void _chunk_plains_gen(struct chunk* chunk);

int chunk_gen(struct world* world, vec2 coord, struct chunk **chunk) {
    *chunk = malloc(sizeof(struct chunk));
    memcpy((*chunk)->coord,coord, sizeof(vec2));
    // struct chunk* neighbor_top = { 0 };
    // struct chunk* neighbor_bottom = { 0 };
    // struct chunk* neighbor_left = { 0 };
    // struct chunk* neighbor_right = { 0 };
    // vec2 top = { 0, 1 };
    // vec2 bottom = { 0, -1 };
    // vec2 left = { -1, 0 };
    // vec2 right = { 1, 0 };
    // glm_vec2_add(top, coord, top);
    // glm_vec2_add(bottom, coord, bottom);
    // glm_vec2_add(left, coord, left);
    // glm_vec2_add(right, coord, right);
    // world_get_chunk(world, top, &neighbor_top);
    // world_get_chunk(world, bottom, &neighbor_bottom);
    // world_get_chunk(world, left, &neighbor_left);
    // world_get_chunk(world, right, &neighbor_right);

    // world_get_chunk(world, chunk->coord, &neighbor);
    _chunk_plains_gen(*chunk);
    // switch (chunk->biome) {
    //     case JUNK_BIOME_PLAINS:
    //         _
    //         break;
    //     default:
    //         break;
    // }
    return 0;
}

float _chunk_plains_get_z(vec2 target, vec3 poi, float m, int base_z) {
    vec2 unit = { (target[0] - poi[0]), (target[1] - poi[1]) };
    glm_vec2_normalize(unit);
    // Line direction vector
    vec3 r = { unit[0], unit[1], m };
    // r*t + poi = { x, y, z }, solve first for t, then get z
    float t = 0;
    // Parallel to X-axis
    if (r[0] == 0.0f && r[1] != 0.0f) {
        t = (target[1] - poi[1]) / r[1];
    }
    // Parallel to Y-axis
    else if (r[1] == 0.0f && r[0] != 0.0f) {
        t = (target[0] - poi[0]) / r[0];
    }
    else if (r[0] != 0.0f && r[1] != 0.0f) {
        // Non-parallel line, either X or y works
        t = (target[0] - poi[0]) / r[0];
    }
    else {
        //Else we are POI itself, no need to do anything. t will be zero and
        //the value of z == poi[2]. We subtract -1 from base_z because
        //otherwise the POI will be a single block at it's z, all others will
        //always be strictly less than it. This levels the plains
        base_z -= 1;
    }
    float z_off = poi[2] + t * r[2];

    return MAX(base_z, base_z + z_off);
}

/**
 * Basic Plains chunk generation
 * Algorithm: Pick 2 points of interest (POI). These points will either be elevations or depressions.
 * Each block will get a invisible "offset" value based on their distance from the chosen point.
 * Chosen point height itself will range from some non zero value to another, plus in negative.
 * The offset value determines how block heights are created
 *
 */
void _chunk_plains_gen(struct chunk* chunk) {
    // ============ KNOBS ============
    // Minimum ground
    int z = 2;
    // Min POI block height
    int poi_min = 3;
    // Max POI block height
    int poi_max = 5;
    // Descent/Ascent rate
    float m = -.5;
    memset(chunk->blocks, 0, CHUNK_HEIGHT * CHUNK_LENGTH * CHUNK_WIDTH * sizeof(struct block*));
    // X, Y, POI Height
    vec3 poi1 = { rand() % CHUNK_WIDTH, rand() % CHUNK_LENGTH, poi_min + (rand() % (poi_max - poi_min))};
    vec3 poi2 = { rand() % CHUNK_WIDTH, rand() % CHUNK_LENGTH, -poi_min + (rand() % (poi_max - poi_min))};
    
    for (int x = 0; x < CHUNK_LENGTH; x++) {
        for (int y = 0; y < CHUNK_WIDTH; y++) {
            // Minimum z height
            // Interpolation formula - simple linear
            vec2 target = { x, y };
            float z1 = _chunk_plains_get_z(target, poi1, m, z);
            float z2 = _chunk_plains_get_z(target, poi2, -m, z);
            int z_final = (z1 + z2) / 2;
            for (int h = 0; h < z_final; h++) {
                struct block* blk = malloc(sizeof(struct block));
                // Adjust block coordinates with global chunk coordinates
                vec3 pos = {x + (CHUNK_WIDTH * chunk->coord[0]), h, -y - (CHUNK_LENGTH * chunk->coord[1])};
                block_init(pos, blk);
                chunk->blocks[x][y][h] = blk;
            }
        }
    }
}
