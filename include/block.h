#ifndef FS_BLOCK
#define FS_BLOCK

#include "fuse_utils.h"

#define BLOCK_SIZE 512u
#define MAX_BLOCK_NUM 4194304u

struct Block
{
    char data[BLOCK_SIZE];
};

struct BlockController
{
    char *data;
    bool created;
};

struct BlockController controller;

void controllerInit(char *);
void readBlock(char *, int);
void writeBlock(const char *, int);

void reportError(char *);
int getBit(const struct Block *, int);
void setBit(struct Block *, int);
void resetBit(struct Block *, int);

#endif