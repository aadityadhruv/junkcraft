#pragma once
#include "glad/glad.h"
struct texture {
    GLuint _tbo;
};

void texture_init(struct texture** texture);
void texture_load(struct texture* texture);
void texture_load_items(struct texture* texture);
void texture_use(struct texture* texture);
