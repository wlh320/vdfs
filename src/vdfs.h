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


// Virtual Disk File System
class VDFileSys
{
private:
    BufMgr *bufmgr;
    DiskMgr *dskmgr;
public:
    VDFileSys();
    ~VDFileSys();

    //功能接口
    int openDisk(char*);
    int creatDisk(char*);
    int closeDisk();
    int mkfs(); //格式化磁盘
    void ls();  //list
    int fopen(char*, int); // open file
    void fclose(int); // close file
    int fread(int, char*, int); // read file
    int fwrite(int, char*, int); // write file
    int flseek(int, int); // seek
    int fcreat(char*, int); // create file
    int fdelete(char*); // delete file
};

#endif // VDFS_H
