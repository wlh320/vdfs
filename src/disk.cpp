#include "disk.h"

template<int size>
int Bitmap<size>::alloc()
{
    int blkno = 0;
    for(unsigned int i = 0 ; i < size; ++i)
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

template<int size>
void Bitmap<size>::release(int blkno)
{
    int index = blkno / (sizeof(unsigned int) * 8);
    int pos = blkno % (sizeof(unsigned int) * 8);
    clearBit(&bitmap[index], 0x01 << pos);
}

DiskMgr::DiskMgr()
{
    disk = NULL;
}

DiskMgr::~DiskMgr()
{
    if (disk != NULL)
    {
        disk->close();
        delete disk;
    }
}

bool DiskMgr::isOpen()
{
    return (disk != NULL);
}

int DiskMgr::openDisk(const char *file)
{
    disk = new fstream();
    disk->open(file, ios::in | ios::out | ios::binary);

    if(disk->fail())
    {
        return ERR;
    }

    return 0;
}

int DiskMgr::creatDisk(const char *file)
{// 创建一个全0的空白磁盘
    fstream *tmp = new fstream();
    tmp->open(file, ios::out | ios::binary);
    if(tmp->fail())
    {
        return ERR;
    }

    tmp->seekg(DISK_SIZE - 1);
    *tmp << '\0';
    tmp->close();
    return 0;
}

int DiskMgr::closeDisk()
{
    if (disk)
    {
        disk->close();
        delete disk;
    }
    disk = NULL;
    return 0;
}

void DiskMgr::read(int blkno, byte *buffer)
{
    disk->seekg(blkno*BLOCK_SIZE);
    disk->read(buffer, BLOCK_SIZE);
}

void DiskMgr::write(int blkno, byte *buffer)
{
    disk->seekg(blkno*BLOCK_SIZE);
    disk->write(buffer, BLOCK_SIZE);
}

int DiskMgr::devStart(Buf* bp)
{
    if (!disk->is_open())
    {
        printf("Error:没有虚拟磁盘!\n");
        return 0;
    }

    if ( (bp->b_flags & Buf::B_READ) != 0)
    {
        this->read(bp->b_blkno, bp->b_addr);
    }

    if ( (bp->b_flags & Buf::B_WRITE) != 0)
    {
        this->write(bp->b_blkno, bp->b_addr);
    }

    return 0;
}
