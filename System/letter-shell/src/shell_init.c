#include "shell_init.h"

// 如果发现系统不正常重启，大概率是执行了地址为0的函数（可能因为函数未定义）

void load_shell(){
    char shellBuffer[1024];
    char shellPathBuffer[1024] = "/";

#ifdef CONFIG_COMPONENT_FLASH_FS
    ShellFs shellfs;
    Shell shell;

    printf("\r\n");
    printf("Waiting for filesystem booting...\r\n");

    load_filesystem();

    printf("Waiting for shell_fs booting...\r\n");  
    shellfs.chdir = chdir;
    shellfs.getcwd = getcwd;
    shellfs.listdir = listdir;
    shellfs.createfile = createfile;
    shellFsInit(&shellfs, shellPathBuffer, 1024);

    printf("Waiting for shell booting...\r\n");
    shell.read = shellRead;
    shell.write = shellWrite;
    shellSetPath(&shell,shellPathBuffer);
    shellInit(&shell, shellBuffer, 1024);
    shellCompanionAdd(&shell, SHELL_COMPANION_ID_FS, &shellfs);
#else
    Shell shell;

    printf("\r\n");
    printf("Waiting for shell booting...\r\n");
    shell.read = shellRead;
    shell.write = shellWrite;
    shellSetPath(&shell,shellPathBuffer);
    shellInit(&shell, shellBuffer, 1024);
#endif

    while(1){
        shellTask(&shell);
    }
}