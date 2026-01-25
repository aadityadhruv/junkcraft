#include "util.h"

void create_vbo(GLuint *vbo, void* buf, int size) {
    //tell opengl we want mem for 1 buffer object
    glGenBuffers(1, vbo);
    // set aray_buffer to above
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    // copy vertex data to gpu memory
    glBufferData(GL_ARRAY_BUFFER, size, buf, GL_STATIC_DRAW);
    // cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void create_ebo(GLuint *ebo, void* buf, int size) {
    //tell opengl we want mem for 1 buffer object
    glGenBuffers(1, ebo);
    // set aray_buffer to above
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *ebo);
    // copy vertex data to gpu memory
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, buf, GL_STATIC_DRAW);
    // cleanup
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

