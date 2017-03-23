#include "vdfs.h"

VDFileSys::VDFileSys()
{
    bufmgr = new BufMgr();
    dskmgr = new DiskMgr();
}

VDFileSys::~VDFileSys()
{
    delete bufmgr;
    delete dskmgr;
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
