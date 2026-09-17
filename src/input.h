#pragma once
#include "SDL2/SDL.h"
#include "engine.h"
#include "protocol.h"

void input_handle(struct engine* engine);
pthread_t input_init(struct engine* engine);
void input_join(pthread_t thread, struct engine* engine);
void input_process(struct engine* engine, double dt);
void input_send_mask(struct engine* engine, double dt);
void input_server_process(struct player_data* player, struct world* world, struct ESP* esp);
