#include "input.h"
#include "pthread.h"

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
    }
}
