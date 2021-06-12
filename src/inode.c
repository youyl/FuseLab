#include "../include/inode.h"

// void updateNlinkInode(struct Inode *, int);

int allocBlock()
{
    // search superblock
    int emptyidx = -1;
    for (int i = ZERO_BLK_START; i < ZERO_BLK_END; i++)
    {
        if(getBit(&(bitmap.zeroblk), i) == 0)
        {
            emptyidx = i;
            break;
        }
    }
    if(emptyidx == -1)return -ENOSPC;
    int emptypage = -1, emptypos;
    for (int i = 0; i < (BLOCK_SIZE << 3); i++)
    {
        if(getBit(&(bitmap.indexblk[emptyidx]), i) == 0)
        {
            emptypage = emptyidx * BLOCK_SIZE * 8 + i;
            emptypos = i;
            break;
        }
    }
    if(emptypage == -1)return -ENOSPC;
    // alloc page
    bitmap.placeleft[emptyidx]--;
    if(bitmap.placeleft[emptyidx] == 0)setBit(&(bitmap.zeroblk), emptyidx);
    setBit(&(bitmap.indexblk[emptyidx]), emptypos);
    // push to disk
    putBlock(&(bitmap.zeroblk), 0);
    flushCache(0);
    putBlock(&(bitmap.indexblk[emptyidx]),emptyidx);
    flushCache(emptyidx);
    printf("alloc page complete: pageidx = %d  pagenum = %d\n", emptyidx, emptypage);
    return emptypage;
}

int createInode(struct Inode *inode, int type, int mode)
{
    inode->baseblknum = allocBlock();
    if(inode->baseblknum < 0)return inode->baseblknum;
    getBlock(&(inode->baseblk), inode->baseblknum);
    memset(inode->baseblk.data, 0, BLOCK_SIZE);
    inode->type = type;
    int *ptr = (int *)(inode->baseblk.data);
    ptr[FILE_TYPE_OFFSET] = type;
    updateModeInode(inode, mode);
    if(type == DIR_INODE_TYPE)updateSizInode(inode, 4096);
    else updateSizInode(inode, 0);
    if(type == DIR_INODE_TYPE)updateNlinkInode(inode, 2);
    else updateNlinkInode(inode, 1);
    updateTimeInode(inode);
    putBlock(&(inode->baseblk), inode->baseblknum);
    flushCache(inode->baseblknum);
    return 0;
}

void inodeInit()
{
    // superblock init
    getBlock(&(bitmap.zeroblk), 0);
    for (int i = ZERO_BLK_START; i < ZERO_BLK_END; i++)
    {
        getBlock(&(bitmap.indexblk[i]), i);
        bitmap.placeleft[i] = 4096;
        for (int j = 0; j < (BLOCK_SIZE << 3); j++)
        {
            if(getBit(&(bitmap.indexblk[i]), j) != 0)bitmap.placeleft[i]--;
        }
    }
    // root dir init
    inodecnt = 1;
    if(controller.created)createInode(&(inodelist[ROOTDIR_IDX]), DIR_INODE_TYPE, 0755);
    else retrieveInode(&(inodelist[ROOTDIR_IDX]), ROOTDIR_BLK);
    printf("Inode Init Complete\n");
}

void updateTimeInode(struct Inode *inode)
{
    struct timespec curtime;
    clock_gettime(CLOCK_MONOTONIC, &curtime);
    inode->timec = curtime;
    long *ptr = (long *)(inode->baseblk.data);
    ptr[FILE_TIME_OFFSET] = curtime.tv_sec;
    ptr[FILE_TIME_OFFSET + 1] = curtime.tv_nsec;
}

void updateModeInode(struct Inode *inode, int mode)
{
    inode->mode = mode;
    int *ptr = (int *)(inode->baseblk.data);
    ptr[FILE_MODE_OFFSET] = mode;
}

void updateNlinkInode(struct Inode *inode, int nlink)
{
    inode->nlink = nlink;
    int *ptr = (int *)(inode->baseblk.data);
    ptr[FILE_NLINK_OFFSET] = nlink;
}

void updateSizInode(struct Inode *inode, int siz)
{
    inode->siz = siz;
    int *ptr = (int *)(inode->baseblk.data);
    ptr[FILE_SIZE_OFFSET] = siz;
}

// get one inode from the blocknum, not check if existed, not to use by upper level
void retrieveInode(struct Inode *inode, int blk_num)
{
    inode->baseblknum = blk_num;
    getBlock(&(inode->baseblk), blk_num);
    // get attributes
    long *longptr = (long *)(inode->baseblk.data);
    int *ptr = (int *)(inode->baseblk.data);
    inode->timec.tv_sec = longptr[FILE_TIME_OFFSET];
    inode->timec.tv_nsec = longptr[FILE_TIME_OFFSET + 1];
    inode->type = ptr[FILE_TYPE_OFFSET];
    inode->mode = ptr[FILE_MODE_OFFSET];
    inode->siz = ptr[FILE_SIZE_OFFSET];
    inode->nlink = ptr[FILE_NLINK_OFFSET];
    inode->blkcnt = 0;
    // get idx
    memset(inode->indexblkidx, 0, SECONDARY_INDEX_COUNT * 4);
    for (int i = 0; i < SECONDARY_INDEX_COUNT; i++)
    {
        inode->indexblkidx[i] = getIndex(inode, i);
        if(inode->indexblkidx[i] != 0)getBlock(&(inode->indexblk[i]), inode->indexblkidx[i]);
        else continue;
        for (int j = 0; j < BLOCK_SIZE / 4; j++)
        {
            int tmp = getSecondaryIndex(inode, i, j);
            if(tmp != 0)inode->blkcnt++;
        }
    }
    if(inode->type == DIR_INODE_TYPE)inode->blkcnt = 0;
}

// wrapped retrieve, check duplicated
int putInodetoList(int blk_num)
{
    for (int i = 0; i < inodecnt; i++)
    {
        if(inodelist[i].baseblknum == blk_num)return i;
    }
    for (int i = 0; i < inodecnt; i++)
    {
        if(inodelist[i].baseblknum == 0)
        {
            retrieveInode(&(inodelist[i]), blk_num);
            return i;
        }
    }
    retrieveInode(&(inodelist[inodecnt]), blk_num);
    inodecnt++;
    return inodecnt - 1;
}

// search in dir, not recursively, find the blocknum with name, 0 is not found
int searchDir(const struct Inode *inode, const char *name)
{
    if(inode->type != DIR_INODE_TYPE)return -ENOTDIR;
    if(strlen(name) > NAME_LENGTH_LIMIT)return -ENAMETOOLONG;
    for (int i = 0; i < SECONDARY_INDEX_COUNT; i++)
    {
        if(inode->indexblkidx[i] == 0)break;
        for (int j = 0; j < DIRENT_PER_BLOCK; j++)
        {
            if(getSecondaryDirent(inode, i, j) == 0)continue;
            if(strcmp(inode->indexblk[i].data + (DIRENT_SIZE * 4 * j), name) == 0)
            {
                return getSecondaryDirent(inode, i, j);
            }
        }
    }
    return 0;
}

// search alone path, recursively, return the inode idx in list
int findInode(const struct Inode *inode, const char *path)
{
    if(strlen(path) > PATH_LENGTH_LIMIT)return -ENAMETOOLONG;
    // if found then retrieve
    if(strcmp(path,"/") == 0 || strlen(path) == 0)return putInodetoList(inode->baseblknum);
    // get next stage name
    char name[200], newpath[2000];
    int namelen = 0, newpathlen = 0, flag = 0;
    for (int i = 0; i < strlen(path); i++)
    {
        if(i == 0 && path[i] == '/')continue;
        if(path[i] == '/' && flag == 0)flag = 1;
        else
        {
            if(flag == 0)
            {
                name[namelen] = path[i];
                namelen++;
            }
            else
            {
                newpath[newpathlen] = path[i];
                newpathlen++;
            }
        }
    }
    name[namelen] = 0;
    newpath[newpathlen] = 0;
    // search in Dir
    int res = searchDir(inode, name);
    if(res < 0)return res;
    if(res == 0)return -ENOENT;
    // retrieve node and walk further
    int idx = putInodetoList(res);
    return findInode(&(inodelist[idx]), newpath);
}

// loacte the block num in inode, with certain offset, return block num
int locateBlock(const struct Inode *inode, int offset)
{
    // get idx count and blk count
    int blk_count = offset / BLOCK_SIZE;
    int idx_count = blk_count / (BLOCK_SIZE / 4);
    int inner_count = blk_count % (BLOCK_SIZE / 4);
    int res = getSecondaryIndex(inode, idx_count, inner_count);
    if(res <= 0)return -ENOENT;
    return res;
}

// get blocknum of secondary idx in inode
int getIndex(const struct Inode *inode, int idx)
{
    int *ptr = (int *)(inode->baseblk.data);
    return ptr[FILE_INDEX_OFFSET + idx];
}

// get blocknum of data block in inode
int getSecondaryIndex(const struct Inode *inode, int idx, int sidx)
{
    int *ptr = (int *)(inode->indexblk[idx].data);
    return ptr[sidx];
}

void setIndex(struct Inode *inode, int idx, int val)
{
    int *ptr = (int *)(inode->baseblk.data);
    ptr[FILE_INDEX_OFFSET + idx] = val;
    inode->indexblkidx[idx] = val;
    putBlock(&(inode->baseblk), inode->baseblknum);
    flushCache(inode->baseblknum);
}

void setSecondaryIndex(struct Inode *inode, int idx, int sidx, int val)
{
    int *ptr = (int *)(inode->indexblk[idx].data);
    ptr[sidx] = val;
    putBlock(&(inode->indexblk[idx]), inode->indexblkidx[idx]);
    flushCache(inode->indexblkidx[idx]);
}

int newBlock(struct Inode *inode)
{
    int res = -1;
    for (int i = 0; i < SECONDARY_INDEX_COUNT; i++)
    {
        if(inode->indexblkidx[i] != 0)res = i;
    }
    int inner = -1;
    if(res != -1)
    {
        for (int i = 0; i < BLOCK_SIZE / 4; i++)
        {
            if(getSecondaryIndex(inode, res, i) == 0)
            {
                inner = i;
                break;
            }
        }
    }
    if(inner == -1 && res == SECONDARY_INDEX_COUNT - 1)return -EFBIG;
    if(inner == -1)
    {
        // new secondary index
        int val = allocBlock();
        if(val < 0)return val;
        getBlock(&(inode->indexblk[res + 1]), val);
        memset(inode->indexblk[res + 1].data, 0, BLOCK_SIZE);
        putBlock(&(inode->indexblk[res + 1]), val);
        flushCache(val);
        setIndex(inode, res + 1, val);
        res++;
        inner = 0;
    }
    // new block (res, inner)
    int tmp = allocBlock();
    if(tmp < 0)return tmp;
    setSecondaryIndex(inode, res, inner, tmp);
    inode->blkcnt++;
    return tmp;
}

int getSecondaryDirent(const struct Inode *inode, int idx, int sidx)
{
    int *ptr = (int *)(inode->indexblk[idx].data);
    return ptr[sidx * DIRENT_SIZE + DIRENT_BLKNUM_OFFSET];
}

// not check if existed, please check before call this
int newDirent(struct Inode *inode, const char *name, int type, int mode, int link)
{
    if(strlen(name) > NAME_LENGTH_LIMIT)return -ENAMETOOLONG;
    if(link < 0)
    {
        int res = createInode(&(inodelist[inodecnt]), type, mode);
        if(res < 0)return res;
        link = inodelist[inodecnt].baseblknum;
        inodecnt++;
    }
    int res = -1, inner = -1;
    for (int i = 0; i < SECONDARY_INDEX_COUNT; i++)
    {
        if(inode->indexblkidx[i] != 0)
        {
            res = i;
            for (int j = 0; j < DIRENT_PER_BLOCK; j++)
            {
                if(getSecondaryDirent(inode, i, j) == 0)
                {
                    inner = j;
                    res = i;
                    break;
                }
            }
            if(inner != -1)break;
        }
    }
    if(inner == -1 && res == SECONDARY_INDEX_COUNT - 1)return -ENOSPC;
    if(inner == -1)
    {
        // new secondary dirent
        int val = allocBlock();
        if(val < 0)return val;
        getBlock(&(inode->indexblk[res + 1]), val);
        memset(inode->indexblk[res + 1].data, 0, BLOCK_SIZE);
        putBlock(&(inode->indexblk[res + 1]), val);
        flushCache(val);
        setIndex(inode, res + 1, val);
        res++;
        inner = 0;
        inode->blkcnt++;
    }
    // new block (res, inner)
    int *ptr = (int *)(inode->indexblk[res].data);
    ptr[inner * DIRENT_SIZE + DIRENT_BLKNUM_OFFSET] = link;
    memset(ptr + (inner * DIRENT_SIZE * 4), 0, NAME_LENGTH_LIMIT + 1);
    memcpy(ptr + (inner * DIRENT_SIZE * 4), name, strlen(name));
    putBlock(&(inode->indexblk[res]), inode->indexblkidx[res]);
    flushCache(inode->indexblkidx[res]);
    return 0;
}

void flushInode(const struct Inode *inode)
{
    // flush datablk
    for (int i = 0; i < SECONDARY_INDEX_COUNT; i++)
    {
        if(inode->indexblkidx[i] == 0)continue;
        for (int j = 0; j < BLOCK_SIZE / 4; j++)
        {
            int res = getSecondaryIndex(inode, i, j);
            if(res == 0)continue;
            flushCache(res);
        }
    }
}

void releaseInode(int inodeidx)
{
    flushInode(&(inodelist[inodeidx]));
    releaseBlock(inodelist[inodeidx].baseblknum);
    for (int i = 0; i < SECONDARY_INDEX_COUNT; i++)
    {
        if(inodelist[inodeidx].indexblkidx[i] == 0)continue;
        releaseBlock(inodelist[inodeidx].indexblkidx[i]);
        for (int j = 0; j < BLOCK_SIZE / 4; j++)
        {
            int res = getSecondaryIndex(&(inodelist[inodeidx]), i, j);
            if(res == 0)continue;
            releaseBlock(res);
        }
    }
    inodelist[inodeidx].baseblknum = 0;
}

void releaseBlock(int blk_num)
{
    int idx_num = blk_num / (BLOCK_SIZE << 3);
    int inner_num = (blk_num & ((BLOCK_SIZE << 3) - 1));
    resetBit(&(bitmap.indexblk[idx_num]), inner_num);
    putBlock(&(bitmap.indexblk[idx_num]), idx_num);
    flushCache(idx_num);
    bitmap.placeleft[idx_num]++;
    if(bitmap.placeleft[idx_num] != 0)
    {
        resetBit(&(bitmap.zeroblk), idx_num);
        putBlock(&(bitmap.zeroblk), 0);
        flushCache(0);
    }
}

void decreasenNlink(int inodeidx)
{
    // check nlink, if 0 delete, else decrease nlink
    if(inodelist[inodeidx].nlink == 1)releaseInode(inodeidx);
    else
    {
        int decreasednlink = inodelist[inodeidx].nlink - 1;
        updateNlinkInode(&(inodelist[inodeidx]), decreasednlink);
        putBlock(&(inodelist[inodeidx].baseblk), inodelist[inodeidx].baseblknum);
        flushCache(inodelist[inodeidx].baseblknum);
    }
}