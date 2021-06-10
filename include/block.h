#ifndef FS_BLOCK
#define FS_BLOCK

#include "fuse_utils.h"

#define BLOCK_SIZE 512
#define MAX_BLOCK_NUM 1048576

struct Block
{
    char data[BLOCK_SIZE];
};

struct BlockController
{
    char data[BLOCK_SIZE * MAX_BLOCK_NUM];
};

//void controller_init(struct BlockController*);
//void read_block(struct Block*, const struct BlockController*, int);
//void write_block(const struct Block*, int);

#endif