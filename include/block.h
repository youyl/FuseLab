#ifndef FS_BLOCK
#define FS_BLOCK

#include "fuse_utils.h"

#define BLOCK_SIZE 512
#define MAX_BLOCK_NUM 1048576

struct Block
{
    char data[BLOCK_SIZE];
};

struct List
{
    int num;
    struct List *nxt;
};

struct BlockController
{
    char *data;
    struct List *head;
    struct List *tail;
};

struct BlockController controller;

void controllerInit(char *);
void readBlock(struct Block *, int);
void writeBlock(const struct Block *, int);
void flushBlock(int);
int allocBlock();
void releaseBlock(int);


void reportError(char *);

#endif