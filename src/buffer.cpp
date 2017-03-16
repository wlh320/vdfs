#include "buffer.h"
#include "disk.h"

int BufMgr::init()
{
    //Buf *bp;
    this->freelist.b_forw  = this->freelist.b_back  = &(this->freelist);
    this->freelist.av_forw = this->freelist.av_back = &(this->freelist);

    for(int i = 0; i < NBUF; ++i)
    {
        //bp = &(this->m_Buf[i]);

    }

    return 0;
}

int BufMgr::strategy(Buf* bp)
{
    /* 检查I/O操作块是否超出了该硬盘的扇区数上限 */
    if(bp->b_blkno >= DiskMgr::NSECTOR)
    {
        /* 设置出错标志 */
        bp->b_flags |= Buf::B_ERROR;

        //BufferManager& bm = Kernel::Instance().GetBufferManager();
        /*
         * 出错情况下不真正执行I/O操作，这里相当于模拟磁盘中断
         * 处理程序中调用IODone()唤醒等待I/O操作结束的进程。
         */
        //bm.IODone(bp);
        return 0;	/* GCC likes it ! */
    }

    /* 将bp加入I/O请求队列的队尾，此时I/O队列已经退化到单链表形式，将bp->av_forw标志着链表结尾 */
    bp->av_forw = NULL;

    /* 以下操作将进入临界区，其中临界资源为块设备表g_Atab。
     * 因为除了这里会对块设备表g_Atab的I/O请求队列进行操作，
     * 磁盘中断处理程序也会对I/O请求队列其进行操作，两者是并行的。
     * 实际上这里只需关闭硬盘的中断就可以了。
     */
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

    /* 如果硬盘不忙就立即进行硬盘操作，否则将当前I/O请求送入
     * 块设备表I/O请求队列之后直接返回，当前磁盘的I/O操作完成后
     * 会向CPU发出磁盘中断，系统会在磁盘中断处理程序中执行块设备
     * 表I/O请求队列中的下一个I/O请求。
     */
//    if(this->d_tab->d_active == 0)		/* 磁盘空闲 */
//    {
//        //this->Start();

//        Buf* bp;
//        if( (bp = this->d_tab->d_actf) == 0 )
//            return;		/* 如果I/O请求队列为空，则立即返回 */
//        this->d_tab->d_active++;	/* I/O请求队列不空，设置控制器忙标志 */
//        /* 设置磁盘寄存器，启动I/O操作 */
//        //ATADriver::DevStart(bp);

//    }
    //X86Assembly::STI();

    return 0;	/* GCC likes it ! */
}


