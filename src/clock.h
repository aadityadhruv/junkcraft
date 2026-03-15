#pragma once
#include "cglm/types.h"
#include "glad/glad.h"
#include "player.h"
#include "shader.h"

struct player;

struct clock {
    double curr_time;
    double cycle;
    vec3 day;
    vec3 night;
    GLuint _vao;
    GLuint _vbo;
    // Sun, moon, night_sky
    GLuint _tbo[3];
};
int clock_init(struct clock** clk);
void clock_tick(struct clock* clk, double dt);
void clock_get_sky_color(struct clock* clk, vec3 out);
float clock_get_light_intensity(struct clock* clk);
void clock_draw(struct clock* clock, struct player* player, struct shader* shader);
