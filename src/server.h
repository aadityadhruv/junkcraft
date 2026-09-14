/*
 * A server runs a central "engine" - but without the rendering aspect. Clients connect to a server,
 * get a copy of the engine state, which is then kept in sync with the server. Player actions act first on the local state,
 * and are simultaneously sent to the server
 *
 */

#include "engine.h"
#include <bits/pthreadtypes.h>
#include <stdint.h>
#include <sys/poll.h>
#include "poll.h"

#define POLL_THREADS 1

struct client {
    int64_t uuid;
    int client_fd;
    struct pollfd poll_fd;
    struct player player;
    int locked;
};
struct server {
    struct world* world;
    struct client clients[8];
    int server_fd;
    pthread_t poll_threads[POLL_THREADS];
};

/*
 * Initialize a server struct, creating whatever resources are necessary
 * @return 0 on success, -1 on error
 */
int server_init(struct server* server);
/*
 * Called after server_init, this starts listening based on the configured socket
 * The server loop involves accepting/disconnecting clients and syncing data with them
 * @return 0 on success, -1 on error
 */
int server_start(struct server* server);

/*
 * Stop the server, disconnecting all clients, and freeing any other resources
 * @return 0 on success, -1 on error
 */
int server_stop(struct server* server);
/*
 * Fast, lightweight sync for the engine state
 * @return 0 on success, -1 on error
 */
int server_client_sync(struct server* server);
void* server_client_ssp(void* buf);
