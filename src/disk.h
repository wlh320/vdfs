/*
 * Project     : Unix-style Virtual Disk File System
 * Author      : wlh
 * Date        : 2017/01/30
 * Description : 磁盘相关类
 */

#ifndef DISK_H
#define DISK_H
#include "buffer.h"
#include "utils.h"
#include <fstream>
using namespace std;

//磁盘管理类
class DiskMgr
{
public:
    const static int BLOCK_SIZE = 512;
    const static int NSECTOR = 3 + 128 + 4096;
    const static int DISK_SIZE = NSECTOR * BLOCK_SIZE;

private:
    fstream *disk;

public:
    DiskMgr();
    ~DiskMgr();

    bool isOpen();//是否打开了一个磁盘文件
    int openDisk(const char*); //打开虚拟磁盘文件
    int creatDisk(const char*); //新建一个空的虚拟磁盘文件
    int closeDisk(); //关闭当前磁盘文件

    int devStart(Buf*); //启动磁盘操作

    void read(int, byte*);  //将一个block读到缓存
    void write(int, byte*); //将一个block写回磁盘
};

#endif // DISK_H
