#include "../include/interface.h"

static struct fuse_operations fuse_operations = {
  .open = my_open,
  .create = my_create,
  .chmod = my_chmod,
  .getattr = my_getattr,
  .read = my_read,
  .write = my_write,
  .fsync = my_fsync,
  .readdir = my_readdir,
  .mkdir = my_mkdir,
  .rename = my_rename,
  .release = my_release,
  .rmdir = my_rmdir,
  .link = my_link,
   .unlink = my_unlink,
};

int main(int argc, char *argv[])
{
  controllerInit(argv[1]);
  cacheInit();
  inodeInit();
  interfaceInit();
  return fuse_main(argc, argv, &fuse_operations, NULL);
}