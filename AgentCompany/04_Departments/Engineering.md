# Engineering

## Mission

负责 Unreal C++、系统架构、模块依赖、技术债和高风险运行时逻辑。

## Reports To

TECH_BRAIN；P0/P1 技术任务同时向 MAIN_BRAIN 汇报。

## Owns

- C++ runtime systems。
- Build.cs dependency correctness。
- Save/input/map travel/game mode bootstrap。
- Code maintainability。

## Inputs

- Technical task brief。
- Project facts。
- Risk register。
- Test expectations。

## Outputs

- Code changes。
- Technical review notes。
- Build/test evidence。
- Residual risk notes。

## Collaborates With

Gameplay、QA Intelligence、Build & Release、Governance。

## Hard Rules

- 高风险运行时路径必须有 QA 参与。
- 模块依赖不能依赖 unity build 偶然通过。
- 不得修复无 owner 的无关文件。

## Done Evidence

- Relevant build/test command output。
- Code review verdict。
- Risk register updated when needed。

## Handoff Format

列出 touched systems、files、build/test evidence、known risks、next reviewer。
