# Decision Log

## ADR-000 · 采用 Fleet OS 作为项目控制面
Date: 2026-04-23 | Author: ADMIRAL | Approved: USER

### Context
项目要求单窗口下令、全自治执行、极限并发、工业级质量。

### Options
1. 单代理直做
2. 固定多标签页舰队
3. Fleet OS：固定指挥链 + 弹性执行蜂群 + 独立审查蜂群

### Decision
Option 3.

### Rationale
- 兼顾并发、可审计性、质量门和速度
- 能把长期知识与短期执行分开
- 更适合 Codex 的有界子任务模型

### Consequences
+ 任务流更稳定
+ 评审链更独立
- 需要维护 ContextVault 和事件留痕

### Defense Soundbite
“我们不是把 AI 当单个写代码工具，而是把它组织成有审查链的生产系统。”

## ADR-001 · 用 EventBus 同构开发流程与游戏架构
Date: 2026-04-23 | Author: ORACLE | Approved: ADMIRAL

### Context
项目既需要工程协作总线，也需要答辩层面的架构叙事。

### Decision
所有跨角色协调以 `Event.*` 命名空间记录到 `ContextVault/99_EventBus.md`。

### Defense Soundbite
“我们的开发流程本身就是我们的架构镜像。”

## ADR-002 · 优先 Subsystem 而非 Singleton
Date: 2026-04-23 | Author: ORACLE | Approved: ADMIRAL

### Context
多个系统需要全局协调：事件、保存、AI Director、音频、调试信息。

### Decision
优先使用 UE Subsystem 管理全局服务，避免手写 Singleton。

### Defense Soundbite
“我们使用引擎托管生命周期的 Subsystem，避免反模式式的全局单例。”

## ADR-003 · 主方向锁定为深海 VHS 调查恐怖
Date: 2026-04-24 | Author: ADMIRAL | Approved: USER (autonomous directive)

### Context
项目已具备多个资产方向，但必须在短周期内做出高完成度成品，并保持架构、视听和玩法的一致性。

### Options
1. 工业废墟 bodycam 调查惊悚
2. 深海站体 VHS 调查恐怖
3. 黑暗奇幻地牢探索解谜

### Decision
Option 2.

### Rationale
- `DeepWaterStation` 与 `Bodycam_VHS_Effect` 的题材和视听语法天然匹配
- 第一人称恐怖骨架已存在，技术复用率最高
- 更容易在 `45-75` 分钟短流程内做出辨识度和高级感
- 更符合现有 NorthStar、LevelFlow 与 Puzzle 结构

### Consequences
+ 方向更聚焦，资产拼接风险降低
+ 代码主线可围绕调查、交互、档案、单威胁 AI 收敛
- 放弃重战斗、多敌种和大地图扩张空间

### Defense Soundbite
“我们不是在做题材拼盘，而是在最强资产交集中做一部完成度最高的短篇作品。”

## ADR-004 · 硬截止锚定到 2026-05-14，2026-05-15 至 2026-05-17 仅作缓冲
Date: 2026-04-24 | Author: ADMIRAL | Approved: USER (autonomous directive)

### Context
现有文档采用 `Day 21 / M3` 节奏，用户最新表达提到 `24天*24小时` 连续推进。

### Decision
保留原有硬交付日 `2026-05-14`，将 `2026-05-15` 至 `2026-05-17` 视为 contingency/polish buffer。

### Rationale
- 保住既有里程碑纪律
- 允许额外缓冲而不刺激范围膨胀

### Defense Soundbite
“缓冲是为了保交付，不是为了默许新增大功能。”

## ADR-005 · ContextVault 升级为 Agent Company 操作系统
Date: 2026-04-26 | Author: ADMIRAL/CURATOR | Approved: USER (autonomous high-concurrency directive)

### Context
用户要求 `D:\gptzuo\ContextVault` 作为 Agent team 公司继续扩建，并明确追求质量、效率、速度、协调能力，同时允许极高并发与 token 预算。

### Options
1. 继续仅用 EventBus 和状态文档做松散记录
2. 只增加更多 agent prompt，不建立运营协议
3. 建立 Agent Company OS：RACI、调度协议、并发车道、质量门、交接包、事故手册、实时并发看板

### Decision
Option 3.

### Rationale
- 高并发的主要风险不是 token，而是文件所有权冲突、验证证据缺失、上下文漂移和范围膨胀
- 明确的 lane ownership 与 G0-G5 gates 能让速度和质量同时提升
- ContextVault 需要从“记录库”升级为“生产控制面”

### Consequences
+ 可以安全使用多路 read-only scout 和单写入 lane 的组合
+ 每个 packet 都有统一交接格式和验证要求
+ 风险、阻塞、Editor-only 工作更容易被追踪
- 需要持续维护 ConcurrencyBoard 和 EventBus，避免文档过期

### Defense Soundbite
“我们不靠更多无序 agent 提速，而是把 agent 组织成有 RACI、车道、质量门和事故响应的公司。”
