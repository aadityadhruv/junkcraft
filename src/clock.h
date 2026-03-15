#pragma once
#include "cglm/types.h"
struct clock {
    double curr_time;
    double cycle;
    vec3 day;
    vec3 night;
};
int clock_init(struct clock** clk);
void clock_tick(struct clock* clk, double dt);
void clock_get_sky_color(struct clock* clk, vec3 out);
float clock_get_light_intensity(struct clock* clk);
