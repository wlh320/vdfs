/*
 * Project     : Unix-style Virtual Disk File System
 * Author      : wlh
 * Date        : 2017/03/23
 * Description : 文件系统相关的东西
 */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "buffer.h"

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
typedef Bitmap<128> InodeBitmap;


// 内存Inode
class Inode
{
public:
/* i_flag中标志位 */
    enum INodeFlag
    {
        ILOCK = 0x1,		/* 索引节点上锁 */
        IUPD  = 0x2,		/* 内存inode被修改过，需要更新相应外存inode */
        IACC  = 0x4,		/* 内存inode被访问过，需要修改最近一次访问时间 */
        IMOUNT = 0x8,		/* 内存inode用于挂载子文件系统 */
        IWANT = 0x10,		/* 有进程正在等待该内存inode被解锁，清ILOCK标志时，要唤醒这种进程 */
        ITEXT = 0x20		/* 内存inode对应进程图像的正文段 */
    };

    /* static const member */
    static const unsigned int IALLOC = 0x8000;		/* 文件被使用 */
    static const unsigned int IFMT = 0x6000;		/* 文件类型掩码 */
    static const unsigned int IFDIR = 0x4000;		/* 文件类型：目录文件 */
    static const unsigned int IFCHR = 0x2000;		/* 字符设备特殊类型文件 */
    static const unsigned int IFBLK = 0x6000;		/* 块设备特殊类型文件，为0表示常规数据文件 */
    static const unsigned int ILARG = 0x1000;		/* 文件长度类型：大型或巨型文件 */
    static const unsigned int ISUID = 0x800;		/* 执行时文件时将用户的有效用户ID修改为文件所有者的User ID */
    static const unsigned int ISGID = 0x400;		/* 执行时文件时将用户的有效组ID修改为文件所有者的Group ID */
    static const unsigned int ISVTX = 0x200;		/* 使用后仍然位于交换区上的正文段 */
    static const unsigned int IREAD = 0x100;		/* 对文件的读权限 */
    static const unsigned int IWRITE = 0x80;		/* 对文件的写权限 */
    static const unsigned int IEXEC = 0x40;			/* 对文件的执行权限 */
    static const unsigned int IRWXU = (IREAD|IWRITE|IEXEC);		/* 文件主对文件的读、写、执行权限 */
    static const unsigned int IRWXG = ((IRWXU) >> 3);			/* 文件主同组用户对文件的读、写、执行权限 */
    static const unsigned int IRWXO = ((IRWXU) >> 6);			/* 其他用户对文件的读、写、执行权限 */

    static const int BLOCK_SIZE = 512;		/* 文件逻辑块大小: 512字节 */
    static const int ADDRESS_PER_INDEX_BLOCK = BLOCK_SIZE / sizeof(int);    /* 每个间接索引表(或索引块)包含的物理盘块号 */

    static const int SMALL_FILE_BLOCK = 6;	/* 小型文件：直接索引表最多可寻址的逻辑块号 */
    static const int LARGE_FILE_BLOCK = 128 * 2 + 6;	/* 大型文件：经一次间接索引表最多可寻址的逻辑块号 */
    static const int HUGE_FILE_BLOCK = 128 * 128 * 2 + 128 * 2 + 6;	/* 巨型文件：经二次间接索引最大可寻址文件逻辑块号 */

public:
    unsigned int i_flag;    /* 状态的标志位，定义见enum INodeFlag */
    unsigned int i_mode;    /* 文件工作方式信息 */

    int     i_count;        /* 引用计数 */
    int     i_nlink;        /* 文件联结计数，即该文件在目录树中不同路径名的数量 */

    short   i_dev;			/* 外存inode所在存储设备的设备号 */
    int     i_number;		/* 外存inode区中的编号 */

    short   i_uid;			/* 文件所有者的用户标识数 */
    short   i_gid;			/* 文件所有者的组标识数 */

    int     i_size;         /* 文件大小，字节为单位 */
    int     i_addr[10];     /* 用于文件逻辑块好和物理块号转换的基本索引表 */

    int     i_lastr;        /* 存放最近一次读取文件的逻辑块号，用于判断是否需要预读 */

public:
    Inode();
    void iupdate(int time); //更新inode
    int bmap(int lbn); //由逻辑块号转换为物理块号

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

// 目录项结构
struct DirectoryEntry
{
public:
    static const int DIRSIZE = 28;	/* 目录项中路径部分的最大字符串长度 */

    int m_ino;		/* 目录项中Inode编号部分 */
    char m_name[DIRSIZE];	/* 目录项中路径名部分 */
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
    Inode *inode;
    InodeBitmap *ibmp;
    DataBitmap  *dbmp;

public:
    void init();

    Inode* ialloc();  //分配Inode
    Buf* dalloc();  //分配数据块
    void ifree(int blkno);  //释放Inode
    void dfree(int blkno);  //释放数据块

    void mkfs();    //格式化磁盘
    void update();  //将修改写入磁盘
};

#endif // FILESYSTEM_H
