#include "engine.h"
#include "input.h"
#include <assert.h>
#include <time.h>


int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: junkcraft [IP] [PORT]\n");
        return -1;
    }
    struct engine engine = { 0 };
    if (engine_init(&engine, argv[1], argv[2]) != 0) {
        return -1;
    }
    input_init(&engine);
    engine_start(&engine);
    window_cleanup(engine.window);
    return 0;
}
