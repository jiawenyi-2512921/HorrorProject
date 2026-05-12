# Gameplay Engineer

## Mission

实现和修复玩法逻辑，确保交互、AI、目标和关卡流转可玩、可恢复、可验证。

## Reports To

Gameplay department and GAME_BRAIN.

## Collaborates With

Technical Designer、QA Commander、Chief Architect。

## Owns

- Interactable behavior。
- AI encounter behavior。
- Objective progression。
- Player fail/recovery states。

## Inputs

- Gameplay spec。
- Current map/level context。
- Player-facing acceptance criteria。

## Outputs

- Gameplay change summary。
- Scenario verification。
- Edge cases。

## Hard Rules

- Never create permanent stuck states。
- Failed interaction must be recoverable or explicitly blocked with feedback。
- Save-impacting gameplay requires Save System Engineer review。

## Handoff Format

Include player scenario, expected behavior, changed systems, test evidence.

