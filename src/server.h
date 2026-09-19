/*
 * A server runs a central "engine" - but without the rendering aspect. Clients connect to a server,
 * get a copy of the engine state, which is then kept in sync with the server. Player actions act first on the local state,
 * and are simultaneously sent to the server
 *
 */

#include "engine.h"
#include <bits/pthreadtypes.h>
#include <junk/queue.h>
#include <stdint.h>
#include <sys/poll.h>
#include "poll.h"
#include "cglm/cglm.h"
#include "pthread.h"

#define NUM_CLIENTS 4
#define MAX_QUEUE_EVENTS 30

struct client {
    int active;
    int64_t uuid;
    int client_fd;
    struct player_data player;
    pthread_mutex_t pkt_lock;
    pthread_t sync_thread;
    struct junk_queue input_queue;
    int chunk_mask[WORLD_LENGTH][WORLD_WIDTH];
};
struct server {
    struct world* world;
    struct client clients[NUM_CLIENTS];
    int server_fd;
    int input_fd;
    int connected_clients;
};

/*
 * Initialize a server struct, creating whatever resources are necessary
 * @return 0 on success, -1 on error
 */
int server_init(struct server* server, char* ip, char* port);
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
void* server_client_chunk_gen(void* buf);
/*
 * Ported from engine_update. Really simple logic - for a client, check the chunks
 * that need to be generated. If anything needs to be generated, submit for gen - only
 * calculated on chunk change, same as old engine_update
 * @param server target server
 * @param client target client
 * @return 1 if there is a chunk update, 0 if not
 */
int server_client_chunk_generate(struct server* server, struct client* client);


/*
 * The main server client loop.
 * Separate thread that reads UDP ESP inputs, processes them, and sends the correct
 * output to the player via ESP. Maybe it should use ESP for sending as well, but I think
 * stuff like physics and inventory seem pretty important. Maybe physics can be split
 * The general flow of this function is:
 * - Poll for any events, if there are queue them up
 *   Every tick:
 *   - Process input queue, apply actions
 *   - Send player data to clients
 *   - Look at any dirty chunks, if there are, sync them with client
 *   using server_client_chunk_sync
 *
 */
void* server_client_loop(void* buf);

/*
 * All this does is look around the player, and send a chunk that needs to be
 * sent based on the chunk_mask of the client. It runs in a tight loop and will
 * only send chunks that have to be sent
 * It also looks at things like chunk dirty-ness and whether the structures have
 * been generated yet or not, this is all used to limit how many updates we send
 *
 */
int server_client_chunk_sync(struct server* server, struct client* client);

void client_disconnect(struct server* server, struct client* client);
