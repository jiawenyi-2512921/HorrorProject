# Start Here

Use this file as the concrete workspace entrypoint for the fleet.

## Wake word

The official wake word for this project is `苏醒`.

Interpret `苏醒` as a shorthand for:

`Use $agent-fleet-os to run this task through our fleet in full autonomous mode.`

## Recommended operator prompt

`Use $agent-fleet-os to run this task through our fleet in full autonomous mode. Load .fleet/START-HERE.md, .fleet/manifest.md, the relevant policies and role prompts, then execute with ContextVault, EventBus, architecture review, independent review, and delivery validation.`

## Short forms

- `苏醒，做这个任务。`
- `苏醒，用 fleet 模式处理。`
- `苏醒，带 ORACLE 审查和 REAPER 验证。`

## What the fleet should do automatically

1. compile a mission brief
2. route to the right heads
3. build context packets
4. execute bounded worker packets
5. run independent review
6. update `ContextVault/` and `ContextVault/99_EventBus.md`
7. report outcome, risks, and next actions
