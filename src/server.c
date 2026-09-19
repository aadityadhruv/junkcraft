#include "server.h"
#include "block.h"
#include "engine.h"
#include "input.h"
#include "junk/network.h"
#include "player.h"
#include "sys/socket.h"
#include <junk/queue.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <unistd.h>
#include "protocol.h"
#include "world.h"



#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

pthread_t input_thread;


struct thread_data {
    struct server* server;
    struct client* client;
};

pthread_mutex_t lock;

void cleanup(int sig) {
    exit(0);
}

int server_stop(struct server *server) {
    for (size_t i = 0; i < ARRAY_SIZE(server->clients); i++) {
        if (i != 0) {
            shutdown(server->clients[i].client_fd, SHUT_RDWR);
            close(server->clients[i].client_fd);
            server->clients[i].uuid = -1;
        }
    }
    return 0;
}


int server_init(struct server *server) {
    block_metadata_init();
    item_metadata_init();
    world_init(0, &server->world);
    for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
        for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
            // Pass 1 - generate terrain
            int chunk_coord[2] = { i,  j };
            world_submit_chunk_terrain_gen(server->world, chunk_coord);
            // Pass 2 - generate structures
            world_submit_chunk_structure_gen(server->world, chunk_coord);
        }
    }
    signal(SIGINT, cleanup);
    signal(SIGPIPE, SIG_IGN);
    for (size_t i = 0; i < ARRAY_SIZE(server->clients); i++) {
        server->clients[i].uuid = -1;
    }
    int input_sock = junk_udp_ipv4_bind("127.0.0.1", "8000");
    if (input_sock == -1) {
        fprintf(stderr, "Couldn't create input socket\n");
        return -1;
    }
    server->input_fd = input_sock;
    pthread_create(&input_thread, 0,server_client_loop, server);
    char* ip = "127.0.0.1";
    char* port = "8000";
    int sockfd = junk_tcp_ipv4_bind(ip, port);
    int option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    server->server_fd = sockfd;
    if (sockfd == -1) {
        fprintf(stderr, "Could not start server at %s:%s\n", ip, port);
        return -1;
    }
    if (listen(sockfd, 8) == -1) {
        fprintf(stderr, "Error listening at %s:%s", ip, port);
        return -1;
    }
    fprintf(stderr, "Listening at %s:%s", ip, port);
    return 0;
}

int server_start(struct server *server) {
    while (1) {
        if (server->connected_clients >= NUM_CLIENTS) {
            continue;
        }
        struct sockaddr_in addrinfo;
        int client_sock = 0;
        socklen_t len = sizeof(addrinfo);
        if ((client_sock = accept(server->server_fd, (struct sockaddr*)(&addrinfo), &len)) == -1) {
            // fprintf(stderr, "Could not accept connection.\n");
            continue;
        }
        for (size_t i = 0; i < NUM_CLIENTS; i++) {
            // Found a free slot
            if (server->clients[i].uuid == -1) {
                char ip[32];
                inet_ntop(AF_INET, (void*) &addrinfo.sin_addr, ip, 32);
                fprintf(stderr, "Accepted conn from %s\n", ip);

                server->clients[i].client_fd = client_sock;
                junk_queue_init(&server->clients[i].input_queue);
                struct SSP init_pkt = { 
                    .client_uuid = 10 + i,
                    .id = SSP_INIT,
                    .data_size = 0,
                };
                ssp_send(&init_pkt, client_sock);
                vec3 pos = { 1.0f, 200.0f, -1.0f };
                pthread_mutex_init(&server->clients[i].pkt_lock, 0);
                player_data_init(pos, &server->clients[i].player);
                server->clients[i].uuid = init_pkt.client_uuid;
                server->clients[i].active = 1;
                struct thread_data* data = malloc(sizeof(struct thread_data));
                data->server = server;
                data->client = &server->clients[i];
                server_client_chunk_sync(server, &server->clients[i]);
                pthread_create(&server->clients[i].sync_thread, 0,server_client_chunk_gen, data);
                server->connected_clients++;
                break;
            }
        }
        fprintf(stderr, "added new, accepting again\n");
    }
    return 0;
}

int server_client_chunk_sync(struct server* server, struct client* client) {
    for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
        for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
            int chunk_coord[2] = { i + client->player.chunk_coords[0],  j + client->player.chunk_coords[1] };
            struct chunk* chunk = NULL;
            world_get_chunk_no_gen(server->world, chunk_coord, &chunk);
            if (chunk == NULL) continue;
            // If the chunk has already been sent, the structures have already generated,
            // and if it is not dirty, skip sending the chunk
            if (
                    client->chunk_mask[(int)chunk->data.coord[0]][(int)chunk->data.coord[1]]
                    && !chunk->data.dirty
                    && chunk->data.generated_structures
                ) {
                continue;
            }
            // If the chunk has already been sent, but the chunk hasn't generated all structures
            // yet, don't send it. This is to prevent spam every time structure gen marks the 
            // chunk as dirty TODO revise this soon?
            if (
                    client->chunk_mask[(int)chunk->data.coord[0]][(int)chunk->data.coord[1]]
                    && !chunk->data.generated_structures
                ) {
                continue;
            }

            // fprintf(stderr, "sending chunk: ");
            // glm_vec2_print(chunk->data.coord, stderr);
            // fprintf(stderr, "Dirty: %d | Structures %d | Mask %d\n", chunk->data.dirty, chunk->data.generated_structures, client->chunk_mask[(int)chunk->data.coord[0]][(int)chunk->data.coord[1]]);
            client->chunk_mask[(int)chunk->data.coord[0]][(int)chunk->data.coord[1]] = 1;
            struct SSP send = {
                .client_uuid = client->uuid,
                .data_size = sizeof(struct chunk_data),
                .id = SSP_CHUNK_SYNC,
            };
            pthread_mutex_lock(&client->pkt_lock);
            int ret = ssp_send(&send, client->client_fd);
            if (ret != 0) {
                fprintf(stderr, "client disconnect %ld\n", client->uuid);
                client_disconnect(server, client);
                return ret;
            }
            ret = chunk_data_send(&chunk->data, client->client_fd);
            if (ret != 0) {
                fprintf(stderr, "client disconnect %ld\n", client->uuid);
                client_disconnect(server, client);
                return ret;
            }
            // Set chunk_mask as 1 to signify that the chunk has been sent
            pthread_mutex_unlock(&client->pkt_lock);
            if (ret != 0) return ret;
            // glm_vec2_print(chunk->data.coord, stderr);
            // fprintf(stderr, "sent data for chunk %d %d\n", chunk_coord[0], chunk_coord[1]);
        }
    }
    return 0;
};
/*
 * Generate new chunks around player if need be. Should be done better TODO
 *
 */
int server_client_chunk_generate(struct server* server, struct client* client) {
    // NOTE: OpenGL FLIP
    int curr_chunk[2] = { (int)floorf(client->player.position[0] / (float)CHUNK_WIDTH), (int)floorf(-client->player.position[2] / (float)CHUNK_LENGTH) };
    // Chunk update
    // We moved a chunk - gen new chunks if needed
    if (client->player.chunk_coords[0] != curr_chunk[0] || client->player.chunk_coords[1] != curr_chunk[1]) {
        memcpy(client->player.chunk_coords, curr_chunk, sizeof(curr_chunk));
        for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
            for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
                int chunk_coord[2] = { curr_chunk[0] + i, curr_chunk[1] + j };
                struct chunk* chunk;
                world_get_chunk_no_gen(server->world, chunk_coord, &chunk);
                if (chunk == NULL) {
                    world_submit_chunk_terrain_gen(server->world, chunk_coord);
                    // Pass 2 - generate structures
                    world_submit_chunk_structure_gen(server->world, chunk_coord);
                }
            }
        }
        return 1;
    }
    return 0;
}

void* server_client_loop(void* buf) {
    struct server* server = (struct server*) buf;
    struct pollfd in_pfd = {
        .fd = server->input_fd,
        .events = POLLIN,
    };
    fprintf(stderr, "Started server client input\n");
    // float frames = 0;
    // time_t frame_last_time = time(NULL);
    // float fps = 0.0;
    float ticks_per_second = 60;
    struct timespec last_update;
    clock_gettime(CLOCK_MONOTONIC, &last_update);
    while (1) {
        // time_t now = time(NULL);
        // time_t diff = now - frame_last_time;
        struct timespec curr;
        clock_gettime(CLOCK_MONOTONIC, &curr);
        double dt = (double)(curr.tv_sec - last_update.tv_sec) + ((double)(curr.tv_nsec - last_update.tv_nsec) / ((double) 1000000000));
        // 1 game tick has passed, update physics/input
        if (poll(&in_pfd, 1, 0) > 0) {
            struct ESP* recv = malloc(sizeof(struct ESP));
            memset(recv, 0, sizeof(struct ESP));
            int ret = esp_recv(recv, server->input_fd);
            if (ret != 0) {
                fprintf(stderr, "Couldn't recv input\n");
            }
           for (size_t i = 0; i < NUM_CLIENTS; i++) {
                if (recv->client_uuid == server->clients[i].uuid) {
                    if (junk_queue_length(&server->clients[i].input_queue) == MAX_QUEUE_EVENTS) {
                        void* item = junk_queue_pop(&server->clients[i].input_queue);
                        free(item);
                    }
                    junk_queue_push(&server->clients[i].input_queue, recv);
                }
            }
        }
        if (dt > (1 / ticks_per_second)) {
            // dt = (1 / ticks_per_second);
            clock_gettime(CLOCK_MONOTONIC, &last_update);
            // fprintf(stderr, "Server tick\n");
            for (size_t i = 0; i < NUM_CLIENTS; i++) {
                struct client* client = &server->clients[i];
                if (client->uuid != -1) {
                    struct SSP send = {
                        .client_uuid = client->uuid,
                        .data_size = 0,
                        .id = SSP_PLAYER_DATA,
                    };
                    struct pollfd out_pfd = {
                        .fd = client->client_fd,
                        .events = POLLOUT,
                    };
                    while (junk_queue_length(&client->input_queue) > 0) {
                        struct ESP* esp = junk_queue_pop(&client->input_queue);
                        input_server_process(&client->player, server->world, esp, dt);
                        free(esp);
                    }
                    player_physics(&client->player, server->world, dt);
                    pthread_mutex_lock(&client->pkt_lock);
                    if (poll(&out_pfd, 1, 0) > 0) {
                        int ret = ssp_send(&send, client->client_fd);
                        if (ret != 0) {
                            fprintf(stderr, "client disconnect %ld\n", client->uuid);
                            client_disconnect(server, client);
                            break;
                        }
                        ret = player_data_send(&client->player, client->client_fd);
                        if (ret != 0) {
                            fprintf(stderr, "client disconnect %ld\n", client->uuid);
                            client_disconnect(server, client);
                            break;
                        }
                    }
                    pthread_mutex_unlock(&client->pkt_lock);
                }
            }
            for (size_t i = 0; i < NUM_CLIENTS; i++) {
                struct client* client = &server->clients[i];
                if (client->uuid != -1) {
                    server_client_chunk_sync(server, &server->clients[i]);
                }
            }
            // Reset chunk dirty-ness. If it has to be dirty, it will
            // be recreated in the next tick
            for (int i = 0; i < WORLD_WIDTH; i++) {
                for (int j = 0; j < WORLD_LENGTH; j++) {
                    struct chunk* c = server->world->chunks[i][j];
                    if (c == NULL) continue;
                    c->data.dirty = 0;
                }
            }
        }
    }
}
void* server_client_chunk_gen(void* buf) {
    struct thread_data* data = (struct thread_data*) buf;
    struct server* server = data->server;
    struct client* client = data->client;
    if (client->uuid != -1) {
        while (client->active) {
            // Active client, poll for data
            server_client_chunk_generate(server, client);
        }
    }
    free(data);
    return NULL;
}
void client_disconnect(struct server* server, struct client* client) {
    close(client->client_fd);
    while (junk_queue_length(&client->input_queue) > 0) {
        free(junk_queue_pop(&client->input_queue));
    }
    pthread_mutex_unlock(&client->pkt_lock);
    pthread_mutex_destroy(&client->pkt_lock);
    client->active = 0;
    pthread_join(client->sync_thread, NULL);
    memset(&client->player, 0, sizeof(struct player_data));
    memset(&client->chunk_mask, 0, sizeof(client->chunk_mask));
    for (int i = 0; i < NUM_CLIENTS; i++) {
        if (server->clients[i].uuid == client->uuid) {
            server->clients[i].uuid = -1;
            server->connected_clients--;
            return;
        }
    }

}

int main() {
    struct server server;
    memset(&server, 0, sizeof(struct server));
    server_init(&server);
    server_start(&server);
}
