#ifndef FS_CACHE
#define FS_CACHE

#include "block.h"

#define CACHE_SIZE 65536

struct BlockCache
{
    char data[CACHE_SIZE][BLOCK_SIZE];
    bool dirty[CACHE_SIZE];
    int idx[CACHE_SIZE];
};

struct BlockCache globalcache;

int getHash(int);

void cacheInit();
void flushCache(int);
void validBlock(int);
void flushAllCache();
void setDirty(int);
void getBlock(struct Block *, int);
void putBlock(const struct Block *, int);

#endif