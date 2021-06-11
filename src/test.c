#include "../include/interface.h"

char s[20] = "0123456789";

int main(int argc, char *argv[])
{
    long a;
    printf("long size %d\n",sizeof(a));
    /*
    controllerInit("test");
    cacheInit();
    validBlock(1);
    s[10] = 0;
    memcpy(globalcache.data[1], s, 12);
    setDirty(1);
    flushCache(1);
    */
    int x = open("/tmp/fs/file", O_RDWR, 0644);
    printf("%d %d\n",x,errno);
    return 0;
}