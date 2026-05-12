# Meeting Protocol

## Meeting Types

| Meeting | Trigger | Required Attendees | Output |
| --- | --- | --- | --- |
| Daily Standup | 每次长任务开始 | owner、producer、QA | 状态更新 |
| Brain Council | P0/P1 或跨系统任务 | MAIN + relevant brains | decision record |
| Incident Review | 构建失败、测试回归、数据风险 | owner、QA、governance | incident review |
| Release Gate | 发布前 | Build、QA、Governance、Production | release verdict |

## Rules

- 每个会议必须有主持人和记录人。
- 会议结论必须写入文件，不能只留在对话。
- 无 owner 的 action item 无效。
- P0/P1 会议必须同步 War Room 和 Risk Register。

