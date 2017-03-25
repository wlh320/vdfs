/*
 * Project     : Unix style Virtual Disk File System
 * Author      : wlh
 * Date        : 2017/03/18
 * Description : 一些额外的工具和定义
 */

#ifndef UTILS_H
#define UTILS_H

typedef unsigned char byte;

#define YES 0
#define ERR 1

// bitmap functions
void setBit(unsigned int* value, int bitPosition);
void clearBit(unsigned int* value, int bitPosition);
int firstZeroPos(unsigned int x);

// 出错提示
void printErr(const char *msg);
void printWarn(const char *msg);

// 复制
void IOMove(byte *from, byte *to, int count);

#endif //UTILS_H
