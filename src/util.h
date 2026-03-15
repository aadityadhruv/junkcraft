#pragma once
#include "glad/glad.h"
#include "cglm/types.h"
float lerp(float a, float b, float f);
void create_vbo(GLuint *vbo, void* buf, int size);
void create_vbo_dyn(GLuint *vbo, void* buf, int size);
void create_ebo(GLuint *ebo, void* buf, int size);
void create_texture(GLuint* tbo, void* buf, vec2 size);
