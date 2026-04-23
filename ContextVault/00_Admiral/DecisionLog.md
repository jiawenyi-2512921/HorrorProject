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
