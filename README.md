# Agent Fleet OS

This workspace is configured for a single-window, high-autonomy delivery model.

## Operating model

- You give one instruction in the main thread.
- The fleet compiles it into a mission brief, routes it across heads and swarms, applies independent review, and writes evidence back to the workspace.
- Architecture, memory hygiene, and delivery gates are enforced through `.fleet/` and `ContextVault/`.

## Source of truth

- Fleet contract: `.fleet/manifest.md`
- Fleet bootstrap: `.fleet/START-HERE.md`
- Fleet prompts: `.fleet/prompts/`
- Fleet policies: `.fleet/policies/`
- Shared templates: `.fleet/templates/`
- Project knowledge base: `ContextVault/`
- Design spec: `docs/superpowers/specs/2026-04-23-agent-fleet-orchestrator-design.md`

## How to use

In future tasks, wake the fleet explicitly. The primary wake word is `苏醒`.

In this workspace, `苏醒` is a per-thread latch:

- after you say `苏醒`, the rest of the current conversation defaults to fleet mode
- that default stays active until you explicitly say `休眠`, `退出舰队模式`, or start a different thread without waking the fleet

Recommended examples:

- `苏醒，做这个任务。`
- `苏醒，用 fleet 模式实现这个功能。`
- `苏醒，开启全自治执行和审查。`

Technical fallback:

- `Use $agent-fleet-os to run this task through our fleet in full autonomous mode.`

That wording is intentional. It makes fleet activation unambiguous, which allows aggressive parallel delegation and review.
