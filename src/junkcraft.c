#include "engine.h"
#include <time.h>

int main() {
    srand(time(NULL));
    struct engine engine = { 0 };
    if (engine_init(&engine) != 0) {
        return -1;
    }
    engine_draw(&engine);
    window_cleanup(engine.window);
    return 0;
}
