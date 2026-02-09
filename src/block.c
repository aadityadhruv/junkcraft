#include "glad/glad.h"
#include <string.h>
#include "block.h"

struct block_metadata block_metadata[BLOCK_ID_COUNT];
void block_metadata_init() {
    float x_unit = 1.0f / BLOCK_FACE_COUNT;
    float y_unit = 1.0f / BLOCK_ID_COUNT;
    for (int i = BLOCK_GRASS; i < BLOCK_ID_COUNT; i++) {
        for (int j = BLOCK_FRONT; j < BLOCK_FACE_COUNT; j++) {
            float x_base = ((float)j / BLOCK_FACE_COUNT);
            float y_base = ((float)i / BLOCK_ID_COUNT); 
            vec2 top_right = { x_base + x_unit, y_base };
            vec2 top_left = { x_base, y_base };
            vec2 bottom_left = { x_base, y_base + y_unit };
            vec2 bottom_right = { x_base + x_unit, y_base + y_unit};
            memcpy(block_metadata[i].texture_data[j].top_left, top_left, sizeof(vec2));
            memcpy(block_metadata[i].texture_data[j].top_right, top_right, sizeof(vec2));
            memcpy(block_metadata[i].texture_data[j].bottom_left, bottom_left, sizeof(vec2));
            memcpy(block_metadata[i].texture_data[j].bottom_right, bottom_right, sizeof(vec2));
 

        }
    }

}

int block_init(struct block* blk, enum BLOCK_ID block_id) {
    blk->block_id = block_id;
    return 0;
}
