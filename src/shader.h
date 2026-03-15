#pragma once
#include "cglm/types.h"
#include "glad/glad.h"

struct shader {
    GLuint program; 
};
/*
 *Load a vertex and framgent shader defined in the shaders/ directory. 
 @param shader The shader struct to store the compiled program pointer in
 @return zero on success
 */
int shader_init(struct shader** shader);
int shader_add(struct shader* shader, char* vs_path, char* fs_path);
int set_uniform_mat4(char* var, struct shader* shader, mat4 matrix);
int set_uniform_sampler2d(char* var, struct shader* shader, int texture_id);
int set_uniform_vec3(char* var, struct shader* shader, vec3 vec);
void shader_use(struct shader* shader);
