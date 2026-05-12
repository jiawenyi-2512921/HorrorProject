# Task Board

任务板是所有 agent 工作的入口。未登记的任务不得进入执行。

## Columns

- Intake：收到但未分级。
- Triaged：已分级，有 owner。
- In Progress：正在执行。
- Review：等待 reviewer/QA/governance。
- Blocked：等待外部条件。
- Done：通过所有质量门禁。

## Active P0/P1 Tasks

| ID | Priority | Task | Owner | Reviewer | QA | Governance | Status |
| --- | --- | --- | --- | --- | --- | --- | --- |
| T-P0-001 | P0 | 清理 staged/unstaged 分裂并建立可信提交边界 | Production | MAIN_BRAIN | QA Commander | Risk Auditor | Intake |
| T-P0-002 | P0 | 恢复或重建被删除的 active Day1/Save/UI 测试 | QA Intelligence | Test Recovery Lead | QA Commander | Regulatory Board | Intake |
| T-P0-003 | P0 | 修复 New Game 不删除 autosave 的数据风险 | Engineering | Chief Architect | QA Commander | Risk Auditor | Intake |
| T-P0-004 | P0 | 设计 Continue map-aware 恢复流程 | Engineering | Save System Engineer | QA Commander | Risk Auditor | Intake |
| T-P1-001 | P1 | 解除 Live Coding 构建阻塞并获取 fresh build 证据 | Build & Release | Build Doctor | QA Commander | Regulatory Board | Intake |
| T-P1-002 | P1 | 为 AssetRegistry 使用补齐模块依赖 | Engineering | Chief Architect | Build Doctor | Risk Auditor | Intake |

## Completion Rule

只有 reviewer、QA、governance 三方均签字，任务才可移动到 Done。

