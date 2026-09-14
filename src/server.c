#include "server.h"
#include "cglm/io.h"
#include "cglm/types.h"
#include "engine.h"
#include "junk/network.h"
#include "sys/socket.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include "fcntl.h"
#include <signal.h>
#include <unistd.h>
#include "protocol.h"
#include "world.h"



#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

int server_client_ssp(struct server* server, int client_idx);
void cleanup(int sig) {
    exit(0);
}

int server_stop(struct server *server) {
    for (int i = 0; i < ARRAY_SIZE(server->clients); i++) {
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
    signal(SIGINT, cleanup);
    memset(server->clients, 0, ARRAY_SIZE(server->clients));
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
        int len = sizeof(addrinfo);
        if ((client_sock = accept(server->server_fd, (struct sockaddr*)(&addrinfo), &len)) == -1) {
            // fprintf(stderr, "Could not accept connection.\n");
            continue;
        }
        for (size_t j = 0; j < ARRAY_SIZE(server->clients); j++) {
            // Found a free slot
            if (server->clients[j].uuid == 0) {
                server->clients[j].uuid = 10;
                server->clients[j].client_fd = client_sock;
                char ip[32];
                inet_ntop(AF_INET, (void*) &addrinfo.sin_addr, ip, 32);
                fprintf(stderr, "Accepted conn from %s\n", ip);
                struct SSP init_pkt = { 
                    .client_uuid = server->clients[j].uuid,
                    .id = SSP_INIT,
                    .data_size = 0,
                };
                ssp_send(&init_pkt, client_sock);
                server_client_ssp(server, j);
                break;
            }
        }
        fprintf(stderr, "added new, accepting again\n");
    }
    return 0;
}

int server_client_chunk_sync(struct server* server, int fd) {
    fprintf(stderr, "CLIENT SERVER CHUNK SYNC\n");
    int coords[2] = {1, 1};
    struct chunk* chunk = NULL;
    world_get_chunk(server->world, coords, &chunk);
    struct SSP send = {
        .client_uuid = 10,
        .data_size = sizeof(struct chunk_data),
        .id = SSP_CHUNK_SYNC,
    };
    ssp_send(&send, fd);
    chunk->data.biome = 2;
    chunk_data_send(&chunk->data, fd);
    glm_vec2_print(chunk->data.coord, stderr);
    fprintf(stderr, "sent data\n");
    return 0;
};
int server_client_ssp(struct server* server, int client_idx) {
    int sock = server->clients[client_idx].client_fd;
    server_client_chunk_sync(server, sock);
    // while (1) {
    //     switch (packet.id) {
    //         case SSP_CHUNK_SYNC:
    //             goto exit;
    //         case SSP_CHUNK_EVENT:
    //             goto exit;
    //         default:
    //             goto exit;
    //     }
    //
    // }
    // exit:
    close(sock);
    server->clients[client_idx].uuid = 0;
    return 0;
}

int main() {
    struct server server;
    memset(&server, 0, sizeof(struct server));
    server_init(&server);
    server_start(&server);
}
