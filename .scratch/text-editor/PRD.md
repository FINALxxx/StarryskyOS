# PRD: 全屏彩色文本编辑器

Status: ready-for-agent

## Problem Statement

开发者在通过串口终端调试 MCU 时，如果需要查看或修改 Flash 文件系统上的配置文件、日志或脚本，目前只能通过 letter-shell 自带的 `ls`/`touch` 命令进行最基本的文件操作，无法在设备端直接编辑文件内容。每次修改都需要将文件导出到 PC、编辑、再传回——流程繁琐且打断工作流。

## Solution

在 letter-shell 中新增 `edit <filename>` 命令，启动一个全屏 ANSI 彩色文本编辑器。开发者可以直接在 MobaXterm 串口终端中打开、编辑、保存 FatFs 文件系统上的文本文件，全程无需离开终端。

## User Stories

1. As a 嵌入式开发者, I want to type `edit config.txt` in the shell and see the file content in a full-screen editor, so that I can modify configuration files directly on the device without exporting/importing.

2. As a 嵌入式开发者, I want to use arrow keys to move the cursor around the text, so that I can navigate to any position in the file intuitively.

3. As a 嵌入式开发者, I want typed characters to insert at the cursor position, so that I can add new content anywhere in the file.

4. As a 嵌入式开发者, I want to press Backspace to delete the character before the cursor, so that I can correct mistakes.

5. As a 嵌入式开发者, I want to press Enter to insert a newline at the cursor position, so that I can add new lines.

6. As a 嵌入式开发者, I want to press Ctrl+W to save the file to Flash, so that my edits are persisted.

7. As a 嵌入式开发者, I want to press Esc to exit the editor, so that I can return to the shell prompt.

8. As a 嵌入式开发者, I want to see a warning in the status bar if I try to exit with unsaved changes, so that I don't accidentally lose my work.

9. As a 嵌入式开发者, I want to see the filename and a modified indicator in a top bar, so that I know which file I'm editing and whether it has unsaved changes.

10. As a 嵌入式开发者, I want to see line numbers on the left side of the editing area, so that I can orient myself in the file.

11. As a 嵌入式开发者, I want to see the current cursor position (row, column) in a bottom status bar, so that I know exactly where I am in the file.

12. As a 嵌入式开发者, I want the editor to display content with ANSI color coding (top bar, line numbers, status bar, text body), so that the UI is visually clear and professional.

13. As a 嵌入式开发者, I want `edit <newfile>` to create a new empty file if the specified file does not exist, so that I can create files from within the editor.

14. As a 嵌入式开发者, I want the editor to reject files larger than 32KB, so that the MCU's RAM is never exhausted by a single editing session.

15. As a 嵌入式开发者, I want the shell to return to its normal prompt state immediately after I exit the editor, so that I can continue using other shell commands without disruption.

## Implementation Decisions

### 架构

- **Shell 命令模式**: 编辑器作为 letter-shell 的一个命令 (`edit`) 启动，命令类型为 `SHELL_TYPE_CMD_MAIN`（接收 `argc`/`argv`），通过命令表注册。
- **终端接管**: `edit` 命令的 handler 函数内部直接运行自己的 while 循环，通过 UART 底层函数逐键读取、逐帧渲染。退出循环即 return，shell 无缝恢复。不修改 letter-shell 内核代码。
- **命令注册方式**: 项目使用命令表模式 (`SHELL_USING_CMD_EXPORT=0`)，在 `shellCommandList[]` 数组中添加 `SHELL_CMD_ITEM` 条目。

### 数据模型

- **编辑缓冲区**: 单块连续 `char[]` 缓冲区，存放整个文件的文本内容。最大 32KB。
- **行索引**: 维护一个行起始偏移量数组，在渲染和光标移动时快速定位行边界。插入/删除字符时增量更新。
- **修改标记**: 一个布尔标志，任何编辑操作后置为 `true`，保存后置为 `false`。用于顶栏指示和退出提示。

### 文件 I/O

- **打开**: `f_open()` 读取文件全部内容到编辑缓冲区。文件不存在时创建空缓冲区。
- **保存**: Ctrl+W 触发 `f_open() + f_write() + f_close()` 全量覆盖写回。扇区大小 4096 字节。
- **关闭**: 不保存直接退出，仅释放缓冲区内存。

### 渲染

- **策略**: 每次按键后全量重绘（清屏 + 重画所有可见行 + 状态栏）。
- **终端协议**: 硬编码 VT100/ANSI 转义码子集（光标定位、SGR 颜色、清屏、光标显隐）。不做终端能力协商。
- **UI 布局**:
  - 第 1 行: 顶栏 — `[文件名]` + 修改标记 `[*]`（如已修改）
  - 第 2 行至倒数第 2 行: 编辑区 — 左侧行号 + 文件内容
  - 最后一行: 底栏 — `Ln X, Col Y | Ctrl+W 保存  Esc 退出`（未保存时 Esc 提示变为 `再按 Esc 确认退出`）

### 颜色方案

| 元素 | 前景 | 背景 | 样式 |
|------|------|------|------|
| 顶栏 | 黑色 | 青色 | 粗体 |
| 行号 | 暗灰色 | 默认 | 正常 |
| 正文 | 白色 | 默认 | 正常 |
| 底栏 | 黑色 | 青色 | 粗体 |
| 底栏警告 | 黑色 | 黄色 | 粗体 |

### 按键映射

| 按键 | 操作 |
|------|------|
| `↑` `↓` `←` `→` | 移动光标 |
| 可打印字符 | 在光标处插入 |
| Backspace (`0x08` / `0x7F`) | 删除光标前字符 |
| Enter (`0x0D`) | 插入换行 |
| Ctrl+W (`0x17`) | 保存文件 |
| Esc (`0x1B`) | 退出（如有未保存修改，提示后再按 Esc 确认） |

使用 Ctrl+W 而非 Ctrl+S 保存，因为 Ctrl+S (`0x13`) 在串口通信中是 XOFF（软件流控暂停信号），按下去终端会冻结输出，MCU 侧收不到该按键。

### 代码组织

- 新建 `System/text_editor/` 模块，与 letter-shell、fatfs 平级。
- `text_editor.h` — 导出 `editCmd()` 函数声明。
- `text_editor.c` — 编辑器主逻辑（缓冲区管理、光标、按键分发）。
- `editor_render.c` — ANSI 渲染函数（绘制顶栏、编辑区、底栏）。

### 约束

- 最大文件大小: 32KB
- 编辑缓冲区: 静态分配在 BSS 段，不从堆分配
- 波特率: 115200（全量重绘一帧约 200ms）
- 终端: MobaXterm（VT100 兼容）

## Testing Decisions

### 测试策略

测试外部行为，不测试内部实现细节。通过串口终端发送按键序列，验证屏幕输出和文件内容。

### 测试场景

1. 打开已存在的文件，验证内容正确显示
2. 编辑文本（插入、删除、换行），保存后退出，重新打开验证内容持久化
3. 打开不存在的文件名，验证创建空文件
4. 打开超过 32KB 的文件，验证拒绝并返回错误提示
5. 修改后按 Esc，验证底栏警告提示，再按 Esc 确认退出
6. Ctrl+W 保存后，验证顶栏修改标记消失
7. 保存后 Esc 退出，验证无警告直接退出
8. 方向键在文本中移动，验证行号和列号正确更新
9. 在文件第一行第一个字符处按 Backspace，验证无崩溃
10. 在文件最后一行最后一个字符后按右方向键，验证无崩溃

### 测试接口

- 输入: UART 接收的按键字节序列
- 输出: UART 发送的 ANSI 渲染字节流
- 持久化: FatFs 上的文件内容

## Out of Scope

- 语法高亮
- 撤销/重做 (undo/redo)
- 查找和替换
- 复制/粘贴
- 多文件同时编辑
- 分页/虚拟内存（超过 32KB 的大文件）
- 终端能力协商（固定 ANSI VT100 子集）
- 断电保护（不写临时文件）
- 鼠标支持
- 自动换行（长行水平滚动）

## Further Notes

- 该 MCU 平台拥有 LightCoroutine 协程库，如果未来全量重绘的 200ms 延迟成为瓶颈，可以将渲染拆分到协程中异步执行。当前 MVP 不需要。
- FatFs 配置为 `FF_FS_NORTC=1`（固定时间戳），如需真实文件时间戳，可将 PCF8563 RTC 接入 `get_fattime()`。
- 编辑器退出后缓冲区内存不释放（静态 BSS 分配），不影响系统稳定性。
