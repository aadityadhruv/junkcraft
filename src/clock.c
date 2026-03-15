#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "clock.h"
#include "cglm/cglm.h"
#include "shader.h"
#include "util.h"
#include "stb/stb_image.h"

int clock_init(struct clock** clk) {
    struct clock* clock = malloc(sizeof(struct clock));
    if (clock == NULL) {
        return -1;
    }
    memset(clock, 0, sizeof(struct clock));
    // In seconds (10 minutes)
    clock->cycle = 60*10;
    vec3 night_color = { 4 / 255.0f, 26 / 255.0f, 64 / 255.0f };
    vec3 day_color = { 0.529f, 0.808f, 0.922f };
    memcpy(clock->night, night_color, sizeof(vec3));
    memcpy(clock->day, day_color, sizeof(vec3));
    *clk = clock;
    int width, height, nr_channels;
    glActiveTexture(GL_TEXTURE2);
    unsigned char *sun = stbi_load("static/sun.png", &width, &height, &nr_channels, 0);
    vec2 sun_size = { width , height };
    glm_vec2_print(sun_size, stderr);
    unsigned char *moon = stbi_load("static/moon.png", &width, &height, &nr_channels, 0);
    vec2 moon_size = { width , height };
    unsigned char *night_sky = stbi_load("static/night_sky.png", &width, &height, &nr_channels, 0);
    vec2 night_sky_size = { width , height };
    glGenVertexArrays(1, &clock->_vao);
    glBindVertexArray(clock->_vao);
    create_texture(&clock->_tbo[0], sun, sun_size);
    create_texture(&clock->_tbo[1], moon, moon_size);
    create_texture(&clock->_tbo[2], night_sky, night_sky_size);
    create_vbo(&clock->_vbo, NULL, sizeof(float) * 6 * 5);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, clock->_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glActiveTexture(GL_TEXTURE0);
    stbi_image_free(sun);
    stbi_image_free(moon);
    stbi_image_free(night_sky);

    return 0;
};

void clock_draw(struct clock* clock, struct player* player, struct shader* shader) {
    glBindVertexArray(clock->_vao);
    glActiveTexture(GL_TEXTURE2);
    set_uniform_sampler2d("obj_texture", shader, 2);
    glBindBuffer(GL_ARRAY_BUFFER, clock->_vbo);
    float quad_vertices[6*5] = { 
        1.0f, 1.0f, 0.0f, // top-right
        1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, // top-left
        1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, // bottom-left
        0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, // bottom-left
        0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, // bottom-right
        1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, // top-right
        1.0f, 1.0f,
    };
    vec2 texture_scale = { 1.0f, 1.0f };
    float opacity = 1.0f;
    set_uniform_float("opacity", shader, opacity);
    set_uniform_vec2("texture_scale", shader, texture_scale);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
    // ========= Draw Sun ===========
    glBindTexture(GL_TEXTURE_2D, clock->_tbo[0]);
    mat4 translate;
    glm_mat4_identity(translate);
    vec3 t = { player->position[0], player->position[1], player->position[2] - 10.0f};
    vec3 x_axis = { 1.0f, 0.0f, 0.0f };
    vec3 y_axis = { 0.0f, 1.0f, 0.0f };
    float angle = 2 * M_PI * (clock->curr_time / clock->cycle);
    glm_rotate_at(translate, player->position, angle, x_axis);
    glm_translate(translate, t);
    set_uniform_mat4("model", shader, translate);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // ========= Draw Moon ===========
    glBindTexture(GL_TEXTURE_2D, clock->_tbo[1]);
    glm_mat4_identity(translate);
    angle = 2 * M_PI * (clock->curr_time / clock->cycle) -  M_PI;
    glm_rotate_at(translate, player->position, angle, x_axis);
    glm_translate(translate, t);
    glm_scale_uni(translate, 0.5f);
    set_uniform_mat4("model", shader, translate);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // ========= Draw star sky ===========
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glm_vec2_scale(texture_scale, 5.0f, texture_scale);
    set_uniform_vec2("texture_scale", shader, texture_scale);
    glBindTexture(GL_TEXTURE_2D, clock->_tbo[2]);
    angle = 0;
    float intensity = 1 - clock_get_light_intensity(clock);
    set_uniform_float("opacity", shader, intensity*intensity);
    float star_spin_speed = 1.0f;
    double curr_t = (double)clock->curr_time / (double)clock->cycle;
    for (int i = 0; i < 4; i++) {
        glm_mat4_identity(translate);
        t[2] = player->position[2] - 100.0f;
        glm_rotate_at(translate, player->position, 2 * M_PI * curr_t * star_spin_speed, y_axis);
        glm_rotate_at(translate, player->position, angle, y_axis);
        glm_translate(translate, t);
        glm_scale_uni(translate, 100.0f);
        set_uniform_mat4("model", shader, translate);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        angle += M_PI / 2;
    }
    angle = M_PI / 2;
    // Top 
    glm_mat4_identity(translate);
    t[2] = player->position[2] - 100.0f;
    glm_rotate_at(translate, player->position, 2 * M_PI * curr_t * star_spin_speed, y_axis);
    glm_rotate_at(translate, player->position, angle, x_axis);
    glm_translate(translate, t);
    glm_scale_uni(translate, 100.0f);
    set_uniform_mat4("model", shader, translate);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisable(GL_BLEND);
    // Cleanup
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
}

void clock_tick(struct clock* clk, double dt) {
    clk->curr_time = fmod((clk->curr_time + dt), clk->cycle);
}

void clock_get_sky_color(struct clock* clk, vec3 out) {
    float scale = 1.0f - clock_get_light_intensity(clk);
    vec3 color = { 
        lerp(clk->day[0], clk->night[0], scale),
        lerp(clk->day[1], clk->night[1], scale),
        lerp(clk->day[2], clk->night[2], scale),
    };
    memcpy(out, color, sizeof(vec3));
}
float clock_get_light_intensity(struct clock* clk) {
    double curr_t = (double)clk->curr_time / (double)clk->cycle;
    float scale = sin(2 * M_PI * curr_t) + 1.0f;
    scale /= 2;
    return scale;
}
