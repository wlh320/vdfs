#include "buffer.h"
#include "disk.h"
#include "vdfs.h"
#include <cstring>
int BufMgr::init()
{
    this->freelist.b_forw  = this->freelist.b_back  = &(this->freelist);
    this->freelist.av_forw = this->freelist.av_back = &(this->freelist);

    Buf *bp;
    for(int i = 0; i < NBUF; ++i)
    {
        bp = &(this->m_Buf[i]);
        memset(buffer[i], 0, BUFFER_SIZE);
        bp->b_dev = -1;
        bp->b_addr = this->buffer[i];
        // init nodev
        bp->b_back = &(this->freelist);
        bp->b_forw = this->freelist.av_forw;
        this->freelist.b_forw->b_back = bp;
        this->freelist.b_forw = bp;
        // init free list
        this->brelse(bp);
    }
    return 0;
}

Buf* BufMgr::getBlk(int blkno)
{
    Buf *dp = &(this->freelist);
    Buf *bp;
    for(bp = dp->b_forw; bp != (Buf *)dp; bp = bp->b_forw)
    {
        /* 不是要找的缓存，则继续 */
        if(bp->b_blkno != blkno)
            continue;

        if(bp->b_flags & Buf::B_BUSY)
        {
            bp->b_flags |= Buf::B_WANTED;
        }
        /* 从自由队列中取出 */
        bp->av_back->av_forw = bp->av_forw;
        bp->av_forw->av_back = bp->av_back;
        /* 设置B_BUSY标志 */
        bp->b_flags |= Buf::B_BUSY;
        return bp;
    }
    /* 取自由队列第一个空闲块 */
    bp = this->freelist.av_forw;
    /* 从自由队列中取出 */
    bp->av_back->av_forw = bp->av_forw;
    bp->av_forw->av_back = bp->av_back;
    /* 设置B_BUSY标志 */
    bp->b_flags |= Buf::B_BUSY;

    bp->b_blkno = blkno;
    return bp;
}

Buf* BufMgr::bread(int blkno)
{
    Buf* bp;
    /* 根据设备号，字符块号申请缓存 */
    bp = this->getBlk(blkno);
    /* 如果在设备队列中找到所需缓存，即B_DONE已设置，就不需进行I/O操作 */
    if(bp->b_flags & Buf::B_DONE)
    {
        return bp;
    }
    /* 没有找到相应缓存，构成I/O读请求块 */
    bp->b_flags |= Buf::B_READ;
    bp->b_wcount = BufMgr::BUFFER_SIZE;

    /*
     * 将I/O请求块送入相应设备I/O请求队列，如无其它I/O请求，则将立即执行本次I/O请求；
     * 否则等待当前I/O请求执行完毕后，由中断处理程序启动执行此请求。
     * 注：Strategy()函数将I/O请求块送入设备请求队列后，不等I/O操作执行完毕，就直接返回。
     */
    this->strategy(bp);
    /* 同步读，等待I/O操作结束 */
    return bp;
}

void BufMgr::bwrite(Buf *bp)
{
    unsigned int flags = bp->b_flags;

    bp->b_flags &= ~(Buf::B_READ | Buf::B_DONE | Buf::B_ERROR | Buf::B_DELWRI);
    bp->b_wcount = BufMgr::BUFFER_SIZE;		/* 512字节 */

    bp->b_flags |= Buf::B_WRITE;

    this->strategy(bp);

    if( (flags & Buf::B_ASYNC) == 0 )
    {
        /* 同步写，需要等待I/O操作结束 */
        //this->IOWait(bp);
        this->brelse(bp);
    }
    return;
}

void BufMgr::bdwrite(Buf *bp)
{
    bp->b_flags |= (Buf::B_DELWRI | Buf::B_DONE);
    this->brelse(bp);
    return;
}

// 释放:缓存进入自由缓存队列队尾
void BufMgr::brelse(Buf *bp)
{
    bp->b_flags &= ~(Buf::B_WANTED | Buf::B_BUSY | Buf::B_ASYNC);
    (this->freelist.av_back)->av_forw = bp;
    bp->av_back = this->freelist.av_back;
    bp->av_forw = &(this->freelist);
    this->freelist.av_back = bp;
}

void BufMgr::bclear(Buf *bp)
{
    byte *bc = bp->b_addr;
    memset(bc, 0, BUFFER_SIZE);
}

int BufMgr::strategy(Buf* bp)
{
    /* 检查I/O操作块是否超出了该硬盘的扇区数上限 */
    if(bp->b_blkno >= DiskMgr::NSECTOR)
    {
        /* 设置出错标志 */
        bp->b_flags |= Buf::B_ERROR;

        /*
         * 出错情况下不真正执行I/O操作，这里相当于模拟磁盘中断
         * 处理程序中调用IODone()唤醒等待I/O操作结束的进程。
         */
        //bm.IODone(bp);
        bp->b_flags |= Buf::B_DONE;
        return 0;	/* GCC likes it ! */
    }

    /* 将bp加入I/O请求队列的队尾，此时I/O队列已经退化到单链表形式，将bp->av_forw标志着链表结尾 */
    bp->av_forw = NULL;

    //X86Assembly::CLI();

    if(d_actf == NULL)
    {
        d_actf = bp;
    }
    else
    {
        d_actl->av_forw = bp;
    }
    d_actl = bp;

    if(d_actf == NULL)
        return 0;

    /* 设置磁盘寄存器，启动I/O操作 */
    DiskMgr *dm = VDFileSys::getInstance().getDiskMgr();
    dm->devStart(bp);

    bp->b_flags |= Buf::B_DONE;

    //X86Assembly::STI();

    return 0;
}
