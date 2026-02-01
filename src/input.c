#include "input.h"
#include "camera.h"
#include "cglm/types.h"
#include "pthread.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_video.h>

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
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_SetWindowMouseGrab(engine->window->window, SDL_TRUE);
    while (engine->game_loop) {
        // Quit game
        // TODO: Locks?
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            engine->game_loop = 0;
        }
        if (event.type == SDL_KEYDOWN) {
            SDL_KeyboardEvent key = event.key;
            if (key.keysym.sym == SDLK_w) {
                camera_move(engine->camera, FORWARD);
            }
            if (key.keysym.sym == SDLK_a) {
                camera_move(engine->camera, LEFT);
            }
            if (key.keysym.sym == SDLK_s) {
                camera_move(engine->camera, BACKWARD);
            }
            if (key.keysym.sym == SDLK_d) {
                camera_move(engine->camera, RIGHT);
            }
            if (key.keysym.sym == SDLK_ESCAPE) {
                engine->game_loop = 0;
            }
        }
        if (event.type == SDL_MOUSEMOTION) {
            int x;
            int y;
            SDL_GetRelativeMouseState(&x, &y);
            if (x != 0 || y != 0) {
            vec2 offset = { x, y };
            camera_rotate(engine->camera, offset);
            }
        }
    }
}
