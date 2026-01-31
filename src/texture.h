#pragma once
#include "glad/glad.h"
struct texture {
    GLuint _tbo;
};

void texture_init(struct texture** texture);
void texture_load(struct texture* texture, char* path);
void texture_draw(struct texture* texture);
