#ifndef FS_CACHE
#define FS_CACHE

#include "block.h"

#define CACHE_SIZE 65536

struct BlockCache
{
    char data[CACHE_SIZE][BLOCK_SIZE];
    bool dirty[CACHE_SIZE];
    int idx[BLOCK_SIZE];
};

struct BlockCache cache;

void cacheInit();
void flushCache(int);
void validBlock(int);

#endif