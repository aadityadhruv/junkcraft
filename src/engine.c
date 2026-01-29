#include "engine.h"
#include "block.h"
#include "camera.h"
#include "chunk.h"
#include "input.h"
#include "window.h"
#include "world.h"
#include <junk/vector.h>
#include <time.h>

void _engine_insert_chunk_ptrs(struct engine* engine, struct chunk* chunk);

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
    vector_init(&engine->objects);
    // Setup root chunk
    struct world* world;
    world_init(time(NULL), &world);
    vec2 curr_chunk = { 0, 0 };
    int chunk_distance = 2;
    for (int i = 0; i < chunk_distance; i++) {
        for (int j = 0; j  < chunk_distance; j++) {
            struct chunk* chunk;
            vec2 chunk_coord = { curr_chunk[0] + i, curr_chunk[1] + j };
            world_get_chunk(world, chunk_coord, &chunk);
            _engine_insert_chunk_ptrs(engine, chunk);
        }
    }


    // Setup camera
    camera_init(&engine->camera);
    vec3 camera_pos = { 0.0f, 5.0f, -0.0f };
    camera_set_position(engine->camera, camera_pos);

    // Final step - Start the game
    engine->game_loop = 1;
    return 0;
}
void _engine_insert_chunk_ptrs(struct engine* engine, struct chunk* chunk) {
    int counter = 0;
    for (int i = 0; i < CHUNK_WIDTH; i++) {
        for (int j = 0; j < CHUNK_LENGTH; j++) {
            for (int k = 0; k < CHUNK_HEIGHT; k++) {
                struct block* blk = chunk->blocks[i][j][k];
                if (blk == NULL) {
                    continue;
                }
                if (VECTOR_INSERT(engine->objects, (void*)blk) == -1) exit(1);
                counter += 1;
            }
        }
    }
}

void engine_draw(struct engine* engine) {
    while (engine->game_loop) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
        glUseProgram(engine->shader->program);
        for (int i = 0; i < vector_length(engine->objects); i++) {
            struct block* block = vector_get(engine->objects, i);
            camera_update(engine->camera, engine->shader);
            block_draw(block, engine->shader);
        }
        SDL_RenderPresent(engine->window->renderer);
    }
}
