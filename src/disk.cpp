#include "disk.h"
//#include <ios>

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

void DiskMgr::openDisk(const char *file)
{
    disk = new fstream();
    disk->open(file, ios::in | ios::out | ios::binary);

    if(!disk->is_open())
    {
        printf("Error:加载虚拟磁盘失败!\n");
    }
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

    if (bp->b_flags & Buf::B_READ != 0)
    {
        this->read(bp->b_blkno, bp->b_addr);
    }

    if (bp->b_flags & Buf::B_WRITE != 0)
    {
        this->write(bp->b_blkno, bp->b_addr);
    }

    return 0;
}
