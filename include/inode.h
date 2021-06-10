#ifndef FS_INODE
#define FS_INODE

#include "cache.h"

#define MAX_INODE_COUNT 8192
#define DIR_INODE_TYPE 1
#define FILE_INODE_TYPE 2

/*
block orgnization: 512 byte 4096 bit
block 0: 2047 bit to block 1-2047, 2049 bit reserved
block 1-2047: 4096 bit to block (blknum*4096, blknum*4096+4095)
block 2048-4095: reserved
block 4096+: data block
total block 4096*2048, 2**12*2**11, 23bit
dir block: type mode nlink size time 4 + 4 + 4 + 4 + 8 = 24 
file block: type mode nlink size time 4 + 4 + 4 + 4 + 8 = 24
+ idx 488 rem = 122 block
secondary index 32 * 128 = 128 blocks
dir data: name block 4 entry per block
name 124 byte
block 4 byte
*/

#define INDEX_BLOCK_COUNT 2048
#define SECONDARY_INDEX_COUNT 122
#define FILE_TYPE_OFFSET 0
#define FILE_MODE_OFFSET 1
#define FILE_NLINK_OFFSET 2
#define FILE_SIZE_OFFSET 3
#define FILE_TIME_OFFSET 4
#define FILE_INDEX_OFFSET 6
#define NAME_LENGTH_LIMIT 124
#define ROOTDIR_IDX 0

struct SuperBlock
{
    struct Block zeroblk;
    struct Block indexblk[INDEX_BLOCK_COUNT];
};

struct Inode
{
    int dev;
    int type;
    // int nlink;
    int mode;
    struct timespec timec;
    int baseblknum;
    struct Block baseblk;
    int indexblkidx[SECONDARY_INDEX_COUNT];
    struct Block indexblk[SECONDARY_INDEX_COUNT];
};

struct SuperBlock bitmap;
struct Inode inodelist[MAX_INODE_COUNT];

int allocBlock();
void inodeInit();
void updateTimeInode(struct Inode *);
// void updateNlinkInode(struct Inode *, int);
void updateModeInode(struct Inode *, int);
void retrieveInode(struct Inode *, int);
int searchDir(const struct Inode *, const char *);
int findInode(const char *);
int locateBlock(const struct Inode *, int, struct Block *);

#endif