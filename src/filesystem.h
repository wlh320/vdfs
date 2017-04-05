/*
 * Project     : Unix-style Virtual Disk File System
 * Author      : wlh
 * Date        : 2017/03/23
 * Description : 文件系统相关的东西
 */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "buffer.h"
#include "inode.h"

/*                   VDFS磁盘分布信息
 * ---------------------------------------------------------
 *      | SB SuperBlock   : 一个扇区(0号)
 * Info | IB InodeBitmap  : 一个扇区(1号)
 *      | DB DataBitmap   : 一个扇区(2号)
 * ---------------------------------------------------------
 *      | Inodes       : 128个扇区(共1024个Inode,1024/8=128个扇区)
 * Data |
 *      | FileData     : 4096个扇区(大小 2M, 共 2*1024*1024/512=4096个扇区)
 * ---------------------------------------------------------
 *
 * 0  1  2  3          131                            4227
 * +---------------------------------------------------+
 * |  |  |  |           |                              |
 * |SB|IB|DB|  Inodes   |           Data               |
 * |  |  |  |           |                              |
 * +---------------------------------------------------+
 *  1  1  1     128                 4096
 */

// SuperBlock 结构, 共 1024 bytes
struct SuperBlock
{
    int	s_isize;        /* 外存Inode区占用的盘块数 */
    int	s_fsize;        /* 盘块总数 */

    int	s_nfree;        /* 直接管理的空闲盘块数量 */
    int	s_ninode;       /* 直接管理的空闲外存Inode数量 */

    int	s_flock;        /* 封锁空闲盘块索引表标志 */
    int	s_ilock;        /* 封锁空闲Inode表标志 */

    int	s_fmod;         /* 内存中super block副本被修改标志，意味着需要更新外存对应的Super Block */
    int	s_ronly;        /* 本文件系统只能读出 */
    int	s_time;         /* 最近一次更新时间 */
    int	padding[119];    /* 填充使SuperBlock块大小等于256字节，占据2个扇区 */

    SuperBlock();
};

// Bitmap 定义
template<int bits>
class Bitmap
{
public:
    static const int BMPSIZE = bits / 32;
private:
    unsigned int bitmap[BMPSIZE];
public:
    Bitmap();
    int alloc(); // 找到一个free的block
    void release(int blkno); //释放一个block
};
typedef Bitmap<4096> DataBitmap;
typedef Bitmap<1024> InodeBitmap;

// 目录项结构
struct DirectoryEntry
{
public:
    static const int DIRSIZE = 28;	/* 目录项中路径部分的最大字符串长度 */

    int m_ino;		/* 目录项中Inode编号部分 */
    char m_name[DIRSIZE];	/* 目录项中路径名部分 */
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

    DiskInode();
};

class FileSystem
{
public:
    static const int SUPER_BLOCK_START = 0;
    static const int ROOTINO = 0;			/* 文件系统根目录外存Inode编号 */

    static const int INODE_BITMAP_START = 1;
    static const int DATA_BITMAP_START = 2;

    static const int INODE_NUMBER_PER_SECTOR = 8; /* 外存INode对象长度为64字节，每个磁盘块可以存放512/64 = 8个外存Inode */
    static const int INODE_ZONE_START = 3; /* 外存Inode区位于磁盘上的起始扇区号 */
    static const int INODE_ZONE_SIZE = 128; /* 磁盘上外存Inode区占据的扇区数 */

    static const int DATA_ZONE_START = 131; /* 数据区的起始扇区号 */
    static const int DATA_ZONE_SIZE = 4096;	/* 数据区占据的扇区数量 */
    static const int DATA_ZONE_END = DATA_ZONE_START + DATA_ZONE_SIZE; /* 数据区的结束扇区号 */

private:
    SuperBlock *sb;
    InodeBitmap *ibmp;
    DataBitmap  *dbmp;

public:
    void init();
    void loadSuperBlock();

    Inode* ialloc();  //分配Inode
    Buf* dalloc();  //分配数据块
    void ifree(int number);  //释放Inode
    void dfree(int blkno);  //释放数据块

    void mkfs();    //格式化磁盘
    void update();  //将修改写入磁盘

};

#endif // FILESYSTEM_H
