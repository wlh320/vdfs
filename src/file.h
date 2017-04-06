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

class File
{
public:
    enum FileFlags
    {
        FREAD = 0x1,			/* 读请求类型 */
        FWRITE = 0x2,			/* 写请求类型 */
    };

    unsigned int flag;
    Inode* inode;
    int f_count;
    int f_offset;

public:
    File();
    void close();
    bool isOpen();
};

// 文件管理类
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
    //打开的文件
    File *file;
public:
    void init();


    // 用到的方法
    Inode* namei(const char *path, int mode); // 路径线性搜索
    Inode* mknode(int mode);//创建inode
    void   writeDir(Inode* pInode);
    void   removeDot();

    void creatDir(const char* path);

    // 系统调用
    void ls();
    int chdir(const char *path);
    int  fopen(const char *name, int mode);
    int  fcreat(const char *name, int mode);
    void fclose();
    int  fread(char *buffer, int length);
    int  fwrite(char *buffer, int length);
    int  flseek(int position);
    int  fdelete(char *name);

    void test();
};

#endif // FILE_H
