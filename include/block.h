#ifndef FS_BLOCK
#define FS_BLOCK

#include "fuse_utils.h"

#define BLOCK_SIZE 512u
#define MAX_BLOCK_NUM 8388608u

struct Block
{
    char data[BLOCK_SIZE];
};

struct BlockController
{
    char *data;
};

struct BlockController controller;

void controllerInit(char *);
void readBlock(char *, int);
void writeBlock(const char *, int);

void reportError(char *);

#endif