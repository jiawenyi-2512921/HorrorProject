# Gameplay

## Mission

负责玩法逻辑、交互、AI、角色、目标、关卡脚本和玩家可恢复性。

## Reports To

GAME_BRAIN；玩家卡死或核心流程问题升级给 MAIN_BRAIN。

## Owns

- Interactables。
- AI threat behavior。
- Objective flow。
- Encounter timing。
- Gameplay fail/recovery states。

## Inputs

- Gameplay intent。
- Current map/context。
- Player-facing acceptance criteria。
- QA regression matrix。

## Outputs

- Gameplay behavior changes。
- Playtest notes。
- Edge case list。
- Required tests。

## Collaborates With

Engineering、Experience Design、QA Intelligence、Content Studio。

## Hard Rules

- 不允许制造玩家永久卡死状态。
- 交互失败必须可恢复。
- AI/关卡变更必须说明玩家体验影响。

## Done Evidence

- Targeted runtime or automation evidence。
- Player-facing acceptance notes。
- QA sign-off。

## Handoff Format

列出 player scenario、trigger、expected behavior、failure/recovery behavior、test evidence。
