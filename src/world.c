#include "world.h"
#include "cglm/io.h"
#include "chunk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// LOAD_DISTANCE determines how many chunks are loaded on world creation
#define LOAD_DISTANCE 1

int32_t seed;
enum biome chunk_biomes[WORLD_WIDTH][WORLD_LENGTH];
vec3 chunk_pois[WORLD_WIDTH][WORLD_LENGTH];
void _chunk_poi_plains_raw(vec2 coord, vec3 poi);
void _chunk_poi_desert_raw(vec2 coord, vec3 poi);
void _chunk_poi_snow_raw(vec2 coord, vec3 poi);
void _chunk_poi_mountains_raw(vec2 coord, vec3 poi);

enum biome plains_map[2] = {
    JUNK_BIOME_SNOW,
    JUNK_BIOME_DESERT,
};
enum biome desert_map[2] = {
    JUNK_BIOME_PLAINS,
    JUNK_BIOME_PLAINS,
};
enum biome snow_map[2] = {
    JUNK_BIOME_MOUNTAINS,
    JUNK_BIOME_PLAINS,
};
enum biome mountain_map[2] = {
    JUNK_BIOME_SNOW,
    JUNK_BIOME_SNOW,
};
enum biome b_map[4][2];

#define BIOME_SWITCH_PROBABILITY 0.05
void world_chunk_set_neighbor_biome(int x, int y) {
    enum biome curr_biome = chunk_biomes[x][y];
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int nx = x + i;
            int ny = y + j;
            int w = ((abs(nx) / WORLD_WIDTH) + 1) * WORLD_WIDTH;
            int l = ((abs(nx) / WORLD_LENGTH) + 1) * WORLD_LENGTH;
            int rx = (nx + w) % WORLD_WIDTH;
            int ry = (ny + l) % WORLD_LENGTH;
            if (chunk_biomes[rx][ry] == JUNK_BIOME_COUNT) {
                float p = (float)rand() / RAND_MAX;
                if (p < BIOME_SWITCH_PROBABILITY) {
                   int idx = rand() % 2;
                   chunk_biomes[rx][ry] = b_map[curr_biome][idx];
                    world_chunk_set_neighbor_biome(x + j, y + j);
                } else {
                    chunk_biomes[rx][ry] = curr_biome;
                }
            }
        }
    }
}
/**
 * 
 *
 */
void _chunk_get_poi_raw(vec2 coord, vec3 poi) {
    enum biome b = chunk_get_biome(coord);
    srand(seed + coord[0] * coord[1]);
    switch (b) {
        case JUNK_BIOME_PLAINS:
            _chunk_poi_plains_raw(coord, poi);
            break;
        case JUNK_BIOME_DESERT:
            _chunk_poi_desert_raw(coord, poi);
            break;
        case JUNK_BIOME_SNOW:
            _chunk_poi_plains_raw(coord, poi);
            break;
        case JUNK_BIOME_MOUNTAINS:
            _chunk_poi_mountains_raw(coord, poi);
            break;
        default:
            _chunk_poi_plains_raw(coord, poi);
            break;
    }
}

void _chunk_poi_plains_raw(vec2 coord, vec3 poi) {
    // Min POI block height
    int poi_min = 70;
    // Max POI block height
    int poi_max = 75;
    vec3 poi1 = {  (float)CHUNK_WIDTH / 2,  (float)CHUNK_LENGTH / 2, poi_min + (rand() % (poi_max - poi_min))};
    memcpy(poi, poi1, sizeof(vec3));
}
void _chunk_poi_desert_raw(vec2 coord, vec3 poi) {
    // Min POI block height
    int poi_min = 70;
    // Max POI block height
    int poi_max = 72;
    vec3 poi1 = { (float) CHUNK_WIDTH / 2, (float) CHUNK_LENGTH / 2, poi_min + (rand() % (poi_max - poi_min))};
    memcpy(poi, poi1, sizeof(vec3));
}
void _chunk_poi_snow_raw(vec2 coord, vec3 poi) {
    // Min POI block height
    int poi_min = 72;
    // Max POI block height
    int poi_max = 75;
    vec3 poi1 = { (float) CHUNK_WIDTH / 2, (float) CHUNK_LENGTH / 2, poi_min + (rand() % (poi_max - poi_min))};
    memcpy(poi, poi1, sizeof(vec3));
}
void _chunk_poi_mountains_raw(vec2 coord, vec3 poi) {
    // Min POI block height
    int poi_min = 180;
    // Max POI block height
    int poi_max = 220;
    vec3 poi1 = { (float) CHUNK_WIDTH / 2, (float) CHUNK_LENGTH / 2, poi_min + (rand() % (poi_max - poi_min))};
    memcpy(poi, poi1, sizeof(vec3));
}

void world_all_chunk_gen_pois() {
    // Generate the RAW POIs for all chunks. These are solely based on the biome.
    // We cannot just use these because it leads to janky terrain. The next step 
    // is to average out the POIs based on a neighbor raidus
    for (int i = 0; i < WORLD_WIDTH; i++) {
        for (int j = 0; j < WORLD_LENGTH; j++) {
            vec2 coords = { i, j };
            _chunk_get_poi_raw(coords, chunk_pois[i][j]);

        }
    }
    int neighbor_radius = 3;
    vec3 chunk_pois_average[WORLD_WIDTH][WORLD_LENGTH];
    memcpy(chunk_pois_average, chunk_pois, sizeof(chunk_pois));
    for (int i = 0; i < WORLD_WIDTH; i++) {
        for (int j = 0; j < WORLD_LENGTH; j++) {
            float average_z = 0.0f;
            for (int k = -neighbor_radius; k <= neighbor_radius; k++) {
                int ox = i + k;
                int oy = j + k;
                int w = ((abs(ox) / WORLD_WIDTH) + 1) * WORLD_WIDTH;
                int l = ((abs(oy) / WORLD_LENGTH) + 1) * WORLD_LENGTH;
                int x = (ox + w) % WORLD_WIDTH;
                int y = (oy + l) % WORLD_LENGTH;
                average_z += chunk_pois[x][y][2];
            }
            chunk_pois_average[i][j][2] = (average_z / (neighbor_radius * 2 + 1));
        }
    }
    memcpy(chunk_pois, chunk_pois_average, sizeof(chunk_pois));

    fprintf(stderr, "=================== HEAT MAP ===================\n");
    char* heat_colors[5] = {
        "\033[48;5;17m  \033[0m",
        "\033[48;5;21m  \033[0m",
        "\033[48;5;226m  \033[0m",
        "\033[48;5;208m  \033[0m",
        "\033[48;5;196m  \033[0m",
    };
    for (int i = WORLD_LENGTH - 1; i >= 0; i--) {
        for (int j = 0; j < WORLD_WIDTH; j++) {
            int level = chunk_pois[j][i][2] / 50;
            fprintf(stderr, "%s", heat_colors[level]);
        }
            fprintf(stderr, "\n");
    }
    fprintf(stderr, "==============================+++==========\n");
}
void world_all_chunk_gen_biome() {
    srand(seed);
    for (int i = 0; i < WORLD_WIDTH; i++) {
        for (int j = 0; j < WORLD_LENGTH; j++) {
            chunk_biomes[i][j] = JUNK_BIOME_COUNT;
        }
    }
    for (int i = 0; i < 2; i++) {
        b_map[0][i] = plains_map[i];
        b_map[1][i] = desert_map[i];
        b_map[2][i] = snow_map[i];
        b_map[3][i] = mountain_map[i];
    }
    enum biome root_biome = JUNK_BIOME_SNOW;
    int x = WORLD_WIDTH/2;
    int y = WORLD_LENGTH/2;
    chunk_biomes[WORLD_WIDTH/2][WORLD_LENGTH/2] = root_biome;
    for (int i = 1; i <= WORLD_WIDTH/2;i++) {
        for (int j = -i; j <= i; j++) {
            for (int k = -i; k <= i; k++) {
                if (j == -i && k == -i 
                        || j == -i && k == i
                        || j == i && k == -i
                        || j == i && k == i) {
                    continue;
                }
                // fprintf(stderr, "%d, %d\n", x+j, y+k);
                world_chunk_set_neighbor_biome(x + j, y + k);
            }
        }
    }
    fprintf(stderr, "=================== MAP ===================\n");
    char* text[5] = {  "\033[42m PL \033[0m", "\033[43m DS \033[0m", "\033[47m SN \033[0m", "\033[100m MN \033[0m", "\033[41m UD \033[0m"  };
    char* text2[5] = {  "PL", "DS", "SN", "MN", "UD" };
    for (int i = WORLD_LENGTH - 1; i >= 0; i--) {
        for (int j = 0; j < WORLD_WIDTH; j++) {
            if (i == WORLD_WIDTH/2 && j == WORLD_LENGTH/2) {
                    fprintf(stderr, "\033[41m %s \033[0m", text2[chunk_biomes[j][i]]);
            } else {
            fprintf(stderr, "%s", text[chunk_biomes[j][i]]);
            }
        }
            fprintf(stderr, "\n");
    }
    fprintf(stderr, "==============================+++==========\n");
}

// void world_gen_chunk_pois(struct world* world, int32_t seed) {
//     srand(seed);
//     for (int i = 0; i < WORLD_WIDTH; i++) {
//         for (int j = 0; j < WORLD_LENGTH; j++) {
//             world->chunk_pois[i][j] = rand();
//         }
//     }
// }
// void world_gen_chunk_biome(struct world* world, int32_t seed) {
//     srand(seed);
//     for (int i = 0; i < WORLD_WIDTH; i++) {
//         for (int j = 0; j < WORLD_LENGTH; j++) {
//             world->chunk_biomes[i][j] = rand();
//         }
//     }
// }
// int32_t world_get_chunk_poi(struct world* world, int coord[2]) {
//     int w = ((abs(coord[0]) / WORLD_WIDTH) + 1) * WORLD_WIDTH;
//     int l = ((abs(coord[1]) / WORLD_LENGTH) + 1) * WORLD_LENGTH;
//     int x = (coord[0] + w) % WORLD_WIDTH;
//     int y = (coord[1] + l) % WORLD_LENGTH;
//     vec2 new_coord = { x, y };
//     return world->chunk_pois[x][y];
// }

int world_init(int32_t seed, struct world** world) {
    struct world* wld = malloc(sizeof(struct world));
    memset(wld, 0, sizeof(struct world));
    wld->seed = seed;
    seed = seed;
    world_all_chunk_gen_biome();
    world_all_chunk_gen_pois();
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
