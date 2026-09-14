#include "cglm/io.h"
#include "chunk.h"
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
    ssp_recv(&init_pkt, sock);
    fprintf(stderr, "got back: size: %d\n", init_pkt.data_size);
    struct chunk_data data = { 0 };
    chunk_data_recv(&data, sock);
    glm_vec2_print(data.coord, stderr);
    fprintf(stderr, "b: %d\n", data.biome);
    //Here is the chunk data

    return 0;
}
