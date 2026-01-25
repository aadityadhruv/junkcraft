#pragma once
#include "cglm/types.h"
#include "glad/glad.h"
struct block {
   vec3 coords; 
    int type;
    GLuint _vao;
    GLuint _vbo1;
    GLuint _vbo2;
    GLuint _ebo;
};
int block_init(vec3 pos, struct block* blk);
int block_draw(struct block* blk);
