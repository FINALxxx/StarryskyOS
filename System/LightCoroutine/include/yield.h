#ifndef _YIELD_H_
#define _YIELD_H_

#include "conf.h"

#define CONCAT(a, b) a##b
#define LABEL(line) CONCAT(L, line)

#define TASK_END(t)                                    \
    do{                                                \
        (t)->resume_point = NULL;                      \
        delete_task(t);                                \
    }while(0);                                         \

#ifdef USE_CTX
#define TASK_BEGIN(t)                                  \
    do{                                                \
        enter_load_from_ctx(t, (uint32_t*)&local, sizeof(local));                \
        resume_task(t);                                \
        if((t)->resume_point) goto *(t)->resume_point; \
        ctx_init(t);                                   \
    }while(0)

#define TASK_YIELD_INNER(t, label, post)            \
    do{                                             \
        exit_save_to_ctx(t, (uint32_t*)&local, sizeof(local));        \
        (t)->resume_point = &&label;                \
        post                                        \
        return;                                     \
        label: ;                                    \
    } while(0)

#define TASK_WAIT_INNER(t, label, pre, cond, post_false, post_true)     \
    do{                                     \
        (t)->resume_point = &&label;        \
        label: ;                            \
        pre                                 \
        if(!(cond)){                        \
            exit_save_to_ctx(t, (uint32_t*)&local, sizeof(local));    \
            suspend_task(t);                \
            post_false                      \
            return;                         \
        }                                   \
        post_true                           \
    }while(0)

#define TASK_YIELD_DETAILED(t, post) TASK_YIELD_INNER(t, LABEL(__LINE__), post)
#define TASK_YIELD(t) TASK_YIELD_INNER(t, LABEL(__LINE__), {})
#define TASK_WAIT_DETAILED(t, pre, cond, post_false, post_true) TASK_WAIT_INNER(t, LABEL(__LINE__), pre, cond, post_false, post_true)
#define TASK_WAIT(t, cond) TASK_WAIT_INNER(t, LABEL(__LINE__), {}, cond, {}, {})

#define LOCAL(...)                  \
    struct { __VA_ARGS__ } local;   \
    (void)local;

#else

#define TASK_BEGIN(t)                                  \
    do{                                                \
        resume_task(t);                                \
        if((t)->resume_point) goto *(t)->resume_point; \
    }while(0)

#define TASK_YIELD_INNER(t, label, post)    \
    do{                                     \
        (t)->resume_point = &&label;        \
        post                                \
        return;                             \
        label: ;                            \
    } while(0)


#define TASK_WAIT_INNER(t, label, pre, cond, post_false, post_true)     \
    do{                                     \
        (t)->resume_point = &&label;        \
        label: ;                            \
        pre                                 \
        if(!(cond)){                        \
            suspend_task(t);                \
            post_false                      \
            return;                         \
        }                                   \
        post_true                           \
    }while(0)


#define TASK_YIELD_DETAILED(t, post) TASK_YIELD_INNER(t, LABEL(__LINE__), post)
#define TASK_YIELD(t) TASK_YIELD_INNER(t, LABEL(__LINE__), {})
#define TASK_WAIT_DETAILED(t, pre, cond, post_false, post_true) TASK_WAIT_INNER(t, LABEL(__LINE__), pre, cond, post_false, post_true)
#define TASK_WAIT(t, cond) TASK_WAIT_INNER(t, LABEL(__LINE__), {}, cond, {}, {})

#define LOCAL(...) do{}while(0)
#endif






#endif