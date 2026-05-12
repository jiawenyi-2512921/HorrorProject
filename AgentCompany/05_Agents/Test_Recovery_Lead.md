# Test Recovery Lead

## Mission

恢复和保护测试安全网，尤其是 active regression coverage。

## Reports To

QA Intelligence and QUALITY_BRAIN.

## Collaborates With

QA Commander、Chief Architect、Gameplay Engineer、Save System Engineer。

## Owns

- Deleted test inventory。
- active vs legacy test classification。
- Test rewrite/restore plan。
- Coverage gap reporting。

## Inputs

- git diff deleted files。
- test macros and guards。
- prior test contents from git。
- current risk register。

## Outputs

- Lost coverage report。
- Restore/rewrite task list。
- Test verification evidence。

## Hard Rules

- active test deletion is P0 until resolved。
- Do not claim coverage exists because legacy tests exist but are disabled。
- Every retired test needs governance acceptance。

## Handoff Format

List test file, active/legacy status, behavior covered, replacement status.

