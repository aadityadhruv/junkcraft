#include "engine.h"
#include "block.h"
#include "camera.h"
#include "cglm/io.h"
#include "chunk.h"
#include "input.h"
#include "window.h"
#include "world.h"
#include <junk/vector.h>
#include <string.h>
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
    // memset(engine->loaded_chunks, 0, (1 + CHUNK_DISTANCE * 2) * (1 + CHUNK_DISTANCE * 2));

    // Setup camera
    camera_init(&engine->camera);
    vec3 camera_pos = { 0.0f, 15.0f, 0.0f };
    camera_set_position(engine->camera, camera_pos);

    // Setup root chunk
    struct world* world;
    world_init(time(NULL), &world);
    engine->world = world;
    // Final step - Start the game
    engine->game_loop = 1;
    return 0;
}

void engine_update(struct engine* engine) {
    int curr_chunk[2] = { (engine->camera->position[0] / CHUNK_WIDTH), (engine->camera->position[2]) / CHUNK_LENGTH };
    // Chunk update
    struct chunk* c = {0};
    int coord[2] = { curr_chunk[0], curr_chunk[1] };
    world_get_chunk(engine->world, coord, &c);

    // We moved a chunk - load new chunks with chunk_load
    if (engine->curr_chunk[0] != curr_chunk[0] || engine->curr_chunk[1] != curr_chunk[1]) {
        fprintf(stderr, "CHUNK Update! From (%d, %d) to (%d, %d)\n",
                engine->curr_chunk[0],
                engine->curr_chunk[1],
                curr_chunk[0],
                curr_chunk[1]);
        // Update the curr_chunk
        memcpy(engine->curr_chunk, curr_chunk, sizeof(vec2));
        // Load chunks of CHUNK_DISTANCE around curr_chunk
        for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
            for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
                struct chunk* chunk;
                int chunk_coord[2] = { engine->curr_chunk[0] + i, engine->curr_chunk[1] + j };
                world_get_chunk(engine->world, chunk_coord, &chunk);
                // Get "real" coords as in non-negative numbers, that can go in a array
                int real_coord[2] = { i + CHUNK_DISTANCE, j + CHUNK_DISTANCE };
                // engine->loaded_chunks[real_coord[0]][real_coord[1]] = chunk;
                // Load chunk
                chunk_load(chunk, chunk_coord);
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
        // Update engine managed objects
        engine_update(engine);//(1 + CHUNK_DISTANCE * 2) * (1 + CHUNK_DISTANCE * 2)
        camera_update(engine->camera, engine->shader);
        for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
            for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
                struct chunk* chunk = {0};
                // // Load chunk
                // Ensure the y coordinate is negative, because in OpenGL +z-axis (y in chunk system) is towards
                // user, so we want inwards to be positive, so flip sign
                int chunk_coord[2] = { engine->curr_chunk[0] + i, -engine->curr_chunk[1] + j  };
                world_get_chunk(engine->world, chunk_coord, &chunk);
                // Load chunk
                chunk_load(chunk, chunk_coord);
                chunk_draw(chunk, engine->shader);
            }
        }
        SDL_RenderPresent(engine->window->renderer);
    }
}
