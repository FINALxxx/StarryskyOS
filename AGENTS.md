## Agent skills

### Issue tracker

Issues live as markdown files under `.scratch/<feature-slug>/`. See `docs/agents/issue-tracker.md`.

### Triage labels

Default label vocabulary — `needs-triage`, `needs-info`, `ready-for-agent`, `ready-for-human`, `wontfix`. See `docs/agents/triage-labels.md`.

### Domain docs

Single-context layout — `CONTEXT.md` + `docs/adr/` at the repo root. See `docs/agents/domain.md`.

### 搜索与读取文件

每次搜索和读取时，优先使用codegraph提供的能力，在无法得到结果时再使用Find、Read等指令。

### 不要猜测问题

- 第一次遇到bug时，不要猜测问题，你需要首先将能注入的log等调试信息全部注入，然后通过观察log反馈来推测所有可能的问题，然后再编写修复代码
- 后续通过对话修复该bug时，你需要逐渐删掉多余的log，只保留这次修复所需要的log输出