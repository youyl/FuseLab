#include "../include/cache.h"

int getHash(int x)
{
    return (x & (CACHE_SIZE - 1));
}

void cacheInit()
{
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        globalcache.dirty[i] = false;
        globalcache.idx[i] = -1;
    }
}

void flushCache(int blk_num)
{
    int hashval = getHash(blk_num);
    if(globalcache.idx[hashval] != blk_num)return;
    if(globalcache.dirty[hashval])writeBlock(globalcache.data[hashval], blk_num);
    globalcache.dirty[hashval] = false;
}

void validBlock(int blk_num)
{
    int hashval = getHash(blk_num);
    if(globalcache.idx[hashval] == blk_num)return;
    if(globalcache.dirty[hashval])writeBlock(globalcache.data[hashval], blk_num);
    globalcache.idx[hashval] = blk_num;
    readBlock(globalcache.data[hashval], blk_num);
    globalcache.dirty[hashval] = false;
}

void setDirty(int blk_num)
{
    int hashval = getHash(blk_num);
    if(globalcache.idx[hashval] != blk_num)return;
    globalcache.dirty[hashval] = true;
}

void flushAllCache()
{
    for (int i = 0; i < CACHE_SIZE; i++)
    {
        if(globalcache.dirty[i] && globalcache.idx[i] != -1)
        {
            writeBlock(globalcache.data[i], globalcache.idx[i]);
        }
    }
}

void getBlock(struct Block *blk, int blk_num)
{
    int hashval = getHash(blk_num);
    validBlock(blk_num);
    memcpy(blk->data, globalcache.data[hashval], BLOCK_SIZE);
}

void putBlock(const struct Block *blk, int blk_num)
{
    int hashval = getHash(blk_num);
    validBlock(blk_num);
    memcpy(globalcache.data[hashval], blk->data, BLOCK_SIZE);
    globalcache.dirty[hashval] = true;
}