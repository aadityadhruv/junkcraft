#include "server.h"

int main(int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: junkcraft [IP] [PORT]\n");
        return -1;
    }
    struct server server;
    memset(&server, 0, sizeof(struct server));
    server_init(&server, argv[1], argv[2]);
    server_start(&server);
}
