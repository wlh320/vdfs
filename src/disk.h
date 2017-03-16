/*
 * Project     : Unix-style Virtual Disk File System
 * Author      : wlh
 * Date        : 2017/01/30
 * Description : 磁盘相关
 */

#ifndef DISK_H
#define DISK_H
#include "buffer.h"
#include "utils.h"
#include <fstream>
using namespace std;

/*                      磁盘分布信息
 *
 * SuperBlock区： 2个扇区
 * Inode区：一个扇区8个Inode，共128个Inode,128/8=16个扇区
 * 数据区：大小 2M, 共 2*1024*1024/512 = 4096个扇区
 *
 * 0     1     2             18
 * +---------------------------------------------------+
 * |           |              |                        |
 * | SuperBlock|   Inodes     |  Datas                 |
 * |           |              |                        |
 * +---------------------------------------------------+
 *       2            16                 4096
 */

// SuperBlock 结构, 共 1024 bytes
struct SuperBlock
{
    int	s_isize;        /* 外存Inode区占用的盘块数 */
    int	s_fsize;        /* 盘块总数 */

    int	s_nfree;        /* 直接管理的空闲盘块数量 */
    int	s_free[100];    /* 直接管理的空闲盘块索引表 */

    int	s_ninode;       /* 直接管理的空闲外存Inode数量 */
    int	s_inode[100];   /* 直接管理的空闲外存Inode索引表 */

    int	s_flock;        /* 封锁空闲盘块索引表标志 */
    int	s_ilock;        /* 封锁空闲Inode表标志 */

    int	s_fmod;         /* 内存中super block副本被修改标志，意味着需要更新外存对应的Super Block */
    int	s_ronly;        /* 本文件系统只能读出 */
    int	s_time;         /* 最近一次更新时间 */
    int	padding[47];    /* 填充使SuperBlock块大小等于1024字节，占据2个扇区 */
};

// Bitmap 定义
template<int size>
class Bitmap
{
private:
    unsigned int bitmap[size];
public:
    Bitmap(){memset(bitmap, 0x0, sizeof(bitmap));}
    int alloc(); // 找到一个free的block
    void release(int blkno); //释放一个block
};

typedef Bitmap<128> BlockBitmap;
typedef Bitmap<4> InodeBitmap;


// 内存Inode
class Inode
{
public:
    static const int SMALL_FILE_BLOCK = 6;	/* 小型文件：直接索引表最多可寻址的逻辑块号 */
    static const int LARGE_FILE_BLOCK = 128 * 2 + 6;	/* 大型文件：经一次间接索引表最多可寻址的逻辑块号 */
    static const int HUGE_FILE_BLOCK = 128 * 128 * 2 + 128 * 2 + 6;	/* 巨型文件：经二次间接索引最大可寻址文件逻辑块号 */
private:

};

// 外存 Inode 结构, 一个 64 bytes, 一块有8个
struct DiskInode
{
    unsigned int d_mode;    /* 状态的标志位，定义见enum INodeFlag */
    int     d_nlink;        /* 文件联结计数，即该文件在目录树中不同路径名的数量 */

    short   d_uid;          /* 文件所有者的用户标识数 */
    short   d_gid;          /* 文件所有者的组标识数 */

    int     d_size;         /* 文件大小，字节为单位 */
    int     d_addr[10];     /* 用于文件逻辑块号和物理块号转换的基本索引表 */

    int     d_atime;        /* 最后访问时间 */
    int     d_mtime;        /* 最后修改时间 */
};

// 目录项结构
class DirectoryEntry
{
public:
    static const int DIRSIZE = 28;	/* 目录项中路径部分的最大字符串长度 */

    /* Functions */
public:
    /* Constructors */
    DirectoryEntry();
    /* Destructors */
    ~DirectoryEntry();

    /* Members */
public:
    int m_ino;		/* 目录项中Inode编号部分 */
    char m_name[DIRSIZE];	/* 目录项中路径名部分 */
};


//磁盘管理类
class DiskMgr
{
public:
    const static int BLOCK_SIZE = 512;
    const static int SBLK_NUM = 2;
    const static int INODE_NUM = 16;
    const static int DATA_NUM = 4096;

    const static int NSECTOR = SBLK_NUM + INODE_NUM + DATA_NUM;
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

    int devStart(Buf*);

    void read(int, byte*);  //将一个block读到缓存
    void write(int, byte*); //将一个block写回磁盘
};

#endif // DISK_H
