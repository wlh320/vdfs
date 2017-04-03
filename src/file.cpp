#include "file.h"
#include <cstring>

File::File()
{

}


void FileMgr::init()
{
    this->count = 0;
    this->offset = 0;
    this->base = NULL;
}
