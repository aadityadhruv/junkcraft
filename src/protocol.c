#include "protocol.h"
#include "chunk.h"
#include "junk/network.h"
#include <stdlib.h>
#include <string.h>


int ssp_send(struct SSP* packet, int fd) {
    char buf[sizeof(struct SSP)];
    memset(buf, 0, sizeof(struct SSP));
    int offset = 0;
    memcpy(buf + offset, &(packet->client_uuid), sizeof(packet->client_uuid));
    offset += sizeof(packet->client_uuid);
    memcpy(buf + offset, &(packet->data_size), sizeof(packet->data_size));
    offset += sizeof(packet->data_size);
    memcpy(buf + offset, &(packet->id), sizeof(packet->id));
    offset += sizeof(packet->id);
    return junk_tcp_ipv4_send(fd, (char*)buf, offset);
}
int ssp_recv(struct SSP* packet, int fd) {
    char buf[sizeof(struct SSP)];
    int ret = junk_tcp_ipv4_recv(fd, (char*)buf, sizeof(struct SSP));
    if (ret != 0) return ret;
    int offset = 0;
    memcpy(&(packet->client_uuid), buf + offset, sizeof(packet->client_uuid));
    offset += sizeof(packet->client_uuid);
    memcpy(&(packet->data_size), buf + offset, sizeof(packet->data_size));
    offset += sizeof(packet->data_size);
    memcpy(&(packet->id), buf + offset, sizeof(packet->id));
    offset += sizeof(packet->id);
    return 0;
}

int chunk_data_send(struct chunk_data *data, int fd) {
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
    return junk_tcp_ipv4_send(fd, buf, sizeof(struct chunk_data));
}
int chunk_data_recv(struct chunk_data *data, int fd) {
    char buf[sizeof(struct chunk_data)];
    memset(buf, 0, sizeof(struct chunk_data));
    int r = junk_tcp_ipv4_recv(fd, buf, sizeof(struct chunk_data));
    if (r != 0) return r;
    int offset = 0;
    memcpy(&(data->coord), buf + offset, sizeof(data->coord));
    offset += sizeof(data->coord);
    memcpy(&(data->blocks), buf + offset, sizeof(data->blocks));
    offset += sizeof(data->blocks);
    memcpy(&(data->biome), buf + offset, sizeof(data->biome));
    offset += sizeof(data->biome);
    memcpy(&(data->generated_structures), buf + offset, sizeof(data->generated_structures));
    offset += sizeof(data->generated_structures);
    return 0;
}
int player_data_send(struct player_data *data, int fd) {
    char buf[sizeof(struct player_data)];
    memset(buf, 0, sizeof(struct player_data));
    int offset = 0;
    memcpy(buf + offset, &(data->position), sizeof(data->position));
    offset += sizeof(data->position);
    memcpy(buf + offset, (data->velocity), sizeof(data->velocity));
    offset += sizeof(data->velocity);
    memcpy(buf + offset, (data->accel), sizeof(data->accel));
    offset += sizeof(data->accel);
    memcpy(buf + offset, (data->direction), sizeof(data->direction));
    offset += sizeof(data->direction);
    memcpy(buf + offset, data->items, sizeof(data->items));
    offset += sizeof(data->items);
    memcpy(buf + offset, &(data->curr), sizeof(data->curr));
    offset += sizeof(data->curr);
    return junk_tcp_ipv4_send(fd, buf, offset);
}
int player_data_recv(struct player_data *data, int fd) {
    char buf[sizeof(struct player_data)];
    memset(buf, 0, sizeof(struct player_data));
    int offset = 0;
    int size = sizeof(data->position) + sizeof(data->velocity) + sizeof(data->accel) + sizeof(data->direction) + sizeof(data->items) + sizeof(data->curr);
    int ret = junk_tcp_ipv4_recv(fd, (char*)buf, size);
    if (ret != 0) return ret;
    memcpy(&(data->position), buf + offset, sizeof(data->position));
    offset += sizeof(data->position);
    memcpy((data->velocity), buf + offset, sizeof(data->velocity));
    offset += sizeof(data->velocity);
    memcpy((data->accel), buf + offset, sizeof(data->accel));
    offset += sizeof(data->accel);
    memcpy((data->direction), buf + offset, sizeof(data->direction));
    offset += sizeof(data->direction);
    memcpy(data->items, buf + offset, sizeof(data->items));
    offset += sizeof(data->items);
    memcpy(&(data->curr), buf + offset, sizeof(data->curr));
    offset += sizeof(data->curr);
    return 0;
}
int esp_send(struct ESP* packet, int fd) {
    char buf[sizeof(struct ESP)];
    memset(buf, 0, sizeof(struct ESP));
    int offset = 0;
    memcpy(buf + offset, &(packet->client_uuid), sizeof(packet->client_uuid));
    offset += sizeof(packet->client_uuid);
    memcpy(buf + offset, &(packet->mask), sizeof(packet->mask));
    offset += sizeof(packet->mask);
    memcpy(buf + offset, &(packet->rot_x), sizeof(packet->rot_x));
    offset += sizeof(packet->rot_x);
    memcpy(buf + offset, &(packet->rot_y), sizeof(packet->rot_y));
    offset += sizeof(packet->rot_y);
    return junk_udp_ipv4_send(fd, "127.0.0.1", "8000", (char*)buf, offset);
}
int esp_recv(struct ESP* packet, int fd) {
    char buf[sizeof(struct ESP)];
    int ret = junk_udp_ipv4_recv(fd, (char*)buf, sizeof(struct ESP));
    if (ret != 0) return ret;
    int offset = 0;
    memcpy(&(packet->client_uuid), buf + offset, sizeof(packet->client_uuid));
    offset += sizeof(packet->client_uuid);
    memcpy(&(packet->mask), buf + offset, sizeof(packet->mask));
    offset += sizeof(packet->mask);
    memcpy(&(packet->rot_x), buf + offset, sizeof(packet->rot_x));
    offset += sizeof(packet->rot_x);
    memcpy(&(packet->rot_y), buf + offset, sizeof(packet->rot_y));
    offset += sizeof(packet->rot_y);
    return 0;
}
