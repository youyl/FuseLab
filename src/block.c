#include "../include/block.h"

void reportError(char *str)
{
    printf("Error: %s\n",str);
    exit(0);
}

void controllerInit(char *path)
{
    // create filename
    if(strlen(path) > 200)reportError("too long path");
    char newpath[220];
    int ll = strlen(path);
    memcpy(newpath, path, ll);
    newpath[ll] = '/';
    newpath[ll + 1] = 'd';
    newpath[ll + 2] = 'a';
    newpath[ll + 3] = 't';
    newpath[ll + 4] = 0;
    ll += 4;
    printf("File System on %s\n", newpath);

    // file creation
    int fd = open(newpath, O_RDWR, 0644);
    bool created = false;
    if(fd < 0 && errno == ENOENT)
    {
        fd = open(newpath, O_RDWR | O_CREAT, 0644);
        if(fd < 0)reportError("file creation failed\n");
        if(posix_fallocate(fd, 0, BLOCK_SIZE * MAX_BLOCK_NUM) != 0)reportError("not enough room");
        created = true;
    }
    void *ptr = mmap(NULL, BLOCK_SIZE * MAX_BLOCK_NUM, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(ptr == MAP_FAILED)reportError("mmap failed");
    if(created)memset(ptr, 0, BLOCK_SIZE * MAX_BLOCK_NUM);
    controller.data = ptr;

    // block init
    controller.head = (struct List *)malloc(sizeof(struct List));
    controller.head->num = 0;
    controller.head->nxt = NULL;
    struct List *cur = controller.head;
    for (int i = 1; i < MAX_BLOCK_NUM; i++)
    {
        cur->nxt = (struct List *)malloc(sizeof(struct List));
        cur->nxt->num = i;
        cur->nxt->nxt = NULL;
        controller.tail = cur;
        cur = cur->nxt;
    }
}

void readBlock(char *blk, int blk_num)
{
    memcpy(blk, controller.data + (blk_num * BLOCK_SIZE), BLOCK_SIZE);
}

void writeBlock(const char *blk, int blk_num)
{
    memcpy(controller.data + (blk_num * BLOCK_SIZE), blk, BLOCK_SIZE);
}

void flushBlock(int blk_num)
{
    msync(controller.data + (blk_num * BLOCK_SIZE), BLOCK_SIZE, MS_SYNC);
}

int allocBlock()
{
    if(controller.head == controller.tail)reportError("not enough block");
    int res = controller.head->num;
    struct List *ptr = controller.head;
    controller.head = controller.head->nxt;
    free(ptr);
    return res;
}

void releaseBlock(int blk_num)
{
    controller.tail->nxt = (struct List *)malloc(sizeof(struct List));
    controller.tail = controller.tail->nxt;
    controller.tail->num = blk_num;
    controller.tail->nxt = NULL;
}
