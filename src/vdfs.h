/*
 * Project     : Unix style Virtual Disk File System
 * Author      : wlh
 * Date        : 2017/02/13
 * Description : 实现主要功能类
 */

#ifndef VDFS_H
#define VDFS_H

#include "buffer.h"
#include "disk.h"
#include "filesystem.h"
#include "inode.h"
#include "file.h"

// Virtual Disk File System
class VDFileSys
{
private:
    //模块管理
    static VDFileSys instance;
    BufMgr *bufmgr;
    DiskMgr *dskmgr;
    FileMgr *flmgr;
    FileSystem *fsys;
    InodeTable *ib;

public:
    VDFileSys();
    ~VDFileSys();

    static VDFileSys& getInstance();

    //获取全局变量
    BufMgr* getBufMgr();
    DiskMgr* getDiskMgr();
    FileSystem* getFileSystem();
    FileMgr* getFileMgr();
    InodeTable* getInodeTable();

    //功能接口
    int openDisk(char*); // 加载磁盘
    int creatDisk(char*); // 创建磁盘
    int closeDisk();    // 卸载磁盘
    void loadFilesys(); // 加载文件系统

    int mkfs(); //格式化磁盘
    void mkdir(const char *vpath);
    void ls();  //列目录
    void ls(const char* path); // 重载ls
    int cd(const char* vpath); // 修改路径
    void save(const char* rpath, const char* vpath); // 保存文件到虚拟磁盘
    void load(const char* vpath, const char* rpath); // 从虚拟磁盘读取文件
    void rm(const char* vpath); // 删除文件
    void cat(const char* path); // 输出文件
};

#endif // VDFS_H
