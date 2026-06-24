#include "main.h"
#include "test.h"

void easy_print(void* param){
    printf("I will print: %s\r\n", (char*)param);
}

void mini_coroutine_test(void* param){
    coroutine_entry();
}

void main(){
    hal_sys_uart_init();

    // 为exec_func功能创建函数调用链
    flist[0] = (func_node){
        .func = easy_print,
        .default_param = "Hello, World",
    };

    flist[1] = (func_node){
        .func = mini_coroutine_test,
        .default_param = NULL,
    };

    create_shell_env_varible();

    load_shell();

    while(1){
        printf("Never Reaching here.\r\n");
    }
    
}
