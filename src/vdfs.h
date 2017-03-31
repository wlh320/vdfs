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
    FileSystem *fsys;

    //读写变量
    byte *base; //读写目标区域
    int offset;//当前读写文件的偏移
    int count; //当前剩余读写字节

public:
    VDFileSys();
    ~VDFileSys();

    static VDFileSys& getInstance();

    //获取全局变量
    BufMgr* getBufMgr();
    DiskMgr* getDiskMgr();
    FileSystem* getFileSystem();

    //功能接口
    int openDisk(char*);
    int creatDisk(char*);
    int closeDisk();

    int mkfs(); //格式化磁盘
    void ls();  //list
};

#endif // VDFS_H
