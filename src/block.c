#include "cglm/cglm.h"
#include "cglm/io.h"
#include "cglm/util.h"
#include "cglm/vec3.h"
#include "math.h"
#include "glad/glad.h"
#include "block.h"
#include "shader.h"
#include "util.h"
#include "block.h"
#include <math.h>
#include <string.h>


#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

int block_init(vec3 pos, struct block* blk) {
    // Store buffer data into struct
    // Initialize vbo and ebo for block
    memcpy(blk->coords, pos, sizeof(vec3));
    // ========== Constants of a block ================
    // Local world coordinates
    float vertices[] = {
        1.0f, 1.0f, 0.0f, // top-right
        0.0f, 1.0f, 0.0f, // top-left
        0.0f, 0.0f, 0.0f, // bottom-left
        1.0f, 0.0f, 0.0f, // bottom-right

        1.0f, 1.0f, -1.0f, // top-right (back plane)
        0.0f, 1.0f, -1.0f, // top-left (back plane)
        0.0f, 0.0f, -1.0f, // bottom-left (back plane)
        1.0f, 0.0f, -1.0f, // bottom-right (back plane)
    };
    int vertex_order[] = {
        1, 2, 3,  3, 0, 1, // Front
        5, 6, 7,  7, 4, 5, // Back
        0, 3, 7,  7, 4, 0, // Right
        1, 2, 6,  6, 5, 1, // Left
        5, 1, 0,  0, 4, 5, // Top
        6, 2, 3,  3, 7, 6, // Bottom
    };

    // ================ OpenGL work ================
    create_vbo(&blk->_vbo, (void*)vertices, sizeof(float) * ARRAY_SIZE(vertices));
    create_ebo(&blk->_ebo, (void*)vertex_order, sizeof(int) * ARRAY_SIZE(vertex_order));

    blk->_vertex_count = ARRAY_SIZE(vertex_order);
    glGenVertexArrays(1, &blk->_vao);
    glBindVertexArray(blk->_vao);
    // Enable 2 attribs - position
    glEnableVertexAttribArray(0);
    // set vao_buffer to pos buffer obj
    glBindBuffer(GL_ARRAY_BUFFER, blk->_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // Set EBO to the vertex_order
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, blk->_ebo);
    //NOTE: This is important, otherwise with multiple block_init calls, it
    //creates a segfault since the bindings get all messed up. Why it gets
    //messed up? Let's say we make 2 blocks. Block 1 creates VBOs, then VAO,
    //then binds everything. Now VAO is still bound. Block 2 init starts. First
    //call is create_vbo. Since VAO is already bound, it gets bound to the OLD
    //VAO!! Always clear before use. 
    glBindVertexArray(0);
    return 0;
}

void block_update(struct block* blk) {
    //=============== Matrix Work ==============

    // RTS matrix - rotate, translate, scale
    glm_mat4_identity(blk->model);
    float angle = glm_rad(blk->angle);
    vec3 rot_axis = { 1.0f, 1.0f, 0.0f };
    vec3 axis_y = { 0.0f, 1.0f, 0.0f };
    vec3 pivot = { 0.0f, 0.0f, 0.0f };
    vec3 scale = { 0.25f, 0.25f, 0.25f };
    glm_translate(blk->model, blk->coords);
    glm_scale(blk->model, scale);
    glm_rotate_at(blk->model, pivot, angle, rot_axis);
    // View matrix (camera)
    vec3 camera = { 0.0f, 0.0f, 2.0f };
    vec3 camera_direction = { 0.0f, 0.0f, -1.0f };
    glm_look(camera, camera_direction, axis_y, blk->view);
    // Projection (perspective) matrix
    glm_perspective(glm_rad(45.0f), 800.0f / 600.0f, 0.1f, -10.0f, blk->perspective);

    blk->angle = fmodf(blk->angle + 0.001f, 360.0f);
}

// Register block vbos and ebos to context
int block_draw(struct block* blk, struct shader* shader) {
    glBindVertexArray(blk->_vao);
    set_uniform_mat4("model", shader, blk->model);
    set_uniform_mat4("view", shader, blk->view);
    set_uniform_mat4("perspective", shader, blk->perspective);
    GLuint loc = glGetUniformLocation(shader->program, "face_colors");
    if (loc == -1) {
        fprintf(stderr, "Invalid var %s for get_uniform_mat4: Does not exist\n", "face_colors");
        exit(1);
        return -1;
    }
    float colors[] = {
        1.0f, 0.0f,  0.0f, 
        0.0f, 1.0f,  0.0f, 
        0.0f, 0.0f,  1.0f, 
        1.0f, 1.0f,  0.0f, 
        0.0f, 1.0f,  1.0f, 
        1.0f, 0.0f,  1.0f, 
    };
    glUniform3fv(loc, 6, (void*)colors);
    glDrawElements(GL_TRIANGLES, blk->_vertex_count, GL_UNSIGNED_INT, 0);
    block_update(blk);
    return 0;
}
