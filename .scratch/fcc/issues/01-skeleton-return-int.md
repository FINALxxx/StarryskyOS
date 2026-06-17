# 01: 模块骨架 + return 整数

Status: ready-for-agent

## Parent

[PRD: fcc — Tiny C Compiler](../PRD.md)

## What to build

创建 `fcc` shell 命令的最小端到端通路：词法器、递归下降解析器、RISC-V 代码生成器协同工作，编译一个只含 `return` 整数的程序并通过 `exec` 执行验证。

命令执行时：
1. Shell 解析 `fcc <source.c>`，传入源文件名
2. 从 FatFs 读取 `.c` 源文件到内存缓冲区
3. 词法器将字符流切分为 token 流（关键字 `int`/`return`，整数，括号花括号分号）
4. 解析器识别 `int main() { return <int>; }` 结构
5. 代码生成器输出 RISC-V 函数序言、`li a0, N`、函数尾声到 PSRAM 固定缓冲区
6. `printf` 打印代码地址和大小
7. 用户 `exec <addr>`，shell 显示返回值 N

## Acceptance criteria

- [ ] `fcc` 命令注册成功，`help` 可列出
- [ ] 编译 `int main() { return 0; }` 成功，输出代码地址
- [ ] `exec <addr>` 执行后返回值 0（通过 shell `RETVAL` 变量验证或 `exec_func` 回调）
- [ ] 编译 `int main() { return 42; }` 后 `exec` 返回 42
- [ ] 编译空文件（无 `main`）报错，不生成代码
- [ ] 编译语法错误（如缺少 `}`）报错含行号
- [ ] 编译超过 32KB 输出的源文件（故意写大量 `return` 语句）报错
- [ ] 机器码在 0x40600000 处，`exec` 该地址不崩溃

## Blocked by

None - can start immediately.
