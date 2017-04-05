#include "file.h"
#include "vdfs.h"
#include <cstring>

void FileMgr::init()
{
    this->count = 0;
    this->offset = 0;
    this->base = NULL;

}

void FileMgr::chdir(char *path)
{

}



int FileMgr::fread(int fd, char *buffer, int length)
{

}

int FileMgr::fwrite(int fd, char *buffer, int length)
{

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
    pNode->itrunc();
}
