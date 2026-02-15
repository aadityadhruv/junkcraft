#include "engine.h"
#include "block.h"
#include "cglm/io.h"
#include "chunk.h"
#include "input.h"
#include "player.h"
#include "shader.h"
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
    vector_init(&(engine->shaders));
    struct shader* shader;
    struct shader* debug_shader;
    struct shader* ui_shader;
    shader_init(&shader);
    shader_init(&debug_shader);
    shader_init(&ui_shader);
    char* shaders[2] = { "shaders/vertex.glsl", "shaders/fragment.glsl" };
    char* debug_shaders[2] = { "shaders/vertex_debug.glsl", "shaders/fragment_debug.glsl" };
    char* ui_shaders[2] = { "shaders/vertex_ui.glsl", "shaders/fragment_debug.glsl" };
    shader_add(shader, shaders[0], shaders[1]);
    shader_add(debug_shader, debug_shaders[0], debug_shaders[1]);
    shader_add(ui_shader, ui_shaders[0], ui_shaders[1]);
    VECTOR_INSERT(engine->shaders, shader);
    VECTOR_INSERT(engine->shaders, debug_shader);
    VECTOR_INSERT(engine->shaders, ui_shader);


    // Load Textures
    struct texture* texture = { 0 };
    texture_init(&engine->texture);
    char* textures[] = {
        "textures/01_grass.png",
        "textures/02_stone.png",
        "textures/03_rock.png",
        "textures/04_sand.png",
        "textures/05_snow.png",
    };
    texture_load(engine->texture, textures, sizeof(textures)/sizeof(char*));
    block_metadata_init();

    // Setup Objects to draw
    // memset(engine->loaded_chunks, 0, (1 + CHUNK_DISTANCE * 2) * (1 + CHUNK_DISTANCE * 2));

    // Setup player
    vec3 pos = { 1.0f, 100.0f, -1.0f };
    player_init(pos, &engine->player);

    // Setup root chunk
    struct world* world;
    world_init(1, &world);
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
    const Uint8* numkeys = SDL_GetKeyboardState(NULL);
    engine->numkeys = numkeys;
    return 0;
}

void engine_update(struct engine* engine) {
    // NOTE: OpenGL FLIP
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
            }
        }
        // Update the curr_chunk
        memcpy(engine->curr_chunk, curr_chunk, sizeof(vec2));
    }
    // Reload a chunk if it is dirty
    for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
        for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
            struct chunk* chunk = {0};
            int chunk_coord[2] = { engine->curr_chunk[0] + i, engine->curr_chunk[1] + j  };
            world_get_chunk(engine->world, chunk_coord, &chunk);
            if (!chunk->loaded) {
                chunk_load(engine->world, chunk, chunk_coord);
            }
        }
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
    struct timespec last_update;
    clock_gettime(CLOCK_MONOTONIC, &last_update);
    while (engine->game_loop) {
        time_t now = time(NULL);
        time_t diff = now - frame_last_time;
        struct timespec curr;
        clock_gettime(CLOCK_MONOTONIC, &curr);
        double dt = (curr.tv_sec - last_update.tv_sec) + ((curr.tv_nsec - last_update.tv_nsec) / ((double) 1000000000));
        clock_gettime(CLOCK_MONOTONIC, &last_update);
        // Calculate FPS every second
        if (diff >= 1.0f) {
            fps = frames / diff;
            frames = 0;
            frame_last_time = now;
            fprintf(stderr, "FPS: %.2f\n", fps);
            fprintf(stderr, "x: %d, y: %d\n", engine->curr_chunk[0], engine->curr_chunk[1]);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_STENCIL_TEST);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
        struct shader* default_shader = vector_get(engine->shaders, 0);
        struct shader* debug_shader = vector_get(engine->shaders, 1);
        struct shader* ui_shader = vector_get(engine->shaders, 2);
        // =============== INPUT AND PHYSICS ===============
        // Update engine managed objects
        input_process(engine, dt);
        engine_fps(engine, fps);
        engine_update(engine);
        player_physics(engine->player, engine, dt);

        // =============== DRAW ======================
        // Draw player related data using debug_shader
        shader_use(debug_shader);
        // Set perspective and view matrix
        player_update(engine->player, debug_shader);
        player_draw(engine->player, engine->world, debug_shader);
        // Switch to regular shader
        shader_use(default_shader);
        player_update(engine->player, default_shader);
        for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
            for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
                struct chunk* chunk = {0};
                int chunk_coord[2] = { engine->curr_chunk[0] + i, engine->curr_chunk[1] + j  };
                world_get_chunk(engine->world, chunk_coord, &chunk);
                chunk_draw(chunk, default_shader, engine->texture);
            }
        }
        shader_use(ui_shader);
        player_draw_ui(engine->player, ui_shader);
        SDL_RenderPresent(engine->window->renderer);
        frames += 1;
    }
}
