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
void setBit0(unsigned int* value, int bitPosition);
void setBit1(unsigned int* value, int bitPosition);
int firstOnePos(unsigned int x);

// 出错提示
void printErr(const char *msg);
void printWarn(const char *msg);

// 复制
void IOMove(byte *from, byte *to, int count);

// 时间
int getTime();

// 最小
int min(int a, int b);

//简化路径
char* simplifyPath(char* path);

#endif //UTILS_H
