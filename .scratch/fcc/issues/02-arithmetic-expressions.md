# 02: 算术表达式

Status: ready-for-agent

## Parent

[PRD: fcc — Tiny C Compiler](../PRD.md)

## What to build

在 Issue 01 的基础上，支持 `+` `-` `*` `/` `%` 二元运算符和一元 `-`，递归下降解析带正确优先级和结合性。纯栈式代码生成。

表达式语法层次（低到高优先级）：
- 加法级: `+` `-`（左结合）
- 乘法级: `*` `/` `%`（左结合）
- 一元: `-`
- 原子: 整数、`(` 表达式 `)`

代码生成使用栈直传结果——中间结果 push 到栈上，运算时 pop 到临时寄存器，结果 push 回栈。

## Acceptance criteria

- [ ] `return 3 + 4;` 返回 7
- [ ] `return 10 - 2 * 3;` 返回 4（乘法优先）
- [ ] `return (1 + 2) * 3;` 返回 9（括号覆盖）
- [ ] `return 10 / 3;` 返回 3（整数除法截断）
- [ ] `return 10 % 3;` 返回 1（模运算）
- [ ] `return -5 + 10;` 返回 5（一元负号）
- [ ] `return 1 + 2 + 3 + 4 + 5;` 返回 15（同优先级结合性）
- [ ] `return 100 * 100 / 100;` 返回 100（左结合链）

## Blocked by

- 01-skeleton-return-int
