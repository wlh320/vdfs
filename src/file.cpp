#include "file.h"
#include "vdfs.h"
#include <cstring>

void FileMgr::init()
{
    InodeTable *ib = VDFileSys::getInstance().getInodeTable();
    this->count = 0;
    this->offset = 0;
    this->base = NULL;

    strcpy(curdir, "/");
    this->rootDirInode = ib->iget(0);
    this->cdir = this->rootDirInode;
    this->pdir = this->rootDirInode;
}

Inode* FileMgr::namei(const char* path,int mode)
{
    BufMgr *bufmgr = VDFileSys::getInstance().getBufMgr();
    InodeTable *ib = VDFileSys::getInstance().getInodeTable();
    const char *curchar = path;
    char *pchar;
    Buf *pBuf;
    Inode *pInode = this->cdir;
    int freeEntryOffset = 0;

    if(*curchar++ == '/')
        pInode = this->rootDirInode;
    ib->iget(pInode->i_number);
    while (*curchar == '/')
    {
        curchar++;
    }
    if ('\0' == *curchar && mode != FileMgr::OPEN )
    {
        printErr("namei Failed");
        goto out;
    }
    while (true)
    {
        if ( '\0' == *curchar ) // 成功返回
            return pInode;
        if ( (pInode->i_mode & Inode::IFMT) != Inode::IFDIR )
        {
            printErr("Not a directory");
            break;
        }
        // 权限 暂略
        // 将需要匹配的目录拷到dbuf中
        pchar = &(dbuf[0]);
        while ( '/' != *curchar && '\0' != *curchar)
        {
            if ( pchar < &(dbuf[DirectoryEntry::DIRSIZE]) )
            {
                *pchar = *curchar;
                pchar++;
            }
            curchar++;
        }
        /* 将u_dbuf剩余的部分填充为'\0' */
        while ( pchar < &(dbuf[DirectoryEntry::DIRSIZE]) )
        {
            *pchar = '\0';
            pchar++;
        }
        while (*curchar == '/')
        {
            curchar++;
        }
        /* 内层循环部分对于dbuf[]中的路径名分量，逐个搜寻匹配的目录项 */
        offset = 0;
        /* 设置为目录项个数 ，含空白的目录项*/
        count = pInode->i_size / (DirectoryEntry::DIRSIZE + 4);
        freeEntryOffset = 0;
        pBuf = NULL;

        while(true)
        {
            if (count == 0) // 搜索完毕
            {
                if(pBuf != NULL)
                    bufmgr->brelse(pBuf);
                if ( mode == FileMgr::CREATE && *curchar == '\0' )
                {
                    /* 判断该目录是否可写  略*/
                    /* 将父目录Inode指针保存起来，以后写目录项WriteDir()函数会用到 */
                    pdir = pInode;

                    if ( freeEntryOffset )	/* 此变量存放了空闲目录项位于目录文件中的偏移量 */
                    {
                        /* 将空闲目录项偏移量存入u区中，写目录项WriteDir()会用到 */
                        offset = freeEntryOffset - (DirectoryEntry::DIRSIZE + 4);
                    }
                    else  /*问题：为何if分支没有置IUPD标志？  这是因为文件的长度没有变呀*/
                    {
                        pInode->i_flag |= Inode::IUPD;
                    }
                    /* 找到可以写入的空闲目录项位置，NameI()函数返回 */
                    return NULL;
                }
                goto out;
            }
            if ( offset % Inode::BLOCK_SIZE == 0 )
            {
                if ( NULL != pBuf )
                {
                    bufmgr->brelse(pBuf);
                }
                /* 计算要读的物理盘块号 */
                int phyBlkno = pInode->bmap(offset / Inode::BLOCK_SIZE );
                pBuf = bufmgr->bread(phyBlkno);

            }
            byte* src = (byte *)(pBuf->b_addr + (offset % Inode::BLOCK_SIZE));
            IOMove(src, (byte*)&de, sizeof(DirectoryEntry));

            offset += (DirectoryEntry::DIRSIZE + 4);
            count--;
            /* 如果是空闲目录项，记录该项位于目录文件中偏移量 */
            if ( 0 == de.m_ino )
            {
                if ( 0 == freeEntryOffset )
                {
                    freeEntryOffset = offset;
                }
                /* 跳过空闲目录项，继续比较下一目录项 */
                continue;
            }
            int i;
            for ( i = 0; i < DirectoryEntry::DIRSIZE; i++ )
                if ( dbuf[i] != de.m_name[i] )
                    break;	/* 匹配至某一字符不符，跳出for循环 */
            if( i < DirectoryEntry::DIRSIZE )
                /* 不是要搜索的目录项，继续匹配下一目录项 */
                continue;
            else
                /* 目录项匹配成功，回到外层While(true)循环 */
                break;
        }
        // 匹配成功
        if (pBuf != NULL)
            bufmgr->brelse(pBuf);

        if ( FileMgr::DELETE == mode && '\0' == *curchar )
        {
            /* 如果对父目录没有写的权限  略*/
            return pInode;
        }
        /*
         * 匹配目录项成功，则释放当前目录Inode，根据匹配成功的
         * 目录项m_ino字段获取相应下一级目录或文件的Inode。
         */
        ib->iput(pInode);
        pInode = ib->iget(de.m_ino);
        /* 回到外层While(true)循环，继续匹配Pathname中下一路径分量 */

        if ( NULL == pInode )	/* 获取失败 */
        {
            return NULL;
        }
    }
out:
    ib->iput(pInode);
    return NULL;
}

void FileMgr::writeDir(Inode *pInode)
{
    InodeTable *ib = VDFileSys::getInstance().getInodeTable();
    de.m_ino = pInode->i_number;

    for(int i = 0; i < DirectoryEntry::DIRSIZE; ++i)
        de.m_name[i] = dbuf[i];

    count = DirectoryEntry::DIRSIZE + 4;
    base  = (byte*)(&de);
    pdir->writei();
    ib->iput(pdir);
}

Inode* FileMgr::mknode(int mode)
{
    FileSystem *fsys = VDFileSys::getInstance().getFileSystem();
    Inode *pInode;
    pInode = fsys->ialloc();
    if(pInode == NULL)
        return NULL;
    pInode->i_flag |= (Inode::IACC | Inode::IUPD);
    pInode->i_mode = mode | Inode::IALLOC;
    pInode->i_nlink = 1;

    this->writeDir(pInode);
    return pInode;
}

void FileMgr::chdir(const char *path)
{
    Inode *pInode;
    InodeTable *ib = VDFileSys::getInstance().getInodeTable();
    pInode = namei(path, FileMgr::OPEN);
    if(pInode == NULL)
    {
        printErr("No such Directory.chdir failed");
        return;
    }
    if((pInode->i_mode & Inode::IFMT) != Inode::IFDIR)
    {
        printErr("No such Directory.chdir failed");
        ib->iput(pInode);
        return ;
    }
    ib->iput(cdir);
    cdir = pInode;

    if ( path[0] != '/' )
    {
        int length = strlen(curdir);
        if ( curdir[length - 1] != '/' )
        {
            curdir[length] = '/';
            length++;
        }
        strcpy(curdir + length, path);
    }
    else	/* 如果是从根目录'/'开始，则取代原有工作目录 */
    {
        strcpy(curdir, path);
    }
}

int FileMgr::fread(int fd, char *buffer, int length)
{
    this->count = length;
    this->base  = (byte*)buffer;

    return 0;
}

int FileMgr::fwrite(int fd, char *buffer, int length)
{
    this->count = length;
    this->base  = (byte*)buffer;
    return 0;
}

void FileMgr::test()
{
    FileSystem *fsys = VDFileSys::getInstance().getFileSystem();
    byte *a;
    a = new byte[135000];
    for(int i = 0; i < 135000; ++i)
        a[i] = 'l';
    a[135000] = '\0';
    count = 135000;
    offset = 0;
    base = a;
    Inode *pNode = fsys->ialloc();
    pNode->i_flag |= (Inode::IACC | Inode::IUPD);
    pNode->i_mode = Inode::IALLOC | Inode::IREAD | Inode::IWRITE | Inode::IEXEC | (Inode::IREAD >> 3) | (Inode::IWRITE >> 3) | (Inode::IEXEC >> 3) | (Inode::IREAD >> 6) | (Inode::IWRITE >> 6) | (Inode::IEXEC >> 6);
    pNode->i_nlink = 1;
    pNode->writei();
}
