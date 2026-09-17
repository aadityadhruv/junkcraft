#include "input.h"
#include "protocol.h"
#include "camera.h"
#include "cglm/types.h"
#include "player.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_video.h>

pthread_t input_init(struct engine* engine) {
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_SetWindowMouseGrab(engine->window->window, SDL_TRUE);
    // pthread_create(&thread, NULL, (void*)input_handle, engine);
    // return thread;
    return 0;
}
void input_join(pthread_t thread, struct engine* engine) {
    // pthread_join(thread, NULL);
}

void input_send_mask(struct engine* engine, double dt) {
    const Uint8* numkeys = engine->numkeys;
        // Quit game
        // TODO: Locks?
        SDL_Event event;
        SDL_PollEvent(&event);
        int32_t mask = 0;
        // SDL_PumpEvents();
        if (event.type == SDL_QUIT) {
            engine->game_loop = 0;
       }
            if (numkeys[SDL_SCANCODE_W]) {
                mask += ESP_FORWARD;
            }
            if (numkeys[SDL_SCANCODE_A]) {
                mask += ESP_LEFT;
            }
            if (numkeys[SDL_SCANCODE_S]) {
                mask += ESP_BACK;
            }
            if (numkeys[SDL_SCANCODE_D]) {
                mask += ESP_RIGHT;
            }
            if (numkeys[SDL_SCANCODE_SPACE]) {
                mask += ESP_JUMP;
            }
            if (numkeys[SDL_SCANCODE_ESCAPE]) {
                engine->game_loop = 0;
            }
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            SDL_MouseButtonEvent* b = (SDL_MouseButtonEvent*) &event;
            if (b->button == SDL_BUTTON_LEFT) {
                mask += ESP_PLACE_HIT;
            }
            if (b->button == SDL_BUTTON_RIGHT) {
                mask += ESP_PLAYER_PLACE;
            }
        }
        if (mask != 0) {
        struct ESP esp = {
            .client_uuid = 10,
            .mask = mask,
            .dt = dt,
        };
            esp_send(&esp, engine->server_input_socket);
        }
}
void input_server_process(struct player_data* player, struct world* world, struct ESP* esp) {
    double dt = esp->dt;
    int32_t mask = esp->mask;
    if (mask & ESP_FORWARD) {
        player_move(player, FORWARD, dt);
    }
    if (mask & ESP_LEFT) {
        player_move(player, LEFT, dt);
    }
    if (mask & ESP_BACK) {
        player_move(player, BACKWARD, dt);
    }
    if (mask & ESP_RIGHT) {
        player_move(player, RIGHT, dt);
    }
    if (mask & ESP_JUMP) {
        player_move(player, JUMP, dt);
    }
    if (mask & ESP_PLACE_HIT) {
        player_block_delete(player, world);
    }
    if (mask & ESP_PLAYER_PLACE) {
        // player_use(player, world);
    }
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
                player_move(&engine->player.data, FORWARD, dt);
            }
            if (numkeys[SDL_SCANCODE_A]) {
                player_move(&engine->player.data, LEFT, dt);
            }
            if (numkeys[SDL_SCANCODE_S]) {
                player_move(&engine->player.data, BACKWARD, dt);
            }
            if (numkeys[SDL_SCANCODE_D]) {
                player_move(&engine->player.data, RIGHT, dt);
            }
            if (numkeys[SDL_SCANCODE_SPACE]) {
                player_move(&engine->player.data, JUMP, dt);
            }
            if (numkeys[SDL_SCANCODE_ESCAPE]) {
                engine->game_loop = 0;
            }
        if (event.type == SDL_KEYDOWN) {
        }
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            SDL_MouseButtonEvent* b = (SDL_MouseButtonEvent*) &event;
            if (b->button == SDL_BUTTON_LEFT) {
                player_block_delete(&engine->player.data, engine->world);
            }
            if (b->button == SDL_BUTTON_RIGHT) {
                player_use(&engine->player, engine);
            }
        }
        if (event.type == SDL_MOUSEWHEEL) {
            SDL_MouseWheelEvent* b = (SDL_MouseWheelEvent*) &event;
            player_move_hotbar(&engine->player, b->y);
        }
        if (event.type == SDL_MOUSEMOTION) {
            int x;
            int y;
            SDL_GetRelativeMouseState(&x, &y);
            if (x != 0 || y != 0) {
            vec2 offset = { x, y };
            player_rotate(&engine->player, offset);
            }
        }
}
