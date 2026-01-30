#include "engine.h"
#include "input.h"
#include <time.h>


int main() {
    struct engine engine = { 0 };
    if (engine_init(&engine) != 0) {
        return -1;
    }
    pthread_t input_thread = input_init(&engine);
    engine_start(&engine);
    input_join(input_thread, &engine);
    window_cleanup(engine.window);
    return 0;
}
