# Brain Network

Brain Network 是 AgentCompany 的多脑系统。它的职责不是替代执行部门，而是从多个专业角度对任务进行并行推演，找出最优方案和失败模式。

## 多脑列表

| Brain | 核心问题 | 输出 |
| --- | --- | --- |
| TECH_BRAIN | 架构是否正确、依赖是否完整、编译是否可证明 | 技术方案、依赖风险、代码边界 |
| GAME_BRAIN | 玩家体验是否成立、玩法是否闭环 | 体验判断、玩法验收点 |
| PRODUCTION_BRAIN | 谁做、何时做、依赖是什么 | owner、里程碑、阻塞项 |
| QUALITY_BRAIN | 如何证明没有回归 | 测试矩阵、验收证据 |
| RISK_BRAIN | 最可能失败在哪里 | 风险清单、反例、升级条件 |
| CREATIVE_BRAIN | 方案是否有 3A 表现力 | 创意方向、表达质量 |
| RELEASE_BRAIN | 能否 Cook、打包、发布 | 发布门禁、构建命令、LFS 检查 |
| PLAYER_BRAIN | 玩家会如何误用、卡住或困惑 | UX 风险、可玩性反馈 |

## 使用规则

- P0/P1：至少调用 TECH、QUALITY、RISK、PRODUCTION。
- 玩法/叙事/关卡：必须调用 GAME 和 PLAYER。
- 打包/资产：必须调用 RELEASE 和 CONTENT 相关部门。
- 多脑输出必须进入任务 handoff，不得只停留在对话中。

