#pragma once
#include "SDL2/SDL.h"
#include "engine.h"

void input_handle(struct engine* engine);
pthread_t input_init(struct engine* engine);
void input_join(pthread_t thread, struct engine* engine);
void input_process(struct engine* engine, double dt);
