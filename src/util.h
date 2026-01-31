#pragma once
#include "glad/glad.h"
#include "cglm/types.h"
void create_vbo(GLuint *vbo, void* buf, int size);
void create_ebo(GLuint *ebo, void* buf, int size);
void create_texture(GLuint* tbo, void* buf, vec2 size);
