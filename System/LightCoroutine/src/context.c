#include "context.h"
#include <string.h>

#ifdef USE_CTX
void ctx_init(task_t *task_handler){
    memset(task_handler->ctx, 0, sizeof(task_handler->ctx));
    // task_handler->ctx_top = TASK_CTX_SIZE - 1;
}

void enter_load_from_ctx(task_t *task_handler, uint32_t *local_variable_structure, uint32_t size){
    memcpy(local_variable_structure, task_handler->ctx, size);
}

void exit_save_to_ctx(task_t *task_handler, uint32_t *local_variable_structure, uint32_t size){
    memcpy(task_handler->ctx, local_variable_structure, size);
}

// void ctx_push(task_t *task_handler, uint32_t val){
//     uint32_t* sp = &(task_handler->ctx_top);
//     uint32_t* stk = task_handler->ctx;
//     *sp = *sp - 4;
//     stk[*sp] = val;
// }

// uint32_t ctx_pop(task_t *task_handler){
//     uint32_t* sp = &(task_handler->ctx_top);
//     uint32_t* stk = task_handler->ctx;
//     *sp = *sp + 4;
//     return stk[*sp];
// }

// uint8_t ctx_is_empty(task_t *task_handler){
//     return task_handler->ctx_top == TASK_CTX_SIZE - 1;
// }

// uint8_t ctx_is_full(task_t *task_handler){
//     return task_handler->ctx_top == 0;
// }

#endif