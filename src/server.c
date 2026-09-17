#include "server.h"
#include "cglm/io.h"
#include "cglm/types.h"
#include "engine.h"
#include "input.h"
#include "junk/network.h"
#include "player.h"
#include "sys/socket.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include "fcntl.h"
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <unistd.h>
#include "protocol.h"
#include "world.h"



#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

pthread_t threads[POLL_THREADS];
pthread_t input_thread;


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
    for (int i = 0; i < POLL_THREADS; i++) {
        pthread_create(&threads[i], 0,server_client_loop, server);
    }

    int input_sock = junk_udp_ipv4_socket("127.0.0.1", "8000");
    if (input_sock == -1) {
        fprintf(stderr, "Couldn't create input socket\n");
        return -1;
    }
    server->input_fd = input_sock;
    pthread_create(&input_thread, 0,server_client_input, server);
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
    int i = 0;
    while (1) {
        if (i > 8) {
            continue;
        }
        struct sockaddr_in addrinfo;
        int client_sock = 0;
        socklen_t len = sizeof(addrinfo);
        if ((client_sock = accept(server->server_fd, (struct sockaddr*)(&addrinfo), &len)) == -1) {
            // fprintf(stderr, "Could not accept connection.\n");
            continue;
        }
        for (size_t j = 0; j < ARRAY_SIZE(server->clients); j++) {
            // Found a free slot
            if (server->clients[j].uuid == -1) {
                char ip[32];
                inet_ntop(AF_INET, (void*) &addrinfo.sin_addr, ip, 32);
                fprintf(stderr, "Accepted conn from %s\n", ip);

                server->clients[j].client_fd = client_sock;
                struct SSP init_pkt = { 
                    .client_uuid = 10,
                    .id = SSP_INIT,
                    .data_size = 0,
                };
                ssp_send(&init_pkt, client_sock);
                vec3 pos = { 1.0f, 200.0f, -1.0f };
                player_data_init(pos, &server->clients[j].player);
                server->clients[j].uuid = 10;
                break;
            }
        }
        fprintf(stderr, "added new, accepting again\n");
    }
    return 0;
}

int server_client_chunk_sync(struct server* server, int fd) {
    fprintf(stderr, "CLIENT SERVER CHUNK SYNC\n");
    for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
        for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
            // Pass 1 - generate terrain
            int chunk_coord[2] = { i,  j };
            struct chunk* chunk = NULL;
            //TODO: RACE CONDITION WITH thread gens
            world_get_chunk(server->world, chunk_coord, &chunk);
            struct SSP send = {
                .client_uuid = 10,
                .data_size = sizeof(struct chunk_data),
                .id = SSP_CHUNK_SYNC,
            };
            ssp_send(&send, fd);
            int ret = chunk_data_send(&chunk->data, fd);
            if (ret != 0) return ret;
            glm_vec2_print(chunk->data.coord, stderr);
            fprintf(stderr, "sent data for chunk %d %d\n", chunk_coord[0], chunk_coord[1]);
        }
    }
    return 0;
};
/*
 * Generate new chunks around player if need be. Should be done better TODO
 *
 */
int server_client_chunk_update(struct server* server, struct client* client) {
    // NOTE: OpenGL FLIP
    int curr_chunk[2] = { (int)floorf(client->player.position[0] / (float)CHUNK_WIDTH), (int)floorf(client->player.position[2] / (float)CHUNK_LENGTH) };
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

void* server_client_input(void* buf) {
    struct server* server = (struct server*) buf;
    struct pollfd pfd = {
        .fd = server->input_fd,
        .events = POLLIN,
    };
    fprintf(stderr, "Started server client input\n");
    while (1) {
        if (poll(&pfd, 1, 0) > 0) {
            fprintf(stderr, "POLLED\n");
            struct ESP recv = { };
            int ret = esp_recv(&recv, server->input_fd);
            if (ret != 0) {
                fprintf(stderr, "Couldn't recv input\n");
                continue;
            }
            for (size_t i = 0; i < ARRAY_SIZE(server->clients); i++) {
                if (recv.client_uuid == server->clients[i].uuid) {
                    input_server_process(&server->clients[i].player, server->world, &recv);
                    // player_data_send(&server->clients[i].player, server->input_fd);
                }
            }
        }
    }
}
void* server_client_loop(void* buf) {
    struct server* server = (struct server*) buf;
    while (1) {
        for (size_t i = 0; i < ARRAY_SIZE(server->clients); i++) {
            // Active client, poll for data
            if (server->clients[i].uuid != -1) {
                int need_update = server_client_chunk_update(server, &server->clients[i]);
                if (need_update) {
                    int ret = server_client_chunk_sync(server, server->clients[i].client_fd);
                    if (ret != 0) {
                        server->clients[i].uuid = -1;
                        close(server->clients[i].client_fd);
                        fprintf(stderr, "disconnected, closed\n");
                    } else {
                        fprintf(stderr, "sent, sleeping\n");
                    }
                }
            }
        }
    }
    return 0;
}

int main() {
    struct server server;
    memset(&server, 0, sizeof(struct server));
    server_init(&server);
    server_start(&server);
}
