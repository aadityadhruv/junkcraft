#include "cglm/types.h"
#include "glad/glad.h"
#include "block.h"
#include "util.h"
#include "cglm/cglm.h"
#include "block.h"
#include <string.h>


int block_init(vec3 pos, struct block* blk) {
    // Store buffer data into struct
    // Initialize vbo and ebo for block
    memcpy(blk->coords, pos, sizeof(vec3));
    float vertices[] = {
       0.5f,  0.5f,  0.0f, // top-right
       -0.5f, 0.5f,  0.0f, // top-left
       -0.5f, -0.5f,  0.0f, // bottom-left
       0.5f, -0.5f,  0.0f, // bottom-right
    };
    float colors[] = {
      1.0f, 0.0f,  0.0f, // r,g,b of first point.
      0.0f, 1.0f,  0.0f, // r,g,b of second point.
      0.0f, 0.0f,  1.0f,  // r,g,b of third point.
      1.0f, 1.0f,  1.0f  // r,g,b of fourth point.
    };
    int vertex_order[] = { 1, 2, 3,  3, 0, 1 };

    //TODO:
    //1. Create VAO for mesh
    //2. Create VBOs for vertices and colors
    //3. Set EBO for index order
    create_vbo(&blk->_vbo1, (void*)vertices, sizeof(float) * 12);
    create_vbo(&blk->_vbo2, (void*)colors, sizeof(float) * 12);
    create_ebo(&blk->_ebo, (void*)vertex_order, sizeof(int) * 6);
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

// Register block vbos and ebos to context
int block_draw(struct block* blk) {
    glBindVertexArray(blk->_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    return 0;
}
