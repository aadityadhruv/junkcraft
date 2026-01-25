#include "SDL2/SDL.h"
#include "SDL2/SDL_video.h"
#include "SDL2/SDL_render.h"


struct window {
    SDL_Renderer* renderer;
    SDL_Window* window;
};

/**
 * Get a basic SDL window up and running. This also initializes the OpenGL context, sets up a renderer and stores all the relevant data in the window struct. Memory is managed externally. We usually don't even need to free that allocated memory because it lasts till program death. 
 * @param @out window destination window pointer
 * @return 0 on success, -1 on failure
 *
 */
int window_init(struct window* window);


/**
 * Cleanup the allocated Window and quit SDL. Basically called when the user closes the window
 * 
 * @param window Window to close
 *
 */
void window_cleanup(struct window* window);
