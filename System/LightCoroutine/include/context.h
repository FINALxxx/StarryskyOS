#ifndef CONTEXT_H
#define CONTEXT_H

#include "conf.h"

#ifdef USE_CTX
#include "coroutine.h"

void ctx_init(task_t *task_handler);
void enter_load_from_ctx(task_t *task_handler, uint32_t *local_variable_structure, uint32_t size);
void exit_save_to_ctx(task_t *task_handler, uint32_t *local_variable_structure, uint32_t size);
#endif

#endif