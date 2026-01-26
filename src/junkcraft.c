#include "engine.h"

int main() {
    struct engine engine = { 0 };
    if (engine_init(&engine) != 0) {
        return -1;
    }
    engine_draw(&engine);
    window_cleanup(engine.window);
    return 0;
}
