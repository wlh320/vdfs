/*
 * Project     : Unix-style Virtual Disk File System
 * Author      : wlh
 * Date        : 2017/03/30
 * Description : 文件相关的类
 */

#ifndef FILE_H
#define FILE_H
#include "utils.h"
#include "inode.h"
#include "filesystem.h"

class FileMgr
{
    /* 目录搜索模式，用于NameI()函数 */
    enum DirectorySearchMode
    {
        OPEN = 0,		/* 以打开文件方式搜索目录 */
        CREATE = 1,		/* 以新建文件方式搜索目录 */
        DELETE = 2		/* 以删除文件方式搜索目录 */
    };

public:
    // 记录当前路径信息
    Inode* rootDirInode;/* 根目录内存Inode */
    Inode* cdir;  /* 指向当前目录的Inode指针 */
    Inode* pdir;	/* 指向父目录的Inode指针 */

    DirectoryEntry de;	/* 当前目录的目录项 */
    char dbuf[DirectoryEntry::DIRSIZE]; /* 当前路径分量 */
    char curdir[128]; /* 当前工作目录完整路径 */
    // 读写变量
    byte *base; //读写目标区域
    int offset;//当前读写文件的偏移
    int count; //当前剩余读写字节
public:
    void init();


    // 用到的方法
    Inode* namei(const char *path, int mode); // 路径线性搜索
    Inode* mknode(int mode);//创建inode
    void   writeDir(Inode* pInode);

    // 系统调用
    void ls();
    void chdir(const char *path);
    int  fopen(char *name, int mode);
    int  fcreat(char *name, int mode);
    void fclose(int fd);
    int  fread(int fd, char *buffer, int length);
    int  fwrite(int fd, char *buffer, int length);
    int  flseek(int fd, int position);
    int  fdelete(char *name);

    void test();
};

#endif // FILE_H
