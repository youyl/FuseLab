#include "../include/interface.h"
#include <time.h>
#include <assert.h>

char buf[32];
char s[32] = "11112222333344445555666677778888";

#define write_size 65536

void perf_test()
{
    int fd = open("/tmp/fs/file", O_CREAT | O_RDWR, 0644);
    for (int i = 0; i < write_size; i++)
    {
        int res = write(fd, s, 32);
        assert(res == 32);
    }
    close(fd);
    fd = open("/tmp/fs/file", O_CREAT | O_RDWR, 0644);
    for (int i = 0; i < write_size; i++)
    {
        memset(buf, 0 ,sizeof(buf));
        int res = read(fd, buf, 32);
        assert(res == 32);
        assert(strcmp(buf, s) == 0);
    }
    close(fd);
}

uint64_t current_ts_msec() {
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC, &tp);
  return tp.tv_sec * 1000 + tp.tv_nsec / 1000 / 1000;
}

int main()
{
    uint64_t starttime = current_ts_msec();
    // perf_test();
    for (int i = 0; i < 32; i++)
    {
        perf_test();
        printf("round %d complete\n",i);
    }
    uint64_t finishtime = current_ts_msec();
    printf("time cost = %ld ms\n",finishtime - starttime);
    return 0;
}