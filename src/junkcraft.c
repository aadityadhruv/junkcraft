#include <SDL2/SDL_events.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_video.h"
#include "SDL2/SDL_render.h"
#include "glad/glad.h"

const float vertices[] = {
   0.0f,  0.5f,  0.0f, // x,y,z of first point.
   0.5f, -0.5f,  0.0f, // x,y,z of second point.
  -0.5f, -0.5f,  0.0f  // x,y,z of third point.
};


const char* fragment_shader =
"#version 410 core\n"
"out vec4 frag_colour;"
"in vec3 color;"
"void main() {"
"  frag_colour = vec4( color, 1.0 );"
"}";

const char* vertex_shader =
"#version 410 core\n"
"in vec3 vp;"
"out vec3 color;"
"void main() {"
"  gl_Position = vec4( vp, 1.0 );"
"  color = vp;"
"}";

GLuint shader_init() {
    GLuint vs = glCreateShader( GL_VERTEX_SHADER );
glShaderSource( vs, 1, &vertex_shader, NULL );
glCompileShader( vs );
GLuint fs = glCreateShader( GL_FRAGMENT_SHADER );
glShaderSource( fs, 1, &fragment_shader, NULL );
glCompileShader( fs );
GLuint shader_program = glCreateProgram();
glAttachShader( shader_program, fs );
glAttachShader( shader_program, vs );
glLinkProgram( shader_program );
return shader_program;
}

GLuint vbo;

void quit(int _val) {
    fprintf(stderr, "Quitting");
    exit(1);
}

void gl_buffer_init() {
    //tell opengl we want mem for 1 buffer object
    glGenBuffers(1, &vbo);
    // set aray_buffer to above
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // copy vertex data to gpu memory
    glBufferData(GL_ARRAY_BUFFER, 9* sizeof(float), vertices, GL_STATIC_DRAW);
    // cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

int main() {
    signal(SIGINT, quit);
    SDL_Init(SDL_INIT_VIDEO);
    // This will call SDL_GL_LoadLibrary so you don't need glad to do anything
    SDL_Window* window = SDL_CreateWindow("Junkcraft", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);

    SDL_GLContext* ctx = SDL_GL_CreateContext(window);
    // Let GLAD use the SDL GetProcAddress loader instead of using its own
    int version = gladLoadGLLoader(SDL_GL_GetProcAddress);
        if (version == 0) {
        printf("Failed to initialize OpenGL context\n");
        return -1;
    }
    printf("Loaded OpenGL %d\n", version);


    if (window == NULL) {
        perror("Failed to create window!");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (render == NULL) {
        perror("Failed to create renderer!");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // SDL_RenderClear(render);
    // SDL_Rect rect = { 0, 0, 100, 100 };
    // SDL_SetRenderDrawColor(render, 255, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_ShowWindow(window);
    // SDL_RenderFillRect(render, &rect);


    gl_buffer_init();

    GLuint array;
    // Create VAO
    glGenVertexArrays(1, &array);
    glBindVertexArray(array);
    // set array_buffer to pos buffer obj
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    /// allow vertex shader
    // describe data and enable
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // draw data as a triangle
    //glDrawArrays(GL_TRIANGLES, 0, 3);
    // set viewport
    //glViewport(0, 0, 200, 100);
    //final
    GLuint shader_program = shader_init();
    while (1) {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram( shader_program );
        glBindVertexArray(array);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        SDL_RenderPresent(render);
    }
    return 0;
}
