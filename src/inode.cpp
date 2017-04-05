#include "inode.h"
#include "vdfs.h"
#include "utils.h"
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
            bufmgr->bwrite(pFirstBuf);
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
                bufmgr->bwrite(pFirstBuf);
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
            bufmgr->bwrite(pSecondBuf);
            bufmgr->bwrite(pFirstBuf);
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
    BufMgr  *bufmgr = VDFileSys::getInstance().getBufMgr();
    FileMgr *fmgr   = VDFileSys::getInstance().getFileMgr();

    if(fmgr->count == 0)
        return ;
    this->i_flag |= Inode::IACC;

    int lbn, bn;
    int offset;
    while(fmgr->count != 0)
    {
        lbn = bn = fmgr->offset / Inode::BLOCK_SIZE;
        offset = fmgr->offset % Inode::BLOCK_SIZE;
        int nbytes = min(Inode::BLOCK_SIZE - offset, fmgr->count);

        int remain = this->i_size - fmgr->offset;
        /* 如果已读到超过文件结尾 */
        if( remain <= 0)
            return;

        /* 传送的字节数量还取决于剩余文件的长度 */
        nbytes = min(nbytes, remain);
        if( (bn = this->bmap(lbn)) == 0 )
            return;

        Buf *bp = bufmgr->bread(bn);
        unsigned char* start = bp->b_addr + offset;

        IOMove(start, fmgr->base, nbytes);

        fmgr->base   += nbytes;
        fmgr->offset += nbytes;
        fmgr->count  -= nbytes;

        bufmgr->brelse(bp);
    }
}

void Inode::writei()
{
    BufMgr  *bufmgr = VDFileSys::getInstance().getBufMgr();
    FileMgr *fmgr   = VDFileSys::getInstance().getFileMgr();
    this->i_flag |= (Inode::IACC | Inode::IUPD);
    if( 0 == fmgr->count)
        return;

    int lbn,bn;
    int offset;
    Buf *bp;
    while(fmgr->count != 0)
    {
        lbn = bn = fmgr->offset / Inode::BLOCK_SIZE;
        offset = fmgr->offset % Inode::BLOCK_SIZE;
        int nbytes = min(Inode::BLOCK_SIZE - offset, fmgr->count);
        if( (bn = this->bmap(lbn)) == 0 )
            return;
        if(Inode::BLOCK_SIZE == nbytes)
        {
            /* 如果写入数据正好满一个字符块，则为其分配缓存 */
            bp = bufmgr->getBlk(bn);
        }
        else
        {
            /* 写入数据不满一个字符块，先读后写（读出该字符块以保护不需要重写的数据） */
            bp = bufmgr->bread(bn);
        }
        /* 缓存中数据的起始写位置 */
        byte* start = bp->b_addr + offset;

        /* 写操作: 从用户目标区拷贝数据到缓冲区 */
        IOMove(fmgr->base, start, nbytes);

        fmgr->base   += nbytes;
        fmgr->count  -= nbytes;
        fmgr->offset += nbytes;

        if( (fmgr->offset % Inode::BLOCK_SIZE) == 0 )	/* 如果写满一个字符块 */
        {
            /* 以异步方式将字符块写入磁盘，进程不需等待I/O操作结束，可以继续往下执行 */
            //bufMgr.Bawrite(pBuf);
            bufmgr->bwrite(bp);
        }
        else /* 如果缓冲区未写满 */
        {
            /* 将缓存标记为延迟写，不急于进行I/O操作将字符块输出到磁盘上 */
            //bufmgr->bdwrite(bp);
            bufmgr->bwrite(bp);
        }
        /* 普通文件长度增加 */
        if( (this->i_size < fmgr->offset) && (this->i_mode & (Inode::IFBLK & Inode::IFCHR)) == 0 )
        {
            this->i_size = fmgr->offset;
        }
        this->i_flag |= Inode::IUPD;
    }
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

void Inode::icopy(Buf *bp, int inumber)
{
    DiskInode *di = new DiskInode();
    byte *p = bp->b_addr + (inumber % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode);
    IOMove(p, (byte*)di, sizeof(DiskInode));
    /* 将外存Inode变量dInode中信息复制到内存Inode中 */
    this->i_mode = di->d_mode;
    this->i_nlink = di->d_nlink;
    this->i_uid = di->d_uid;
    this->i_gid = di->d_gid;
    this->i_size = di->d_size;
    for(int i = 0; i < 10; i++)
    {
        this->i_addr[i] = di->d_addr[i];
    }
    delete di;
}

void Inode::iclear()
{
    this->i_mode = 0;
    //this->i_count = 0;
    this->i_nlink = 0;
    //this->i_dev = -1;
    //this->i_number = -1;
    this->i_uid = -1;
    this->i_gid = -1;
    this->i_size = 0;
    this->i_lastr = -1;
    for(int i = 0; i < 10; i++)
    {
        this->i_addr[i] = 0;
    }
}

void Inode::itrunc()
{
    BufMgr *bufmgr = VDFileSys::getInstance().getBufMgr();
    FileSystem *filesys = VDFileSys::getInstance().getFileSystem();

    /* 如果是字符设备或者块设备则退出 */
    if( this->i_mode & (Inode::IFCHR & Inode::IFBLK) )
    {
        return;
    }
    /* 采用FILO方式释放，以尽量使得SuperBlock中记录的空闲盘块号连续。*/
    for(int i = 9; i >= 0; i--)		/* 从i_addr[9]到i_addr[0] */
    {
        /* 如果i_addr[]中第i项存在索引 */
        if( this->i_addr[i] != 0 )
        {
            /* 如果是i_addr[]中的一次间接、两次间接索引项 */
            if( i >= 6 && i <= 9 )
            {
                /* 将间接索引表读入缓存 */
                Buf* pFirstBuf = bufmgr->bread(this->i_addr[i]);
                /* 获取缓冲区首址 */
                int* pFirst = (int *)pFirstBuf->b_addr;

                /* 每张间接索引表记录 512/sizeof(int) = 128个磁盘块号，遍历这全部128个磁盘块 */
                for(int j = 128 - 1; j >= 0; j--)
                {
                    if( pFirst[j] != 0)	/* 如果该项存在索引 */
                    {
                        /*
                         * 如果是两次间接索引表，i_addr[8]或i_addr[9]项，
                         * 那么该字符块记录的是128个一次间接索引表存放的磁盘块号
                         */
                        if( i >= 8 && i <= 9)
                        {
                            Buf* pSecondBuf = bufmgr->bread(pFirst[j]);
                            int* pSecond = (int *)pSecondBuf->b_addr;
                            for(int k = 128 - 1; k >= 0; k--)
                            {
                                if(pSecond[k] != 0)
                                    filesys->dfree(pSecond[k]);
                            }
                            /* 缓存使用完毕，释放以便被其它进程使用 */
                            bufmgr->brelse(pSecondBuf);
                        }
                        filesys->dfree(pFirst[j]);
                    }
                }
                bufmgr->brelse(pFirstBuf);
            }
            /* 释放索引表本身占用的磁盘块 */
            filesys->dfree(this->i_addr[i]);
            this->i_addr[i] = 0;
        }
    }
    // 修改inode
    this->i_size = 0;
    this->i_flag |= Inode::IUPD;
    this->i_mode &= ~(Inode::ILARG & Inode::IRWXU & Inode::IRWXG & Inode::IRWXO);
    this->i_nlink = 1;
}

//////////// Inode Table ///////////////
Inode* InodeTable::getFreeInode()
{
    for(int i = 0; i < NINODE; ++i)
        if (inode[i].i_count == 0)
            return &(this->inode[i]);
    return NULL;
}

int InodeTable::isLoaded(int inumber)
{
    for(int i = 0; i < NINODE; ++i)
        if (this->inode[i].i_count != 0 && this->inode[i].i_number == inumber)
            return i;
    return -1;
}

void InodeTable::update()
{
    for(int i = 0; i < NINODE; ++i)
        if (this->inode[i].i_count != 0)
            inode[i].iupdate(getTime());
}

Inode *InodeTable::iget(int inumber)
{
    Inode *pInode;
    int index = isLoaded(inumber);
    if (index >= 0)
    {
        pInode = &(this->inode[index]);
        pInode->i_count++;
    }
    else
    {
        pInode = getFreeInode();
        if(pInode == NULL) //获取失败
        {
            printErr("Inode table is full");
            return NULL;
        }
        else
        {
            pInode->i_number = inumber;
            pInode->i_count++;
            BufMgr *bufmgr = VDFileSys::getInstance().getBufMgr();
            Buf *bp = bufmgr->bread(FileSystem::INODE_ZONE_START+ inumber / FileSystem::INODE_NUMBER_PER_SECTOR );
            pInode->icopy(bp, inumber);
            bufmgr->brelse(bp);
        }
    }
    return pInode;
}

void InodeTable::iput(Inode *pInode)
{
    if(pInode->i_count == 1)
    {
        if(pInode->i_nlink <= 0)
        {
            pInode->itrunc();
            pInode->i_mode = 0;
        }
        /* 更新外存Inode信息 */
        pInode->iupdate(getTime());

        /* 清除内存Inode的所有标志位 */
        pInode->i_flag = 0;
        pInode->i_number = -1;
    }
    pInode->i_count--;
}
