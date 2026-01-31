#include "cglm/affine.h"
#include "cglm/cam.h"
#include "cglm/cglm.h"
#include "cglm/io.h"
#include "cglm/util.h"
#include "cglm/vec3.h"
#include "math.h"
#include "glad/glad.h"
#include "block.h"
#include "shader.h"
#include "texture.h"
#include "util.h"
#include "block.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

int block_init(struct block* blk, enum BLOCK_ID block_id) {
    blk->block_id = block_id;
    return 0;
}
