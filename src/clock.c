#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clock.h"
#include "util.h"

int clock_init(struct clock** clk) {
    struct clock* clock = malloc(sizeof(struct clock));
    if (clock == NULL) {
        return -1;
    }
    memset(clock, 0, sizeof(struct clock));
    // In seconds
    clock->cycle = 10;
    vec3 night_color = { 4 / 255.0f, 26 / 255.0f, 64 / 255.0f };
    vec3 day_color = { 0.529f, 0.808f, 0.922f };
    memcpy(clock->night, night_color, sizeof(vec3));
    memcpy(clock->day, day_color, sizeof(vec3));
    *clk = clock;
    return 0;
};

void clock_tick(struct clock* clk, double dt) {
    clk->curr_time = fmod((clk->curr_time + dt), clk->cycle);
}

void clock_get_sky_color(struct clock* clk, vec3 out) {
    double curr_t = (double)clk->curr_time / (double)clk->cycle;
    vec3 color = { 
        lerp(clk->day[0], clk->night[0], sin(M_PI * curr_t)),
        lerp(clk->day[1], clk->night[1], sin(M_PI * curr_t)),
        lerp(clk->day[2], clk->night[2], sin(M_PI * curr_t)),
    };
    memcpy(out, color, sizeof(vec3));
}
float clock_get_light_intensity(struct clock* clk) {
    double curr_t = (double)clk->curr_time / (double)clk->cycle;
    return sin(M_PI * curr_t);
}
