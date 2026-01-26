#include "engine.h"
#include "block.h"
#include "window.h"
#include <junk/vector.h>

int engine_init(struct engine *engine) {
    // Setup the Window
    struct window* window = malloc(sizeof(struct window));
    memset(window, 0, sizeof(struct window));
    if (window_init(window) != 0) {
        free(window);
        return -1;
    }
    engine->window = window;

    // Setup Shader pipeline
    struct shader* shader = malloc(sizeof(struct shader));
    memset(shader, 0, sizeof(struct shader));
    if (shader_init(shader)) {
        free(window);
        free(shader);
        return -1;
    };
    engine->shader = shader;

    // Setup Objects to draw
    struct block* blk = malloc(sizeof(struct block));
    memset(blk, 0, sizeof(struct block));
    vec3 pos = { 0, 0, 0 };
    if (block_init(pos, blk) != 0) {
        free(window);
        free(shader);
        free(blk);
        return -1;
    }
    vector_init(&engine->objects);
    VECTOR_INSERT(engine->objects, (void*)blk);

    engine->game_loop = 1;
    return 0;
}

void engine_draw(struct engine* engine) {
    while (engine->game_loop) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Quit game
            if (event.type == SDL_QUIT) {
                engine->game_loop = 0;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
        glUseProgram(engine->shader->program);
        for (int i = 0; i < engine->objects->len; i++) {
            struct block* block = vector_get(engine->objects, i);
            block_draw(block, engine->shader);
        }
        SDL_RenderPresent(engine->window->renderer);
    }
}
