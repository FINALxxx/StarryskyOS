# 05: 函数调用 + exec_func

Status: ready-for-agent

## Parent

[PRD: fcc — Tiny C Compiler](../PRD.md)

## What to build

在 Issue 04 的基础上，支持多函数定义和函数调用（包括调用自身定义的用户函数和调用内置白名单函数 `exec_func`）。

函数调用约定（RISC-V standard calling convention）：
- 调用者：参数放入 `a0`-`a7` 寄存器，`jal` 跳转到被调用函数
- 被调用者：栈帧保存 `ra`，函数体执行，返回值放入 `a0`，`ret` 返回

白名单符号解析：遇到 `exec_func` 标识符时，不查找用户定义的函数，而是直接编码为对 `envFunc` 地址的 `jalr` 间接调用。`envFunc` 的值在编译器初始化时从 shell 环境变量读取。

## Acceptance criteria

- [ ] `int add(int a, int b) { return a + b; }` `int main() { return add(3, 4); }` 返回 7
- [ ] 三个函数 A→B→C 调用链，C 返回 42，main 收到 42
- [ ] `int main() { exec_func(0, 0, 99); return 0; }` 触发固件回调，第三个参数为 99
- [ ] `exec_func` 不声明直接调用，编译器正确处理（内建符号）
- [ ] 函数重名报错
- [ ] 调用未定义函数报错
- [ ] 函数参数数量超过 8 个报错
- [ ] `return` 值类型与函数返回类型一致（只有 `int`，此项自然满足）

## Blocked by

- 04-control-flow-if-while
