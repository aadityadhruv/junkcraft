#pragma once
#include "window.h"
#include "shader.h"
#include "junk/vector.h"
// CHUNK_DISTANCE is essentially render distance, it shows you how many chunks
// around the user you can see
// The number of loaded chunks can be determined as follows:
// We want a square around curr_chunk, and a side of the square will be 1
// (center chunk) + 2 * CHUNK_DISTANCE (either side of center)
// loaded chunks = (1 + CHUNK_DISTANCE * 2)^2
#define CHUNK_DISTANCE 5

struct engine {
    struct window* window;
    struct shader* shader;
    struct camera* camera;
    int game_loop;
    int curr_chunk[2];
    struct world* world;
};

/**
 * Initalize the Engine, a struct that oversees the rest of the components in the game
 * @param @out engine The engine struct to store data in
 * @return 0 on success
 */
int engine_init(struct engine* engine);


/**
 * Take all objects in the engine, apply the shader pipeline and draw on the window
 * Apply block, chunk and camera updates as well. This is the main game loop
 *
 * @param engine The target engine
 */
void engine_start(struct engine* engine);
