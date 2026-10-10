#include "engine.h"
#include "input.h"
#include <assert.h>
#include <time.h>
#include "server.h"


int main(int argc, char** argv) {
    // Start local server
    if (argc == 1) {
        struct server server;
        char* local_ip = "127.0.0.1";
        char* local_port = "8000";
        memset(&server, 0, sizeof(struct server));
        int ret = server_init(&server, local_ip, local_port);
        if (ret != 0) {
            fprintf(stderr, "failed to start server!");
            return -1;
        }
        pthread_t server_thread;
        pthread_create(&server_thread, NULL, (void*)server_start, &server);
        struct engine engine = { 0 };
        if (engine_init(&engine, local_ip, local_port) != 0) {
            return -1;
        }
        input_init(&engine);
        engine_start(&engine);
        window_cleanup(engine.window);
        return 0;
    }

    else if (argc != 3) {
        fprintf(stderr, "usage: junkcraft [ip] [port]\n");
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
