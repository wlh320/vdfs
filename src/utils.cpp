#include <cstdint>
#include <cstdio>
#include <ctime>
#include "utils.h"

//set bitPostion in value to 1
void setBit1(unsigned int* value, int bitPosition)
{
    *value |= bitPosition;
}

//set bitPostion in value to 0
void setBit0(unsigned int* value, int bitPosition)
{
    *value &= ~(bitPosition);
}

// 奇技淫巧返回第一个为1的位置
// http://www.matrix67.com/blog/archives/3985
int firstOnePos(unsigned int x)
{
    static const int MultiplyDeBruijnBitPosition[32] =
    {
      0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
      31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
    };

    return MultiplyDeBruijnBitPosition[((uint32_t)((x & -x) * 0x077CB531U)) >> 27];
}

void printErr(const char *msg)
{
    printf("Error: %s !\n", msg);
}

void printWarn(const char *msg)
{
    printf("Warning: %s !\n", msg);
}

void IOMove(byte *from, byte *to, int count)
{
    while(count--)
        *to++ = *from++;
}

int getTime()
{
    return std::time(nullptr);
}


int min(int a, int b)
{
    return a < b ? a : b;
}

char* simplifyPath(char* path)
{
    int top = -1;
    int i, j;

    for(i = 0; path[i] != '\0'; ++i)
    {
        path[++top] = path[i];
        if(top >= 1 && path[top - 1] == '/' && path[top] == '.' && (path[i + 1] == '/' || path[i + 1] == '\0'))
            top -= 2;
        else if(top >= 2 && path[top - 2] == '/' && path[top - 1] == '.' && path[top] == '.' && (path[i + 1] == '/' || path[i + 1] == '\0'))
        {
            for(j = top - 3; j >= 0; --j)
                if(path[j] == '/')
                    break;
            if(j < 0)
                top = -1;
            else
                top = j - 1;
        }
        else if(path[top] == '/' && path[i + 1] == '/')
            --top;
    }
    if(top > 0)
    {
        if(path[top] == '/') path[top] = '\0';
        else path[top + 1] = '\0';
    }
    else if(top == 0)
        path[top + 1] = '\0';
    else
    {
        path[0] = '/';
        path[1] = '\0';
    }
    return path;
}
