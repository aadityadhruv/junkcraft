#include "texture.h"
#include "cglm/io.h"
#include "util.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include <stdlib.h>
#include <string.h>
void texture_init(struct texture** texture) {
    *texture = malloc(sizeof(struct texture));
    memset(*texture, 0, sizeof(struct texture));
}

void texture_load(struct texture* texture, char* path) {
    int width, height, nr_channels;
    unsigned char *data = stbi_load(path, &width, &height, &nr_channels, 0);
    vec2 size = { width, height };
    glm_vec2_print(size, stderr);
    create_texture(&texture->_tbo, data, size);
    stbi_image_free(data);
    fprintf(stderr, "Loaded texture\n");
}

void texture_draw(struct texture* texture) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->_tbo);
}
