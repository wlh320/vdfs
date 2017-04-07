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
    padding[118]   = 0x686c7700; // _(:зゝ∠)_
}

////////////FileSystem 类///////////////

void FileSystem::init()
{
    sb   = new SuperBlock();
    dbmp = new DataBitmap();
    ibmp = new InodeBitmap();
}

void FileSystem::loadSuperBlock()
{
    BufMgr *bufmgr = VDFileSys::getInstance().getBufMgr();
    // load SuperBlock
    Buf *bp = bufmgr->bread(SUPER_BLOCK_START);
    IOMove(bp->b_addr, (byte *)sb, sizeof(SuperBlock));
    // load Bitmap
    bp = bufmgr->bread(INODE_BITMAP_START);
    IOMove(bp->b_addr, (byte*)ibmp, sizeof(InodeBitmap));
    bp = bufmgr->bread(DATA_BITMAP_START);
    IOMove(bp->b_addr, (byte*)dbmp, sizeof(DataBitmap));
}
void FileSystem::creatRoot()
{
    // make rootdir
    InodeTable *ib = VDFileSys::getInstance().getInodeTable();
    BufMgr *bufmgr = VDFileSys::getInstance().getBufMgr();
    Inode *pInode = ialloc();
    // alloc disk block
    Buf *newBuf = dalloc();
    DirectoryEntry initde;
    initde.m_ino = pInode->i_number;
    initde.m_name[0] = '.';
    for(int i = 1; i < DirectoryEntry::DIRSIZE; ++i)
        initde.m_name[i] = '\0';
    IOMove((byte*)&initde, newBuf->b_addr, sizeof(DirectoryEntry));
    initde.m_name[1] = '.';
    IOMove((byte*)&initde, newBuf->b_addr + sizeof(DirectoryEntry), sizeof(DirectoryEntry));
    bufmgr->bwrite(newBuf);

    pInode->i_flag |= (Inode::IACC | Inode::IUPD);
    pInode->i_mode = Inode::IALLOC | Inode::IFDIR /* Most vital!! */;
    pInode->i_nlink = 1;
    pInode->i_addr[0] = newBuf->b_blkno;
    pInode->i_size = 2 * sizeof(DirectoryEntry);
    ib->iput(pInode);
}

void FileSystem::mkfs()
{
    this->init();
    BufMgr *bufmgr = VDFileSys::getInstance().getBufMgr();
    // rootdir
    creatRoot();
    // init SuperBlock
    Buf *bp = bufmgr->bread(SUPER_BLOCK_START);
    IOMove((byte *)sb, bp->b_addr, sizeof(SuperBlock));
    bufmgr->bwrite(bp);
    // init Bitmap
    bp = bufmgr->bread(INODE_BITMAP_START);
    IOMove((byte*)ibmp, bp->b_addr, sizeof(InodeBitmap));
    bufmgr->bwrite(bp);
    bp = bufmgr->bread(DATA_BITMAP_START);
    IOMove((byte*)dbmp, bp->b_addr, sizeof(DataBitmap));
    bufmgr->bwrite(bp);
}

Inode* FileSystem::ialloc()
{
    InodeTable *ib = VDFileSys::getInstance().getInodeTable();
    int ino = ibmp->alloc();
    Inode *pinode = ib->iget(ino);
    if(pinode == NULL)
        return NULL;
    if(pinode->i_mode == 0)
    {
        pinode->iclear();
        sb->s_fmod = 1;
        return pinode;
    }
    return pinode;
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
        Buf* bp = bufmgr->getBlk(DATA_ZONE_START + blkno);
        bufmgr->bclear(bp);
        sb->s_nfree--;
        sb->s_fmod = 1;
        return bp;
    }
}

void FileSystem::ifree(int number)
{
    if(sb->s_flock)
        return ;

    ibmp->release(number);

    sb->s_fmod = 1;
}

void FileSystem::dfree(int blkno)
{
    if(sb->s_flock)
        return ;

    dbmp->release(blkno - DATA_ZONE_START);

    sb->s_fmod = 1;
}

// 更新整张磁盘的内容
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
    bp = bufmgr->bread(INODE_BITMAP_START);
    IOMove((byte*)ibmp, bp->b_addr, sizeof(InodeBitmap));
    bufmgr->bwrite(bp);
    bp = bufmgr->bread(DATA_BITMAP_START);
    IOMove((byte*)dbmp, bp->b_addr, sizeof(DataBitmap));
    bufmgr->bwrite(bp);
    // update inode table
    InodeTable *ib = VDFileSys::getInstance().getInodeTable();
    ib->update();
}
