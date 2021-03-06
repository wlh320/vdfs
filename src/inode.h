/*
 * Project     : Unix-style Virtual Disk File System
 * Author      : wlh
 * Date        : 2017/01/30
 * Description : Inode 和 InodeTable
 */

#ifndef INODE_H
#define INODE_H
#include "buffer.h"

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
    };

    /* static const member */
    static const unsigned int IALLOC = 0x8000;		/* 文件被使用 */
    static const unsigned int IFMT = 0x6000;		/* 文件类型掩码 */
    static const unsigned int IFDIR = 0x4000;		/* 文件类型：目录文件 */
    static const unsigned int ILARG = 0x1000;		/* 文件长度类型：大型或巨型文件 */

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
    int     i_addr[10];     /* 用于文件逻辑块和物理块号转换的基本索引表 */

    int     i_lastr;        /* 存放最近一次读取文件的逻辑块号，用于判断是否需要预读 */

public:
    Inode();
    void iupdate(int time); //更新inode
    void icopy(Buf *bp, int inumber); //将外存inode拷贝进来
    void iclear(); //清空inode
    void itrunc();

    int bmap(int lbn); //由逻辑块号转换为物理块号
    void readi();  // 读取inode中的数据
    void writei(); // 数据写入inode
};

//InodeTable 内存中的Inode
class InodeTable
{
public:
    static const int NINODE = 100;
private:
    Inode inode[NINODE];
public:
    void init();
    Inode* getFreeInode(); //从table中返回一个
    int isLoaded(int inumber); //某个Inode是否在内存中
    void update(); // Inode更新到外存
    Inode* iget(int inumber); // 获取某个inode,内存有则返回，没有则加载
    void iput(Inode* pInode); // 释放对某个inode的引用
};

#endif // INODE_H
