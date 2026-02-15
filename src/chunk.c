#include "chunk.h"
#include "block.h"
#include "cglm/cglm.h"
#include "shader.h"
#include "util.h"
#include "world.h"
#include <junk/vector.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


#define MIN(x, y) (x < y) ? x : y
#define MAX(x, y) (x > y) ? x : y
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define CAVERN_LAYER 60
#define UNDERGROUND_LAYER 30

extern struct block_metadata block_metadata[BLOCK_ID_COUNT];
extern enum biome chunk_biomes[WORLD_WIDTH][WORLD_LENGTH];
extern int32_t seed;
void _chunk_plains_gen(struct chunk* chunk);
void _chunk_desert_gen(struct chunk* chunk);
void _chunk_snow_gen(struct chunk* chunk);
void _chunk_mountains_gen(struct chunk* chunk);
enum biome chunk_get_biome(vec2 coord);
void chunk_get_poi(vec2 coord, vec3 poi);
void _chunk_poi_plains(vec2 coord, vec3 poi);
void _chunk_poi_desert(vec2 coord, vec3 poi);
void _chunk_poi_snow(vec2 coord, vec3 poi);
void _chunk_poi_mountains(vec2 coord, vec3 poi);

enum biome chunk_get_biome(vec2 coord) {
    return chunk_biomes[(int)coord[0]][(int)coord[1]];
}
void chunk_get_poi(vec2 coord, vec3 poi) {
    enum biome b = chunk_get_biome(coord);
    srand(seed + coord[0] * coord[1]);
    switch (b) {
        case JUNK_BIOME_PLAINS:
            _chunk_poi_plains(coord, poi);
            break;
        case JUNK_BIOME_DESERT:
            _chunk_poi_desert(coord, poi);
            break;
        case JUNK_BIOME_SNOW:
            _chunk_poi_plains(coord, poi);
            break;
        case JUNK_BIOME_MOUNTAINS:
            _chunk_poi_mountains(coord, poi);
            break;
        default:
            _chunk_poi_plains(coord, poi);
            break;
    }
}

void _chunk_poi_plains(vec2 coord, vec3 poi) {
    // Min POI block height
    int poi_min = 70;
    // Max POI block height
    int poi_max = 75;
    vec3 poi1 = { rand() % CHUNK_WIDTH, rand() % CHUNK_LENGTH, poi_min + (rand() % (poi_max - poi_min))};
    memcpy(poi, poi1, sizeof(vec3));
}
void _chunk_poi_desert(vec2 coord, vec3 poi) {
    // Min POI block height
    int poi_min = 70;
    // Max POI block height
    int poi_max = 72;
    vec3 poi1 = { rand() % CHUNK_WIDTH, rand() % CHUNK_LENGTH, poi_min + (rand() % (poi_max - poi_min))};
    memcpy(poi, poi1, sizeof(vec3));
}
void _chunk_poi_snow(vec2 coord, vec3 poi) {
    // Min POI block height
    int poi_min = 72;
    // Max POI block height
    int poi_max = 75;
    vec3 poi1 = { rand() % CHUNK_WIDTH, rand() % CHUNK_LENGTH, poi_min + (rand() % (poi_max - poi_min))};
    memcpy(poi, poi1, sizeof(vec3));
}
void _chunk_poi_mountains(vec2 coord, vec3 poi) {
    // Min POI block height
    int poi_min = 120;
    // Max POI block height
    int poi_max = 140;
    vec3 poi1 = { rand() % CHUNK_WIDTH, rand() % CHUNK_LENGTH, poi_min + (rand() % (poi_max - poi_min))};
    memcpy(poi, poi1, sizeof(vec3));
}




int chunk_gen(struct world* world, vec2 coord, struct chunk **chunk) {
    *chunk = malloc(sizeof(struct chunk));
    memcpy((*chunk)->coord,coord, sizeof(vec2));
    (*chunk)->biome = chunk_get_biome(coord);
    switch ((*chunk)->biome) {
        case JUNK_BIOME_PLAINS:
            _chunk_plains_gen(*chunk);
            break;
        case JUNK_BIOME_DESERT:
            _chunk_desert_gen(*chunk);
            break;
        case JUNK_BIOME_SNOW:
            _chunk_snow_gen(*chunk);
            break;
        case JUNK_BIOME_MOUNTAINS:
            _chunk_mountains_gen(*chunk);
            break;
        default:
            fprintf(stderr, "%d\n", (*chunk)->biome);
            exit(1);
            break;
    }
    return 0;
}

/**
 * Helper function for _chunk_plains_gen. It calculates how much of a z-value
 * needs to be added for a target point so it aligns with the line from poi to a
 * neighboring POI We know the (x, y) (top down) of a point. We need to figure
 * out what the height should be. This is how we use POIs to figure out the
 * height: For every neighboring chunk's POI, we must get the local POI to
 * neighbor POI line. So there will be 8 lines going from our chunk's POI to the
 * neighbor's POI. Our target doesn't necessarily lie on this, so we need to
 * take some averages. To visualize this, our POI-Target vector will be between
 * two POI-POI lines. What we do is project this POI-Target vector onto the 2
 * POIs, calculate the z-value of that point, and average it. We can do this
 * because we know the line equation of the POI-POI lines. Note that the
 * projection happens for the x-y coords.
 * @param target Point for which we are trying to figure out the z
 * @param poi Starting point of the line, in local chunk coordinates
 * @param neighbor_poi 8 POIs from the neighboring chunk, in world coordinates
 * @param coord World coordinates of this chunk
 * caluclated z
 */
float _chunk_plains_get_z(vec2 target, vec3 poi, vec3 neighbor_poi[8], vec2 coord) {
    // Exit early if we are POI
    if (target[0] == poi[0] && target[1] == poi[1]) return poi[2];
    // This vector is in "CHUNK COORDINATES"
    vec2 diff = { (target[0] - poi[0]), (target[1] - poi[1]) };
    vec2 unit = {0};
    glm_vec2_normalize_to(diff, unit);
    // Indcies for neighbors that gave max dot products
    int max_index[2] = { 0, 0 };
    // Store max dot product values
    float maxes[2] = { -1.0f, -1.0f };
    // Find max dot product neighbors
    for (int i = 0; i < 8;i++) {
        // This is the POI-POI line vector
        vec3 snap_vec = { 0 };
        // Because neighbor POIs are in world coordinates,
        // we need to convert our local POI to world coordinates for this math
        int x_offset = coord[0] * CHUNK_WIDTH;
        int y_offset = coord[1] * CHUNK_LENGTH;
        vec3 world_poi = { poi[0] + x_offset, poi[1] + y_offset, poi[2] };
        glm_vec3_sub(neighbor_poi[i], world_poi, snap_vec);
        glm_vec3_normalize(snap_vec);
        // X-Y version of snap_vec
        vec2 snap_vec_2d = { snap_vec[0], snap_vec[1] };
        glm_vec2_normalize(snap_vec_2d);
        // Dot will be positive for -90 to 90
        // Also since it is normalized, the value will be between -1 and 1
        // Can be used to control "snappiness"
        float dot = glm_vec2_dot(unit, snap_vec_2d);
        if (dot > maxes[0]) {
            maxes[1] = maxes[0];
            max_index[1] = max_index[0];
            maxes[0] = dot;
            max_index[0] = i;
        }
        else if (dot > maxes[1]) {
            maxes[1] = dot;
            max_index[1] = i;
        }
    }
    float average_z = 0;
    // For every neighbor POI-POI line, find the one that is within -90 to 90 of unit
    for (int i = 0; i < 2; i++) {
        // This is the POI-POI line vector
        vec3 snap_vec = { 0 };
        // Because neighbor POIs are in world coordinates,
        // we need to convert our local POI to world coordinates for this math
        int x_offset = coord[0] * CHUNK_WIDTH;
        int y_offset = coord[1] * CHUNK_LENGTH;
        vec3 world_poi = { poi[0] + x_offset, poi[1] + y_offset, poi[2] };
        glm_vec3_sub(neighbor_poi[max_index[i]], world_poi, snap_vec);
        glm_vec3_normalize(snap_vec);
        // X-Y version of snap_vec
        vec2 snap_vec_2d = { snap_vec[0], snap_vec[1] };
        glm_vec2_normalize(snap_vec_2d);
        // Line direction vector
        // Literally just snap_vec, but renamed 
        // to match eq of line formula naming
        vec3 r = { snap_vec[0], snap_vec[1], snap_vec[2] * 1.25f };
        // The point on the POI-POI line who's z-value will be used
        // to calculate our target's z-value
        vec2 proj_target = { 0 };
        vec2 proj_start = { poi[0], poi[1]};
        // Project the target-poi line onto the snap_vec 2d line
        float scale = glm_vec2_dot(diff,snap_vec_2d);
        glm_vec2_scale(snap_vec_2d, scale, proj_target);
        // The vector needs to start at POI to get the actual target 
        // point
        glm_vec2_add(proj_start, proj_target, proj_target);
        // r*t + poi = { x, y, z }, solve first for t, then get z
        float t = 0;
        // Parallel to X-axis
        if (r[0] == 0.0f && r[1] != 0.0f) {
            t = (proj_target[1] - poi[1]) / r[1];
        }
        // Parallel to Y-axis
        else if (r[1] == 0.0f && r[0] != 0.0f) {
            t = (proj_target[0] - poi[0]) / r[0];
        }
        else if (r[0] != 0.0f && r[1] != 0.0f) {
            // Non-parallel line, either X or y works
            t = (proj_target[0] - poi[0]) / r[0];
        }
        else {
          // Else we are POI itself, no need to do anything. t will be zero and
          // the value of z == poi[2]. We subtract -1 from base_z because
          // otherwise the POI will be a single block at it's z, all others will
          // always be strictly less than it. This levels the plains
          // NOTE: We never really reach this condition due to early exit at
          // top, but I've just kept it for historic reasons
        }
        float z_off = poi[2] + t * r[2];
        average_z += z_off;
    }
    average_z /= 2;
    return MAX(1, average_z);
}

/**
 * Check if a given block at coord in chunk is a block or not. It is useful to calculate neighbours
 * of a block
 *
 * @param chunk Target chunk
 * @param coord block to test in the target chunk
 * @return 1 if there is a block at coordinates coord, 0 otherwise
 */
int _chunk_check_neighbor_block(struct world* world, struct chunk* chunk, vec3 coord) {
    int x = coord[0];
    int y = coord[1];
    int z = coord[2];
    // ==== Pre-checks for neighbor chunks =====
    //
    // If we are a boundary block (x,y only, don't care for z), check if there is a neighboring
    // block in the neighboring chunk
    if (x == -1.0) {
        vec2 c = { 0 };
        vec2 left = { -1.0f, 0.0f };
        glm_vec2_add(left, chunk->coord, c);
        int neighbor[] = { c[0], c[1] };
        struct chunk* left_chunk = { 0 };
        world_get_chunk(world, neighbor, &left_chunk);
        // If not created, we don't care, it's not being rendered, so mark as no neighbor
        // TODO: Previously had chunk->loaded == 0, but this causes a problem. When we move
        // from one chunk to another, everything gets unloaded, and then we start loading everything. 
        // This means that sometimes because of order of evaluation a chunk might think it's neighbor
        // isn't loaded even though it will be
        if (left_chunk == NULL) {
            return 0;
        }
        // Otherwise we check if the neighbor block exists
        vec3 left_neighbor_block = { CHUNK_WIDTH - 1, y, z };
        return _chunk_check_neighbor_block(world, left_chunk, left_neighbor_block);
    }
    if (x == CHUNK_WIDTH) {
        vec2 c = { 0 };
        vec2 right = { 1.0f,  0.0f };
        glm_vec2_add(right, chunk->coord, c);
        int neighbor[] = { c[0], c[1] };
        struct chunk* right_chunk = { 0 };
        world_get_chunk(world, neighbor, &right_chunk);
        // If unloaded, we don't care, it's not being rendered, so mark as no neighbor
        if (right_chunk == NULL) {
            return 0;
        }
        // Otherwise we check if the neighbor block exists
        vec3 left_neighbor_block = { 0, y, z };
        return _chunk_check_neighbor_block(world, right_chunk, left_neighbor_block);
    }
    if (y == -1.0) {
        vec2 c = { 0 };
        vec2 bottom = { 0.0f, -1.0f };
        glm_vec2_add(bottom, chunk->coord, c);
        int neighbor[] = { c[0], c[1] };
        struct chunk* bottom_chunk = { 0 };
        world_get_chunk(world, neighbor, &bottom_chunk);
        // If unloaded, we don't care, it's not being rendered, so mark as no neighbor
        if (bottom_chunk == NULL) {
            return 0;
        }
        // Otherwise we check if the neighbor block exists
        vec3 left_neighbor_block = { x, CHUNK_LENGTH - 1, z };
        return _chunk_check_neighbor_block(world, bottom_chunk, left_neighbor_block);
    }
    if (y == CHUNK_LENGTH) {
        vec2 c = { 0 };
        vec2 top = { 0.0f,  1.0f };
        glm_vec2_add(top, chunk->coord, c);
        int neighbor[] = { c[0], c[1] };
        struct chunk* top_chunk = { 0 };
        world_get_chunk(world, neighbor, &top_chunk);
        // If unloaded, we don't care, it's not being rendered, so mark as no neighbor
        if (top_chunk == NULL) {
            return 0;
        }
        // Otherwise we check if the neighbor block exists
        vec3 left_neighbor_block = { x, 0, z };
        return _chunk_check_neighbor_block(world, top_chunk, left_neighbor_block);
    }
    if (x < 0 || y < 0 || z < 0) {
        return 0;
    }
    if (x >= CHUNK_WIDTH  || y >= CHUNK_LENGTH || z >= CHUNK_HEIGHT) { 
        return 0;
    }
    // Air block
    if (chunk->blocks[x][y][z] == NULL) {
        return 0;
    }
    return 1;
}
/**
 * Helper function for _chunk_mountains_gen. 
 * TODO: doc add
 * similar to plains gen 
 * @param target Point for which we are trying to figure out the z
 * @param poi Starting point of the line, in local chunk coordinates
 * @param neighbor_poi 8 POIs from the neighboring chunk, in world coordinates
 * @param coord World coordinates of this chunk
 * caluclated z
 */
float _chunk_mountains_get_z(vec2 target, vec3 poi, vec3 neighbor_poi[8], vec2 coord) {
    // Exit early if we are POI
    if (target[0] == poi[0] && target[1] == poi[1]) return poi[2];
    // This vector is in "CHUNK COORDINATES"
    vec2 diff = { (target[0] - poi[0]), (target[1] - poi[1]) };
    vec2 unit = {0};
    glm_vec2_normalize_to(diff, unit);
    // Indcies for neighbors that gave max dot products
    int max_index[2] = { 0, 0 };
    // Store max dot product values
    float maxes[2] = { -1.0f, -1.0f };
    // Find max dot product neighbors
    for (int i = 0; i < 8;i++) {
        // This is the POI-POI line vector
        vec3 snap_vec = { 0 };
        // Because neighbor POIs are in world coordinates,
        // we need to convert our local POI to world coordinates for this math
        int x_offset = coord[0] * CHUNK_WIDTH;
        int y_offset = coord[1] * CHUNK_LENGTH;
        vec3 world_poi = { poi[0] + x_offset, poi[1] + y_offset, poi[2] };
        glm_vec3_sub(neighbor_poi[i], world_poi, snap_vec);
        glm_vec3_normalize(snap_vec);
        // X-Y version of snap_vec
        vec2 snap_vec_2d = { snap_vec[0], snap_vec[1] };
        glm_vec2_normalize(snap_vec_2d);
        // Dot will be positive for -90 to 90
        // Also since it is normalized, the value will be between -1 and 1
        // Can be used to control "snappiness"
        float dot = glm_vec2_dot(unit, snap_vec_2d);
        if (dot > maxes[0]) {
            maxes[1] = maxes[0];
            max_index[1] = max_index[0];
            maxes[0] = dot;
            max_index[0] = i;
        }
        else if (dot > maxes[1]) {
            maxes[1] = dot;
            max_index[1] = i;
        }
    }
    float average_z = 0;
    // For every neighbor POI-POI line, find the one that is within -90 to 90 of unit
    for (int i = 0; i < 2; i++) {
        // This is the POI-POI line vector
        vec3 snap_vec = { 0 };
        // Because neighbor POIs are in world coordinates,
        // we need to convert our local POI to world coordinates for this math
        int x_offset = coord[0] * CHUNK_WIDTH;
        int y_offset = coord[1] * CHUNK_LENGTH;
        vec3 world_poi = { poi[0] + x_offset, poi[1] + y_offset, poi[2] };
        glm_vec3_sub(neighbor_poi[max_index[i]], world_poi, snap_vec);
        glm_vec3_normalize(snap_vec);
        // X-Y version of snap_vec
        vec2 snap_vec_2d = { snap_vec[0], snap_vec[1] };
        glm_vec2_normalize(snap_vec_2d);
        // Line direction vector
        // Literally just snap_vec, but renamed 
        // to match eq of line formula naming
        vec3 r = { snap_vec[0], snap_vec[1], snap_vec[2]  * 1 };
        // The point on the POI-POI line who's z-value will be used
        // to calculate our target's z-value
        vec2 proj_target = { 0 };
        vec2 proj_start = { poi[0], poi[1]};
        // Project the target-poi line onto the snap_vec 2d line
        float scale = glm_vec2_dot(diff,snap_vec_2d);
        glm_vec2_scale(snap_vec_2d, scale, proj_target);
        // The vector needs to start at POI to get the actual target 
        // point
        glm_vec2_add(proj_start, proj_target, proj_target);
        // r*t + poi = { x, y, z }, solve first for t, then get z
        float t = 0;
        // Parallel to X-axis
        if (r[0] == 0.0f && r[1] != 0.0f) {
            t = (proj_target[1] - poi[1]) / r[1];
        }
        // Parallel to Y-axis
        else if (r[1] == 0.0f && r[0] != 0.0f) {
            t = (proj_target[0] - poi[0]) / r[0];
        }
        else if (r[0] != 0.0f && r[1] != 0.0f) {
            // Non-parallel line, either X or y works
            t = (proj_target[0] - poi[0]) / r[0];
        }
        else {
          // Else we are POI itself, no need to do anything. t will be zero and
          // the value of z == poi[2]. We subtract -1 from base_z because
          // otherwise the POI will be a single block at it's z, all others will
          // always be strictly less than it. This levels the plains
          // NOTE: We never really reach this condition due to early exit at
          // top, but I've just kept it for historic reasons
        }
        float z_off = poi[2] + t * r[2];
        average_z += z_off;
    }
    average_z /= 2;
    return MAX(1, average_z);
}

/**
 * Basic Mountain chunk generation
 * Algorithm: Pick 1 point of interest (POI). Interpolate block heights based on z-values returned
 * from the helper _chunk_mountains_get_z()
 *
 */
void _chunk_mountains_gen(struct chunk* chunk) {
    srand(seed);
    memset(chunk->blocks, 0, CHUNK_HEIGHT * CHUNK_LENGTH * CHUNK_WIDTH * sizeof(struct block*));
    vec3 poi = { 0 };
    chunk_get_poi(chunk->coord, poi);

    vec3 neighbor_pois[8];
    int neighbor_counter = 0;
    // Get neighboring POI values
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            // This basically just converts the world coordinates to array mappable coordinates
            // Same code used in world_get_chunk to wrap around. TODO: move to func? 
            int old_x = chunk->coord[0] + i;
            int old_y = chunk->coord[1] + j;
            int w = ((abs(old_x) / WORLD_WIDTH) + 1) * WORLD_WIDTH;
            int l = ((abs(old_y) / WORLD_LENGTH) + 1) * WORLD_LENGTH;
            int x = (old_x + w) % WORLD_WIDTH;
            int y = (old_y + l) % WORLD_LENGTH;
            vec2 new_coord = { x , y };
            chunk_get_poi(new_coord, neighbor_pois[neighbor_counter]);
            // Translate neighbor chunk POIs to world coordinates
            int x_offset = old_x * CHUNK_WIDTH;
            int y_offset = old_y * CHUNK_LENGTH;
            neighbor_pois[neighbor_counter][0] += x_offset;
            neighbor_pois[neighbor_counter][1] += y_offset;
            neighbor_counter+=1;
        }
    }
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_LENGTH; y++) {
            // Minimum z height
            // Interpolation formula - simple linear
            vec2 target = { x, y };
            float z = _chunk_mountains_get_z(target, poi, neighbor_pois, chunk->coord);
            for (int h = 0; h < z; h++) {
                struct block* blk = malloc(sizeof(struct block));
                // Adjust block coordinates with global chunk coordinates
                if (h <= UNDERGROUND_LAYER) {
                    block_init(blk, BLOCK_ROCK);
                }
                else if (h <= CAVERN_LAYER) {
                    block_init(blk, BLOCK_STONE);
                } 
                else if (h <= 130) {
                    float p = (float)rand() / RAND_MAX;
                    if (p < 0.3) {
                        block_init(blk, BLOCK_STONE);
                    } else {
                        block_init(blk, BLOCK_SNOW);
                    }
                }
                else {
                    block_init(blk, BLOCK_SNOW);
                }
                chunk->blocks[x][y][h] = blk;
            }
        }
    }
}

/**
 * Basic Snow chunk generation
 * Algorithm: Pick 1 point of interest (POI). Interpolate block heights based on z-values returned
 * from the helper _chunk_plains_get_z()
 *
 */
void _chunk_snow_gen(struct chunk* chunk) {
    memset(chunk->blocks, 0, CHUNK_HEIGHT * CHUNK_LENGTH * CHUNK_WIDTH * sizeof(struct block*));
    vec3 poi = { 0 };
    chunk_get_poi(chunk->coord, poi);

    vec3 neighbor_pois[8];
    int neighbor_counter = 0;
    // Get neighboring POI values
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            // This basically just converts the world coordinates to array mappable coordinates
            // Same code used in world_get_chunk to wrap around. TODO: move to func? 
            int old_x = chunk->coord[0] + i;
            int old_y = chunk->coord[1] + j;
            int w = ((abs(old_x) / WORLD_WIDTH) + 1) * WORLD_WIDTH;
            int l = ((abs(old_y) / WORLD_LENGTH) + 1) * WORLD_LENGTH;
            int x = (old_x + w) % WORLD_WIDTH;
            int y = (old_y + l) % WORLD_LENGTH;
            vec2 new_coord = { x , y };
            chunk_get_poi(new_coord, neighbor_pois[neighbor_counter]);
            // Translate neighbor chunk POIs to world coordinates
            int x_offset = old_x * CHUNK_WIDTH;
            int y_offset = old_y * CHUNK_LENGTH;
            neighbor_pois[neighbor_counter][0] += x_offset;
            neighbor_pois[neighbor_counter][1] += y_offset;
            neighbor_counter+=1;
        }
    }
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_LENGTH; y++) {
            // Minimum z height
            // Interpolation formula - simple linear
            vec2 target = { x, y };
            float z = _chunk_plains_get_z(target, poi, neighbor_pois, chunk->coord);
            for (int h = 0; h < z; h++) {
                struct block* blk = malloc(sizeof(struct block));
                // Adjust block coordinates with global chunk coordinates
                if (h <= UNDERGROUND_LAYER) {
                    block_init(blk, BLOCK_ROCK);
                }
                else if (h <= CAVERN_LAYER) {
                    block_init(blk, BLOCK_STONE);
                } else {
                    block_init(blk, BLOCK_SNOW);
                }
                chunk->blocks[x][y][h] = blk;
            }
        }
    }
}

/**
 * Basic Desert chunk generation
 * Algorithm: Pick 1 point of interest (POI). Interpolate block heights based on z-values returned
 * from the helper _chunk_plains_get_z()
 *
 */
void _chunk_desert_gen(struct chunk* chunk) {
    memset(chunk->blocks, 0, CHUNK_HEIGHT * CHUNK_LENGTH * CHUNK_WIDTH * sizeof(struct block*));
    vec3 poi = { 0 };
    chunk_get_poi(chunk->coord, poi);

    vec3 neighbor_pois[8];
    int neighbor_counter = 0;
    // Get neighboring POI values
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            // This basically just converts the world coordinates to array mappable coordinates
            // Same code used in world_get_chunk to wrap around. TODO: move to func? 
            int old_x = chunk->coord[0] + i;
            int old_y = chunk->coord[1] + j;
            int w = ((abs(old_x) / WORLD_WIDTH) + 1) * WORLD_WIDTH;
            int l = ((abs(old_y) / WORLD_LENGTH) + 1) * WORLD_LENGTH;
            int x = (old_x + w) % WORLD_WIDTH;
            int y = (old_y + l) % WORLD_LENGTH;
            vec2 new_coord = { x , y };
            chunk_get_poi(new_coord, neighbor_pois[neighbor_counter]);
            // Translate neighbor chunk POIs to world coordinates
            int x_offset = old_x * CHUNK_WIDTH;
            int y_offset = old_y * CHUNK_LENGTH;
            neighbor_pois[neighbor_counter][0] += x_offset;
            neighbor_pois[neighbor_counter][1] += y_offset;
            neighbor_counter+=1;
        }
    }
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_LENGTH; y++) {
            // Minimum z height
            // Interpolation formula - simple linear
            vec2 target = { x, y };
            float z = _chunk_plains_get_z(target, poi, neighbor_pois, chunk->coord);
            for (int h = 0; h < z; h++) {
                struct block* blk = malloc(sizeof(struct block));
                // Adjust block coordinates with global chunk coordinates
                if (h <= UNDERGROUND_LAYER) {
                    block_init(blk, BLOCK_ROCK);
                }
                else if (h <= CAVERN_LAYER) {
                    block_init(blk, BLOCK_STONE);
                } else {
                    block_init(blk, BLOCK_SAND);
                }
                chunk->blocks[x][y][h] = blk;
            }
        }
    }
}
/**
 * Basic Plains chunk generation
 * Algorithm: Pick 1 point of interest (POI). Interpolate block heights based on z-values returned
 * from the helper _chunk_plains_get_z()
 *
 */
void _chunk_plains_gen(struct chunk* chunk) {
    memset(chunk->blocks, 0, CHUNK_HEIGHT * CHUNK_LENGTH * CHUNK_WIDTH * sizeof(struct block*));
    vec3 poi = { 0 };
    chunk_get_poi(chunk->coord, poi);

    vec3 neighbor_pois[8];
    int neighbor_counter = 0;
    // Get neighboring POI values
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i == 0 && j == 0) continue;
            // This basically just converts the world coordinates to array mappable coordinates
            // Same code used in world_get_chunk to wrap around. TODO: move to func? 
            int old_x = chunk->coord[0] + i;
            int old_y = chunk->coord[1] + j;
            int w = ((abs(old_x) / WORLD_WIDTH) + 1) * WORLD_WIDTH;
            int l = ((abs(old_y) / WORLD_LENGTH) + 1) * WORLD_LENGTH;
            int x = (old_x + w) % WORLD_WIDTH;
            int y = (old_y + l) % WORLD_LENGTH;
            vec2 new_coord = { x , y };
            chunk_get_poi(new_coord, neighbor_pois[neighbor_counter]);
            // Translate neighbor chunk POIs to world coordinates
            int x_offset = old_x * CHUNK_WIDTH;
            int y_offset = old_y * CHUNK_LENGTH;
            neighbor_pois[neighbor_counter][0] += x_offset;
            neighbor_pois[neighbor_counter][1] += y_offset;
            neighbor_counter+=1;
        }
    }
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_LENGTH; y++) {
            // Minimum z height
            // Interpolation formula - simple linear
            vec2 target = { x, y };
            float z = _chunk_plains_get_z(target, poi, neighbor_pois, chunk->coord);
            for (int h = 0; h < z; h++) {
                struct block* blk = malloc(sizeof(struct block));
                // Adjust block coordinates with global chunk coordinates
                if (h <= UNDERGROUND_LAYER) {
                    block_init(blk, BLOCK_ROCK);
                }
                else if (h <= CAVERN_LAYER) {
                    block_init(blk, BLOCK_STONE);
                } else {
                    block_init(blk, BLOCK_GRASS);
                }
                chunk->blocks[x][y][h] = blk;
            }
        }
    }
}

void chunk_block_face_vertex_set(float* face, enum block_face face_side, struct block *block) {
    int x_start = 6;
    int y_start = 7;
    // To next vertex in face (face has 4 vertex and each is 8 floats)
    int step = 8;
    face[x_start] = block_metadata[block->block_id].texture_data[face_side].top_right[0];
    face[y_start] = block_metadata[block->block_id].texture_data[face_side].top_right[1];

    x_start += step;
    y_start += step;

    face[x_start] = block_metadata[block->block_id].texture_data[face_side].top_left[0];
    face[y_start] = block_metadata[block->block_id].texture_data[face_side].top_left[1];

    x_start += step;
    y_start += step;

    face[x_start] = block_metadata[block->block_id].texture_data[face_side].bottom_left[0];
    face[y_start] = block_metadata[block->block_id].texture_data[face_side].bottom_left[1];

    x_start += step;
    y_start += step;

    face[x_start] = block_metadata[block->block_id].texture_data[face_side].bottom_right[0];
    face[y_start] = block_metadata[block->block_id].texture_data[face_side].bottom_right[1];
}


int* chunk_face_order_add(int* face_order, int size, int idx) {
    int* buf = malloc(size);
    memcpy(buf, face_order, size);
    for (int i = 0; i < size / sizeof(int); i++) {
        buf[i] += idx;
    }
    return buf;
}
/**
 * Generates a face in chunk local coordinates
 */
float* _chunk_face_add(float* face, int size, vec3 pos) {
    // "Hack" to update the face coords,
    // glm_vec3_add just does a[0] = a[0] + b[0], so using
    // offsets will work
    int unit = 8;
    float* buf = malloc(size);
    memcpy(buf, face, size);
    glm_vec3_add(buf, pos, buf);
    glm_vec3_add(buf + unit, pos, buf + unit);
    glm_vec3_add(buf + 2 * unit, pos, buf + 2 * unit);
    glm_vec3_add(buf + 3 * unit, pos, buf + 3 * unit);
    return buf;
}

/**
 * Two step function:
 * 1. Generate mesh based on neighboring block data
 * 2. Send data to GPU 
 *
 * NOTE: GPU
 */
void chunk_load(struct world* world, struct chunk *chunk, int coord[2]) {
    // If we are already loaded, no need to do any GPU work at all. Just update the coordinates
    if (chunk->loaded == 1) {
    vec3 translation = {CHUNK_WIDTH * coord[0], 0, - (CHUNK_LENGTH * coord[1])};
    // Set the matrix for world coordinate translation
    glm_mat4_identity(chunk->model);
    glm_translate(chunk->model, translation);
    chunk->loaded = 1;
    chunk->staged_for_load = 0;
    return;
        
    }
    // fprintf(stderr, "Loaded chunk (%d, %d)\n", coord[0], coord[1]);
    // ================ OpenGL work ================
    // Initalize vertices and vertex order vectors. These will be dynamically
    // sized buffer data we send to the GPU
    struct vector* vertices;
    struct vector* vertex_order;
    vector_init(&vertices);
    vector_init(&vertex_order);

    // =============== Face Data ===================
    float front_face[] = {
        1.0f, 1.0f, 0.0f, // top-right
        0.0f, 0.0f, 1.0f, // Front normal
        1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, // top-left
        0.0f, 0.0f, 1.0f, // Front normal
        0.5f, 0.0f,
        0.0f, 0.0f, 0.0f, // bottom-left
        0.0f, 0.0f, 1.0f, // Front normal
        0.5f, 0.5f,
        1.0f, 0.0f, 0.0f, // bottom-right
        0.0f, 0.0f, 1.0f, // Front normal
        1.0f, 0.5f,
    };
    float back_face[] = {
        0.0f, 1.0f, -1.0f, // top-left (back plane)
        0.0f, 0.0f, -1.0f, // Back normal
        1.0f, 0.0f,
        1.0f, 1.0f, -1.0f, // top-right (back plane)
        0.0f, 0.0f, -1.0f, // Back normal
        0.5f, 0.0f,
        1.0f, 0.0f, -1.0f, // bottom-right (back plane)
        0.0f, 0.0f, -1.0f, // Back normal
        0.5f, 0.5f,
        0.0f, 0.0f, -1.0f, // bottom-left (back plane)
        0.0f, 0.0f, -1.0f, // Back normal
        1.0f, 0.5f,
    };
    float right_face[] = {
        1.0f, 1.0f, -1.0f, // top-right (back plane)
        1.0f, 0.0f, 0.0f, // Right normal
        1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, // top-right
        1.0f, 0.0f, 0.0f, // Right normal
        0.5f, 0.0f,
        1.0f, 0.0f, 0.0f, // bottom-right
        1.0f, 0.0f, 0.0f, // Right normal
        0.5f, 0.5f,
        1.0f, 0.0f, -1.0f, // bottom-right (back plane)
        1.0f, 0.0f, 0.0f, // Right normal
        1.0f, 0.5f,
    };
    float left_face[] = {
        0.0f, 1.0f, 0.0f, // top-left
        -1.0f, 0.0f, 0.0f, // Left normal
        1.0f, 0.0f,
        0.0f, 1.0f, -1.0f, // top-left (back plane)
        -1.0f, 0.0f, 0.0f, // Left normal
        0.5f, 0.0f,
        0.0f, 0.0f, -1.0f, // bottom-left (back plane)
        -1.0f, 0.0f, 0.0f, // Left normal
        0.5f, 0.5f,
        0.0f, 0.0f, 0.0f, // bottom-left
        -1.0f, 0.0f, 0.0f, // Left normal
        1.0f, 0.5f,
    };
    float top_face[] = {
        1.0f, 1.0f, -1.0f, // top-right (back plane)
        0.0f, 1.0f, 0.0f, // Top normal
        0.5f, 0.5f,
        0.0f, 1.0f, -1.0f, // top-left (back plane)
        0.0f, 1.0f, 0.0f, // Top normal
        0.0f, 0.5f,
        0.0f, 1.0f, 0.0f, // top-left
        0.0f, 1.0f, 0.0f, // Top normal
        0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, // top-right
        0.0f, 1.0f, 0.0f, // Top normal
        0.5f, 1.0f,
    };
    float bottom_face[] = {
        0.0f, 0.0f, -1.0f, // bottom-left (back plane)
        0.0f, -1.0f, 0.0f, // Bottom normal
        0.5f, 0.0f,
        1.0f, 0.0f, -1.0f, // bottom-right (back plane)
        0.0f, -1.0f, 0.0f, // Bottom normal
        0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, // bottom-right
        0.0f, -1.0f, 0.0f, // Bottom normal
        0.0f, 0.5f,
        0.0f, 0.0f, 0.0f, // bottom-left
        0.0f, -1.0f, 0.0f, // Bottom normal
        0.5f, 0.5f,
    };

    int vertex_draw_order[] = {
        1, 2, 3,   3, 0, 1, // CCW 2-triangles (quad)
    };
    // ============= Face detection algorithm =============
    int vertex_index = 0;
    int v_count[6] = { 0 };
    int blk_c = 0;
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_LENGTH; y++) {
            for (int z = 0; z < CHUNK_HEIGHT; z++) {
                struct block* blk = chunk->blocks[x][y][z];
                // If not air block
                if (blk != NULL) {
                    blk_c += 1;
                    // Position of block in OpenGL coords
                    // NOTE: OpenGL FLIP
                    vec3 pos = { x, z, -y };
                    vec3 front = { x, y - 1, z };
                    vec3 back = { x, y + 1, z };
                    vec3 right = { x + 1, y, z };
                    vec3 left = { x - 1, y, z };
                    vec3 top = { x, y, z + 1 };
                    vec3 bottom = { x, y, z - 1 };

                    if (_chunk_check_neighbor_block(world, chunk, front) == 0) {
                        chunk_block_face_vertex_set(front_face, BLOCK_FRONT, blk);
                        VECTOR_INSERT(vertices, _chunk_face_add(front_face,
                                    sizeof(front_face), pos));
                        VECTOR_INSERT(vertex_order,
                                chunk_face_order_add(vertex_draw_order,
                                    sizeof(vertex_draw_order), vertex_index));
                        vertex_index += 4;
                        v_count[0] += 1;
                    }

                    if (_chunk_check_neighbor_block(world, chunk, back) == 0) {
                        chunk_block_face_vertex_set(back_face, BLOCK_BACK, blk);
                        VECTOR_INSERT(vertices, _chunk_face_add(back_face,
                                    sizeof(back_face), pos));
                        VECTOR_INSERT(vertex_order,
                                chunk_face_order_add(vertex_draw_order,
                                    sizeof(vertex_draw_order), vertex_index));
                        vertex_index += 4;
                        v_count[1] += 1;
                    }

                    if (_chunk_check_neighbor_block(world, chunk, right) == 0) {
                        chunk_block_face_vertex_set(right_face, BLOCK_RIGHT, blk);
                        VECTOR_INSERT(vertices, _chunk_face_add(right_face,
                                    sizeof(right_face), pos));
                        VECTOR_INSERT(vertex_order,
                                chunk_face_order_add(vertex_draw_order,
                                    sizeof(vertex_draw_order), vertex_index));
                        vertex_index += 4;
                        v_count[2] += 1;
                    }

                    if (_chunk_check_neighbor_block(world, chunk, left) == 0) {
                        chunk_block_face_vertex_set(left_face, BLOCK_LEFT, blk);
                        VECTOR_INSERT(vertices, _chunk_face_add(left_face,
                                    sizeof(left_face), pos));
                        VECTOR_INSERT(vertex_order,
                                chunk_face_order_add(vertex_draw_order,
                                    sizeof(vertex_draw_order), vertex_index));
                        vertex_index += 4;
                        v_count[3] += 1;
                    }

                    if (_chunk_check_neighbor_block(world, chunk, top) == 0) {
                        chunk_block_face_vertex_set(top_face, BLOCK_TOP, blk);
                        VECTOR_INSERT(vertices, _chunk_face_add(top_face,
                                    sizeof(top_face), pos));
                        VECTOR_INSERT(vertex_order,
                                chunk_face_order_add(vertex_draw_order,
                                    sizeof(vertex_draw_order), vertex_index));
                        vertex_index += 4;
                        v_count[4] += 1;
                    }

                    if (_chunk_check_neighbor_block(world, chunk, bottom) == 0) {
                        chunk_block_face_vertex_set(bottom_face, BLOCK_BOTTOM, blk);
                        VECTOR_INSERT(vertices, _chunk_face_add(bottom_face,
                                    sizeof(bottom_face), pos));
                        VECTOR_INSERT(vertex_order,
                                chunk_face_order_add(vertex_draw_order,
                                    sizeof(vertex_draw_order), vertex_index));
                        vertex_index += 4;
                        v_count[5] += 1;
                    }
                }
            }
        }
    }
    float tmp_vertex[vector_length(vertices) * sizeof(front_face)];
    int tmp_order[vector_length(vertex_order) * sizeof(vertex_draw_order)];
    // fprintf(stderr, "Chunk blk_c: %d v_s: %d, v_o: %d\n", blk_c, vector_length(vertices) / 6, vector_length(vertex_order) / 6);
    // fprintf(stderr, "%d|%d|%d|%d|%d|%d|", v_count[0], v_count[1], v_count[2], v_count[3], v_count[4], v_count[5]);
    for (int i = 0; i < vector_length(vertices); i++) {
        float* face = vector_get(vertices, i);
        // Copy from heap mem to tmp buffer, and then free
        memcpy(tmp_vertex + (i*ARRAY_SIZE(front_face)), face, sizeof(front_face));
        free(face);
    }
    for (int i = 0; i < vector_length(vertex_order); i++) {
        int* order = vector_get(vertex_order, i);
        // Copy from heap mem to tmp buffer, and then free
        memcpy(tmp_order + (i*ARRAY_SIZE(vertex_draw_order)), order, sizeof(vertex_draw_order));
        free(order);
    }
    
    // Create VBO and EBO buffer data
    // VBO EBO size is sizeof() because we want TOTAL BYTES (float * count)
    create_vbo(&chunk->_vbo, (void*)tmp_vertex, sizeof(tmp_vertex));
    create_ebo(&chunk->_ebo, (void*)tmp_order, sizeof(tmp_order));
    // Here we only want ARRAY_SIZE, not float * count
    chunk->vertex_count = vector_length(vertex_order) * ARRAY_SIZE(vertex_draw_order);


    glGenVertexArrays(1, &chunk->_vao);
    glBindVertexArray(chunk->_vao);
    // Enable 3 attribs - position normals texture
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    // set vao_buffer to pos buffer obj
    glBindBuffer(GL_ARRAY_BUFFER, chunk->_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    // set vao_buffer to normals buffer obj
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(3*sizeof(float)));
    // set vao_buffer to texture buffer obj
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(6*sizeof(float)));
    // Set EBO to the vertex_order
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->_ebo);
    //NOTE: This is important, otherwise with multiple chunk_load calls, it
    //creates a segfault since the bindings get all messed up. Why it gets
    //messed up? Let's say we make 2 chunks. Chunk 1 creates VBOs, then VAO,
    //then binds everything. Now VAO is still bound. Chunk 2 init starts. First
    //call is create_vbo. Since VAO is already bound, it gets bound to the OLD
    //VAO!! Always clear before use. 
    glBindVertexArray(0);
    // Translation to WORLD units
    // NOTE: OpenGL FLIP
    vec3 translation = {CHUNK_WIDTH * coord[0], 0, - (CHUNK_LENGTH * coord[1])};
    // Set the matrix for world coordinate translation
    glm_mat4_identity(chunk->model);
    glm_translate(chunk->model, translation);
    chunk->loaded = 1;
    chunk->staged_for_load = 0;
}

void chunk_draw(struct chunk* chunk, struct shader* shader, struct texture* texture) {
    glBindVertexArray(chunk->_vao);
    set_uniform_mat4("model", shader, chunk->model);
    glDrawElements(GL_TRIANGLES, chunk->vertex_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void chunk_unload(struct chunk* chunk) {
    chunk->loaded = 0;
    // Clear VBO data
    glDeleteBuffers(1, &chunk->_vbo);
    // Clear EBO data
    glDeleteBuffers(1, &chunk->_ebo);
    // Clear VAO
    glDeleteVertexArrays(1, &chunk->_vao);
    chunk->loaded = 0;
    chunk->staged_for_load = 0;
}

// Regenerate chunk data
void chunk_update(struct chunk *chunk) {
}
int chunk_block_get(struct chunk* chunk, vec3 pos, struct block** block) {
    int x = pos[0];
    int y = pos[1];
    int z = pos[2];
    if (x < 0 || y < 0 || z < 0) {
        return 1;
    }
    if (x >= CHUNK_WIDTH || y >= CHUNK_LENGTH || z >= CHUNK_HEIGHT) {
        return 1;
    }
    if (chunk->blocks[x][y][z] != NULL) {
        if (block != NULL) {
            *block = chunk->blocks[x][y][z];
        }
        return 0;
    }
    return 1;
}

int chunk_block_place(struct chunk* chunk, vec3 pos) {
    int x = pos[0];
    int y = pos[1];
    int z = pos[2];
    if (x < 0 || y < 0 || z < 0) {
        return 1;
    }
    if (x >= CHUNK_WIDTH || y >= CHUNK_LENGTH || z >= CHUNK_HEIGHT) {
        return 1;
    }
    if (chunk->blocks[x][y][z] == NULL) {
        struct block* blk = malloc(sizeof(struct block));
        block_init(blk, BLOCK_STONE);
        chunk->blocks[x][y][z] = blk;
        chunk->loaded = 0;
        return 0;
    }
    return 1;
}
//TODO: Trigger a neighbor chunk load if on axis, otherwise 
//neighbor block face is not rendered
int chunk_block_delete(struct chunk* chunk, vec3 pos) {
    int x = pos[0];
    int y = pos[1];
    int z = pos[2];
    if (x < 0 || y < 0 || z < 0) {
        return 1;
    }
    if (x >= CHUNK_WIDTH || y >= CHUNK_LENGTH || z >= CHUNK_HEIGHT) {
        return 1;
    }
    if (chunk->blocks[x][y][z] != NULL) {
        free(chunk->blocks[x][y][z]);
        chunk->blocks[x][y][z] = NULL;
        chunk->loaded = 0;
        return 0;
    }
    return 1;
}
