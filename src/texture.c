#include "texture.h"
#include "block.h"
#include "cglm/io.h"
#include "cglm/vec2.h"
#include "util.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <stdlib.h>
#include <string.h>

char* textures[] = {
    "textures/001_grass.png",
    "textures/002_stone.png",
    "textures/003_rock.png",
    "textures/004_sand.png",
    "textures/005_snow.png",
    "textures/006_wood.png",
    "textures/007_leaf.png",
    "textures/008_aluminium_ore.png",
    "textures/009_coal_ore.png",
    "textures/010_iron_ore.png",
    "textures/011_copper_ore.png",
    "textures/012_gold_ore.png",
    "textures/013_diamond_ore.png",
    "textures/014_graphite.png",
};

void texture_init(struct texture** texture) {
    *texture = malloc(sizeof(struct texture));
    memset(*texture, 0, sizeof(struct texture));
}

void texture_load(struct texture* texture) {
    unsigned char* texture_data = NULL;
    int atlas_width = 192;
    int total_buffer_size = 0;
    vec2 atlas_size = { atlas_width,  0 };
    int textures_len = sizeof(textures)/sizeof(char*);
    for (int i = 0; i < textures_len; i++) {
        int width, height, nr_channels;
        char tpath[200];
        memset(tpath, 0, 200);
        char* d = "/home/aaditya/git/junkcraft/";
        strcat(tpath, d);
        strcat(tpath, textures[i]);
        unsigned char *data = stbi_load(tpath, &width, &height, &nr_channels, 0);
        int data_size = width * height * nr_channels;
        texture_data = realloc(texture_data, total_buffer_size + data_size);
        memcpy(texture_data + total_buffer_size, data, data_size);
        atlas_size[1] += height;
        total_buffer_size += data_size;
        stbi_image_free(data);
    fprintf(stderr, "Texture load %s\n", tpath);
    }
    create_texture(&texture->_tbo, texture_data, atlas_size);
    fprintf(stderr, "Loaded all textures\n");
}

void texture_draw(struct texture* texture) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->_tbo);
}
