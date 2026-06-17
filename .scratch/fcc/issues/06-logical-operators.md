# 06: 逻辑运算符

Status: ready-for-agent

## Parent

[PRD: fcc — Tiny C Compiler](../PRD.md)

## What to build

在 Issue 05 的基础上，支持 `&&` `||` `!` 逻辑运算符，含短路求值。

短路语义：
- `a && b`: 先求值 `a`，若 `a` 为 0（假），跳过 `b` 直接返回 0
- `a || b`: 先求值 `a`，若 `a` 非 0（真），跳过 `b` 直接返回 1
- `!a`: 一元否定，`a` 为 0 返回 1，否则返回 0

代码生成使用条件跳转实现短路。`&&` 和 `||` 优先级低于比较运算符，`&&` 高于 `||`。

## Acceptance criteria

- [ ] `return 1 && 1;` 返回 1
- [ ] `return 1 && 0;` 返回 0
- [ ] `return 0 || 1;` 返回 1
- [ ] `return 0 || 0;` 返回 0
- [ ] `return !0;` 返回 1
- [ ] `return !1;` 返回 0
- [ ] `int x = 0; return (0 && (x = 1)) || x;` 返回 0（短路验证：`x = 1` 未执行）
- [ ] `int x = 0; return (1 || (x = 1)) && x;` 返回 0（短路验证：`x = 1` 未执行）
- [ ] `return 1 < 2 && 3 == 3;` 返回 1（比较+逻辑优先级）

## Blocked by

- 05-function-call-exec-func
