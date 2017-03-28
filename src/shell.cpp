#include "shell.h"
#include "utils.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>

// 内置命令入口表的定义
CmdTblEntry Shell::cte[CTE_MAX] =
{
    //命令      入口函数           命令功能描述              命令帮助信息
    { "exit",  &Shell::do_exit,  "保存所有更改并退出shell", "exit" },
    { "help",  &Shell::do_help,  "打印命令帮助信息",        "help [命令]" },
    { "ls",    &Shell::do_ls,    "列出路径下的文件",        "ls [路径]" },
    { "cd",    &Shell::do_cd,    "修改当前路径",           "cd 路径" },
    { "load",  &Shell::do_load,  "加载虚拟磁盘文件",        "load 磁盘文件路径"},
    { "eject", &Shell::do_eject, "卸载当前虚拟磁盘文件",     "eject"},
    { NULL,   NULL,            NULL,                   NULL }
};

Shell::Shell()
{
    this->cwd = new char[PATH_MAX];
    strcpy(this->cwd, "/");
    this->disk = new char[PATH_MAX];
    strcpy(this->disk, "No Disk");
    this->cmdbuf = new char[CBUF_MAX];
    this->command = NULL;
    this->args = new char*[ARG_MAX];
    this->argc = 0;

    printf("Welcome to VDFS(Virtual Disk File System) shell 1.0\n");
    printf("Type \"help\" for more information. Type \"exit\" to quit.\n\n");
}

Shell::~Shell()
{
    delete this->cwd;
    delete this->disk;
    delete this->cmdbuf;
    delete this->args;
}

void Shell::printPrompt()
{
    printf("[ %s ] %s", disk, cwd);
    printf(" $ ");
}

// 读取命令
void Shell::inputCommand()
{
    memset((void*)cmdbuf, 0, CBUF_MAX); // 命令buffer置零

    fgets(cmdbuf, CBUF_MAX, stdin);
    cmdbuf[strlen(cmdbuf)-1] = '\0'; // 因为读入了回车
}

//判断是不是分隔符,解析命令时用
bool isDelim(char ch)
{
    if (ch == ' ' || ch == '\t' || ch == '\n')
        return true;
    else
        return false;
}

// 将用户输入翻译为命令和参数
void Shell::parseCommand()
{
    // 命令解析初始化
    this->argc = 0;
    memset((void*)args, 0, ARG_MAX*sizeof(char*));
    int len = strlen(cmdbuf);

    //取出命令
    command = cmdbuf;

    // 分割参数
    for(int i = 0; i < len; ++i)
    {
        if (isDelim(cmdbuf[i]) && !isDelim(cmdbuf[i+1]))
        {
            cmdbuf[i] = '\0'; // 分隔符置为尾0来分割字符串
            args[argc++] = cmdbuf + (i + 1);
            if (argc > ARG_MAX)
                break;
        }
    }
}

// 在命令表中查找并执行命令
void Shell::executeCommand()
{
    int i;
    for(i = 0; cte[i].call != NULL; ++i)
    {
        if (!strcmp(cte[i].trigger, command))
        {
            (this->*Shell::cte[i].CmdTblEntry::call)();//喵喵喵喵喵???
            break;
        }
    }
    if (cte[i].call == NULL)
    {
        printErr("Command not found");
    }
    printf("\n");
}

/////////////////////////////////////////////////////////////////
///              下面是shell内置命令的入口函数                    ///
/////////////////////////////////////////////////////////////////

// 帮助
void Shell::do_help()
{
    if (argc == 0) // 输出所有命令和描述
    {
        printf("\nCommand List\n=============\n");
        printf("\nCommand\t\tDescription");
        printf("\n-------\t\t-----------\n");
        for(int i = 0; cte[i].call != NULL; ++i)
        {
            printf("%s\t\t%s\n", cte[i].trigger, cte[i].desc);
        }
    }
    else // 输出某个命令的详细用法
    {
        int i;
        char *cmdname = args[0];
        for(i = 0; cte[i].call != NULL; ++i)
        {
            if (!strcmp(cte[i].trigger, cmdname))
            {
                printf("Usage: %s\n", cte[i].helpinfo);
                break;
            }
        }
        if (cte[i].call == NULL)
        {
            printf("Usage: Command not found!\n");
        }
    }
}

// 挂载磁盘文件
void Shell::do_load()
{
    if (argc == 0)
    {
        printErr("No disk file name");
    }
    else
    {
        //先尝试打开
        int res = VDFileSys::getInstance().openDisk(this->args[0]);
        VDFileSys::getInstance().mkfs();
        if (res == ERR)
        {
            //打开不成功，创建并格式化一个新磁盘
            printf("WARN: Load disk failed, we'll make one for you. :)\n");
            VDFileSys::getInstance().creatDisk(this->args[0]);
            VDFileSys::getInstance().openDisk(this->args[0]);
            VDFileSys::getInstance().mkfs();
        }
        strcpy(this->disk, this->args[0]);
    }
}

// 卸载磁盘文件
void Shell::do_eject()
{
    if(strcmp(this->disk, "No Disk"))
    {
        VDFileSys::getInstance().closeDisk();
        strcpy(this->disk, "No Disk");
    }
    else
    {
        printf("WARN: No disk is open!");
    }
}

// 退出
void Shell::do_exit()
{
    printf("Bye!\n");
    // 保存修改
    if (this->disk != NULL)
    {
        VDFileSys::getInstance().closeDisk();
    }
    exit(0);
}

void Shell::do_ls()
{
    if (argc == 0) // 列出当前目录
    {

    }
    else // 列出参数目录下的文件
    {
        for(int i = 0; i < argc; ++i)
        {
            printf("%s :\n", args[i]);
            printf("\n");
        }
    }
}

void Shell::do_cd()
{

}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

void Shell::run()
{
    while (true)
    {
        //打印提示
        printPrompt();
        //输入命令
        inputCommand();
        //处理命令
        parseCommand();
        //执行命令
        if (strlen(command) > 0)
            executeCommand();
    }
}
