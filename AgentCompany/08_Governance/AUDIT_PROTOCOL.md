# Audit Protocol

## Audit Types

- Pre-implementation audit：执行前确认范围和风险。
- Mid-task audit：长任务中途确认没有偏航。
- Completion audit：完成声明前检查证据。
- Incident audit：失败后找根因和制度漏洞。

## Audit Evidence

审计必须引用：

- 文件路径。
- 命令输出。
- task id。
- risk id。
- reviewer verdict。

## Audit Verdict

- `CLEAR`：允许继续。
- `CLEAR_WITH_CONDITIONS`：允许继续但必须完成条件。
- `HOLD`：暂停执行。
- `REJECT`：不能交付。

