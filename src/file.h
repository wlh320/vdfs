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
    File();
};


class FileMgr
{
public:
    // 记录当前路径信息
    Inode* rootDirInode;/* 根目录内存Inode */
    Inode* u_cdir;  /* 指向当前目录的Inode指针 */
    Inode* u_pdir;	/* 指向父目录的Inode指针 */
    DirectoryEntry u_dent;	/* 当前目录的目录项 */
    char u_dbuf[DirectoryEntry::DIRSIZE]; /* 当前路径分量 */
    char u_curdir[128]; /* 当前工作目录完整路径 */
    // 读写变量
    byte *base; //读写目标区域
    int offset;//当前读写文件的偏移
    int count; //当前剩余读写字节
public:
    void init();
    Inode* namei(); // 路径线性搜索

    // 系统调用
    void ls();
    void chdir();
    int  fopen(char *name, int mode);
    int  fcreat(char *name, int mode);
    void fclose(int fd);
    int  fread(int fd, char *buffer, int length);
    int  fwrite(int fd, char *buffer, int length);
    int  flseek(int fd, int position);
    int  fdelete(char *name);
};

#endif // FILE_H
