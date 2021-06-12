#include "../include/interface.h"

char s[20] = "0123456789";

void create_open_test()
{
    int fd1 = open("/tmp/fs/create_open_test_1", O_CREAT | O_RDWR, 0644);
    close(fd1);
    int fd2 = open("/tmp/fs/create_open_test_2", O_CREAT | O_RDWR, 0755);
    close(fd2);
}

void mkdir_rmdir_test()
{}

void read_write_test()
{}

void link_unlink_test()
{}

void rename_test()
{}

void large_file_test()
{}

void multi_file_test()
{}

int main(int argc, char *argv[])
{
    // create_open_test(); printf("create_open_test over\n");
    // mkdir_rmdir_test(); printf("mkdir_rmdir_test over\n");
    // read_write_test(); printf("read_write_test over\n");
    // link_unlink_test(); printf("link_unlink_test over\n");
    // rename_test(); printf("rename_test over\n");
    // large_file_test(); printf("large_file_test over\n");
    // multi_file_test(); printf("multi_file_test over\n");
    return 0;
}