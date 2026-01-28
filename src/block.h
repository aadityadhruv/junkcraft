#pragma once
#include "cglm/types.h"
#include "glad/glad.h"
#include "shader.h"

struct block {
    vec3 coords; 
    GLuint _vao;
    GLuint _vbo;
    GLuint _ebo;
    int _vertex_count;
    mat4 model;
    float angle;
};
int block_init(vec3 pos, struct block* blk);
int block_draw(struct block* blk, struct shader* shader);
void block_debug(struct block* blk);
