#include "item.h"
#include "player.h"
#include "util.h"
#include "block.h"
#include <string.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))


struct item_metadata item_metadata[ITEM_ID_COUNT];
struct item_graphics item_graphics;
void item_block_item_use(void* data);
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

        item_metadata[i].action_use = item_block_item_use;

    }

}

int item_init(struct item* item, enum ITEM_ID item_id) {
    item->item_id= item_id;
    return 0;
}
void item_load() {
    int item_draw_order[] = {
        1, 2, 3,   3, 0, 1, // CCW 2-triangles (quad)
    };
    float item_draw_vertices[] = {
        1.0f, 1.0f, // top-right
        1.0f, 1.0f,
        0.0f, 1.0f, // top-left
        0.0f, 1.0f,
        0.0f, 0.0f, // bottom-left
        0.0f, 0.0f,
        1.0f, 0.0f, // bottom-right
        1.0f, 0.0f,
    };
    // Init graphics data
    item_graphics.vertex_count = ARRAY_SIZE(item_draw_order);
    glGenVertexArrays(1, &item_graphics._vao);
    glBindVertexArray(item_graphics._vao);
    create_vbo_dyn(&item_graphics._vbo, (void*)item_draw_vertices, sizeof(item_draw_vertices));
    create_ebo(&item_graphics._ebo, (void*)item_draw_order, sizeof(item_draw_order));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, item_graphics._vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*) (2 * sizeof(float)));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, item_graphics._ebo);
    glBindVertexArray(0);
}

void item_draw(enum ITEM_ID id) {
    glBindVertexArray(item_graphics._vao);
    float item_draw_vertices[] = {
        1.0f, 1.0f, // top-right
        item_metadata[id].texture_data.top_right[0], item_metadata[id].texture_data.top_right[1], 
        0.0f, 1.0f, // top-left
        item_metadata[id].texture_data.top_left[0], item_metadata[id].texture_data.top_left[1], 
        0.0f, 0.0f, // bottom-left
        item_metadata[id].texture_data.bottom_left[0], item_metadata[id].texture_data.bottom_left[1], 
        1.0f, 0.0f, // bottom-right
        item_metadata[id].texture_data.bottom_right[0], item_metadata[id].texture_data.bottom_right[1], 
    };
    glBindBuffer(GL_ARRAY_BUFFER, item_graphics._vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(item_draw_vertices), item_draw_vertices);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

enum BLOCK_ID item_item_to_block(enum ITEM_ID id) {
    switch (id) {
        case ITEM_BLOCK_GRASS:
            return BLOCK_GRASS;
        case ITEM_BLOCK_STONE:
            return BLOCK_STONE;
        case ITEM_BLOCK_ROCK:
            return BLOCK_ROCK;
        case ITEM_BLOCK_SAND:
            return BLOCK_SAND;
        case ITEM_BLOCK_SNOW:
            return BLOCK_SNOW;
        case ITEM_BLOCK_WOOD:
            return BLOCK_WOOD;
        case ITEM_BLOCK_LEAF:
            return BLOCK_LEAF;
        case ITEM_BLOCK_WATER:
            return BLOCK_WATER;
        default:
            return BLOCK_ID_COUNT;
    }
}

void item_block_item_use(void* data) {
    struct engine* engine = (struct engine*) data;
    enum ITEM_ID item = engine->player->inventory.items[engine->player->inventory.curr];
    enum BLOCK_ID blk_id = item_item_to_block(item);
    if (blk_id == BLOCK_ID_COUNT) return;
    player_block_place(engine->player, engine, blk_id);
}
