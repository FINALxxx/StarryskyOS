# 04: 控制流 (if / while)

Status: ready-for-agent

## Parent

[PRD: fcc — Tiny C Compiler](../PRD.md)

## What to build

在 Issue 03 的基础上，支持 `if`/`else` 条件分支和 `while` 循环，以及比较运算符。

比较运算符: `==` `!=` `<` `>` `<=` `>=`。结果是 `int`（0 或 1），遵循 C 语义（真=1，假=0）。

控制流通过 RISC-V 条件分支指令实现。需要 label 回填机制：生成前向跳转指令时暂留占位符，到达目标位置后回填偏移量。`if`/`else` 生成两条跳转（条件 false 跳到 else 块 / 没有 else 时跳到 `if` 后，`if` 块尾跳到 `else` 后）。`while` 在循环尾生成无条件跳回循环头的指令。

## Acceptance criteria

- [ ] `if (1) return 1; else return 2;` 返回 1（if 分支）
- [ ] `if (0) return 1; else return 2;` 返回 2（else 分支）
- [ ] `int x = 0; while (x < 5) x = x + 1; return x;` 返回 5（循环）
- [ ] `return 3 == 3;` 返回 1（相等为真）
- [ ] `return 3 != 3;` 返回 0（不等为假）
- [ ] `return 3 < 5;` 返回 1
- [ ] `return 5 <= 5;` 返回 1
- [ ] `return 5 > 3;` 返回 1
- [ ] `return 3 >= 5;` 返回 0
- [ ] 嵌套 if-else 和嵌套 while 正确执行

## Blocked by

- 03-variables-and-assignment
