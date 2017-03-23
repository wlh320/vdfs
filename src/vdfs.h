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

// Virtual Disk File System
class VDFileSys
{
private:
    BufMgr *bufmgr;
    DiskMgr *dskmgr;
    FileSystem *fsys;
public:
    VDFileSys();
    ~VDFileSys();

    //功能接口
    int openDisk(char*);
    int creatDisk(char*);
    int closeDisk();

    int mkfs(); //格式化磁盘
    void ls();  //list
};

#endif // VDFS_H
