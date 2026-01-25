#include "shader.h"
#include <stdio.h>
#include <stdlib.h>



/**
 * Simple method to read shaders from path and return a string
 * containing the code. Function allocates memory which needs 
 * to be cleaned up
 * @param path Path to shader GLSL file
 * @return buf Contents of shader file as a string
 */
char* read_shader(char* path) {
    FILE* file = fopen(path, "r");
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    rewind(file);
    char* buf = malloc(file_size);
    fread(buf, file_size, 1, file);
    return buf;
}

/**
 * Compile some given shader code
 * @param type The type of shader (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER etc)
 * @param code The string corresponding to the shader code
 * @return shader The GLuint pointer to the compiled shader
 */
GLuint compile_shader(GLuint type, const char* code) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &code, NULL);
    glCompileShader(shader);
    return shader;
}

/*
 * Given a vertex and fragment shader, link and compile a shader program,
 * returning the GLuint pointing to that program
 * @param vs The vertex shader string, usually returend from read_shader
 * @param fs The fragment shader string, usually returend from read_shader
 * @return shader_program The pointer to the compiled shader
 */
GLuint create_shader_program(const char* vs, const char* fs) {
    GLuint shader_program = glCreateProgram();
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vs);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fs);
    glAttachShader(shader_program, fragment_shader);
    glAttachShader(shader_program, vertex_shader);
    glLinkProgram(shader_program);
    return shader_program;
}


GLuint shader_init() {
    char* shaders[2] = { "shaders/fragment.glsl", "shaders/vertex.glsl" };
    char* fragment_shader = read_shader(shaders[0]);
    char* vertex_shader = read_shader(shaders[1]);
    GLuint shader_program = create_shader_program(vertex_shader, fragment_shader);
    free(vertex_shader);
    free(fragment_shader);
    return shader_program;
}
