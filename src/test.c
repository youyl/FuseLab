#include "../include/interface.h"

char s[20] = "0123456789";

int main(int argc, char *argv[])
{
    controllerInit("test");
    cacheInit();
    validBlock(1);
    s[10] = 0;
    memcpy(globalcache.data[1], s, 12);
    setDirty(1);
    flushCache(1);
    return 0;
}