#include "../include/interface.h"

void interfaceInit()
{
    fdcnt = 2;
}

int getFD(int blknum, int flag)
{
    fdcnt++;
    if(fdcnt == MAX_FD_COUNT)return -EMFILE;
    fd2inode[fdcnt] = blknum;
    fdflag[fdcnt] = flag;
    return fdcnt;
}

int my_open(const char *path, struct fuse_file_info *fi)
{
    if(strlen(path) > PATH_LENGTH_LIMIT)return -ENAMETOOLONG;
    printf("Fuse syscall: open: %s\n",path);
    int res = findInode(&(inodelist[ROOTDIR_IDX]), path);
    // printf("find Inode res: %d\n",res);
    if(res < 0)return res;
    if(inodelist[res].type == DIR_INODE_TYPE)return -EISDIR;
    fi->fh = getFD(inodelist[res].baseblknum, fi->flags);
    printf("opened success, return fd = %ld\n",fi->fh);
    return 0;
}

int my_create(const char *path, mode_t md, struct fuse_file_info *fi)
{
    if(strlen(path) > PATH_LENGTH_LIMIT)return -ENAMETOOLONG;
    printf("Fuse syscall: create: %s\n",path);
    // try to open exist
    int res = findInode(&(inodelist[ROOTDIR_IDX]), path);
    // printf("find Inode res: %d\n",res);
    if(res >= 0)printf("already exist, call my_open\n");
    if(res < 0 && res != -ENOENT)return res;
    if(res >= 0)return my_open(path, fi);
    // get new path
    char newpath[2000];
    int tail = strlen(path) - 1;
    if(path[tail] == '/')tail--;
    while (tail > 0)
    {
        if(path[tail] == '/')break;
        tail--;
    }
    memset(newpath, 0, sizeof(newpath));
    memcpy(newpath, path, tail);
    char name[200];
    memset(name, 0, 200);
    memcpy(name, path + tail + 1, strlen(path) - tail -1);
    // printf("get newpath and name: %s, %s\n",newpath, name);
    // find new path
    res = findInode(&(inodelist[ROOTDIR_IDX]), newpath);
    // printf("find father Inode res: %d\n",res);
    if(res < 0)return res;
    // add dirent
    if(inodelist[res].type != DIR_INODE_TYPE)return -ENOTDIR;
    res = newDirent(&(inodelist[res]), name, FILE_INODE_TYPE, md, -1);
    if(res < 0)return res;
    return my_open(path, fi);
}

int my_read(const char *path, char *buf, size_t sz, off_t offset, struct fuse_file_info *fi)
{
    if(fi == NULL)if(strlen(path) > PATH_LENGTH_LIMIT)return -ENAMETOOLONG;
    printf("Fuse syscall: read: %s\n",path);
    // check can read (fi)
    int blk = -1, res = -ENOENT;
    if(fi != NULL)
    {
        if(((fi->flags & O_RDWR) == O_RDWR) || ((fi->flags & O_RDONLY) == O_RDONLY))blk = fd2inode[fi->fh];
        else return -EPERM;
        res = putInodetoList(blk);
    }
    else res = findInode(&(inodelist[ROOTDIR_IDX]), path);
    if(res < 0)return res;
    if(inodelist[res].type == DIR_INODE_TYPE)return -EISDIR;
    if(!checkAuthority(&(inodelist[res]), true))return -EPERM;
    // read like send packet
    if(sz + offset > inodelist[res].siz)sz = inodelist[res].siz - offset;
    int remlength = sz;
    int blockstart = (offset & (BLOCK_SIZE - 1));
    int blocksiz;
    int curptr = 0;
    while (remlength != 0)
    {
        blocksiz = BLOCK_SIZE - blockstart;
        if(blocksiz > remlength)blocksiz = remlength;
        int curblk = locateBlock(&(inodelist[res]), offset);
        struct Block thisblk;
        // printf("read: read by send packet: blockstart = %d blocksiz = %d curblk = %d\n",blockstart, blocksiz, curblk);
        getBlock(&thisblk, curblk);
        // printf("%s\n", thisblk.data);
        memcpy(buf + curptr, thisblk.data + blockstart, blocksiz);
        curptr += blocksiz;
        blockstart = 0;
        remlength -= blocksiz;
        offset += blocksiz;
    }
    printf("read: result: %ld %s\n",sz, buf);
    updateTimeInode(&(inodelist[res]));
    putBlock(&(inodelist[res].baseblk), inodelist[res].baseblknum);
    flushCache(inodelist[res].baseblknum);
    return sz;
}

int my_write(const char *path, const char *data, size_t sz, off_t offset, struct fuse_file_info *fi)
{
    if(fi == NULL)if(strlen(path) > PATH_LENGTH_LIMIT)return -ENAMETOOLONG;
    printf("Fuse syscall: write: %s %ld %ld\n",path,sz,offset);
    // check can write (fi)
    int blk = -1, res = -ENOENT;
    if(fi != NULL)
    {
        if(((fi->flags & O_RDWR) == O_RDWR) || ((fi->flags & O_WRONLY) == O_WRONLY))blk = fd2inode[fi->fh];
        else return -EPERM;
        res = putInodetoList(blk);
    }
    else res = findInode(&(inodelist[ROOTDIR_IDX]), path);
    printf("write: find inode: %d\n",res);
    if(res < 0)return res;
    if(inodelist[res].type == DIR_INODE_TYPE)return -EISDIR;
    if(!checkAuthority(&(inodelist[res]), false))return -EPERM;
    // update sz
    int newsz = offset + sz;
    while (newsz > inodelist[res].blkcnt * BLOCK_SIZE)newBlock(&(inodelist[res]));
    if(inodelist[res].siz < newsz)
    {
        updateSizInode(&(inodelist[res]), newsz);
        putBlock(&(inodelist[res].baseblk), inodelist[res].baseblknum);
        flushCache(inodelist[res].baseblknum);
    }
    // write like send packet
    printf("write: size after expansion: %d\n",inodelist[res].siz);
    int remlength = sz;
    int blockstart = (offset & (BLOCK_SIZE - 1));
    int blocksiz;
    int curptr = 0;
    while (remlength != 0)
    {
        blocksiz = BLOCK_SIZE - blockstart;
        if(blocksiz > remlength)blocksiz = remlength;
        int curblk = locateBlock(&(inodelist[res]), offset);
        struct Block thisblk;
        // printf("write: write by send packet: blockstart = %d blocksiz = %d\n",blockstart, blocksiz);
        getBlock(&thisblk, curblk);
        memcpy(thisblk.data + blockstart, data + curptr, blocksiz);
        putBlock(&thisblk, curblk);
        curptr += blocksiz;
        blockstart = 0;
        remlength -= blocksiz;
        offset += blocksiz;
    }
    updateTimeInode(&(inodelist[res]));
    putBlock(&(inodelist[res].baseblk), inodelist[res].baseblknum);
    flushCache(inodelist[res].baseblknum);
    return sz;
}

int my_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
    if(fi == NULL)if(strlen(path) > PATH_LENGTH_LIMIT)return -ENAMETOOLONG;
    printf("Fuse syscall: fsync: %s\n",path);
    // find inode
    int res = findInode(&(inodelist[ROOTDIR_IDX]), path);
    // printf("fsync: find Inode res: %d\n",res);
    if(res < 0)return res;
    if(inodelist[res].type == DIR_INODE_TYPE)return -EISDIR;
    // flush file content
    flushInode(&(inodelist[res]));
    return 0;
}

int my_getattr(const char *path, struct stat *st)
{
    if(strlen(path) > PATH_LENGTH_LIMIT)return -ENAMETOOLONG;
    printf("Fuse syscall: getattr: %s\n",path);
    int res = findInode(&(inodelist[ROOTDIR_IDX]), path);
    if(res < 0)return res;
    memset(st, 0, sizeof(struct stat));
    // mode
    st->st_mode = inodelist[res].mode;
    if(inodelist[res].type == DIR_INODE_TYPE)st->st_mode |= S_IFDIR;
    else st->st_mode |= S_IFREG;
    // nlink
    st->st_nlink = 2;
    // time
    st->st_ctim = inodelist[res].timec;
    st->st_atim = inodelist[res].timec;
    st->st_mtim = inodelist[res].timec;
    // printf("getattr: gettime: %ld %ld\n",inodelist[res].timec.tv_sec, inodelist[res].timec.tv_nsec);
    // siz
    st->st_size = inodelist[res].siz;
    // user
    st->st_uid = inodelist[res].uid;
    st->st_gid = inodelist[res].gid;
    return 0;
}

int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    if(strlen(path) > PATH_LENGTH_LIMIT)return -ENAMETOOLONG;
    printf("Fuse syscall: readdir: %s\n",path);
    int res = findInode(&(inodelist[ROOTDIR_IDX]), path);
    // printf("find Inode res: %d\n",res);
    if(res < 0)return res;
    if(inodelist[res].type != DIR_INODE_TYPE)return -ENOTDIR;
    // add normal files
    char newpath[2000];
    memset(newpath, 0, sizeof(newpath));
    memcpy(newpath, path, strlen(path));
    for (int i = 0; i < SECONDARY_INDEX_COUNT; i++)
    {
        if(inodelist[res].indexblkidx[i] == 0)continue;
        for (int j = 0; j < DIRENT_PER_BLOCK; j++)
        {
            if(getSecondaryDirent(&(inodelist[res]), i, j) == 0)continue;
            char *ptr = (char *)(inodelist[res].indexblk[i].data + (DIRENT_SIZE * 4 * j));
            struct stat stbuf;
            memcpy(newpath + strlen(path), ptr, strlen(ptr) + 1);
            // printf("get dir entry newpath: %s\n", newpath);
            my_getattr(newpath, &stbuf);
            filler(buf, ptr, &stbuf, 0);
        }
    }
    // add .
    struct stat stbuf;
    res = my_getattr(path, &stbuf);
    if(res < 0)return res;
    filler(buf, ".", &stbuf, 0);
    // add ..
    if(strcmp(path, "/") == 0 || strlen(path) == 0)filler(buf, "..", &stbuf, 0);
    else
    {
        int tail = strlen(path) - 1;
        if(path[tail] == '/')tail--;
        while (tail > 0)
        {
            if(path[tail] == '/')break;
            tail--;
        }
        memset(newpath, 0, sizeof(newpath));
        memcpy(newpath, path, tail);
        // printf("get newpath: %s\n",newpath);
        res = my_getattr(path, &stbuf);
        if(res < 0)return res;
        filler(buf, "..", &stbuf, 0);
    }
    return 0;
}

int my_mkdir(const char *path, mode_t md)
{
    if(strlen(path) > PATH_LENGTH_LIMIT)return -ENAMETOOLONG;
    printf("Fuse syscall: mkdir: %s\n",path);
    int res = findInode(&(inodelist[ROOTDIR_IDX]), path);
    // printf("find Inode res: %d\n",res);
    if(res >= 0)return -EEXIST;
    // get new path
    char newpath[2000];
    int tail = strlen(path) - 1;
    if(path[tail] == '/')tail--;
    while (tail > 0)
    {
        if(path[tail] == '/')break;
        tail--;
    }
    memset(newpath, 0, sizeof(newpath));
    memcpy(newpath, path, tail);
    char name[200];
    memset(name, 0, 200);
    memcpy(name, path + tail + 1, strlen(path) - tail -1);
    // printf("get newpath and name: %s, %s\n",newpath, name);
    // find new path
    res = findInode(&(inodelist[ROOTDIR_IDX]), newpath);
    // printf("find father Inode res: %d\n",res);
    if(res < 0)return res;
    // add dirent
    if(inodelist[res].type != DIR_INODE_TYPE)return -ENOTDIR;
    res = newDirent(&(inodelist[res]), name, DIR_INODE_TYPE, md, -1);
    if(res < 0)return res;
    return 0;
}

int my_rename(const char *oldname, const char *newname)
{
    printf("Fuse syscall: rename: %s, %s\n", oldname, newname);
    // link new name
    int res = my_link(oldname, newname);
    if(res < 0)return res;
    // unlink old name
    res = my_unlink(oldname);
    if(res < 0)return res;
    return 0;
}

int my_chmod(const char *path, mode_t md)
{
    if(strlen(path) > PATH_LENGTH_LIMIT)return -ENAMETOOLONG;
    printf("Fuse syscall: chmod: %s\n",path);
    // find inode
    int res = findInode(&(inodelist[ROOTDIR_IDX]), path);
    // printf("find Inode res: %d\n",res);
    if(res < 0)return res;
    // change mode and flush
    updateModeInode(&(inodelist[res]), md);
    updateTimeInode(&(inodelist[res]));
    putBlock(&(inodelist[res].baseblk), inodelist[res].baseblknum);
    flushCache(inodelist[res].baseblknum);
    return 0;
}

int my_rmdir(const char *path)
{
    if(strlen(path) > PATH_LENGTH_LIMIT)return -ENAMETOOLONG;
    printf("Fuse syscall: rmdir: %s\n",path);
    int res = findInode(&(inodelist[ROOTDIR_IDX]), path);
    // printf("find Inode res: %d\n",res);
    if(res < 0)return res;
    if(inodelist[res].type != DIR_INODE_TYPE)return -ENOTDIR;
    int orires = res;
    // find newpath
    char newpath[2000];
    int tail = strlen(path) - 1;
    if(path[tail] == '/')tail--;
    while (tail > 0)
    {
        if(path[tail] == '/')break;
        tail--;
    }
    memset(newpath, 0, sizeof(newpath));
    memcpy(newpath, path, tail);
    char name[200];
    memset(name, 0, 200);
    memcpy(name, path + tail + 1, strlen(path) - tail -1);
    // printf("get newpath and name: %s, %s\n",newpath, name);
    // walk new path
    res = findInode(&(inodelist[ROOTDIR_IDX]), newpath);
    // printf("find father Inode res: %d\n",res);
    if(res < 0)return res;
    if(inodelist[res].type != DIR_INODE_TYPE)return -ENOTDIR;
    // find dirent and remove
    bool found = false;
    for (int i = 0; i < SECONDARY_INDEX_COUNT; i++)
    {
        if(inodelist[res].indexblkidx[i] == 0)break;
        for (int j = 0; j < DIRENT_PER_BLOCK; j++)
        {
            if(getSecondaryDirent(&(inodelist[res]), i, j) == 0)continue;
            if(strcmp(inodelist[res].indexblk[i].data + (DIRENT_SIZE * 4 * j), name) == 0)
            {
                int *ptr = (int *)(inodelist[res].indexblk[i].data);
                ptr[j * DIRENT_SIZE + DIRENT_BLKNUM_OFFSET] = 0;
                putBlock(&(inodelist[res].indexblk[i]), inodelist[res].indexblkidx[i]);
                flushCache(inodelist[res].indexblkidx[i]);
                found = true;
                break;
            }
        }
        if(found)break;
    }
    if(found == false)return -ENOENT;
    // remove inode
    releaseInode(orires);
    return 0;
}

int my_release(const char *path, struct fuse_file_info *fi)
{
    printf("Fuse syscall: release: %ld\n", fi->fh);
    // flush
    if(fi == NULL)return -ENOENT;
    if(fi->fh < 3)return -EBADFD;
    int blk_num = fd2inode[fi->fh];
    for (int i = 0; i < inodecnt; i++)
    {
        if(inodelist[i].baseblknum == blk_num)
        {
            flushInode(&(inodelist[i]));
            return 0;
        }
    }
    return -ENOENT;
}

int my_unlink(const char *path)
{
    if(strlen(path) > PATH_LENGTH_LIMIT)return -ENAMETOOLONG;
    printf("Fuse syscall: unlink: %s\n",path);
    int res = findInode(&(inodelist[ROOTDIR_IDX]), path);
    // printf("find Inode res: %d\n",res);
    if(res < 0)return res;
    int desblk = res;
    if(inodelist[res].type != FILE_INODE_TYPE)return -EISDIR;
    // get new path
    char newpath[2000];
    int tail = strlen(path) - 1;
    if(path[tail] == '/')tail--;
    while (tail > 0)
    {
        if(path[tail] == '/')break;
        tail--;
    }
    memset(newpath, 0, sizeof(newpath));
    memcpy(newpath, path, tail);
    char name[200];
    memset(name, 0, 200);
    memcpy(name, path + tail + 1, strlen(path) - tail -1);
    // printf("get newpath and name: %s, %s\n",newpath, name);
    // find dirent and remove
    res = findInode(&(inodelist[ROOTDIR_IDX]), newpath);
    // printf("find father Inode res: %d\n",res);
    if(res < 0)return res;
    if(inodelist[res].type != DIR_INODE_TYPE)return -ENOTDIR;
    bool found = false;
    for (int i = 0; i < SECONDARY_INDEX_COUNT; i++)
    {
        if(inodelist[res].indexblkidx[i] == 0)break;
        for (int j = 0; j < DIRENT_PER_BLOCK; j++)
        {
            if(getSecondaryDirent(&(inodelist[res]), i, j) == 0)continue;
            if(strcmp(inodelist[res].indexblk[i].data + (DIRENT_SIZE * 4 * j), name) == 0)
            {
                int *ptr = (int *)(inodelist[res].indexblk[i].data);
                ptr[j * DIRENT_SIZE + DIRENT_BLKNUM_OFFSET] = 0;
                putBlock(&(inodelist[res].indexblk[i]), inodelist[res].indexblkidx[i]);
                flushCache(inodelist[res].indexblkidx[i]);
                found = true;
                break;
            }
        }
        if(found)break;
    }
    if(found == false)return -ENOENT;
    // reduce nlink cnt
    decreasenNlink(desblk);
    return 0;
}


int my_link(const char *frompath, const char *topath) // from is existed
{
    if(strlen(frompath) > PATH_LENGTH_LIMIT)return -ENAMETOOLONG;
    if(strlen(topath) > PATH_LENGTH_LIMIT)return -ENAMETOOLONG;
    printf("Fuse syscall: link: %s, %s\n",frompath, topath);
    // find to frompath inode
    int res = findInode(&(inodelist[ROOTDIR_IDX]), frompath);
    // printf("find Inode res: %d\n",res);
    if(res < 0)return res;
    int fromblk = inodelist[res].baseblknum;
    int frommode = inodelist[res].mode;
    int fromtype = inodelist[res].type;
    if(fromtype != FILE_INODE_TYPE)return -EISDIR;
    int fromres = res;
    int fromnlink = inodelist[res].nlink;
    // find newpath
    char newpath[2000];
    int tail = strlen(topath) - 1;
    if(topath[tail] == '/')tail--;
    while (tail > 0)
    {
        if(topath[tail] == '/')break;
        tail--;
    }
    memset(newpath, 0, sizeof(newpath));
    memcpy(newpath, topath, tail);
    char name[200];
    memset(name, 0, 200);
    memcpy(name, topath + tail + 1, strlen(topath) - tail -1);
    // printf("get newpath and name: %s, %s\n",newpath, name);
    // find newpath of topath
    res = findInode(&(inodelist[ROOTDIR_IDX]), newpath);
    // printf("find father Inode res: %d\n",res);
    if(res < 0)return res;
    if(inodelist[res].type != DIR_INODE_TYPE)return -ENOTDIR;
    // add dirent
    res = newDirent(&(inodelist[res]), name, fromtype, frommode, fromblk);
    if(res < 0)return res;
    // add nlink
    updateNlinkInode(&(inodelist[fromres]), fromnlink + 1);
    putBlock(&(inodelist[fromres].baseblk), inodelist[fromres].baseblknum);
    flushCache(inodelist[fromres].baseblknum);
    return 0;
}
