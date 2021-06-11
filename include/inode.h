#ifndef FS_INODE
#define FS_INODE

#include "cache.h"

#define MAX_INODE_COUNT 8192
#define DIR_INODE_TYPE 1
#define FILE_INODE_TYPE 2

/*
block orgnization: 512 byte 4096 bit
block 0: 1023 bit to block 1-1023, 3073 bit reserved
*/
#define ZERO_BLK_START 1
#define ZERO_BLK_END 1024
/*
block 1-1023: 4096 bit to block (blknum*4096, blknum*4096+4095)
block 1024-4095: reserved
block 4096+: data block
*/
#define ROOTDIR_BLK 4096
/*
total block 4096*2048, 2**12*2**11, 23bit
dir block: type mode nlink size time 4 + 4 + 4 + 4 + 8 = 24 
file block: type mode nlink size time 4 + 4 + 4 + 4 + 8 = 24
+ idx 488 rem = 122 block
secondary index 32 * 128 = 128 blocks
*/
#define INDEX_BLOCK_COUNT 1024
#define SECONDARY_INDEX_COUNT 122
#define FILE_TYPE_OFFSET 0
#define FILE_MODE_OFFSET 1
#define FILE_NLINK_OFFSET 2
#define FILE_SIZE_OFFSET 3
#define FILE_TIME_OFFSET 4
#define FILE_INDEX_OFFSET 6
/*
dir data: name block 4 entry per block
dir: not 2 level
name 124 byte
block 4 byte
*/
#define DIRENT_BLKNUM_OFFSET 31
#define DIRENT_SIZE 32
#define DIRENT_PER_BLOCK 4
#define NAME_LENGTH_LIMIT 120
#define ROOTDIR_IDX 0
#define PATH_LENGTH_LIMIT 1800

struct SuperBlock
{
    struct Block zeroblk;
    int placeleft[INDEX_BLOCK_COUNT];
    struct Block indexblk[INDEX_BLOCK_COUNT];
};

struct Inode
{
    // int dev;
    int type;
    int nlink;
    int mode;
    struct timespec timec;
    int baseblknum;
    struct Block baseblk;
    int indexblkidx[SECONDARY_INDEX_COUNT];
    struct Block indexblk[SECONDARY_INDEX_COUNT];
    int siz;
    int blkcnt;
};

struct SuperBlock bitmap;
int inodecnt;
struct Inode inodelist[MAX_INODE_COUNT];

int allocBlock();
void releaseBlock(int);
void inodeInit();
void updateTimeInode(struct Inode *);
// void updateNlinkInode(struct Inode *, int);
void updateModeInode(struct Inode *, int);
void retrieveInode(struct Inode *, int);
int searchDir(const struct Inode *, const char *);
int findInode(const struct Inode *, const char *);
int locateBlock(const struct Inode *, int);

int createInode(struct Inode *, int, int);

// file utils
int getIndex(const struct Inode *, int); // both for dir and file
int getSecondaryIndex(const struct Inode *, int, int); // only for file
void setIndex(struct Inode *, int, int);
void setSecondaryIndex(struct Inode *, int, int, int);
int newBlock(struct Inode *);

// dir utils
int getSecondaryDirent(const struct Inode *, int, int); // only for file
int newDirent(struct Inode *, const char *, int, int, int);

int putInodetoList(int);

void flushInode(const struct Inode *);
void releaseInode(int);

#endif