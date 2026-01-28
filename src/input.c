#include "input.h"
#include "camera.h"
#include "cglm/types.h"
#include "pthread.h"
#include <SDL2/SDL_events.h>

pthread_t input_init(struct engine* engine) {
    pthread_t thread;
    pthread_create(&thread, NULL, (void*)input_handle, engine);
    return thread;
}
void input_join(pthread_t thread, struct engine* engine) {
    pthread_join(thread, NULL);
}
void input_handle(struct engine *engine) {
    SDL_Event event;
    while (engine->game_loop) {
        // Quit game
        // TODO: Locks?
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            engine->game_loop = 0;
        }
        if (event.type == SDL_KEYDOWN) {
            SDL_KeyboardEvent key = event.key;
            vec3 ahead = { 0.0f, 0.0f, -1.0f };
            vec3 behind = { 0.0f, 0.0f, 1.0f };
            vec3 left = { -1.0f, 0.0f, 0.0f };
            vec3 right = { 1.0f, 0.0f, 0.0f };
            switch (key.keysym.sym) {
                case SDLK_w:
                    camera_move(engine->camera, ahead);
                    break;
                case SDLK_a:
                    camera_move(engine->camera, left);
                    break;
                case SDLK_s:
                    camera_move(engine->camera, behind);
                    break;
                case SDLK_d:
                    camera_move(engine->camera, right);
                    break;
                case SDLK_ESCAPE:
                    engine->game_loop = 0;
                    break;
            }
        }
    }
}
