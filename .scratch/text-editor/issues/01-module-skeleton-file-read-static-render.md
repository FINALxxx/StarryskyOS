# 01: 模块骨架 + 文件读取 + 静态渲染

Status: ready-for-agent

## Parent

[PRD: 全屏彩色文本编辑器](../PRD.md)

## What to build

创建 `edit` shell 命令的最小端到端通路：从命令注册到 FatFs 文件读取，再到 ANSI 全屏渲染。这是穿过所有集成层的最细 tracer bullet。

命令执行时：
1. Shell 解析 `edit <filename>`，传入文件名
2. 通过 FatFs 打开文件，将内容全量读入静态编辑缓冲区（最大 32KB）
3. 文件不存在时创建空缓冲区
4. 文件超过 32KB 时打印错误并 return
5. 清屏并渲染全屏 ANSI 界面：顶栏（文件名）、编辑区（行号 + 正文）、底栏（静态提示文字）
6. 等待任意按键，退出，shell 恢复

## Acceptance criteria

- [ ] 在 shell 中执行 `edit <已存在的文件>`，屏幕清空并显示文件内容，含行号、顶栏文件名、底栏提示
- [ ] 在 shell 中执行 `edit <不存在的文件>`，屏幕清空并显示空编辑区，行号 `1` 后无内容
- [ ] 执行 `edit <超过32KB的文件>`，打印错误信息并直接返回 shell
- [ ] 按任意键后退出编辑器，shell 提示符正常恢复
- [ ] 顶栏显示 `[文件名]`（无修改标记），底栏显示 `Ctrl+W 保存  Esc 退出`
- [ ] 颜色方案：顶栏黑字青底，行号暗灰色，正文白色，底栏黑字青底

## Blocked by

None - can start immediately.
