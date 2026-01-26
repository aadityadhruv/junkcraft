#pragma once
#include "cglm/types.h"
#include "glad/glad.h"
#include "shader.h"

struct block {
    vec3 coords; 
    int type;
    GLuint _vao;
    GLuint _vbo;
    GLuint _ebo;
    int _vertex_count;
    mat4 model;
    mat4 view;
    mat4 perspective;
    float angle;
};
int block_init(vec3 pos, struct block* blk);
int block_draw(struct block* blk, struct shader* shader);
