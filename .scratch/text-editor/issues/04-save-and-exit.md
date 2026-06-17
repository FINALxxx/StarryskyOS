# 04: 保存与退出

Status: ready-for-agent

## Parent

[PRD: 全屏彩色文本编辑器](../PRD.md)

## What to build

在 Issue 03 的编辑能力基础上，加入 Ctrl+W 保存和 Esc 退出逻辑。这是编辑器交互闭环的最后一步。

核心行为：
1. Ctrl+W（0x17）：将编辑缓冲区全量写入 FatFs 文件（`f_open` + `f_write` + `f_close`），清除 dirty flag，顶栏 `[*]` 消失
2. Esc（0x1B）：退出编辑器
   - 如果 dirty flag 为 false（无修改）：直接退出，shell 恢复
   - 如果 dirty flag 为 true（有修改）：不退出，底栏切换为黄色警告 `[未保存! 再按 Esc 确认退出]`，再按 Esc 才退出
3. 退出后 shell 提示符正常恢复，可继续使用其他命令
4. 保存失败时（如 Flash 写错误），底栏显示错误信息，不退出

## Acceptance criteria

- [ ] 编辑文件后按 Ctrl+W，文件内容正确写入 FatFs，顶栏 `[*]` 消失
- [ ] 保存后重新 `edit` 同一文件，内容与保存时一致
- [ ] 新建文件、编辑、Ctrl+W 保存后，文件在 FatFs 中正确创建
- [ ] 无修改时按 Esc，直接退出，底栏无警告
- [ ] 有修改时按 Esc，底栏显示黄色 `[未保存! 再按 Esc 确认退出]`，不退出
- [ ] 警告状态下再按 Esc，退出编辑器，shell 恢复
- [ ] 警告状态下按其他键（非 Esc），恢复正常编辑模式和底栏
- [ ] Ctrl+W 保存失败时底栏显示错误信息，编辑器不退出

## Blocked by

- 03-text-editing
