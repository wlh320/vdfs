#include "filesystem.h"
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

