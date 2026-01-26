#include "cglm/cglm.h"
#include "cglm/io.h"
#include "cglm/util.h"
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
       0.5f,  0.5f,  0.0f, // top-right
       -0.5f, 0.5f,  0.0f, // top-left
       -0.5f, -0.5f,  0.0f, // bottom-left
       0.5f, -0.5f,  0.0f, // bottom-right

       0.5f,  0.5f,  -0.5f, // top-right (back plane)
       -0.5f, 0.5f,  -0.5f, // top-left (back plane)
       -0.5f, -0.5f, -0.5f, // bottom-left (back plane)
       0.5f, -0.5f,  -0.5f, // bottom-right (back plane)
    };
    float colors[] = {
      1.0f, 0.0f,  0.0f, 
      1.0f, 0.0f,  0.0f, 
      0.0f, 1.0f,  0.0f, 
      0.0f, 1.0f,  0.0f, 

      0.0f, 0.0f,  1.0f, 
      0.0f, 0.0f,  1.0f, 
      1.0f, 1.0f,  0.0f, 
      1.0f, 1.0f,  0.0f, 

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
    create_vbo(&blk->_vbo1, (void*)vertices, sizeof(float) * ARRAY_SIZE(vertices));
    create_vbo(&blk->_vbo2, (void*)colors, sizeof(float) * ARRAY_SIZE(colors));
    create_ebo(&blk->_ebo, (void*)vertex_order, sizeof(int) * ARRAY_SIZE(vertex_order));

    blk->_vertex_count = ARRAY_SIZE(vertex_order);
    glGenVertexArrays(1, &blk->_vao);
    glBindVertexArray(blk->_vao);
    // Enable 2 attribs - position and color
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    // set vao_buffer to pos buffer obj
    glBindBuffer(GL_ARRAY_BUFFER, blk->_vbo1);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // set vao_buffer to color buffer obj
    glBindBuffer(GL_ARRAY_BUFFER, blk->_vbo2);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, blk->_ebo);

    return 0;
}

void block_update(struct block* blk) {
    //=============== Matrix Work ==============

    // RTS matrix - rotate, translate, scale
    glm_mat4_identity(blk->model);
    float angle = glm_rad(blk->angle);
    vec3 axis_y = { 0.0f, 1.0f, 0.0f };
    vec3 pivot = { 0.0f, 0.0f, -0.25f };
    vec3 translation = { 0.0f, 0, -2.0f };
    glm_translate(blk->model, translation);
    glm_rotate_at(blk->model, pivot, angle, axis_y);
    glm_mat4_print(blk->model, stderr);
    // View matrix (camera)
    vec3 camera = { 0.0f, 0.0f, -2.0f };
    vec3 camera_direction = { 0.0f, 0.0f, 1.0f };
    glm_look(camera, camera_direction, axis_y, blk->view);

    // Projection (perspective) matrix

    glm_perspective(glm_rad(45.0f), 800.0f / 600.0f, 0.1f, -10.0f, blk->perspective);

    blk->angle = fmodf(blk->angle + 0.01f, 360.0f);
}

// Register block vbos and ebos to context
int block_draw(struct block* blk, struct shader* shader) {
    glBindVertexArray(blk->_vao);
    set_uniform_mat4("model", shader, blk->model);
//    set_uniform_mat4("view", shader, blk->view);
   set_uniform_mat4("perspective", shader, blk->perspective);
    glDrawElements(GL_TRIANGLES, blk->_vertex_count, GL_UNSIGNED_INT, 0);
    block_update(blk);
    return 0;
}
