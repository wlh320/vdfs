#include "vdfs.h"


//////////////////////////////////////////
// !!!全局单例变量
VDFileSys VDFileSys::instance;
//////////////////////////////////////////

VDFileSys::VDFileSys()
{
    this->bufmgr = new BufMgr();
    this->dskmgr = new DiskMgr();
    this->fsys   = new FileSystem();

    bufmgr->init();
    fsys->init();
}

VDFileSys::~VDFileSys()
{
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

FileSystem* VDFileSys::getFileSystem()
{
    return this->fsys;
}

int VDFileSys::mkfs()
{
    fsys->mkfs();
    return 0;
}

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
