#include "texture.h"
#include "block.h"
#include "cglm/io.h"
#include "cglm/vec2.h"
#include "util.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <stdlib.h>
#include <string.h>


void texture_init(struct texture** texture) {
    *texture = malloc(sizeof(struct texture));
    memset(*texture, 0, sizeof(struct texture));
}

void texture_load(struct texture* texture, char** path, int size) {
    unsigned char* texture_data = NULL;
    int atlas_width = 192;
    int atlas_height = 32;
    int total_buffer_size = 0;
    vec2 atlas_size = { atlas_width,  0 };
    for (int i = 0; i < size; i++) {
        int width, height, nr_channels;
        unsigned char *data = stbi_load(path[i], &width, &height, &nr_channels, 0);
        int data_size = width * height * nr_channels;
        texture_data = realloc(texture_data, total_buffer_size + data_size);
        memcpy(texture_data + total_buffer_size, data, data_size);
        atlas_size[1] += height;
        total_buffer_size += data_size;
        stbi_image_free(data);
    fprintf(stderr, "Texture load %s\n", path[i]);
    }
    create_texture(&texture->_tbo, texture_data, atlas_size);
    fprintf(stderr, "Loaded all textures\n");
}

void texture_draw(struct texture* texture) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->_tbo);
}
