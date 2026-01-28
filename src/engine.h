#pragma once
#include "window.h"
#include "shader.h"
#include "junk/vector.h"
struct engine {
    struct window* window;
    struct shader* shader;
    struct vector* objects;
    struct camera* camera;
    int game_loop;
};

/**
 * Initalize the Engine, a struct that oversees the rest of the components in the game
 * @param @out engine The engine struct to store data in
 * @return 0 on success
 */
int engine_init(struct engine* engine);


/**
 * Take all objects in the engine, apply the shader pipeline and draw on the window
 * Event handling is also processed here, though should maybe moved to a separate thread
 *
 * @param engine The target engine
 */
void engine_draw(struct engine* engine);
