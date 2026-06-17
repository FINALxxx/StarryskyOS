# 03: 变量与赋值

Status: ready-for-agent

## Parent

[PRD: fcc — Tiny C Compiler](../PRD.md)

## What to build

在 Issue 02 的基础上，支持局部变量声明（`int x;`）、局部变量赋值（`x = 10;`）、变量引用（表达式中使用 `x`），以及全局变量（函数外声明）。

局部变量在栈上分配——解析时记录每个变量的栈帧偏移量（`sp + offset`），代码生成使用 `sw`/`lw` 读写。函数退出时恢复 sp。

全局变量在代码缓冲区之后分配固定地址，使用 `lui`/`sw`/`lw` 绝对地址访问。

赋值表达式是右结合：`x = y = 0;` 等价于 `x = (y = 0);`。

## Acceptance criteria

- [ ] 局部变量声明后未初始化，读取返回 0
- [ ] `int x = 3; return x;` 返回 3（声明+初始化）
- [ ] `int x; x = 5; return x;` 返回 5（赋值语句+引用）
- [ ] `int a = 1; int b = 2; return a + b;` 返回 3（多变量）
- [ ] 全局变量 `int g;` + 函数内 `g = 7; return g;` 返回 7
- [ ] 两个函数共享全局变量，验证状态传递正确
- [ ] `int x; int y; x = y = 3; return x + y;` 返回 6（多赋值）
- [ ] 深层嵌套作用域（if/while 块内声明变量）在 MVP 中不支持，报语法错误

## Blocked by

- 02-arithmetic-expressions
