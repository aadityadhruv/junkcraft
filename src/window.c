#include "window.h"
#include "glad/glad.h"
#include "config.h"
#include <stdio.h>

int window_init(struct window* window) {
    SDL_Init(SDL_INIT_VIDEO);
    // This will call SDL_GL_LoadLibrary so you don't need glad to do anything
    window->window = SDL_CreateWindow("Junkcraft", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);

    SDL_GLContext* ctx = SDL_GL_CreateContext(window->window);
    // Let GLAD use the SDL GetProcAddress loader instead of using its own
    int version = gladLoadGLLoader(SDL_GL_GetProcAddress);
    if (version == 0) {
        printf("Failed to initialize OpenGL context\n");
        return -1;
    }
    printf("Loaded OpenGL %s\n", glGetString(GL_VERSION));
    printf("System:  %s\n", glGetString(GL_RENDERER));


    if (window == NULL) {
        perror("Failed to create window!");
        SDL_DestroyWindow(window->window);
        SDL_Quit();
        return -1;
    }

    window->renderer = SDL_CreateRenderer(window->window, -1, SDL_RENDERER_ACCELERATED);
    if (window->renderer == NULL) {
        perror("Failed to create renderer!");
        SDL_DestroyWindow(window->window);
        SDL_Quit();
        return -1;
    }
    SDL_ShowWindow(window->window);
    return 0;
}
void window_cleanup(struct window* window) {
    SDL_DestroyWindow(window->window);
    SDL_Quit();
}
