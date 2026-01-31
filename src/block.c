#include "cglm/affine.h"
#include "cglm/cam.h"
#include "cglm/cglm.h"
#include "cglm/io.h"
#include "cglm/util.h"
#include "cglm/vec3.h"
#include "math.h"
#include "glad/glad.h"
#include "block.h"
#include "shader.h"
#include "texture.h"
#include "util.h"
#include "block.h"
#include <math.h>
#include <stdio.h>
#include <string.h>


#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

void block_update(struct block* blk);
int block_init(vec3 pos, struct block* blk) {
    // Store buffer data into struct
    // Initialize vbo and ebo for block
    
    memcpy(blk->coords, pos, sizeof(vec3));
    block_load_gpu(blk);
    block_update(blk);
    return 0;
}

void block_load_gpu(struct block* blk) {
    // ========== Constants of a block ================
    // Local world coordinates
    float vertices[] = {
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

        1.0f, 1.0f, -1.0f, // top-right (back plane)
        0.0f, 0.0f, -1.0f, // Back normal
        1.0f, 1.0f,
        0.0f, 1.0f, -1.0f, // top-left (back plane)
        0.0f, 0.0f, -1.0f, // Back normal
        0.0f, 1.0f,
        0.0f, 0.0f, -1.0f, // bottom-left (back plane)
        0.0f, 0.0f, -1.0f, // Back normal
        0.0f, 0.0f,
        1.0f, 0.0f, -1.0f, // bottom-right (back plane)
        0.0f, 0.0f, -1.0f, // Back normal
        1.0f, 0.0f,

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
    // int vertex_order[] = {
    //     1, 2, 3,  3, 0, 1, // Front
    //     5, 6, 7,  7, 4, 5, // Back
    //     9, 10, 11, 11, 8, 9, // Right
    //     13, 14, 15,   15, 12, 13, // Left
    //     17, 18, 19,   19, 16, 17, // Top
    //     21, 22, 23,   23, 20, 21, // Bottom
    //
    // };
    int vertex_order[] = {
        1, 2, 3,   3, 0, 1, // Front
        5, 6, 7,   7, 4, 5, // Back
        9, 10, 11, 11, 8, 9, // Right
        13, 14, 15,   15, 12, 13, // Left
        17, 18, 19,   19, 16, 17, // Top
        21, 22, 23,   23, 20, 21, // Bottom
    };

    // ================ OpenGL work ================
    create_vbo(&blk->_vbo, (void*)vertices, sizeof(float) * ARRAY_SIZE(vertices));
    create_ebo(&blk->_ebo, (void*)vertex_order, sizeof(int) * ARRAY_SIZE(vertex_order));


    blk->_vertex_count = ARRAY_SIZE(vertex_order);
    glGenVertexArrays(1, &blk->_vao);
    glBindVertexArray(blk->_vao);
    // Enable 3 attribs - position normals texture
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    // set vao_buffer to pos buffer obj
    glBindBuffer(GL_ARRAY_BUFFER, blk->_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    // set vao_buffer to normals buffer obj
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(3*sizeof(float)));
    // set vao_buffer to texture buffer obj
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (GLvoid*)(6*sizeof(float)));
    // Set EBO to the vertex_order
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, blk->_ebo);
    //NOTE: This is important, otherwise with multiple block_init calls, it
    //creates a segfault since the bindings get all messed up. Why it gets
    //messed up? Let's say we make 2 blocks. Block 1 creates VBOs, then VAO,
    //then binds everything. Now VAO is still bound. Block 2 init starts. First
    //call is create_vbo. Since VAO is already bound, it gets bound to the OLD
    //VAO!! Always clear before use. 
    glBindVertexArray(0);
}

void block_update(struct block* blk) {
    //=============== Matrix Work ==============

    // RTS matrix - rotate, translate, scale
    glm_mat4_identity(blk->model);
    float angle = glm_rad(blk->angle);
    // vec3 scale = { 0.90f, 0.90f, 0.90f };
    glm_translate(blk->model, blk->coords);
    // glm_scale(blk->model, scale);
    // glm_rotate_at(blk->model, pivot, angle, rot_axis);
    // View matrix (camera)
    //blk->angle = fmodf(blk->angle + 0.005f, 360.0f);
}

// Register block vbos and ebos to context
int block_draw(struct block* blk, struct shader* shader, struct texture* texture) {
    glBindVertexArray(blk->_vao);
    set_uniform_mat4("model", shader, blk->model);
    // GLuint loc = glGetUniformLocation(shader->program, "face_colors");
    // if (loc == -1) {
    //     fprintf(stderr, "Invalid var %s for get_uniform_mat4: Does not exist\n", "face_colors");
    //     exit(1);
    //     return -1;
    // }
    // float colors[] = {
    //     0.761f, 0.424f,  0.0f, 
    //     0.761f, 0.424f,  0.0f, 
    //     0.761f, 0.424f,  0.0f, 
    //     0.761f, 0.424f,  0.0f, 
    //     0.404f, 0.776f,  0.027f, 
    //     0.761f, 0.424f,  0.0f, 
    // };
    // glUniform3fv(loc, 6, (void*)colors);
    // texture_draw(texture);
    glDrawElements(GL_TRIANGLES, blk->_vertex_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    return 0;
}

void block_debug(struct block *blk) {
    fprintf(stderr, "==== Block Debug ====\n");
    fprintf(stderr, "==== Block Coords ====\n");
    glm_vec3_print(blk->coords, stderr);
    fprintf(stderr, "==== Block Model ====\n");
    glm_mat4_print(blk->model, stderr);

}

void block_unload(struct block *blk) {
    // Clear VBO data
    glDeleteBuffers(1, &blk->_vbo);
    // Clear EBO data
    glDeleteBuffers(1, &blk->_ebo);
    // Clear VAO
    glDeleteVertexArrays(1, &blk->_vao);
}
