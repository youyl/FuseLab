#include "../include/interface.h"

int my_open(const char *path, struct fuse_file_info *fi)
{
    // int res = findInode(&(inodelist[ROOTDIR_IDX]), path);
    // if(res < 0)return res;
    return 0;
}

int my_create(const char *path, mode_t md, struct fuse_file_info *fi)
{
    return 0;
}

int my_read(const char *path, char *buf, size_t sz, off_t offset, struct fuse_file_info *fi)
{
    // not supported
    return 0;
}

int my_write(const char *path, const char *data, size_t sz, off_t offset, struct fuse_file_info *fi)
{
    // not supported
    return 0;
}

int my_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
    // not supported
    return 0;
}

int my_getattr(const char *path, struct stat *st)
{
    // int res = findInode(&(inodelist[ROOTDIR_IDX]), path);
    // if(res < 0)return res;
    return 0;
}

int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    return 0;
}

int my_mkdir(const char *path, mode_t md)
{
    return 0;
}

int my_rename(const char *oldname, const char *newname)
{
    return 0;
}

int my_chmod(const char *path, mode_t md)
{
    return 0;
}

int my_rmdir(const char *path)
{
    return 0;
}

int my_release(const char *path, struct fuse_file_info *fi)
{
    return 0;
}

int my_unlink(const char *path)
{
    return 0;
}

/*
int my_link(const char *path1, const char *path2)
{
    return 0;
}
*/