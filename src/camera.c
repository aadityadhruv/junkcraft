#include "camera.h"
#include "cglm/cam.h"
#include "cglm/io.h"
#include "cglm/mat4.h"
#include "cglm/vec3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void camera_init(struct camera** camera) {
    struct camera* cam = malloc(sizeof(struct camera));
    memset(cam, 0, sizeof(struct camera));
    vec3 camera_direction = { 0.0f, -1.0f, -5.0f };
    vec3 camera_up = { 0.0f, 1.0f, 0.0f };
    memcpy(cam->direction, camera_direction, sizeof(vec3));
    memcpy(cam->up, camera_up, sizeof(vec3));
    glm_mat4_identity(cam->view);
    glm_mat4_identity(cam->perspective);
    cam->fov = glm_rad(45.0f);
    *camera = cam;
}
void camera_set_position(struct camera* camera, vec3 pos) {
    memcpy(camera->position, pos, sizeof(vec3));
}

void camera_update(struct camera* camera, struct shader* shader) {
    // vec3 camera = { 8.0f, 10.0f, 15.0f };
    // vec3 cam_pivot = { 8.0f, 0.0f, -8.0f };
    glm_look(camera->position, camera->direction, camera->up, camera->view);
    // glm_lookat(camera, cam_pivot, axis_y, blk->view);
    // glm_rotate_at(blk->view, cam_pivot, angle, axis_y);
    // Projection (perspective) matrix
    glm_perspective(camera->fov, 800.0f / 600.0f, 0.1f, -10.0f, camera->perspective);
    set_uniform_mat4("view", shader, camera->view);
    set_uniform_mat4("perspective", shader, camera->perspective);
    // fprintf(stderr, "==== Block View ====\n");
    // glm_mat4_print(camera->view, stderr);
    // fprintf(stderr, "==== Block Perspective ====\n");
    // glm_mat4_print(camera->perspective, stderr);
}

void camera_move(struct camera *camera, float *move) {
    glm_vec3_add(camera->position, move, camera->position);
}
