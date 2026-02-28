#include "window.h"
#include "glad/glad.h"
#include "config.h"
#include <stdio.h>

int window_init(struct window* window) {
    SDL_Init(SDL_INIT_VIDEO);
    // This will call SDL_GL_LoadLibrary so you don't need glad to do anything
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,24);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    window->window = SDL_CreateWindow("Junkcraft", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GLContext ctx = SDL_GL_CreateContext(window->window);
    // Let GLAD use the SDL GetProcAddress loader instead of using its own
    int version = gladLoadGLLoader(SDL_GL_GetProcAddress);
    if (version == 0) {
        printf("Failed to initialize OpenGL context\n");
        return -1;
    }
    printf("Loaded OpenGL %s\n", glGetString(GL_VERSION));
    printf("System:  %s\n", glGetString(GL_RENDERER));
    // Disable VSYNC - it causes horrible performance loss
    SDL_GL_SetSwapInterval(0);


    if (window->window == NULL) {
        perror("Failed to create window!");
        SDL_DestroyWindow(window->window);
        SDL_Quit();
        return -1;
    }
    return 0;
}
void window_cleanup(struct window* window) {
    SDL_DestroyWindow(window->window);
    SDL_Quit();
}
