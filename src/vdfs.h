/*
 * Project     : Unix style Virtual Disk File System
 * Author      : wlh
 * Date        : 2017/02/13
 * Description : 主要的类
 */


#ifndef VDFS_H
#define VDFS_H

#include "buffer.h"
#include "disk.h"
#include "shell.h"


// Virtual Disk File System
class VDFileSys
{
private:
    BufMgr *bufmgr;
    DiskMgr *dskmgr;
public:
    VDFileSys();
    ~VDFileSys();

    //功能
    int mkfs(); //格式化磁盘
};

#endif // VDFS_H
