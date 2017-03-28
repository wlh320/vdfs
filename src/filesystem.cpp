#include "filesystem.h"
#include "vdfs.h"
#include "utils.h"

//////////// Bitmap 模板类实现 ///////////////
template<int bits>
int Bitmap<bits>::alloc()
{
    int blkno = 0;
    for(unsigned int i = 0 ; i < BMPSIZE; ++i)
    {
        if(bitmap[i] == 0xffffffff)
            blkno += sizeof(unsigned int) * 8;
        else
        {
            int pos = firstZeroPos(bitmap[i]);
            blkno += pos;
            setBit(&bitmap[i], 0x01 << pos);
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
    clearBit(&bitmap[index], 0x01 << pos);
}

//////////// Inode 类 ///////////////
Inode::Inode()
{
    this->i_flag = 0;
    this->i_mode = 0;
    this->i_count = 0;
    this->i_nlink = 0;
    this->i_dev = -1;
    this->i_number = -1;
    this->i_uid = -1;
    this->i_gid = -1;
    this->i_size = 0;
    this->i_lastr = -1;
    for(int i = 0; i < 10; i++)
    {
        this->i_addr[i] = 0;
    }
}


////////////FileSystem 类///////////////

void FileSystem::init()
{
}

void FileSystem::mkfs()
{
    BufMgr *bufmgr = VDFileSys::getInstance().getBufMgr();
}

Inode* FileSystem::ialloc()
{

}

Buf* FileSystem::dalloc()
{

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
