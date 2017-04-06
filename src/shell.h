/*
 * Project     : Unix-style Virtual Disk File System
 * Author      : wlh
 * Date        : 2017/01/30
 * Description : 与用户交互的shell
 */

#ifndef SHELL_H
#define SHELL_H

#include "vdfs.h"

const int PATH_MAX = 1024; //路径的最大长度
const int CBUF_MAX = 1024; //命令buffer的最大长度
const int ARG_MAX = 32;    //命令参数的最大个数
const int CTE_MAX = 128;   //命令入口表的最大长度

class Shell;

//内置命令入口表
struct CmdTblEntry
{
    const char* trigger;   //命令的触发标志
    void (Shell::*call)(); //处理命令的函数
    const char* desc;      //对命令的描述
    const char* helpinfo;  //命令的帮助信息
};

class Shell
{

private:
    char *disk;    // Disk 虚拟磁盘文件
    char *cwd;     // Current Working Directory 当前路径
    char *cmdbuf;  // Command Buffer 存储用户输入字符的缓冲

    char *command; // Command 命令
    char **args;    // Arguments array 参数数组
    int  argc;      // Arguments Count 参数个数

    static CmdTblEntry cte[CTE_MAX]; //Built-in Command Entry Table

public:
    Shell();
    ~Shell();

    void printPrompt();   //打印prompt
    void inputCommand();  //输入命令
    void parseCommand();  //解析命令
    void executeCommand();//执行命令
    void run(); //死循环执行

    // 内置命令入口函数
    void do_help();
    void do_exit();

    void do_mount();
    void do_eject();

    void do_cd();
    void do_ls();
    void do_save();
    void do_load();
    void do_rm();
    void do_mkfs();
    void do_mkdir();

};

#endif // SHELL_H
