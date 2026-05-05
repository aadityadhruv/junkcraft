#include "item.h"
#include <string.h>


struct item_metadata item_metadata[ITEM_ID_COUNT];
void item_metadata_init() {
    float x_unit = 1.0f;
    float y_unit = 1.0f / ITEM_ID_COUNT;
    for (int i = ITEM_BLOCK_GRASS; i < ITEM_ID_COUNT; i++) {
        float x_base = 0.0f;
        float y_base = ((float)i / ITEM_ID_COUNT); 
        vec2 top_right = { x_base + x_unit, y_base };
        vec2 top_left = { x_base, y_base };
        vec2 bottom_left = { x_base, y_base + y_unit };
        vec2 bottom_right = { x_base + x_unit, y_base + y_unit};
        memcpy(item_metadata[i].texture_data.top_left, top_left, sizeof(vec2));
        memcpy(item_metadata[i].texture_data.top_right, top_right, sizeof(vec2));
        memcpy(item_metadata[i].texture_data.bottom_left, bottom_left, sizeof(vec2));
        memcpy(item_metadata[i].texture_data.bottom_right, bottom_right, sizeof(vec2));
    }

}

int item_init(struct item* item, enum ITEM_ID item_id) {
    item->item_id= item_id;
    return 0;
}
