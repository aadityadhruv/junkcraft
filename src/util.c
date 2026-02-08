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

void create_texture(GLuint* tbo, void* buf, vec2 size) {
    glGenTextures(1, tbo);
    glBindTexture(GL_TEXTURE_2D, *tbo);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size[0], size[1], 0, GL_RGB, GL_UNSIGNED_BYTE, buf);
    glGenerateMipmap(GL_TEXTURE_2D);
    // glBindTexture(GL_TEXTURE_2D, 0);
}
