#include "inode.h"
#include "vdfs.h"
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
    BufMgr *bufmgr   = VDFileSys::getInstance().getBufMgr();
    FileSystem *fsys = VDFileSys::getInstance().getFileSystem();
    int phyblkno;
    Buf* pFirstBuf;
    Buf* pSecondBuf;

    if(lbn > Inode::HUGE_FILE_BLOCK)
        return 0;

    if(lbn < Inode::SMALL_FILE_BLOCK)
    {
        phyblkno = this->i_addr[lbn];
        if( phyblkno == 0 && (pFirstBuf = fsys->dalloc()) != NULL )
        {
            // 延迟写
            bufmgr->bdwrite(pFirstBuf);
            phyblkno = pFirstBuf->b_blkno;
            /* 将逻辑块号lbn映射到物理盘块号phyBlkno */
            this->i_addr[lbn] = phyblkno;
            this->i_flag |= Inode::IUPD;
        }
        return phyblkno;
    }
    else
    {
        int index;
        if(lbn < Inode::LARGE_FILE_BLOCK)
            index = (lbn - Inode::SMALL_FILE_BLOCK) / Inode::ADDRESS_PER_INDEX_BLOCK + 6;
        else
            index = (lbn - Inode::LARGE_FILE_BLOCK) / (Inode::ADDRESS_PER_INDEX_BLOCK * Inode::ADDRESS_PER_INDEX_BLOCK) + 8;

        phyblkno = this->i_addr[index];
        /* 若该项为零，则表示不存在相应的间接索引表块 */
        if( 0 == phyblkno )
        {
            this->i_flag |= Inode::IUPD;
            /* 分配一空闲盘块存放间接索引表 */
            if( (pFirstBuf = fsys->dalloc()) == NULL )
            {
                return 0;	/* 分配失败 */
            }
            /* i_addr[index]中记录间接索引表的物理盘块号 */
            this->i_addr[index] = pFirstBuf->b_blkno;
        }
        else
        {
            /* 读出存储间接索引表的字符块 */
            pFirstBuf = bufmgr->bread(phyblkno);
        }
        /* 获取缓冲区首址 */
        int *iTable = (int *)pFirstBuf->b_addr;

        if(index >= 8)	/* ASSERT: 8 <= index <= 9 */
        {
            /*
             * 对于巨型文件的情况，pFirstBuf中是二次间接索引表，
             * 还需根据逻辑块号，经由二次间接索引表找到一次间接索引表
             */
            index = ( (lbn - Inode::LARGE_FILE_BLOCK) / Inode::ADDRESS_PER_INDEX_BLOCK ) % Inode::ADDRESS_PER_INDEX_BLOCK;

            /* iTable指向缓存中的二次间接索引表。该项为零，不存在一次间接索引表 */
            phyblkno = iTable[index];
            if( 0 == phyblkno )
            {
                if( (pSecondBuf = fsys->dalloc()) == NULL)
                {
                    /* 分配一次间接索引表磁盘块失败，释放缓存中的二次间接索引表，然后返回 */
                    bufmgr->brelse(pFirstBuf);
                    return 0;
                }
                /* 将新分配的一次间接索引表磁盘块号，记入二次间接索引表相应项 */
                iTable[index] = pSecondBuf->b_blkno;
                /* 将更改后的二次间接索引表延迟写方式输出到磁盘 */
                bufmgr->bdwrite(pFirstBuf);
            }
            else
            {
                /* 释放二次间接索引表占用的缓存，并读入一次间接索引表 */
                bufmgr->brelse(pFirstBuf);
                pSecondBuf = bufmgr->bread(phyblkno);
            }
            pFirstBuf = pSecondBuf;
            /* 令iTable指向一次间接索引表 */
            iTable = (int *)pSecondBuf->b_addr;
        }

        /* 计算逻辑块号lbn最终位于一次间接索引表中的表项序号index */
        if( lbn < Inode::LARGE_FILE_BLOCK )
            index = (lbn - Inode::SMALL_FILE_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;
        else
            index = (lbn - Inode::LARGE_FILE_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;

        if( (phyblkno = iTable[index]) == 0 && (pSecondBuf = fsys->dalloc()) != NULL)
        {
            /* 将分配到的文件数据盘块号登记在一次间接索引表中 */
            phyblkno = pSecondBuf->b_blkno;
            iTable[index] = phyblkno;
            /* 将数据盘块、更改后的一次间接索引表用延迟写方式输出到磁盘 */
            bufmgr->bdwrite(pSecondBuf);
            bufmgr->bdwrite(pFirstBuf);
        }
        else
        {
            /* 释放一次间接索引表占用缓存 */
            bufmgr->brelse(pFirstBuf);
        }
    }
    return phyblkno;
}

void Inode::readi()
{

}

void Inode::writei()
{

}

void Inode::iupdate(int time)
{
    BufMgr *bufmgr   = VDFileSys::getInstance().getBufMgr();
    //FileSystem *fsys = VDFileSys::getInstance().getFileSystem();
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
