#ifndef FS_INTERFACE
#define FS_INTERFACE

#include "inode.h"

// organize fd
// inode ref cnt
#define MAX_FD_COUNT 8192

int fd2inode[MAX_FD_COUNT];
int fdflag[MAX_FD_COUNT];
int fdcnt;

void interfaceInit();
int getFD(int, int);

int my_open(const char *, struct fuse_file_info *);
int my_create(const char *, mode_t, struct fuse_file_info *);
int my_read(const char *, char *, size_t, off_t, struct fuse_file_info *);
int my_write(const char *, const char *, size_t, off_t, struct fuse_file_info *);
int my_fsync(const char *, int, struct fuse_file_info *);
int my_getattr(const char *, struct stat *);
int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int my_mkdir(const char *, mode_t);
int my_rename(const char *, const char *);
int my_chmod(const char *, mode_t);
int my_rmdir(const char *);
int my_release(const char *, struct fuse_file_info *);
int my_unlink(const char *);
int my_link(const char *, const char *);

#endif