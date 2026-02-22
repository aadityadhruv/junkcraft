#include "chunk.h"
#include "block.h"
#include "cglm/cglm.h"
#include "shader.h"
#include "util.h"
#include "world.h"
#include <junk/vector.h>
#include <stdbool.h>
#include <stdlib.h>
#include "random.h"
#include <string.h>


#define MIN(x, y) (x < y) ? x : y
#define MAX(x, y) (x > y) ? x : y
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define MOUNTAIN_HEIGHT 100
#define SNOW_HEIGHT 100
#define BIOME_BASE 70
#define PLAINS_HEIGHT 100
#define DESERT_HEIGHT 100
#define CAVERN_LAYER 60
#define UNDERGROUND_LAYER 30

extern struct block_metadata block_metadata[BLOCK_ID_COUNT];
extern enum biome chunk_biomes[WORLD_WIDTH][WORLD_LENGTH];
extern int32_t seed;
struct block* _chunk_plains_gen(float h);
struct block* _chunk_desert_gen(float h);
struct block* _chunk_snow_gen(float h);
struct block* _chunk_mountains_gen(float h);
void chunk_block_gen(int x, int y, float z_val, struct chunk* chunk) {
    enum biome b = JUNK_BIOME_PLAINS;
    if (z_val > 0.6f) {
        b = JUNK_BIOME_MOUNTAINS;
        float z = BIOME_BASE + z_val * MOUNTAIN_HEIGHT;
        for (int h = 0; h < z; h++) {
            struct block* blk = _chunk_mountains_gen(h);
            chunk->blocks[x][y][h] = blk;
        }
    }
    else if (z_val > 0.4f) {
        b = JUNK_BIOME_SNOW;
        float z = BIOME_BASE + z_val * SNOW_HEIGHT;
        for (int h = 0; h < z; h++) {
            struct block* blk = _chunk_snow_gen(h);
            chunk->blocks[x][y][h] = blk;
        }
    }
    else   {
        float heat = noise_heat(x + chunk->coord[0]*CHUNK_WIDTH, y + chunk->coord[1]*CHUNK_LENGTH);
        if (heat > 0.6) {
            b = JUNK_BIOME_DESERT;
            float z = BIOME_BASE + z_val * DESERT_HEIGHT;
            for (int h = 0; h < z; h++) {
                struct block* blk = _chunk_desert_gen(h);
                chunk->blocks[x][y][h] = blk;
            }
        }
        else {
            b = JUNK_BIOME_PLAINS;
            int z = BIOME_BASE + z_val * PLAINS_HEIGHT;
            for (int h = 0; h < z; h++) {
                struct block* blk = _chunk_plains_gen(h);
                chunk->blocks[x][y][h] = blk;
            }
            // Generate tree at the point
            if (rand() % 100 == 0.0f) {
                float max_height = MIN(CHUNK_HEIGHT, z + 4);
                for (int h = z; h < max_height; h++) {
                    struct block* blk = malloc(sizeof(struct block));
                    block_init(blk, BLOCK_WOOD);
                    chunk->blocks[x][y][h] = blk;
                }
                for (int i = -2; i <= 2; i++) {
                    for (int j = -2; j <= 2; j++) {
                        vec3 pos1 = { x + i, y + j, max_height - 2};
                        chunk_block_place(chunk, pos1, BLOCK_LEAF);
                    }
                }
                for (int i = -1; i <= 1; i++) {
                    for (int j = -1; j <= 1; j++) {
                        vec3 pos1 = { x + i, y + j, max_height - 1};
                        chunk_block_place(chunk, pos1, BLOCK_LEAF);
                    }
                }
                vec3 pos0 = { x, y, max_height };
                chunk_block_place(chunk, pos0, BLOCK_LEAF);
                vec3 pos1 = { x, y + 1, max_height };
                chunk_block_place(chunk, pos1, BLOCK_LEAF);
                vec3 pos2 = { x + 1, y, max_height };
                chunk_block_place(chunk, pos2, BLOCK_LEAF);
                vec3 pos3 = { x - 1, y, max_height };
                chunk_block_place(chunk, pos3, BLOCK_LEAF);
                vec3 pos4 = { x, y - 1, max_height };
                chunk_block_place(chunk, pos4, BLOCK_LEAF);
            }
        }
    }
}

int chunk_gen(struct world* world, vec2 coord, struct chunk **c) {
    struct chunk* chunk = malloc(sizeof(struct chunk));
    memcpy(chunk->coord,coord, sizeof(vec2));
    memset(chunk->blocks, 0, CHUNK_HEIGHT * CHUNK_LENGTH * CHUNK_WIDTH * sizeof(struct block*));
    // fprintf(stderr, "=================== CHUNK (%f, %f) ===================\n", chunk->coord[0], chunk->coord[1]);
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_LENGTH; y++) {
            // Minimum z height
            // Interpolation formula - simple linear
            vec2 target = { x, y };
            float z_val = noise_terrain(x + chunk->coord[0]*CHUNK_WIDTH, y + chunk->coord[1]*CHUNK_LENGTH);
            z_val = MAX(0, z_val);
            struct block* blk;
            chunk_block_gen(x, y, z_val, chunk);
        }
    }
    *c = chunk;
    return 0;
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
    // See-through block TODO: Might want to rename this function or handle it differently.
    // Despite there being a leaf block we mark as no neighbor because leaves are see-through.
    if (chunk->blocks[x][y][z]->block_id == BLOCK_LEAF) {
        return 0;
    }
    return 1;
}

/**
 * Basic Mountain chunk generation
 *
 */
struct block* _chunk_mountains_gen(float h) {
    struct block* blk = malloc(sizeof(struct block));
    if (h <= UNDERGROUND_LAYER) {
        block_init(blk, BLOCK_ROCK);
    }
    else if (h <= CAVERN_LAYER) {
        block_init(blk, BLOCK_STONE);
    } 
    else if (h <= (BIOME_BASE + MOUNTAIN_HEIGHT) * 0.6) {
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
    return blk;
}

/**
 * Basic Snow chunk generation
 *
 */
struct block* _chunk_snow_gen(float h) {
    struct block* blk = malloc(sizeof(struct block));
    if (h <= UNDERGROUND_LAYER) {
        block_init(blk, BLOCK_ROCK);
    }
    else if (h <= CAVERN_LAYER) {
        block_init(blk, BLOCK_STONE);
    } else {
        block_init(blk, BLOCK_SNOW);
    }
    return blk;
}

/**
 * Basic Desert chunk generation
 *
 */
struct block* _chunk_desert_gen(float h) {
    struct block* blk = malloc(sizeof(struct block));
    if (h <= UNDERGROUND_LAYER) {
        block_init(blk, BLOCK_ROCK);
    }
    else if (h <= CAVERN_LAYER) {
        block_init(blk, BLOCK_STONE);
    } else {
        block_init(blk, BLOCK_SAND);
    }
    return blk;
}
/**
 * Basic Plains chunk generation
 *
 */
struct block* _chunk_plains_gen(float h) {
    struct block* blk = malloc(sizeof(struct block));
    if (h <= UNDERGROUND_LAYER) {
        block_init(blk, BLOCK_ROCK);
    }
    else if (h <= CAVERN_LAYER) {
        block_init(blk, BLOCK_STONE);
    } else {
        block_init(blk, BLOCK_GRASS);
    }
    return blk;
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

int chunk_block_place(struct chunk* chunk, vec3 pos, enum BLOCK_ID block_id) {
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
        block_init(blk, block_id);
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
        // Loaded is set to zero so that OpenGL redraws
        chunk->loaded = 0;
        return 0;
    }
    return 1;
}
