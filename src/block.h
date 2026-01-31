#pragma once

enum BLOCK_ID {
    BLOCK_GRASS,
};
struct block {
    enum BLOCK_ID block_id;
};
/**
 * A block struct defines what kind of block we will be rendering. It's the metadata of the block array in a chunk
 *
 */
int block_init(struct block* blk, enum BLOCK_ID block_id);
