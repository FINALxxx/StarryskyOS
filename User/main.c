#include "main.h"

void easy_print(void* param){
    printf("I will print: %s\r\n", (char*)param);
}

// Tips: Letter-shell uses slow sys_uart leaving hp_uart available for high-demand situation.
//       if there exists a communication loss, it'll be fine by setting uart boudrate to 9600.
void main(){
    hal_sys_uart_init();

    func_node func_node = {
        .func = easy_print,
        .param = "Hello, World",
        .next = NULL,
    };
    set_fnode(&func_node);

    create_shell_env_varible();

    load_shell();

    while(1){
        printf("Never Reaching here.\r\n");
    }
    
}
