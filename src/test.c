#include "../include/interface.h"

char s[20] = "0123456789";
char buf[1000];

void create_open_test()
{
    int fd1 = open("/tmp/fs/create_open_test_1", O_CREAT | O_RDWR, 0644);
    close(fd1);
    int fd2 = open("/tmp/fs/create_open_test_2", O_CREAT | O_RDWR, 0755);
    close(fd2);
}

void mkdir_rmdir_test()
{
    mkdir("/tmp/fs/mkdir_test_dir", 0644);
}

void read_write_test()
{
    mkdir("/tmp/fs/rw_test_dir", 0644);
    rmdir("/tmp/fs/mkdir_test_dir");
    // write
    int fd = open("/tmp/fs/rw_test_dir/rw_test", O_CREAT | O_RDWR, 0755);
    write(fd, s, 10);
    memset(buf, 0, 1000);
    close(fd);
    // read
    fd = open("/tmp/fs/rw_test_dir/rw_test", O_RDWR);
    int res = read(fd, buf, 10);
    printf("read result res = %d\n",res);
    printf("read result str = %s\n",buf);
    close(fd);
}

void link_unlink_test()
{
    mkdir("/tmp/fs/link_dir", 0644);
    link("/tmp/fs/rw_test_dir/rw_test","/tmp/fs/link_dir/link_test");
}

void rename_test()
{
    unlink("/tmp/fs/link_dir/link_test");
    mkdir("/tmp/fs/name_dir_1", 0644);
    mkdir("/tmp/fs/name_dir_2", 0644);
    int fd = open("/tmp/fs/name_dir_1/file", O_CREAT | O_RDWR, 0755);
    close(fd);
}

void large_file_test()
{
    rename("/tmp/fs/name_dir_1/file", "/tmp/fs/name_dir_2/file");
    int fd = open("/tmp/fs/large_file", O_CREAT | O_RDWR, 0755);
    for (int i = 0; i < 512; i++)
    {
        write(fd, s, 10);
    }
    close(fd);
}

void authority_test()
{
    int fd = open("/tmp/fs/file_auth", O_CREAT | O_RDWR, 0444);
    int res = write(fd, s, 10);
    printf("auth_test(ouid) res = %d\n", res);
    close(fd);
    fd = open("/tmp/fs/file_auth_all", O_CREAT | O_RDWR, 0644);
    res = write(fd, s, 10);
    printf("auth_test(ouid) res = %d\n", res);
    close(fd);
}

int main(int argc, char *argv[])
{
    create_open_test(); printf("create_open_test over\n");
    mkdir_rmdir_test(); printf("mkdir_rmdir_test over\n");
    read_write_test(); printf("read_write_test over\n");
    link_unlink_test(); printf("link_unlink_test over\n");
    rename_test(); printf("rename_test over\n");
    large_file_test(); printf("large_file_test over\n");
    authority_test(); printf("authority_test over\n");
    return 0;
}