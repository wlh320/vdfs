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
    flmgr->init();
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
    fsys->mkfs();
    return 0;
}

void VDFileSys::ls()
{
    //flmgr->ls();
}

void VDFileSys::cd(const char* vpath)
{
    //flmgr->chdir();
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
