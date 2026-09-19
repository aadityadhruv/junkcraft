#pragma once
#include "player.h"
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
    SSP_PLAYER_DATA,
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
enum ESP_INPUT_BIT {
    ESP_FORWARD = 1 << 0,
    ESP_LEFT = 1 << 1,
    ESP_BACK = 1 << 2,
    ESP_RIGHT = 1 << 3,
    ESP_JUMP = 1 << 4,
    ESP_PLAYER_PLACE = 1 << 5,
    ESP_PLACE_HIT = 1 << 6,
    ESP_ROTATE = 1 << 7,
    ESP_SCROLL = 1 << 8,
};
enum ESP_ID {
    ESP_INPUT,
    ESP_POS
};
struct ESP {
    int64_t client_uuid;
    int32_t mask;
    int rot_x;
    int rot_y;
    int scroll;
};



int ssp_recv(struct SSP* packet, int fd);
int ssp_send(struct SSP* packet, int fd);

int chunk_data_recv(struct chunk_data* data, int fd);
int chunk_data_send(struct chunk_data* data, int fd);

int esp_send(struct ESP* packet, int fd);
int esp_recv(struct ESP* packet, int fd);


int player_data_send(struct player_data *data, int fd);
int player_data_recv(struct player_data *data, int fd);
