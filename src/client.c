#include "cglm/io.h"
#include "chunk.h"
#include "engine.h"
#include "protocol.h"
#include "junk/network.h"
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

int main() {
    int sock = junk_tcp_ipv4_connect("127.0.0.1", "8000");
    if (sock == -1) {
        fprintf(stderr, "Failed to connect\n");
        return -1;
    }
    struct SSP init_pkt;
    ssp_recv(&init_pkt, sock);
    fprintf(stderr, "Got init packet: %ld\n", init_pkt.client_uuid);
    fprintf(stderr, "got back: size: %d\n", init_pkt.data_size);
    for (int i = -CHUNK_DISTANCE; i <= CHUNK_DISTANCE; i++) {
        for (int j = -CHUNK_DISTANCE; j  <= CHUNK_DISTANCE; j++) {
            struct chunk chunk = {};
            struct SSP recv = { };
            ssp_recv(&recv, sock);
            chunk_data_recv(&chunk.data, sock);
            exit(1);
            glm_vec2_print(chunk.data.coord, stderr);
        }
    }


    return 0;
}
