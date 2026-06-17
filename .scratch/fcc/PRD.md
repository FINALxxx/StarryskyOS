# PRD: fcc — Tiny C Compiler for RISC-V MCU

Status: ready-for-agent

## Problem Statement

开发者已经可以在 MCU 上通过文本编辑器创建和编辑 `.c` 源文件，但编辑后的代码无法在设备上运行。当前只能调用已知固件函数地址（`exec` 命令），不能动态编译新代码。需要一个能直接在 MCU 上编译 C 源码并输出可执行机器码的编译器。

## Solution

新增 shell 命令 `fcc <source.c>`，读取 FatFs 上的 C 源文件，单遍编译为 RISC-V RV32IM 机器码，输出到 PSRAM 固定地址。开发者随后使用 `exec <addr>` 执行编译后的代码。编译器实现为极简 C 子集，纯栈式代码生成，编译错误直接打印到终端。

## User Stories

1. As a 嵌入式开发者, I want to type `fcc hello.c` in the shell and have it compile my C source file, so that I can turn my source code into executable machine code directly on the device.

2. As a 嵌入式开发者, I want the compiler to output the address and size of the generated code, so that I know what address to pass to `exec`.

3. As a 嵌入式开发者, I want to call `exec_func` from within compiled code, so that my programs can interact with the firmware's function dispatch mechanism.

4. As a 嵌入式开发者, I want to define and call my own functions, so that I can structure my programs with modular, reusable code.

5. As a 嵌入式开发者, I want to use `if`/`else` conditional statements, so that I can write programs with branching logic.

6. As a 嵌入式开发者, I want to use `while` loops, so that I can write iterative algorithms.

7. As a 嵌入式开发者, I want to use integer arithmetic and comparison operators, so that I can compute values and make decisions based on them.

8. As a 嵌入式开发者, I want to declare local variables within functions, so that I can store intermediate computation results.

9. As a 嵌入式开发者, I want to declare global variables, so that I can share state across function calls.

10. As a 嵌入式开发者, I want to use decimal integer literals, so that I can express constant values in my code.

11. As a 嵌入式开发者, I want to see clear error messages with line numbers when my source code has syntax errors, so that I can fix my code quickly.

12. As a 嵌入式开发者, I want `fcc` to work as a standard shell command alongside `edit` and `exec`, so that my development workflow (edit → compile → execute) is seamless.

13. As a 嵌入式开发者, I want the compiler to reject source files that use unsupported C features with a clear message, so that I understand the compiler's limitations.

## Implementation Decisions

### 架构

- **Shell 命令模式**: `fcc` 注册为 `SHELL_TYPE_CMD_MAIN` 命令（接收 `argc`/`argv`），通过命令表注册到 letter-shell。
- **执行模式**: 编译和执行为两步分离操作——`fcc hello.c` 编译后打印代码地址，用户手动 `exec <addr>` 执行。中间可检查编译是否成功。
- **输出缓冲**: 机器码写入 PSRAM 固定缓冲区（基地址 `FCC_CODE_BASE`），最大 32KB。每次编译覆盖前次内容。
- **符号白名单**: 编译器内置 `exec_func` 的函数地址（从 shell 环境变量 `envFunc` 读取），编译后的代码可调用它。不实现完整的符号解析。

### C 语言子集

**支持**:
- 类型: `int`（32 位有符号）
- 运算符: `+` `-` `*` `/` `%` `==` `!=` `<` `>` `<=` `>=` `&&` `||` `!` `=` `-` (一元)
- 语句: 表达式语句、`if`/`else`、`while`、`return`
- 函数: 定义（含参数和返回值）、调用
- 变量: 局部变量（函数作用域、栈分配）、全局变量（BSS 段）
- 字面量: 十进制整数（`0`-`2147483647`）

**不支持**:
- `char`、`short`、`long`、`void`、指针、数组、`struct`、`typedef`、`enum`
- `for`、`do`-`while`、`switch`、`break`、`continue`、`goto`
- 预处理器（`#include`、`#define`）
- 类型转换、`sizeof`、三元运算符 `?:`
- 浮点数、字符串字面量

### 编译器内部

- **遍数**: 单遍。递归下降解析器直接从 token 流生成机器码，无中间 AST。
- **寄存器策略**: 纯栈式。所有表达式结果通过栈传递，运算使用 `t0`-`t2` 临时寄存器，调用约定用 `a0`-`a7` 传参、`a0` 返回。
- **代码生成**: 输出 RV32IM 指令（I 基础整数 + M 乘除法扩展），与 MCU 架构一致。
- **函数序言/尾声**: 标准栈帧——`addi sp, sp, -N` / `sw ra, offset(sp)` 等保存/恢复。
- **函数调用约定**: 参数通过 `a0`-`a7` 传递（超过 8 个参数不支持），返回值在 `a0`。

### 内存布局

- **代码缓冲区**: PSRAM 固定地址，编译时直接写入。
- **全局变量**: 在代码缓冲区之后分配空间（编译时确定）。
- **栈**: 编译后代码使用调用者的栈（SP 当前值），编译器生成标准的栈帧管理指令。

### 错误处理

- 编译错误直接 `printf` 到串口终端，包含行号和错误描述。
- 错误时不生成任何代码，返回非零值。
- 不进入 full-screen 模式——编译器是纯 CLI 工具。

### 代码组织

- 新建 `System/fcc/` 模块，与 `System/text_editor/`、`System/letter-shell/` 平级。
- `fcc_main.c` — 命令入口、文件读取、驱动编译流程。
- `fcc_lexer.c` — 词法分析：字符流 → token 流。
- `fcc_parser.c` — 递归下降语法分析 + 语义动作。
- `fcc_codegen.c` — RISC-V 指令编码、代码缓冲区管理。

### 命令注册

- 在 `shellCommandList[]` 的 `CONFIG_COMPONENT_FLASH_FS` 块内添加 `SHELL_CMD_ITEM`，`edit` 命令旁。

## Testing Decisions

### 测试策略

测试外部行为：给定一个 C 源文件作为输入，验证编译是否成功、机器码是否正确、执行后行为是否符合预期。不测试编译器内部实现细节。

### 测试场景

1. 编译空函数 `int main() { return 0; }`，验证编译成功，`exec` 后返回 0
2. 编译算术表达式 `return 3 + 4 * 5;`，验证计算结果为 23
3. 编译含 `if`/`else` 分支的函数，验证条件正确执行
4. 编译含 `while` 循环的函数，验证循环体执行正确次数
5. 编译函数调用链 `a()` → `b()` → `return 42`，验证返回值正确传递
6. 编译调用 `exec_func(0, 0, N)` 的程序，验证固件回调被触发
7. 编译含局部变量的函数，验证栈分配/回收正确
8. 编译含全局变量的程序，验证跨函数状态共享
9. 编译含语法错误的源文件，验证打印错误信息且不生成代码
10. 编译超过 32KB 机器码限制的源文件，验证拒绝并报错
11. 编译使用不支持特性（如指针）的源文件，验证报错并提示

### 测试接口

- 输入: FatFs 上的 `.c` 源文件
- 输出: `printf` 编译状态（成功/失败信息 + 代码地址和大小）
- 持久化: PSRAM 中的机器码缓冲区
- 执行: `exec <addr>` 调用编译后的函数

## Out of Scope

- 完整 C 标准（C89/C99/C11）合规
- 指针、数组、`char`、`struct`、`typedef`
- `for`、`switch`、`break`、`continue`
- 预处理器（`#include`、`#define`、条件编译）
- 优化（常量折叠、死代码消除、寄存器分配）
- 多文件编译/链接
- 除 `exec_func` 以外的外部符号解析
- 编译后代码的断点调试
- 错误恢复（遇到第一个错误即停止）
- 命令行参数（`-O`、`-o` 等）

## Further Notes

- 编译器本身运行在 MCU 上，72MHz RISC-V + 8MB PSRAM。词法分析和解析的源码缓冲区最大 64KB（与编辑器文件限制一致）。
- 编译后的代码和编译器共享同一个 CPU 和内存空间。错误的用户代码可能导致崩溃或内存破坏（如无限递归触发栈溢出）。
- RV32IM 指令集包含整数乘除法指令（`mul`/`div`/`rem`），M 扩展已由 MCU 硬件支持。
- `exec_func` 的函数签名是 `void exec_func(uint32_t st, uint32_t ed, uint32_t loopTime)`，编译后代码通过 `a0`-`a2` 传递三个参数。
- 该编译器后续可自举（用 fcc 编译 fcc），但 MVP 不做。
