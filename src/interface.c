#include "../include/interface.h"

int my_open(const char *path, struct fuse_file_info *fi)
{
    printf("Fuse syscall: open: %s flag = %d\n", path, fi->flags);
    return 0;
}

int my_create(const char *path, mode_t md, struct fuse_file_info *fi)
{
    printf("Fuse syscall: create: %s flag = %d mode = %d\n", path, fi->flags, md);
    return 0;
}

int my_read(const char *path, char *buf, size_t sz, off_t offset, struct fuse_file_info *fi)
{
    printf("Fuse syscall: read: %s siz = %ld offset = %ld\n", path, sz, offset);
    return 0;
}

int my_write(const char *path, const char *data, size_t sz, off_t offset, struct fuse_file_info *fi)
{
    printf("Fuse syscall: write: %s siz = %ld offset = %ld\n", path, sz, offset);
    return 0;
}

int my_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
    printf("Fuse syscall: fsync: %s isdatasync = %d\n", path, isdatasync);
    return 0;
}

int my_getattr(const char *path, struct stat *st)
{
    printf("Fuse syscall: getattr: %s\n", path);
    return 0;
}

int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    printf("Fuse syscall: readdir: %s\n", path);
    return 0;
}

int my_mkdir(const char *path, mode_t md)
{
    printf("Fuse syscall: mkdir: %s mode = %d\n", path, md);
    return 0;
}

int my_rename(const char *oldname, const char *newname)
{
    printf("Fuse syscall: rename: %s %s\n", oldname, newname);
    return 0;
}

int my_chmod(const char *path, mode_t md)
{
    printf("Fuse syscall: chmod: %s mode = %d\n", path, md);
    return 0;
}

int my_rmdir(const char *path)
{
    printf("Fuse syscall: rmdir: %s\n", path);
    return 0;
}

int my_release(const char *path, struct fuse_file_info *fi)
{
    printf("Fuse syscall: release: %s\n", path);
    return 0;
}