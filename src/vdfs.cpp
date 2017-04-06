#include "vdfs.h"
#include <fstream>

//////////////////////////////////////////
// !!!全局单例变量
VDFileSys VDFileSys::instance;
//////////////////////////////////////////

VDFileSys::VDFileSys()
{
    this->bufmgr = new BufMgr();
    this->dskmgr = new DiskMgr();
    this->flmgr  = new FileMgr();
    this->fsys   = new FileSystem();
    this->ib     = new InodeTable();

    bufmgr->init();
    fsys->init();
}

VDFileSys::~VDFileSys()
{
    delete fsys;
    delete flmgr;
    delete dskmgr;
    delete bufmgr;
}

VDFileSys& VDFileSys::getInstance()
{
    return VDFileSys::instance;
}

BufMgr* VDFileSys::getBufMgr()
{
    return this->bufmgr;
}

DiskMgr* VDFileSys::getDiskMgr()
{
    return this->dskmgr;
}

FileMgr* VDFileSys::getFileMgr()
{
    return this->flmgr;
}

FileSystem* VDFileSys::getFileSystem()
{
    return this->fsys;
}

InodeTable* VDFileSys::getInodeTable()
{
    return this->ib;
}

///////////////////////////////////////////////

int VDFileSys::openDisk(char *diskfile)
{
    int res = dskmgr->openDisk(diskfile);
    return res;
}

int VDFileSys::creatDisk(char *diskfile)
{
    int res = dskmgr->creatDisk(diskfile);
    return res;
}

int VDFileSys::closeDisk()
{
    dskmgr->closeDisk();
    return 0;
}

void VDFileSys::loadFilesys()
{
    fsys->loadSuperBlock();
}

int VDFileSys::mkfs()
{
    if (dskmgr->isOpen())
    {
        fsys->mkfs();
        return 0;
    }
    return -1;
}

void VDFileSys::ls()
{
    if(dskmgr->isOpen())
    {
        DirectoryEntry tde;
        flmgr->fopen(flmgr->curdir,File::FREAD);
        int cnt = 0;
        while(true)
        {
            cnt = flmgr->fread((char*)&tde,32);
            if (cnt == 0)
                break;
            printf("%s\t", tde.m_name);
        }
        flmgr->fclose();
    }
    printf("\n");
}

int VDFileSys::cd(const char *vpath)
{
    if(dskmgr->isOpen())
    {
        int res = flmgr->chdir(vpath);
        return res;
    }
    return -1;
}

void VDFileSys::mkdir(const char *vpath)
{
    if(dskmgr->isOpen())
    {
        flmgr->creatDir(vpath);
    }
}

void VDFileSys::save(const char* rpath, const char* vpath)
{

}

void VDFileSys::load(const char* vpath, const char* rpath)
{

}

void VDFileSys::rm(const char* vpath)
{

}
