#pragma once
#include "cglm/types.h"
#include "chunk.h"
#include "stdint.h"
/*
 * The protocols that are used between client and server
 * over TCP and UDP
 *
 */


/*
 * The Junkcraft Steady State Protocol.
 * Used for critical updates, such as world/chunk state, player inventories
 * etc.
 */
enum SSP_ID {
    SSP_NONE,
    SSP_INIT,
    SSP_CHUNK_SYNC,
    SSP_CHUNK_EVENT,
};

struct ssp_chunk_sync {
    int x;
    int y;
};
struct SSP {
    int64_t client_uuid;
    int32_t data_size;
    enum SSP_ID id;
};

/*
 * The Junkcraft Epehemeral State Protocol
 * UDP protocol to send data such as player positions, player actions etc
 */
enum ESP_ID {
    ESP_PLAYER_XYZ,
    ESP_PLAYER_PLACE,
    ESP_PLACE_HIT,
};
struct esp_player_xyz {
    float x;
    float y;
    float z;
};
struct ESP {
    int64_t client_uuid;
    char data[32];
    enum ESP_ID id;
};



void ssp_recv(struct SSP* packet, int fd);
void ssp_send(struct SSP* packet, int fd);

void chunk_data_recv(struct chunk_data* data, int fd);
void chunk_data_send(struct chunk_data* data, int fd);
