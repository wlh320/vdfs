/*
 * Project     : Unix-style Virtual Disk File System
 * Author      : wlh
 * Date        : 2017/01/30
 * Description : 缓存相关
 */

#ifndef BUFFER_H
#define BUFFER_H

//缓存控制块
class Buf
{
public:
    enum BufFlag	/* b_flags中标志位 */
    {
        B_WRITE = 0x1,		/* 写操作。将缓存中的信息写到硬盘上去 */
        B_READ	= 0x2,		/* 读操作。从盘读取信息到缓存中 */
        B_DONE	= 0x4,		/* I/O操作结束 */
        B_ERROR	= 0x8,		/* I/O因出错而终止 */
        B_BUSY	= 0x10,		/* 相应缓存正在使用中 */
        B_WANTED = 0x20,	/* 有进程正在等待使用该buf管理的资源，清B_BUSY标志时，要唤醒这种进程 */
        B_ASYNC	= 0x40,		/* 异步I/O，不需要等待其结束 */
        B_DELWRI = 0x80		/* 延迟写，在相应缓存要移做他用时，再将其内容写到相应块设备上 */
    };

public:
    unsigned int b_flags;	/* 缓存控制块标志位 */

    int     padding;		/* 4字节填充，使得b_forw和b_back在Buf类中与Devtab类
                             * 中的字段顺序能够一致，否则强制转换会出错。 */
    /* 缓存控制块队列勾连指针 */
    Buf*    b_forw;
    Buf*    b_back;
    Buf*    av_forw;
    Buf*    av_back;

    short   b_dev;			/* 主、次设备号，其中高8位是主设备号，低8位是次设备号 */
    int     b_wcount;       /* 需传送的字节数 */
    unsigned char* b_addr;         /* 指向该缓存控制块所管理的缓冲区的首地址 */
    int     b_blkno;		/* 磁盘逻辑块号 */
    int     b_error;		/* I/O出错时信息 */
    int     b_resid;		/* I/O出错时尚未传送的剩余字节数 */
};

// 缓存管理类
class BufMgr
{
public:
    static const int NBUF = 15;			/* 缓存控制块、缓冲区的数量 */
    static const int BUFFER_SIZE = 512;	/* 缓冲区大小。 以字节为单位 */
private:
    Buf m_Buf[NBUF];					/* 缓存控制块数组 */
    unsigned char buffer[NBUF][BUFFER_SIZE];	/* 缓冲区数组 */

    Buf* d_actf; // I/O请求队列队首
    Buf* d_actl; // I/O请求队列队尾

    Buf freelist; //自由缓存队列头结点

public:
    int  init();            //初始化
    Buf* getBlk(int blkno); //申请一块缓存
    Buf* bread(int blkno);  //读入缓存
    void bwrite(Buf *bp);   //缓存写入硬盘
    void brelse(Buf *bp);   //释放缓存
    void bclear(Buf *bp);   //清空缓存块

    int strategy(Buf* bp);  //调度
};

#endif // BUFFER_H
