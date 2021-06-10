#ifndef FS_CACHE
#define FS_CACHE

#include "block.h"

#define CACHE_SIZE 16384

struct Location
{
    int blk_num;
    int offset;
};

struct BlockCache
{
    char data[CACHE_SIZE][BLOCK_SIZE];
    bool dirty[CACHE_SIZE];
};

#endif