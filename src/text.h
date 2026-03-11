#pragma once
#include "glad/glad.h"
#include "cglm/cglm.h"
#include "shader.h"


struct character {
    unsigned int texture;
    vec2 size;
    vec2 bearing;
    unsigned int advance;
};

struct text {
    struct character chars[129];
    GLuint _vao;
    GLuint _vbo;

};

int text_init(struct text** t);
void text_draw(struct text* text, struct shader* shader, char* string, float x, float y, float scale, int size);
