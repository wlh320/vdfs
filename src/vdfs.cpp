#include "vdfs.h"
//#include <fstream>
#include <cstdio>

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
    ib->init();
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
        // 创建基础目录
        flmgr->init();
        flmgr->creatDir("/bin");
        flmgr->creatDir("/etc");
        flmgr->creatDir("/usr");
        flmgr->creatDir("/home");
        return 0;
    }
    return -1;
}

void VDFileSys::ls()
{
    if(dskmgr->isOpen())
    {
        DirectoryEntry tde;
        int res = flmgr->fopen(flmgr->curdir,File::FREAD);
        if (res == 1)
        {
            int cnt = 0;
            while(true)
            {
                cnt = flmgr->fread((char*)&tde,32);
                if (cnt == 0)
                    break;
                if (tde.m_ino != 0 || tde.m_name[0] == '.')
                    printf("%s\t", tde.m_name);
            }
            printf("\n");
        }
        else
            printErr("Not a dir");

        flmgr->fclose();
    }
}

void VDFileSys::ls(const char* path)
{
    if(dskmgr->isOpen())
    {
        DirectoryEntry tde;
        int res = flmgr->fopen(path,File::FREAD);
        if (res == 1)
        {
            int cnt = 0;
            while(true)
            {
                cnt = flmgr->fread((char*)&tde,32);
                if (cnt == 0)
                    break;
                if (tde.m_ino != 0 || tde.m_name[0] == '.')
                    printf("%s\t", tde.m_name);
            }
            printf("\n");
        }
        else
            printErr("Not a dir");
        flmgr->fclose();
    }
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
    int cnt;
    // C标准库打开外部文件
    FILE *rfile = fopen(rpath,"rb");
    int vfile = flmgr->fopen(vpath,File::FWRITE);
    if (rfile == NULL)
    {
        printErr("Read real file failed");
        return;
    }
    if (vfile == -1)
    {
        flmgr->fcreat(vpath, Inode::IALLOC);
        flmgr->fopen(vpath, File::FWRITE);
    }
    else if(vfile == 1)
    {
        printErr("your input is NOT a file");
        return ;
    }

    char *buffer = new char[512];
    while(true)
    {
        cnt = fread(buffer, 1, 512, rfile);
        if(cnt == 0)
            break;
        flmgr->fwrite(buffer, cnt);
    }
    fclose(rfile);
    flmgr->fclose();
    delete []buffer;
}

void VDFileSys::load(const char* vpath, const char* rpath)
{
    int cnt;
    // C标准库打开外部文件
    FILE *rfile = fopen(rpath,"wb");
    // 我的调用打开内部文件
    int vfile = flmgr->fopen(vpath,File::FREAD);

    if (vfile != 0)
    {
        printErr("Read virtual file failed");
        return;
    }

    char *buffer = new char[512];
    while(true)
    {
        cnt = flmgr->fread(buffer, 512);
        if(cnt == 0)
            break;
        fwrite(buffer, 1, cnt, rfile);
    }
    if (rfile)
        fclose(rfile);
    flmgr->fclose();
    delete []buffer;
}

void VDFileSys::rm(const char* vpath)
{
    if(dskmgr->isOpen())
    {
        flmgr->fdelete(vpath);
    }
}

void VDFileSys::cat(const char *path)
{
    if(dskmgr->isOpen())
    {
        char *buffer = new char[512];
        int res = flmgr->fopen(path,File::FREAD);
        if (res == 0)
        {
            int cnt = 0;
            while(true)
            {
                cnt = flmgr->fread(buffer,512);
                if (cnt == 0)
                    break;
                for(int i = 0; i < cnt; ++i)
                    printf("%c", buffer[i]);
            }
            printf("\n");
        }
        else
            printErr("Not a file");
        flmgr->fclose();
        delete []buffer;
    }
}
