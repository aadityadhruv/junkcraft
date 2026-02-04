#include "input.h"
#include "camera.h"
#include "cglm/types.h"
#include "pthread.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_video.h>

pthread_t input_init(struct engine* engine) {
    pthread_t thread;
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_SetWindowMouseGrab(engine->window->window, SDL_TRUE);
    // pthread_create(&thread, NULL, (void*)input_handle, engine);
    // return thread;
    return 0;
}
void input_join(pthread_t thread, struct engine* engine) {
    // pthread_join(thread, NULL);
}

void input_process(struct engine* engine, double dt) {
    const Uint8* numkeys = engine->numkeys;
        // Quit game
        // TODO: Locks?
        SDL_Event event;
        SDL_PollEvent(&event);
        // SDL_PumpEvents();
        if (event.type == SDL_QUIT) {
            engine->game_loop = 0;
       }
            if (numkeys[SDL_SCANCODE_W]) {
                player_move(engine->player, engine, FORWARD, dt);
            }
            if (numkeys[SDL_SCANCODE_A]) {
                player_move(engine->player, engine, LEFT, dt);
            }
            if (numkeys[SDL_SCANCODE_S]) {
                player_move(engine->player, engine, BACKWARD, dt);
            }
            if (numkeys[SDL_SCANCODE_D]) {
                player_move(engine->player, engine, RIGHT, dt);
            }
            if (numkeys[SDL_SCANCODE_SPACE]) {
                player_move(engine->player, engine, JUMP, dt);
            }
            if (numkeys[SDL_SCANCODE_ESCAPE]) {
                engine->game_loop = 0;
            }
        if (event.type == SDL_KEYDOWN) {
        }
        if (event.type == SDL_MOUSEMOTION) {
            int x;
            int y;
            SDL_GetRelativeMouseState(&x, &y);
            if (x != 0 || y != 0) {
            vec2 offset = { x, y };
            player_rotate(engine->player, offset);
            }
        }
}
void input_handle(struct engine *engine) {
    while (engine->game_loop) {
    }
}
