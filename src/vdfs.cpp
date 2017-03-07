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
