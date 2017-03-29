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

//////////// DiskInode ///////////////
DiskInode::DiskInode()
{
    this->d_mode = 0;
    this->d_nlink = 0;
    this->d_uid = -1;
    this->d_gid = -1;
    this->d_size = 0;
    for(int i = 0; i < 10; i++)
    {
        this->d_addr[i] = 0;
    }
    this->d_atime = 0;
    this->d_mtime = 0;
}

//////////// Inode 类 ///////////////
Inode::Inode()
{
    this->i_flag   = 0;
    this->i_mode   = 0;
    this->i_count  = 0;
    this->i_nlink  = 0;
    this->i_dev    = -1;
    this->i_number = -1;
    this->i_uid    = -1;
    this->i_gid    = -1;
    this->i_size   = 0;
    this->i_lastr  = -1;

    for(int i = 0; i < 10; i++)
        this->i_addr[i] = 0;
}

int Inode::bmap(int lbn)
{
    int phyblkno;
    BufMgr *bufmgr   = VDFileSys::getInstance().getBufMgr();
    FileSystem *fsys = VDFileSys::getInstance().getFileSystem();

    return phyblkno;
}

void Inode::iupdate(int time)
{
    BufMgr *bufmgr   = VDFileSys::getInstance().getBufMgr();
    FileSystem *fsys = VDFileSys::getInstance().getFileSystem();
    DiskInode di;
    if( (this->i_flag & (Inode::IUPD | Inode::IACC))!= 0 )
    {
        Buf* bp = bufmgr->bread(FileSystem::INODE_ZONE_START + this->i_number / FileSystem::INODE_NUMBER_PER_SECTOR);
        di.d_mode  = this->i_mode;
        di.d_nlink = this->i_nlink;
        di.d_uid   = this->i_uid;
        di.d_gid   = this->i_gid;
        di.d_size  = this->i_size;
        for (int i = 0; i < 10; i++)
            di.d_addr[i] = this->i_addr[i];
        if (this->i_flag & Inode::IACC)
            di.d_atime = time;
        if (this->i_flag & Inode::IUPD)
            di.d_mtime = time;

        /* 将p指向缓存区中旧外存Inode的偏移位置 */
        byte* p = bp->b_addr + (this->i_number % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode);
        DiskInode* pNode = &di;

        /* 用dInode中的新数据覆盖缓存中的旧外存Inode */
        IOMove((byte*)pNode, (byte*)p, sizeof(DiskInode));

        /* 将缓存写回至磁盘，达到更新旧外存Inode的目的 */
        bufmgr->bwrite(bp);
    }
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


