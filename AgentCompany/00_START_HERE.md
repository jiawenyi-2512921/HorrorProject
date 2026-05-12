# AgentCompany 启动入口

你正在进入本项目的自治 3A Agent Studio。这里不是资料夹，而是公司级工作系统。任何 agent 在阅读、修改、审查、汇报本项目之前，必须先按本文件启动。

## 启动顺序

1. 读 `AgentCompany/01_Constitution/COMPANY_CHARTER.md`，确认最高原则和质量红线。
2. 读 `AgentCompany/CALL_COMPANY.md`，确认如何调用主脑、部门、员工和监管。
3. 读 `AgentCompany/02_CommandCenter/MAIN_BRAIN.md`，确认主脑调度方式。
4. 读 `AgentCompany/10_Ops/WAR_ROOM.md`，确认当前战情、P0/P1 风险和阻塞项。
5. 读与你任务相关的部门文件：`AgentCompany/04_Departments/`。
6. 读与你身份相关的员工角色卡：`AgentCompany/05_Agents/`。
7. 创建或接收任务前，使用 `AgentCompany/06_CommunicationBus/HANDOFF_TEMPLATE.md` 和 `AgentCompany/10_Ops/TASK_BOARD.md`。
8. 宣称完成前，必须通过 `AgentCompany/08_Governance/QUALITY_GATES.md`。

## 强制工作模式

- 默认高并发，但禁止无 owner 的并发。
- 默认证据驱动，没有验证证据就没有完成。
- 默认监管介入，`Governance` 可以否决任何完成声明。
- 默认保护现有工作区，不能回滚或覆盖未归属自己的改动。
- 默认先理解再实现，遇到异常先找根因。

## 完成声明格式

任何完成声明必须包含：

- 任务 id 或任务名称。
- owner、reviewer、QA signer、governance signer。
- 改动范围。
- 验证命令和结果。
- 残余风险。
- 下一步建议。

如果缺少上述任一项，只能说“已推进到某状态”，不能说“完成”。
