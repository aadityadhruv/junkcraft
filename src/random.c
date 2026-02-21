#include "random.h"
#include "chunk.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
extern int seed;
vec4 perlin_vecs[] = {
    { -1,-1,-1,-1 },{ -1,-1,-1,0 },{ -1,-1,-1,1 },{ -1,-1,0,-1 },
    { -1,-1,0,0 },{ -1,-1,0,1 },{ -1,-1,1,-1 },{ -1,-1,1,0 },
    { -1,-1,1,1 },{ -1,0,-1,-1 },{ -1,0,-1,0 },{ -1,0,-1,1 },
    { -1,0,0,-1 },{ -1,0,0,0 },{ -1,0,0,1 },{ -1,0,1,-1 },
    { -1,0,1,0 },{ -1,0,1,1 },{ -1,1,-1,-1 },{ -1,1,-1,0 },
    { -1,1,-1,1 },{ -1,1,0,-1 },{ -1,1,0,0 },{ -1,1,0,1 },
    { -1,1,1,-1 },{ -1,1,1,0 },{ -1,1,1,1 },{ 0,-1,-1,-1 },
    { 0,-1,-1,0 },{ 0,-1,-1,1 },{ 0,-1,0,-1 },{ 0,-1,0,0 },
    { 0,-1,0,1 },{ 0,-1,1,-1 },{ 0,-1,1,0 },{ 0,-1,1,1 },
    { 0,0,-1,-1 },{ 0,0,-1,0 },{ 0,0,-1,1 },{ 0,0,0,-1 },
    { 0,0,0,0 },{ 0,0,0,1 },{ 0,0,1,-1 },{ 0,0,1,0 },
    { 0,0,1,1 },{ 0,1,-1,-1 },{ 0,1,-1,0 },{ 0,1,-1,1 },
    { 0,1,0,-1 },{ 0,1,0,0 },{ 0,1,0,1 },{ 0,1,1,-1 },
    { 0,1,1,0 },{ 0,1,1,1 },{ 1,-1,-1,-1 },{ 1,-1,-1,0 },
    { 1,-1,-1,1 },{ 1,-1,0,-1 },{ 1,-1,0,0 },{ 1,-1,0,1 },
    { 1,-1,1,-1 },{ 1,-1,1,0 },{ 1,-1,1,1 },{ 1,0,-1,-1 },
    { 1,0,-1,0 },{ 1,0,-1,1 },{ 1,0,0,-1 },{ 1,0,0,0 },
    { 1,0,0,1 },{ 1,0,1,-1 },{ 1,0,1,0 },{ 1,0,1,1 },
    { 1,1,-1,-1 },{ 1,1,-1,0 },{ 1,1,-1,1 },{ 1,1,0,-1 },
    { 1,1,0,0 },{ 1,1,0,1 },{ 1,1,1,-1 },{ 1,1,1,0 }
};

#define MIN(x, y) (x < y) ? x : y
#define MAX(x, y) (x > y) ? x : y
float _noise_2d(float x, float y);
float _noise_4d(float x1, float x2, float y1, float y2);
float lerp(float a, float b, float f)
{
    return a * (1.0 - f) + (b * f);
}

float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10); 
}
float noise_heat(float x, float y) {
    float unit_diameter = 2 * M_PI;
    x = x / (WORLD_WIDTH * CHUNK_WIDTH);
    y = y / (WORLD_LENGTH * CHUNK_LENGTH);
    float angle_x =  unit_diameter * x;
    float angle_y =  unit_diameter * y;
    float freq = 2.0f;
    // Get heatmap
    float e = _noise_4d(freq * cosf(angle_x)/unit_diameter, freq* sinf(angle_x)/unit_diameter, freq* cosf(angle_y)/unit_diameter, freq* sinf(angle_y)/unit_diameter);
    return e*e;
}
float noise_terrain(float x, float y) {
    float unit_diameter = 2 * M_PI;
    x = x / (WORLD_WIDTH * CHUNK_WIDTH);
    y = y / (WORLD_LENGTH * CHUNK_LENGTH);
    float angle_x =  unit_diameter * x;
    float angle_y =  unit_diameter * y;
    // Controls how much variation you get - we are spreading
    // points over a larger lattice when this goes up so we are influcenced by more vectors
    float freq =  1.0f;
    // This one generates a big chunk of elevated terrain, so we have like a decently sized mountain range, but not everywhere
    float val1 = 1 * _noise_4d(freq * cosf(angle_x)/unit_diameter, freq* sinf(angle_x)/unit_diameter, freq* cosf(angle_y)/unit_diameter, freq* sinf(angle_y)/unit_diameter);
    val1 = val1*val1*val1;
    freq = 16.0f;
    // This val is for the plains, we are generating flatter plains like terrarin
    float val2 = 0.5 * _noise_4d(freq * cosf(angle_x)/unit_diameter, freq* sinf(angle_x)/unit_diameter, freq* cosf(angle_y)/unit_diameter, freq* sinf(angle_y)/unit_diameter);
    float e = (val1 + val2)/ (1.5);
    return e*e;
}
float _noise_4d(float x1, float x2, float y1, float y2) {
    // Frequency
    vec4 gradients[16] = {  };
    vec4 distances[16] = {  };
    int counter = 0;
    
    vec4 point = { x1, x2, y1, y2 };
    // glm_vec4_print(point, stderr);
    for (int i = 0; i <= 1; i++) {
        for (int j = 0; j <= 1; j++) {
            for (int k = 0; k <= 1; k++) {
                for (int l = 0; l <= 1; l++) {
                    vec4 base = { (int)floorf(x1) + i, (int)floorf(x2) + j, (int)floorf(y1) + k, (int)floorf(y2) + l };
                    srand(seed + ((base[0] * base[1]) + (base[2] * base[3])));
                    memcpy(gradients[counter], perlin_vecs[rand() % 80], sizeof(vec4));
                    vec4 dist;
                    glm_vec4_sub(point, base, dist);
                    memcpy(distances[counter], dist, sizeof(vec4));
                    counter += 1;
                }
            }
        }
    }
    float dots[16];
    for (int i = 0; i < 16; i++) {
            glm_vec4_normalize(gradients[i]);
             glm_vec4_normalize(distances[i]);
            dots[i] = glm_vec4_dot(gradients[i], distances[i]);
            // fprintf(stderr, "dot: %f\n", dots[i]);
    }
    vec4 base = { floorf(x1), floorf(x2), floorf(y1), floorf(y2) };
    vec4 unit = { 0 };
    glm_vec4_sub(point, base, unit);
    // glm_vec4_normalize(point);
    float u1 = unit[0];
    float v1 = unit[1];
    float u2 = unit[2];
    float v2 = unit[3];
    u1 = fade(u1);
    v1 = fade(v1);
    u2 = fade(u2);
    v2 = fade(v2);
    float l_lerps[8];
    int lerp_counter = 0;
    for (int i = 0; i < 8; i++) {
        l_lerps[i] = lerp(dots[lerp_counter], dots[lerp_counter + 1], v2);
        lerp_counter += 2;
    }
    float k_lerps[4];
    lerp_counter = 0;
    for (int i = 0; i < 4; i++) {
        k_lerps[i] = lerp(l_lerps[lerp_counter], l_lerps[lerp_counter + 1], u2);
        lerp_counter += 2;
    }
    float j_lerps[2];
    lerp_counter = 0;
    for (int i = 0; i < 2; i++) {
        j_lerps[i] = lerp(k_lerps[lerp_counter], k_lerps[lerp_counter + 1], v1);
        lerp_counter += 2;
    }
    float i_lerp = lerp(j_lerps[0], j_lerps[1], u1);
    return ((i_lerp + 1) / 2.0f);
}

float _noise_2d(float x, float y) {
    int cx = x / CHUNK_WIDTH;
    int cy = y / CHUNK_LENGTH;
    vec2 chunk_coords = { cx, cy };
    vec2 local_coords = { x - cx*CHUNK_WIDTH, y - cy*CHUNK_LENGTH };
    // Frequency
    vec2 gradients[4] = {  };
    int counter = 0;

    // Order - bottom left, top left, bottom right, top right
    for (int i = 0; i <= 1; i++) {
        for (int j = 0; j <= 1; j++) {
            int wx = (int)(chunk_coords[0] + i) % (WORLD_WIDTH - 1);
            int wy = (int)(chunk_coords[1] + j) % (WORLD_LENGTH - 1);
            srand(seed +  wx * wy);
            vec2  g = { rand(), rand() };
            memcpy(gradients[counter], g, sizeof(vec2));
            counter += 1;
        }
    }
    vec2 distances[4] = { 
        { local_coords[0], local_coords[1] }, //bottom left
        { local_coords[0], local_coords[1] - CHUNK_LENGTH }, // top left
        { local_coords[0] - CHUNK_WIDTH, local_coords[1] }, // bottom right
        { local_coords[0] - CHUNK_WIDTH, local_coords[1] - CHUNK_LENGTH }, //top right
    };
    float dots[4];
    for (int i = 0; i < 4; i++) {
            glm_vec2_normalize(gradients[i]);
             glm_vec2_normalize(distances[i]);
            dots[i] = glm_vec2_dot(gradients[i], distances[i]);
    }
    float u = local_coords[0] / CHUNK_WIDTH;
    float v = local_coords[1] / CHUNK_LENGTH;
    u = fade(u);
    v = fade(v);
    float y1 = lerp(dots[0], dots[1], v);
    float y2 = lerp(dots[2], dots[3], v);
    float average = lerp(y1, y2, u);
    return ((average + 1) / 2.0f);
}
