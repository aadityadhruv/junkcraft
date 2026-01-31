#include "chunk.h"
#include "block.h"
#include "cglm/types.h"
#include "cglm/vec3.h"
#include "shader.h"
#include "util.h"
#include "world.h"
#include "cglm/cglm.h"
#include <junk/vector.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MIN(x, y) (x < y) ? x : y
#define MAX(x, y) (x > y) ? x : y
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

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
    
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_LENGTH; y++) {
            // Minimum z height
            // Interpolation formula - simple linear
            vec2 target = { x, y };
            float z1 = _chunk_plains_get_z(target, poi1, m, z);
            float z2 = _chunk_plains_get_z(target, poi2, -m, z);
            int z_final = (z1 + z2) / 2;
            for (int h = 0; h < z_final; h++) {
                struct block* blk = malloc(sizeof(struct block));
                // Adjust block coordinates with global chunk coordinates
                block_init(blk, BLOCK_GRASS);
                chunk->blocks[x][y][h] = blk;
            }
        }
    }
    chunk->loaded = 1;
}


int* _chunk_face_order_add(int* face_order, int size, int idx) {
    int* buf = malloc(size);
    memcpy(buf, face_order, size);
    for (int i = 0; i < size / sizeof(int); i++) {
        buf[i] += idx;
    }
    return buf;
}
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
void chunk_load(struct chunk *chunk, int coord[2]) {
    fprintf(stderr, "Loaded chunk (%d, %d)\n", coord[0], coord[1]);
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
        1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, // top-left
        0.0f, 0.0f, 1.0f, // Front normal
        0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, // bottom-left
        0.0f, 0.0f, 1.0f, // Front normal
        0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, // bottom-right
        0.0f, 0.0f, 1.0f, // Front normal
        1.0f, 0.0f,
    };
    float back_face[] = {
        0.0f, 1.0f, -1.0f, // top-left (back plane)
        0.0f, 0.0f, -1.0f, // Back normal
        0.0f, 1.0f,
        1.0f, 1.0f, -1.0f, // top-right (back plane)
        0.0f, 0.0f, -1.0f, // Back normal
        1.0f, 1.0f,
        1.0f, 0.0f, -1.0f, // bottom-right (back plane)
        0.0f, 0.0f, -1.0f, // Back normal
        1.0f, 0.0f,
        0.0f, 0.0f, -1.0f, // bottom-left (back plane)
        0.0f, 0.0f, -1.0f, // Back normal
        0.0f, 0.0f,
    };
    float right_face[] = {
        1.0f, 1.0f, -1.0f, // top-right (back plane)
        1.0f, 0.0f, 0.0f, // Right normal
        1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, // top-right
        1.0f, 0.0f, 0.0f, // Right normal
        0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, // bottom-right
        1.0f, 0.0f, 0.0f, // Right normal
        0.0f, 0.0f,
        1.0f, 0.0f, -1.0f, // bottom-right (back plane)
        1.0f, 0.0f, 0.0f, // Right normal
        1.0f, 0.0f,
    };
    float left_face[] = {
        0.0f, 1.0f, 0.0f, // top-left
        -1.0f, 0.0f, 0.0f, // Left normal
        1.0f, 1.0f,
        0.0f, 1.0f, -1.0f, // top-left (back plane)
        -1.0f, 0.0f, 0.0f, // Left normal
        0.0f, 1.0f,
        0.0f, 0.0f, -1.0f, // bottom-left (back plane)
        -1.0f, 0.0f, 0.0f, // Left normal
        0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, // bottom-left
        -1.0f, 0.0f, 0.0f, // Left normal
        1.0f, 0.0f,
    };
    float top_face[] = {
        1.0f, 1.0f, -1.0f, // top-right (back plane)
        0.0f, 1.0f, 0.0f, // Top normal
        1.0f, 1.0f,
        0.0f, 1.0f, -1.0f, // top-left (back plane)
        0.0f, 1.0f, 0.0f, // Top normal
        0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, // top-left
        0.0f, 1.0f, 0.0f, // Top normal
        0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, // top-right
        0.0f, 1.0f, 0.0f, // Top normal
        1.0f, 0.0f,
    };
    float bottom_face[] = {
        1.0f, 0.0f, -1.0f, // bottom-right (back plane)
        0.0f, -1.0f, 0.0f, // Bottom normal
        1.0f, 1.0f,
        0.0f, 0.0f, -1.0f, // bottom-left (back plane)
        0.0f, -1.0f, 0.0f, // Bottom normal
        0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, // bottom-left
        0.0f, -1.0f, 0.0f, // Bottom normal
        0.0f, 0.0f,
        1.0f, 0.0f, 0.0f, // bottom-right
        0.0f, -1.0f, 0.0f, // Bottom normal
        1.0f, 0.0f,
    };

    int vertex_draw_order[] = {
        1, 2, 3,   3, 0, 1, // CCW 2-triangles (quad)
    };
    // ============= Face detection algorithm =============
    int vertex_index = 0;
    int blk_c = 0;
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_LENGTH; z++) {
                struct block* blk = chunk->blocks[x][z][y];
                // If not air block
                if (blk != NULL) {
                    blk_c += 1;
                    vec3 pos = { x, y, -z };
                    // Position of block in world coords
                    // glm_vec3_add(pos, translation, pos);

                    VECTOR_INSERT(vertices, _chunk_face_add(front_face,
                                sizeof(front_face), pos));
                    VECTOR_INSERT(vertex_order,
                            _chunk_face_order_add(vertex_draw_order,
                                sizeof(vertex_draw_order), vertex_index));
                    vertex_index += 4;

                    VECTOR_INSERT(vertices, _chunk_face_add(back_face,
                                sizeof(back_face), pos));
                    VECTOR_INSERT(vertex_order,
                            _chunk_face_order_add(vertex_draw_order,
                                sizeof(vertex_draw_order), vertex_index));
                    vertex_index += 4;

                    VECTOR_INSERT(vertices, _chunk_face_add(right_face,
                                sizeof(right_face), pos));
                    VECTOR_INSERT(vertex_order,
                            _chunk_face_order_add(vertex_draw_order,
                                sizeof(vertex_draw_order), vertex_index));
                    vertex_index += 4;

                    VECTOR_INSERT(vertices, _chunk_face_add(left_face,
                                sizeof(left_face), pos));
                    VECTOR_INSERT(vertex_order,
                            _chunk_face_order_add(vertex_draw_order,
                                sizeof(vertex_draw_order), vertex_index));
                    vertex_index += 4;

                    VECTOR_INSERT(vertices, _chunk_face_add(top_face,
                                sizeof(top_face), pos));
                    VECTOR_INSERT(vertex_order,
                            _chunk_face_order_add(vertex_draw_order,
                                sizeof(vertex_draw_order), vertex_index));
                    vertex_index += 4;

                    VECTOR_INSERT(vertices, _chunk_face_add(bottom_face,
                                sizeof(bottom_face), pos));
                    VECTOR_INSERT(vertex_order,
                            _chunk_face_order_add(vertex_draw_order,
                                sizeof(vertex_draw_order), vertex_index));
                    vertex_index += 4;
                }
            }
        }
    }
    fprintf(stderr, "Chunk blk_c: %d\n", blk_c);

    float tmp_vertex[vector_length(vertices) * sizeof(front_face)];
    int tmp_order[vector_length(vertex_order) * sizeof(vertex_draw_order)];
    fprintf(stderr, "Chunk blk_c: %d v_s: %d, v_o: %d\n", blk_c, vector_length(vertices) / 6, vector_length(vertex_order) / 6);
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
    vec3 translation = {CHUNK_WIDTH * coord[0], 0, - (CHUNK_LENGTH * coord[1])};
    // Set the matrix for world coordinate translation
    glm_mat4_identity(chunk->model);
    glm_translate(chunk->model, translation);
    chunk->loaded = 1;
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
}

// Regenerate chunk data
void chunk_update(struct chunk *chunk) {
}
