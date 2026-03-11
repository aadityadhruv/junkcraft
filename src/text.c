#include "cglm/cam.h"
#include "freetype/freetype.h"
#include "glad/glad.h"
#include <ft2build.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "shader.h"
#include "text.h"
#include "util.h"



int text_init(struct text** t) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        perror("Error loading freetype library\n");
        return -1;
    }
    FT_Face face;
    if (FT_New_Face(ft, "fonts/pokemon_classic.ttf", 0, &face)) {
        perror("Error loading freetype face\n");
        return -1;
    }
    FT_Set_Pixel_Sizes(face, 0, 20);

    struct text* text = malloc(sizeof(struct text));
    memset(text, 0, sizeof(struct text));
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    
    glActiveTexture(GL_TEXTURE1);
    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            fprintf(stderr, "Failed to load character %c\n", c);
            continue;
        }

        glGenTextures(1, &text->chars[c].texture);
        glBindTexture(GL_TEXTURE_2D, text->chars[c].texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
                );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        text->chars[c].advance = face->glyph->advance.x;
        vec2 size = { face->glyph->bitmap.width, face->glyph->bitmap.rows };
        vec2 bearing = { face->glyph->bitmap_left, face->glyph->bitmap_top };
        memcpy(text->chars[c].bearing, bearing, sizeof(vec2));
        memcpy(text->chars[c].size, size, sizeof(vec2));
        text->chars[c].advance = face->glyph->advance.x;
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glGenVertexArrays(1, &text->_vao);
    glBindVertexArray(text->_vao);

    create_vbo_dyn(&text->_vbo, NULL, sizeof(float)*6*4);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, text->_vbo);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindVertexArray(0);

    *t = text;
    glActiveTexture(GL_TEXTURE0);
    return 0;
}


void text_draw(struct text* text, struct shader* shader, char* string, float x, float y, float scale, int size) {
    glBindVertexArray(text->_vao);
    glActiveTexture(GL_TEXTURE1);
    set_uniform_sampler2d("text", shader, 1);
    mat4 ortho;
    glm_ortho(0.0f, SCREEN_WIDTH, 0.0f, SCREEN_HEIGHT, 0.0f, 1.0f, ortho);
    set_uniform_mat4("projection", shader, ortho);
    for (int i = 0; i < size; i++) {
        struct character c = text->chars[(int)string[i]];
        float xpos = x + c.bearing[0] * scale;
        float ypos = y - (c.size[1] - c.bearing[1]) * scale;

        float w = c.size[0] * scale;
        float h = c.size[1] * scale;

        vec4 vertices[6] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        glBindTexture(GL_TEXTURE_2D, c.texture);
        glBindBuffer(GL_ARRAY_BUFFER, text->_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (c.advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
}
