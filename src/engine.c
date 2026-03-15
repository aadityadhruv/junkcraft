#include "engine.h"
#include "block.h"
#include "cglm/cglm.h"
#include "clock.h"
#include "config.h"
#include "chunk.h"
#include "input.h"
#include "player.h"
#include "shader.h"
#include "text.h"
#include "texture.h"
#include "window.h"
#include "world.h"
#include <SDL2/SDL_render.h>
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
    junk_vector_init(&(engine->shaders));
    struct shader* shader;
    struct shader* debug_shader;
    struct shader* ui_shader;
    struct shader* text_shader;
    struct shader* sky_shader;
    shader_init(&shader);
    shader_init(&debug_shader);
    shader_init(&ui_shader);
    shader_init(&text_shader);
    shader_init(&sky_shader);
    fprintf(stderr, "Loading shaders...\n");
    char* shaders[2] = { "shaders/vertex.glsl", "shaders/fragment.glsl" };
    char* debug_shaders[2] = { "shaders/vertex_debug.glsl", "shaders/fragment_debug.glsl" };
    char* ui_shaders[2] = { "shaders/vertex_ui.glsl", "shaders/fragment_debug.glsl" };
    char* text_shaders[2] = { "shaders/vertex_text.glsl", "shaders/fragment_text.glsl" };
    char* sky_shaders[2] = { "shaders/vertex_sky.glsl", "shaders/fragment_sky.glsl" };
    shader_add(shader, shaders[0], shaders[1]);
    shader_add(debug_shader, debug_shaders[0], debug_shaders[1]);
    shader_add(ui_shader, ui_shaders[0], ui_shaders[1]);
    shader_add(text_shader, text_shaders[0], text_shaders[1]);
    shader_add(sky_shader, sky_shaders[0], sky_shaders[1]);
    JUNK_VECTOR_INSERT(engine->shaders, shader);
    JUNK_VECTOR_INSERT(engine->shaders, debug_shader);
    JUNK_VECTOR_INSERT(engine->shaders, ui_shader);
    JUNK_VECTOR_INSERT(engine->shaders, text_shader);
    JUNK_VECTOR_INSERT(engine->shaders, sky_shader);
    fprintf(stderr, "Shaders compiled and loaded\n");


    // Load Textures
    texture_init(&engine->texture);
    texture_load(engine->texture);
    block_metadata_init();

    // Load text data
    if (text_init(&engine->text) != 0) {
        perror("Failed to load text subsystem!");
        return -1;
    }


    // Load clock
    if (clock_init(&engine->clk) != 0) {
        perror("Failed to load clock!");
        return -1;
    }


    // Setup player
    vec3 pos = { 1.0f, 200.0f, -1.0f };
    player_init(pos, &engine->player);

    // Setup root chunk
    struct world* world;
    world_init(1, &world);
    engine->world = world;
    //TODO: Move this loop to a function and flip chunk_coord sign correctly ONCE
    // Pass 1 - generate terrain
    for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
        for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
            struct chunk* chunk;
            int chunk_coord[2] = { engine->curr_chunk[0] + i, engine->curr_chunk[1] + j };
            world_get_chunk(engine->world, chunk_coord, &chunk);
        }
    }
    // Pass 2 - generate structures
    for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
        for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
            struct chunk* chunk;
            int chunk_coord[2] = { engine->curr_chunk[0] + i, engine->curr_chunk[1] + j };
            world_get_chunk(engine->world, chunk_coord, &chunk);
            chunk_structure_gen(engine->world, chunk);
        }
    }
    // Pass 3 - Load data to GPU
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
    int curr_chunk[2] = { (int)floorf(engine->player->position[0] / (float)CHUNK_WIDTH), (int)floorf(-engine->player->position[2] / (float)CHUNK_LENGTH) };
    // Chunk update
    // We moved a chunk - load new chunks with chunk_load
    if (engine->curr_chunk[0] != curr_chunk[0] || engine->curr_chunk[1] != curr_chunk[1]) {
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
        // Pass 2 - generate structures for newly generated chunks from above for loop
        for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
            for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
                struct chunk* chunk;
                int chunk_coord[2] = { curr_chunk[0] + i, curr_chunk[1] + j };
                world_get_chunk(engine->world, chunk_coord, &chunk);
                chunk_structure_gen(engine->world, chunk);
            }
        }
        // Unload existing chunks
        for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
            for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
                struct chunk* chunk;
                int chunk_coord[2] = { engine->curr_chunk[0] + i, engine->curr_chunk[1] + j };
                world_get_chunk(engine->world, chunk_coord, &chunk);
                // unload chunk
                // Note: Chunk could possibly be already unloaded if 
                // it's not in view frustum so check for it
                if (chunk->staged_for_load == 0 && chunk->loaded != 0) {
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
            if (chunk->dirty) {
                chunk_unload(chunk);
                chunk_load(engine->world, chunk, chunk_coord);
            }
        }
    }

}

void engine_debug(struct engine* engine, struct shader* text_shader, float fps) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        shader_use(text_shader);
        char frames[40];
        memset(frames, 0, 20);
        int l  = snprintf(frames, 40, "FPS: %d", (int)fps);
        text_draw(engine->text, text_shader, frames, 0.0f, SCREEN_HEIGHT * 9.0f/10.0f, 1.0f, l);
        l  = snprintf(frames, 40, "Chunk:[%d, %d]",engine->curr_chunk[0], engine->curr_chunk[1]);
        text_draw(engine->text, text_shader, frames, 0.0f, SCREEN_HEIGHT * 8.0f/10.0f, 1.0f, l);
        l  = snprintf(frames, 40, "Position :[%.2f, %.2f, %.2f]",
                (double)engine->player->position[0],
                (double)engine->player->position[1],
                (double)engine->player->position[2]);
        text_draw(engine->text, text_shader, frames, 0.0f, SCREEN_HEIGHT * 7.0f/10.0f, 1.0f, l);
        glDisable(GL_BLEND);
}

void engine_start(struct engine* engine) {
    float frames = 0;
    time_t frame_last_time = time(NULL);
    float fps = 0.0;
    struct timespec last_update;
    clock_gettime(CLOCK_MONOTONIC, &last_update);
    while (engine->game_loop) {
        time_t now = time(NULL);
        time_t diff = now - frame_last_time;
        struct timespec curr;
        clock_gettime(CLOCK_MONOTONIC, &curr);
        double dt = (double)(curr.tv_sec - last_update.tv_sec) + ((double)(curr.tv_nsec - last_update.tv_nsec) / ((double) 1000000000));
        clock_gettime(CLOCK_MONOTONIC, &last_update);
        // Calculate FPS every second
        if (diff >= 1.0l) {
            fps = frames / (float)diff;
            frames = 0;
            frame_last_time = now;
        }

        clock_tick(engine->clk, dt);
        vec3 sky_color;
        clock_get_sky_color(engine->clk, sky_color);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glClearColor(sky_color[0], sky_color[1], sky_color[2], 1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_STENCIL_TEST);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        //glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
        struct shader* default_shader = junk_vector_get(engine->shaders, 0);
        struct shader* debug_shader = junk_vector_get(engine->shaders, 1);
        struct shader* ui_shader = junk_vector_get(engine->shaders, 2);
        struct shader* text_shader = junk_vector_get(engine->shaders, 3);
        struct shader* sky_shader = junk_vector_get(engine->shaders, 4);
        // =============== INPUT AND PHYSICS ===============
        // Update engine managed objects
        input_process(engine, dt);
        engine_update(engine);
        player_physics(engine->player, engine, dt);

        // =============== DRAW ======================
        // Draw sky objects
        glDisable(GL_DEPTH_TEST);
        shader_use(sky_shader);
        player_update(engine->player, sky_shader);
        clock_draw(engine->clk, engine->player, sky_shader);
        glEnable(GL_DEPTH_TEST);
        // Draw player related data using debug_shader
        shader_use(debug_shader);
        // Set perspective and view matrix
        player_update(engine->player, debug_shader);
        player_draw(engine->player, engine->world, debug_shader);
        // Switch to regular shader
        shader_use(default_shader);
        float light_intensity = clock_get_light_intensity(engine->clk);
        vec3 light_color = { light_intensity, light_intensity, light_intensity };
        set_uniform_vec3("light_color", default_shader, light_color);
        player_update(engine->player, default_shader);
        for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
            for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
                struct chunk* chunk = {0};
                int chunk_coord[2] = { engine->curr_chunk[0] + i, engine->curr_chunk[1] + j };
                world_get_chunk(engine->world, chunk_coord, &chunk);
                vec2 frustum_check_chunk_coord = { (float)chunk_coord[0], (float)chunk_coord[1] };
                if (player_is_point_in_frustum(engine->player, frustum_check_chunk_coord)) {
                    if (chunk->loaded == 0) chunk_load(engine->world, chunk, chunk_coord);
                    chunk_draw(chunk, default_shader, engine->texture);
                } else {
                    if (chunk->loaded) {
                        chunk_unload(chunk);
                    }
                }
            }
        }
        // Second pass of "no depth" draws
        // UI and text need to be in front
        glDisable(GL_DEPTH_TEST);
        engine_debug(engine, text_shader, fps);
        shader_use(ui_shader);
        player_draw_ui(engine->player, ui_shader);
        glEnable(GL_DEPTH_TEST);

        SDL_GL_SwapWindow(engine->window->window);
        frames += 1;
    }
}
