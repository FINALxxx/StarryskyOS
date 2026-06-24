#include "test.h"

#define INITNUM 10
task_t handlerA;
task_t handlerB;
task_t handlerC;

// 全局信号
uint32_t spareArea = INITNUM;
uint32_t produceCnt = 0;
uint32_t consumeCnt = 0;

void taskA(task_t* t){ // Producer
    LOCAL();
    TASK_BEGIN(t);

    spareArea = INITNUM;
    while(1){
        printf("A: producing a request.\r\n");
        TASK_YIELD(t); // 假设外部设备生产一个请求需要一点时间
        printf("A: produced a request.\r\n");
        spareArea--;
        produceCnt++;
        TASK_WAIT(t, (spareArea > 0));
    }

    TASK_END(t);
}

void taskB(task_t* t){ // Consumer
    LOCAL();
    TASK_BEGIN(t);

    while(1){
        TASK_WAIT(t, (spareArea < INITNUM));
        printf("B: consumed a request.\r\n");
        spareArea++;
        consumeCnt++;
    }

    TASK_END(t);
}

void taskC(task_t* t){ // Consumer
    LOCAL(
        uint32_t lastProduceCnt; 
        uint32_t lastConsumeCnt;
    );
    TASK_BEGIN(t);
    // 切出函数后不会丢失数据的局部变量
    local.lastConsumeCnt = 0;
    local.lastProduceCnt = 0;
    // 切出函数后会丢失数据的局部临时变量
    uint32_t newConsumeCnt = 0;
    uint32_t newProduceCnt = 0;
    while(1){
        TASK_WAIT_DETAILED(t, 
            {
                // the statement at the pre-jugdement
                newConsumeCnt = consumeCnt - local.lastConsumeCnt;
                newProduceCnt = produceCnt - local.lastProduceCnt;
            },
            (
                // the condition in the judgement
                newConsumeCnt + newProduceCnt > 10
            ),
            {
                // the statement at the post-judgement when JUDGEMENT IS FALSE
            },
            {
                // the statement at the post-judgement when JUDGEMENT IS TRUE
                local.lastConsumeCnt = consumeCnt;
                local.lastProduceCnt = produceCnt;
            }
        );
        
        printf("C: newConsumeCnt + newProduceCnt > 10, stop?(Y/n)\r\n");
        if(hal_sys_getchar() == 'Y'){
            delete_task(&handlerA);
            delete_task(&handlerB);
            printf("C: stopped\r\n");
            break;
        }else{
            printf("C: continue...\r\n");
        }
    }

    TASK_END(t);
}

int coroutine_entry(){
    handlerA = create_task(0, taskA, NULL);
    handlerB = create_task(1, taskB, NULL);
    handlerC = create_task(2, taskC, NULL);
    task_scheduler(-1);
    return 0;
}
