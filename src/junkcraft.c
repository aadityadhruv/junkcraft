#include <SDL2/SDL_events.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <signal.h>
#include <stdio.h>
#include "SDL2/SDL_render.h"
#include "glad/glad.h"
#include "shader.h"
#include "block.h"
#include "window.h"

// Global OpenGL Context
GLuint shader_program; 
int game_loop = 1;

void draw(struct window* window, struct block* block) {
    while (game_loop) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Quit game
            if (event.type == SDL_QUIT) {
                game_loop = 0;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader_program);
        block_draw(block);
        SDL_RenderPresent(window->renderer);
    }
}

int main() {
    struct window window = {0};
    if (window_init(&window) != 0) {
        return -1;
    }
    struct block blk = {0};
    vec3 pos = { 0, 0, 0 };
    block_init(pos, &blk);
    shader_program = shader_init();
    draw(&window, &blk);
    window_cleanup(&window);
    return 0;
}
