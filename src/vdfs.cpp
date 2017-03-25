#include "vdfs.h"


//////////////////////////////////////////
// !!!全局单例变量
BufMgr g_bufmgr;
DiskMgr g_dskmgr;
FileSystem g_fsys;
VDFileSys VDFileSys::instance;
//////////////////////////////////////////

VDFileSys::VDFileSys()
{
    this->bufmgr = &g_bufmgr;
    this->dskmgr = &g_dskmgr;
    this->fsys   = &g_fsys;
}

VDFileSys::~VDFileSys()
{
}

VDFileSys& VDFileSys::getInstance()
{
    return VDFileSys::instance;
}

BufMgr& VDFileSys::getBufMgr()
{
    return *(this->bufmgr);
}

DiskMgr& VDFileSys::getDiskMgr()
{
    return *(this->dskmgr);
}

FileSystem& VDFileSys::getFileSystem()
{
    return *(this->fsys);
}

int VDFileSys::mkfs()
{
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
