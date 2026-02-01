#include "engine.h"
#include "block.h"
#include "cglm/io.h"
#include "chunk.h"
#include "input.h"
#include "player.h"
#include "texture.h"
#include "window.h"
#include "world.h"
#include <SDL2/SDL_render.h>
#include <bits/types/timer_t.h>
#include <junk/vector.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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


    // Load Textures
    struct texture* texture = { 0 };
    texture_init(&engine->texture);
    texture_load(engine->texture, "textures/grass.jpg");

    // Setup Objects to draw
    // memset(engine->loaded_chunks, 0, (1 + CHUNK_DISTANCE * 2) * (1 + CHUNK_DISTANCE * 2));

    // Setup player
    vec3 pos = { 0.0f, 40.0f, 0.0f };
    player_init(pos, &engine->player);

    // Setup root chunk
    struct world* world;
    world_init(time(NULL), &world);
    engine->world = world;
    //TODO: Move this loop to a function and flip chunk_coord sign correctly ONCE
    for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
        for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
            struct chunk* chunk;
            int chunk_coord[2] = { engine->curr_chunk[0] + i, engine->curr_chunk[1] + j };
            world_get_chunk(engine->world, chunk_coord, &chunk);
            // Load chunk
            chunk_load(world, chunk, chunk_coord);
        }
    }
    // Final step - Start the game
    engine->game_loop = 1;
    return 0;
}

void engine_update(struct engine* engine) {
    //NOTE: OpenGL FLIP
    int curr_chunk[2] = { floorf(engine->player->position[0] / (float)CHUNK_WIDTH), floorf(-engine->player->position[2] / (float)CHUNK_LENGTH) };
    // Chunk update
    // We moved a chunk - load new chunks with chunk_load
    if (engine->curr_chunk[0] != curr_chunk[0] || engine->curr_chunk[1] != curr_chunk[1]) {
        fprintf(stderr, "CHUNK Update! From (%d, %d) to (%d, %d)\n",
                engine->curr_chunk[0],
                engine->curr_chunk[1],
                curr_chunk[0],
                curr_chunk[1]);
        // Stage relevant chunks for loading
        for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
            for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
                struct chunk* chunk;
                int chunk_coord[2] = { curr_chunk[0] + i, curr_chunk[1] + j };
                world_get_chunk(engine->world, chunk_coord, &chunk);
                // Stage for loading chunk
                chunk->staged_for_load = 1;
                // fprintf(stderr, "Staged (%d, %d)\n", chunk_coord[0], chunk_coord[1]);
            }
        }
        // Unload existing chunks
        for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
            for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
                struct chunk* chunk;
                int chunk_coord[2] = { engine->curr_chunk[0] + i, engine->curr_chunk[1] + j };
                world_get_chunk(engine->world, chunk_coord, &chunk);
                // unload chunk
                if (chunk->staged_for_load == 0) {
                // fprintf(stderr, "Unloaded (%d, %d)\n", chunk_coord[0], chunk_coord[1]);
                    chunk_unload(chunk);
                }
            }
        }
        // Load chunks that are needed
        for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
            for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
                struct chunk* chunk;
                int chunk_coord[2] = { curr_chunk[0] + i, curr_chunk[1] + j };
                world_get_chunk(engine->world, chunk_coord, &chunk);
                // Load chunks - if already loaded will only update coords
                chunk_load(engine->world, chunk, chunk_coord);
                fprintf(stderr, "Loaded (%d, %d)\n", chunk_coord[0], chunk_coord[1]);
            }
        }
        // Update the curr_chunk
        memcpy(engine->curr_chunk, curr_chunk, sizeof(vec2));
    }
}

void engine_fps(struct engine* engine, float fps) {
    // TTF_Font* font = TTF_OpenFont("fonts/PixelifySans-Regular.ttf", 8.0f);
    // SDL_RenderClear(engine->window->renderer);
    // SDL_Color black = {0, 0, 0};
    // char fps_text[36];
    // snprintf(fps_text, 36, "FPS: %.2f", fps);
    // SDL_Surface* text = TTF_RenderText_Solid(font, fps_text, black);
    // SDL_Texture* texture = SDL_CreateTextureFromSurface(engine->window->renderer, text);
    // SDL_RenderCopy(engine->window->renderer, texture, NULL, &rect);
}

void engine_start(struct engine* engine) {
    float frames = 0;
    time_t frame_last_time = time(NULL);
    float fps = 0.0f;
    while (engine->game_loop) {
        time_t now = time(NULL);
        time_t diff = now - frame_last_time;
        // Calculate FPS every second
        if (diff >= 1.0f) {
            fps = frames / diff;
            frames = 0;
            frame_last_time = now;
            fprintf(stderr, "FPS: %.2f\n", fps);
            fprintf(stderr, "x: %d, y: %d\n", engine->curr_chunk[0], engine->curr_chunk[1]);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
        glUseProgram(engine->shader->program);
        // Update engine managed objects
        engine_fps(engine, fps);
        engine_update(engine);
        player_update(engine->player, engine->shader);
        for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
            for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
                struct chunk* chunk = {0};
                int chunk_coord[2] = { engine->curr_chunk[0] + i, engine->curr_chunk[1] + j  };
                world_get_chunk(engine->world, chunk_coord, &chunk);
                chunk_draw(chunk, engine->shader, engine->texture);
            }
        }
        SDL_RenderPresent(engine->window->renderer);
        frames += 1;
    }
}
