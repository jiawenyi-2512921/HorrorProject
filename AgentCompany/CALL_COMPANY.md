# How To Call AgentCompany

本文件定义如何“调用公司”。调用公司不是喊某个单独 agent，而是启动一套公司级协作流程：主脑接单、多脑审议、部门分派、员工执行、QA 验证、监管签字。

## 最短调用语

```text
调用 AgentCompany 处理：<你的任务>
```

默认行为：

1. MAIN_BRAIN 读取任务。
2. 根据风险分级 P0/P1/P2/P3。
3. 选择需要参与的大脑。
4. 指定 owner、reviewer、QA signer、governance signer。
5. 写入或引用 Task Board。
6. 执行前检查 War Room 和 Risk Register。
7. 完成前走 Quality Gates。

## 高强度调用语

```text
调用 AgentCompany 战时严格制处理：<你的任务>
要求：多脑并行、监管介入、证据驱动、完成前必须 QA 和 Governance 签字。
```

适用于：

- 构建失败。
- 存档/输入/地图/发布风险。
- 大规模重构。
- 测试恢复。
- 项目健康扫描。
- 任何你希望“拉满质量”的任务。

## 调用主脑

```text
调用 MAIN_BRAIN：把 <任务> 拆成公司任务，分配 owner/reviewer/QA/governance，并给出执行顺序。
```

输出应包含：

- priority。
- owner。
- reviewer。
- QA signer。
- governance signer。
- required brains。
- required evidence。

## 调用多脑会议

```text
召开 Brain Council 审议：<问题>
参与：TECH_BRAIN、QUALITY_BRAIN、RISK_BRAIN、PRODUCTION_BRAIN，必要时加入 GAME_BRAIN / RELEASE_BRAIN / CREATIVE_BRAIN。
```

适用于高风险决策。会议必须留下 Decision Record。

## 调用部门

```text
调用 <Department> 处理：<任务>
按 AgentCompany 部门协议输出输入、风险、交付物和验收证据。
```

示例：

```text
调用 Build & Release 处理：解除 Live Coding 构建阻塞并给出 fresh build 证据。
```

## 调用员工

```text
调用 <Agent Role> 执行：<具体任务>
必须遵守角色卡，输出 handoff、验证证据和残余风险。
```

示例：

```text
调用 Save System Engineer 执行：修复 New Game 不删除 autosave 的数据风险。
```

## 调用监管

```text
调用 Regulatory Board / Risk Auditor / Red Team Reviewer 审查：<完成声明或方案>
重点检查：证据、风险、dirty git、构建、测试、资产、玩家数据安全。
```

监管输出只能是：

- `APPROVED`
- `APPROVED_WITH_REGISTERED_RISK`
- `REJECTED`

## 调用项目体检

```text
调用 AgentCompany 做项目健康扫描：
范围：git、构建、测试、资产、存档、输入、地图、发布配置。
输出：P0/P1/P2/P3 风险、owner、证据、下一步。
```

## 调用任务执行

```text
调用 AgentCompany 执行 Task Board 中的 <Task ID>。
要求：先读 War Room 和 Risk Register；执行后走 reviewer、QA、governance。
```

## 对当前项目的推荐默认调用

```text
调用 AgentCompany 战时严格制执行 T-P0-001：
清理 staged/unstaged 分裂并建立可信提交边界。
必须由 Production owner，Governance 签字，禁止回滚用户改动。
```

然后依次调用：

1. `T-P0-002`：恢复或重建 active 测试。
2. `T-P0-003`：修复 New Game autosave 风险。
3. `T-P0-004`：设计 Continue map-aware 恢复。
4. `T-P1-001`：解除 Live Coding 构建阻塞并获取 fresh build。
5. `T-P1-002`：补齐 AssetRegistry 依赖。

