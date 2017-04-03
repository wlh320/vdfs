#include "filesystem.h"
#include "vdfs.h"
#include "utils.h"
#include <cstring>

//////////// Bitmap 模板类实现 ///////////////

template<int bits>
Bitmap<bits>::Bitmap()
{
    memset(bitmap, 0xff, sizeof(bitmap));
}

template<int bits>
int Bitmap<bits>::alloc()
{
    int blkno = 0;
    for(unsigned int i = 0 ; i < BMPSIZE; ++i)
    {
        if(bitmap[i] == 0x0)
            blkno += sizeof(unsigned int) * 8;
        else
        {
            int pos = firstOnePos(bitmap[i]);
            blkno += pos;
            setBit0(&bitmap[i], 0x01 << pos);
            return blkno;
        }
    }
    return -1; //没申请到
}

template<int bits>
void Bitmap<bits>::release(int blkno)
{
    int index = blkno / (sizeof(unsigned int) * 8);
    int pos = blkno % (sizeof(unsigned int) * 8);
    setBit1(&bitmap[index], 0x01 << pos);
}

//////////// SuperBlock 构造 ///////////////
SuperBlock::SuperBlock()
{
    this->s_isize  = FileSystem::INODE_ZONE_SIZE;
    this->s_fsize  = DiskMgr::NSECTOR;
    this->s_nfree  = FileSystem::DATA_ZONE_SIZE;
    this->s_ninode = FileSystem::INODE_ZONE_SIZE;
    this->s_flock  = 0;
    this->s_ilock  = 0;
    this->s_fmod   = 0;
    this->s_ronly  = 0;
    this->s_time   = getTime();
}

////////////FileSystem 类///////////////

void FileSystem::init()
{
    sb   = new SuperBlock();
    dbmp = new DataBitmap();
    ibmp = new InodeBitmap();
}

void FileSystem::mkfs()
{
    BufMgr *bufmgr = VDFileSys::getInstance().getBufMgr();
    // init SuperBlock
    Buf *bp = bufmgr->getBlk(SUPER_BLOCK_START);
    IOMove((byte *)sb, bp->b_addr, sizeof(SuperBlock));
    bufmgr->bwrite(bp);
    // init Bitmap
    bp = bufmgr->getBlk(INODE_BITMAP_START);
    IOMove((byte*)ibmp, bp->b_addr, sizeof(InodeBitmap));
    bufmgr->bwrite(bp);
    bp = bufmgr->getBlk(DATA_BITMAP_START);
    IOMove((byte*)dbmp, bp->b_addr, sizeof(DataBitmap));
    bufmgr->bwrite(bp);
    // init Inode table

}

Inode* FileSystem::ialloc()
{
    BufMgr *bufmgr = VDFileSys::getInstance().getBufMgr();
    return NULL;
}

Buf* FileSystem::dalloc()
{
    BufMgr *bufmgr = VDFileSys::getInstance().getBufMgr();
    int blkno = dbmp->alloc();
    if (blkno == -1)
    {
        printErr("No free disk space!");
        return NULL;
    }
    else
    {
        Buf* bp = bufmgr->getBlk(blkno);
        bufmgr->bclear(bp);
        sb->s_nfree--;
        sb->s_fmod = 1;
        return bp;
    }
}

void FileSystem::ifree(int blkno)
{
    if(sb->s_flock)
        return ;

    ibmp->release(blkno - INODE_ZONE_START);

    sb->s_fmod = 1;
}

void FileSystem::dfree(int blkno)
{
    if(sb->s_flock)
        return ;

    dbmp->release(blkno - DATA_ZONE_START);

    sb->s_fmod = 1;
}

void FileSystem::update()
{
    BufMgr *bufmgr = VDFileSys::getInstance().getBufMgr();
    Buf *bp;

    // update superblock
    if (sb->s_fmod)
    {
        bp = bufmgr->getBlk(SUPER_BLOCK_START);
        IOMove((byte*)sb, bp->b_addr, sizeof(SuperBlock));
        bufmgr->bwrite(bp);
    }
    // update bitmap
    bp = bufmgr->getBlk(INODE_BITMAP_START);
    IOMove((byte*)ibmp, bp->b_addr, sizeof(InodeBitmap));
    bufmgr->bwrite(bp);
    bp = bufmgr->getBlk(DATA_BITMAP_START);
    IOMove((byte*)dbmp, bp->b_addr, sizeof(DataBitmap));
    bufmgr->bwrite(bp);

    // update inode table

}
