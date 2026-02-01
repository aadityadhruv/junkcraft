#include "camera.h"
#include "cglm/cam.h"
#include "cglm/io.h"
#include "config.h"
#include "cglm/mat4.h"
#include "cglm/util.h"
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
    glm_look(camera->position, camera->direction, camera->up, camera->view);
    // Projection (perspective) matrix
    glm_perspective(camera->fov, SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, -10.0f, camera->perspective);
    set_uniform_mat4("view", shader, camera->view);
    set_uniform_mat4("perspective", shader, camera->perspective);
}

void camera_move(struct camera *camera, enum DIRECTION move) {
    vec3 unit_direction = { 0 };
    glm_normalize_to(camera->direction, unit_direction);
    if (move == FORWARD) {
        // Do nothing, we move in unit_direction direction
    } else if (move == BACKWARD) {
        // Go in the reverse direction
        vec3 neg = { -1.0f, -1.0f, -1.0f };
        glm_vec3_mul(neg, unit_direction, unit_direction);
    } else if (move == LEFT) {
        // Right hand rule - this will be on the left (negative)
        glm_vec3_crossn(camera->up, unit_direction, unit_direction);
    } else if (move == RIGHT) {
        // Right hand rule - this will be on the righ (positive)
        glm_vec3_crossn(unit_direction, camera->up, unit_direction);
    }
    float scale = 0.8f;
    glm_vec3_scale(unit_direction, scale, unit_direction);
    glm_vec3_add(camera->position, unit_direction, camera->position);
}

void camera_rotate(struct camera* camera, vec2 offset) {
    vec3 axis = { 0 };
    float rot_angle = glm_rad(1);
    glm_vec3_crossn(camera->direction, camera->up, axis);
    // Up and down rotation (pitch)
    glm_vec3_rotate(camera->direction, -rot_angle * offset[1], axis);
    // Left and right rotation (yaw)
    glm_vec3_rotate(camera->direction, -rot_angle * offset[0], camera->up);

}
