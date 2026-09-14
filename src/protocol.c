#include "protocol.h"
#include "chunk.h"
#include "junk/network.h"
#include <stdlib.h>
#include <string.h>


void ssp_send(struct SSP* packet, int fd) {
    char buf[sizeof(struct SSP)];
    memset(buf, 0, sizeof(struct SSP));
    int offset = 0;
    memcpy(buf + offset, &(packet->client_uuid), sizeof(packet->client_uuid));
    offset += sizeof(packet->client_uuid);
    memcpy(buf + offset, &(packet->data_size), sizeof(packet->data_size));
    offset += sizeof(packet->data_size);
    memcpy(buf + offset, &(packet->id), sizeof(packet->id));
    offset += sizeof(packet->id);
    junk_tcp_ipv4_send(fd, (char*)buf, offset);
}
void ssp_recv(struct SSP* packet, int fd) {
    char buf[sizeof(struct SSP)];
    junk_tcp_ipv4_recv(fd, (char*)buf, sizeof(struct SSP));
    int offset = 0;
    memcpy(&(packet->client_uuid), buf + offset, sizeof(packet->client_uuid));
    offset += sizeof(packet->client_uuid);
    memcpy(&(packet->data_size), buf + offset, sizeof(packet->data_size));
    offset += sizeof(packet->data_size);
    memcpy(&(packet->id), buf + offset, sizeof(packet->id));
    offset += sizeof(packet->id);
}

void chunk_data_send(struct chunk_data *data, int fd) {
    char buf[sizeof(struct chunk_data)];
    memset(buf, 0, sizeof(struct chunk_data));
    int offset = 0;
    memcpy(buf + offset, data->coord, sizeof(data->coord));
    offset += sizeof(data->coord);
    memcpy(buf + offset, data->blocks, sizeof(data->blocks));
    offset += sizeof(data->blocks);
    memcpy(buf + offset, &(data->biome), sizeof(data->biome));
    offset += sizeof(data->biome);
    memcpy(buf + offset, &(data->generated_structures), sizeof(data->generated_structures));
    offset += sizeof(data->generated_structures);
    junk_tcp_ipv4_send(fd, buf, sizeof(struct chunk_data));
}
void chunk_data_recv(struct chunk_data *data, int fd) {
    char buf[sizeof(struct chunk_data)];
    fprintf(stderr, "ack got\n");
    memset(buf, 0, sizeof(struct chunk_data));
    junk_tcp_ipv4_recv(fd, buf, sizeof(struct chunk_data));
    fprintf(stderr, "ack got\n");
    int offset = 0;
    memcpy(&(data->coord), buf + offset, sizeof(data->coord));
    offset += sizeof(data->coord);
    memcpy(&(data->blocks), buf + offset, sizeof(data->blocks));
    offset += sizeof(data->blocks);
    memcpy(&(data->biome), buf + offset, sizeof(data->biome));
    offset += sizeof(data->biome);
    memcpy(&(data->generated_structures), buf + offset, sizeof(data->generated_structures));
    offset += sizeof(data->generated_structures);
}
