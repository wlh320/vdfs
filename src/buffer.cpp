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
        /* 从自由队列中取出 */
        bp->av_back->av_forw = bp->av_forw;
        bp->av_forw->av_back = bp->av_back;
        /* 设置B_BUSY标志 */
        bp->b_flags |= Buf::B_BUSY;
        return bp;
    }
    bp->b_flags = Buf::B_BUSY;

    /* 取自由队列第一个空闲块 */
    bp = this->freelist.av_forw;
    /* 从自由队列中取出 */
    bp->av_back->av_forw = bp->av_forw;
    bp->av_forw->av_back = bp->av_back;
    /* 设置B_BUSY标志 */
    bp->b_flags |= Buf::B_BUSY;
    bclear(bp);
    bp->b_blkno = blkno;
    return bp;
}

Buf* BufMgr::bread(int blkno)
{
    Buf* bp;
    bp = this->getBlk(blkno);
    /* 如果在设备队列中找到所需缓存，即B_DONE已设置，就不需进行I/O操作 */
    if(bp->b_flags & Buf::B_DONE)
    {
        return bp;
    }
    /* 没有找到相应缓存，构成I/O读请求块 */
    bp->b_flags |= Buf::B_READ;
    bp->b_wcount = BufMgr::BUFFER_SIZE;

    this->strategy(bp);
    return bp;
}

void BufMgr::bwrite(Buf *bp)
{
    bp->b_flags &= ~(Buf::B_READ | Buf::B_DONE);
    bp->b_wcount = BufMgr::BUFFER_SIZE;		/* 512字节 */

    bp->b_flags |= Buf::B_WRITE;

    this->strategy(bp);

    this->brelse(bp);

    return;
}

// 释放:缓存进入自由缓存队列队尾
void BufMgr::brelse(Buf *bp)
{
    bp->b_flags &= ~(Buf::B_BUSY);
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
        return 0;

    /* 将bp加入I/O请求队列的队尾 */
    bp->av_forw = NULL;

    if(d_actf == NULL)
        d_actf = bp;
    else
        d_actl->av_forw = bp;
    d_actl = bp;

    if(d_actf == NULL)
        return 0;

    /* 设置磁盘寄存器，启动I/O操作 */
    DiskMgr *dm = VDFileSys::getInstance().getDiskMgr();
    dm->devStart(bp);

    return 0;
}
