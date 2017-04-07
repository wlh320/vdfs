#include "file.h"
#include "utils.h"
#include "vdfs.h"
#include <cstring>
#include <cstdio>

File::File()
{
    this->inode = NULL;
    this->flag = 0;
    this->f_count = 0;
    this->f_offset = 0;
}
void File::close()
{
    this->inode = NULL;
    this->flag = 0;
    this->f_count = 0;
    this->f_offset = 0;
}
bool File::isOpen()
{
    return (this->inode != NULL);
}

/////////////////////////filemanager//////////////////////////

void FileMgr::init()
{
    InodeTable *ib = VDFileSys::getInstance().getInodeTable();
    this->count = 0;
    this->offset = 0;
    this->base = NULL;

    strcpy(curdir, "/");
    this->rootDirInode = ib->iget(FileSystem::ROOTINO);
    this->cdir = ib->iget(FileSystem::ROOTINO);
    this->pdir = ib->iget(FileSystem::ROOTINO);
    this->file = new File();
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

    if(*curchar == '/')
    {
        pInode = this->rootDirInode;
        curchar++;
    }
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
                    else
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
                    bufmgr->brelse(pBuf);
                /* 计算要读的物理盘块号 */
                int phyBlkno = pInode->bmap(offset / Inode::BLOCK_SIZE );
                pBuf = bufmgr->bread(phyBlkno);

            }
            byte* src = (byte *)(pBuf->b_addr + (offset % Inode::BLOCK_SIZE));
            IOMove(src, (byte*)&de, sizeof(DirectoryEntry));

            offset += (DirectoryEntry::DIRSIZE + 4);
            count--;
            /* 如果是空闲目录项，记录该项位于目录文件中偏移量 */
            if ( 0 == de.m_ino && offset > 64)
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
            // 删除模式,返回父目录的inode
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
    BufMgr *bufmgr = VDFileSys::getInstance().getBufMgr();
    Inode *pInode;
    pInode = fsys->ialloc();
    if(pInode == NULL)
        return NULL;
    pInode->i_flag |= (Inode::IACC | Inode::IUPD);
    pInode->i_mode = mode | Inode::IALLOC;
    pInode->i_nlink = 1;

    if ( (pInode->i_mode & Inode::IFMT) == Inode::IFDIR ) //初始化.和..
    {
        Buf *newBuf = fsys->dalloc();
        DirectoryEntry initde;
        initde.m_ino = pInode->i_number;
        initde.m_name[0] = '.';
        for(int i = 1; i < DirectoryEntry::DIRSIZE; ++i)
            initde.m_name[i] = '\0';
        IOMove((byte*)&initde, newBuf->b_addr, sizeof(DirectoryEntry));
        initde.m_ino = pdir->i_number;
        initde.m_name[1] = '.';
        IOMove((byte*)&initde, newBuf->b_addr + sizeof(DirectoryEntry), sizeof(DirectoryEntry));
        pInode->i_addr[0] = newBuf->b_blkno;
        pInode->i_size = 2 * sizeof(DirectoryEntry);
        bufmgr->bwrite(newBuf);
    }

    this->writeDir(pInode);
    return pInode;
}

int FileMgr::chdir(const char *path)
{
    Inode *pInode;
    InodeTable *ib = VDFileSys::getInstance().getInodeTable();
    pInode = namei(path, FileMgr::OPEN);
    if(pInode == NULL)
    {
        printErr("No such Directory.chdir failed");
        return -1;
    }
    if((pInode->i_mode & Inode::IFMT) != Inode::IFDIR)
    {
        printErr("No such Directory.chdir failed");
        ib->iput(pInode);
        return -1;
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
    simplifyPath(curdir); // 路径简化 去掉.和..
    return 0;
}

void FileMgr::creatDir(const char *path)
{
    InodeTable *ib = VDFileSys::getInstance().getInodeTable();
    Inode* pInode = namei(path, FileMgr::CREATE);
    if(pInode == NULL)
    {
        pInode = mknode(Inode::IFDIR | 0x1ff);
        ib->iput(pInode);
    }
}

int FileMgr::fopen(const char *name, int mode)
{
    Inode *pInode = namei(name, FileMgr::OPEN);
    if (pInode == NULL)
        return -1;
    file->inode = pInode;
    file->flag  = mode;

    if ((pInode->i_mode & Inode::IFMT) == Inode::IFDIR)
        return 1; // 打开目录

    return 0; // 打开文件
}

void FileMgr::fclose()
{
    InodeTable *ib = VDFileSys::getInstance().getInodeTable();
    ib->iput(file->inode);
    file->close();
}

int FileMgr::fcreat(const char *name, int mode)
{
    Inode *pInode = namei(name, FileMgr::CREATE);
    if(pInode == NULL)
    {
        pInode = mknode(mode | 0x1ff);
        if(pInode == NULL)
            return -1;
    }
    else
        pInode->itrunc();
    return 0;
}

int FileMgr::fread(char *buffer, int length)
{
    if(file->isOpen() && file->flag == File::FREAD)
    {
        this->count  = length;
        this->base   = (byte*)buffer;
        this->offset = file->f_offset;
        file->inode->readi();

        file->f_offset = this->offset;
    }
    return (length - this->count);
}

int FileMgr::fwrite(char *buffer, int length)
{
    if(file->isOpen() && file->flag == File::FWRITE)
    {
        this->count  = length;
        this->base   = (byte*)buffer;
        this->offset = file->f_offset;
        file->inode->writei();

        file->f_offset = this->offset;
    }
    return (length - this->count);
}

void FileMgr::fdelete(const char *name)
{
    Inode *pInode;
    Inode *pParentInode;
    InodeTable *ib = VDFileSys::getInstance().getInodeTable();

    pParentInode = namei(name, FileMgr::DELETE);
    if (pParentInode == NULL)
    {
        printErr("Delete:No such file");
        return ;
    }
    pInode = ib->iget(de.m_ino);
    /* 写入清零后的目录项 */
    this->offset -= sizeof(DirectoryEntry);
    this->base = (byte*)&de;
    this->count = sizeof(DirectoryEntry);

    de.m_ino = 0;
    pParentInode->writei();
    /* 修改inode项 */
    pInode->i_nlink--;
    pInode->i_flag |= Inode::IUPD;

    ib->iput(pParentInode);
    ib->iput(pInode);
}
