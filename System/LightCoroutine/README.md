# 仓库说明
本仓库实现了一个轻量化的半有栈协程库，可用于无中断嵌入式设备上的多任务需求。

## 使用说明
1. 半有栈协程库：复杂度和灵活度介于有栈协程、无栈协程之间，可以提供比有栈协程更高的性能，也能在协程中使用无栈协程不能使用的局部变量
2. 有编译器限制：本项目使用了LabelPointer、ComputedGoto功能，这些功能是GCC的非标准扩展，因此代码必须使用GCC或兼容的编译器编译
3. 无嵌入式设备限制：
    - 能够支持无malloc/free的特殊场景（局部变量栈使用数组创建）
    - 能够支持任意内核的嵌入式设备（局部变量栈不保存内核上下文信息）
4. 项目内部使用了resume表示就绪态、suspend表示阻塞态，这只是因为作者不想重构变量名导致的（鉴定为懒）

## 使用语法
该协程库可以设置是否启用局部变量栈，以在性能-灵活度之间进行tradeoff
### 1. 配置
```c
#define USE_IDLE_TASK  // 启用空闲任务
#define MAX_TASK_NUM 10 // 设置非空闲的任务数量
#define EXEC_WAIT_POLICY (resume_cnt <= suspend_cnt) // 调度器进入阻塞态任务的策略（当wait任务不满足该策略时，不会进入任务以提高性能）

#define USE_CTX  // 启用局部变量栈
#define TASK_CTX_SIZE 64 // 设置栈中局部变量的个数（每一个变量都固定为uint32_t）
```
### 2. 无栈协程语法
- 前提：不启用USE_CTX
- 兼容性：有栈语法包含了所有的无栈语法，因此不启用USE_CTX也可以执行有栈语法，但有栈语法的特性不会生效
- 增删任务
    - create_task()：将任务标记为resume=1,suspend=0(运行态)，并返回对应只读的任务handler
    - delete_task()：将任务标记为resume=0,suspend=0(删除态)
    - resume_task()：将任务标记为resume=1,suspend=0(运行态)
    - suspend_task()：将任务标记为resume=0,suspend=1(挂起态)
- 任务函数框架
    ```c
    void task(task_t* handler){
        TASK_BEGIN(handler);
        // 编写任务函数
        TASK_END(handler);
    }
    ```
- 任务函数语法
    - TASK_BEGIN(handler)：运行到此处时，会自动跳转到上次退出的位置
    - TASK_END(handler)：运行到此处时，会结束掉任务

    - TASK_YIELD(handler)：运行到此处时，会退出函数（立即让出CPU），下次进入时将从下一条指令执行
    - TASK_YIELD_DETAILED(t, {post_true})：同上，但在退出之前会执行post_true内的语句
    
    - TASK_WAIT(handler, (cond))：
        - 运行到此处时，若cond成立，将从下一条指令执行；
        - 若cond不成立，将会退出函数（立即让出CPU），下次进入时再次判断cond
    - TASK_WAIT_DETAILED(t, {pre}, (cond), {post_false}, {post_true})：
        - 同上
        - 在判断cond之前先执行一次pre
        - 在cond成立、执行下一条指令之前先执行一次post_true
        - 在cond不成立、退出之前执行post_false
- 示例
    ```c
    uint32_t sig = 0x0;
    void task(task_t* t){
        TASK_BEGIN(t);
        printf("step 1\n");
        TASK_YIELD(t);

        printf("step 2\n");
        TASK_WAIT(t, (sig == 0x1));

        printf("step 3\n");
        TASK_YIELD(t);

        printf(">>>done\n");
        TASK_END(t);
    }
    ```
### 2. 半有栈协程语法
- 前提：启用USE_CTX
- 兼容性：有栈语法包含了所有的无栈语法，因此启用USE_CTX也可以执行无栈语法，但有栈语法必须添加的部分不能缺失
- 增删任务
    - create_task()：将任务标记为resume=1,suspend=0(就绪态)，并创建局部变量栈，然后返回对应只读的任务handler
    - delete_task()：将任务标记为resume=0,suspend=0(删除态)
    - resume_task()：将任务标记为resume=1,suspend=0(就绪态)
    - suspend_task()：将任务标记为resume=0,suspend=1(挂起态)
- 任务函数框架
    ```c
    void task(task_t* handler){
        LOCAL(
            // 定义uint32_t类型的局部变量（自动初始化）
        );
        TASK_BEGIN(handler);
        // 编写任务函数
        TASK_END(handler);
    }
    ```
- 任务函数语法
    - TASK_BEGIN(handler)：运行到此处时，将会导入局部变量栈，然后自动跳转到上次退出的位置
    - TASK_END(handler)：运行到此处时，会结束掉任务
    
    - TASK_YIELD(handler)：运行到此处时，会退出函数（立即让出CPU）、导出局部变量栈，下次进入时将从下一条指令执行
    - TASK_YIELD_DETAILED(t, {post_true})：同上，但在退出之前会执行post_true内的语句
    
    - TASK_WAIT(handler, (cond))：
        - 运行到此处时，若cond成立，将从下一条指令执行；
        - 若cond不成立，将会退出函数（立即让出CPU）、导出局部变量栈，下次进入时再次判断cond
    - TASK_WAIT_DETAILED(t, {pre}, (cond), {post_false}, {post_true})：
        - 同上
        - 在判断cond之前先执行一次pre
        - 在cond成立、执行下一条指令之前先执行一次post_true
        - 在cond不成立、退出之前执行post_false
    
    - LOCAL(...)：必须存在，且在函数开头使用该语法，用于声明使用和未使用的局部变量
- 示例
    ```c
    void task(task_t* t){
        LOCAL(
            uint32_t a;
            uint32_t b;
        );
        TASK_BEGIN(t);
        local.a = 1;
        local.b = 2;
        printf("step 1\n");
        TASK_YIELD(t);

        printf("step2: local.a = %d, local.b = %d\n", local.a, local.b);
        local.a = 3;
        local.b = 4;
        TASK_YIELD(t);
        
        printf("step2: local.a = %d, local.b = %d\n", local.a, local.b);
        printf(">>>done\n");
        TASK_END(t);
    }
    ```
### 启动调度器
- 创建任务应在启动调度器之前进行
- 主动关闭调度器是UB行为
- 启动调度器语法
    - task_scheduler(-1)：启动调度器，并永久执行，不会关闭调度器
    - task_scheduler(round)：启动调度器，并执行round轮调度，然后会自动关闭调度器
- 调度器策略
    - 使用无时间片的Round-Robin调度，因此一个任务不能占用CPU过久
    - 调度器总是会执行处于resume态(就绪态)的任务
    - 调度器会自动跳过处于suspend态(阻塞态)的任务，除非任务满足EXEC_WAIT_POLICY策略
    - **进入**suspend态任务不代表**执行**suspend态任务，进入它只是为了再次考察内部的等待条件是否满足


## 注意事项
1. 在设计协程时需要规避死锁（互斥条件、请求并保持、不可剥夺、循环等待）
2. 空闲任务可以统计调度器中各运行状态的任务数，以便于调试
3. 若任务需要永久执行，可以在TASK_BEGIN-TASK_END之间使用while(1)，但不要在TASK_END之后使用while(1)
4. 使用该仓库时请注意相关LICENSE的限制