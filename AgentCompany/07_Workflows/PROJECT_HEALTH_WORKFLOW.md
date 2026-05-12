# Project Health Workflow

## Scope

Use this workflow for broad repository scans, release readiness, or “project not healthy” investigations.

## Checks

- Git status and staged/unstaged split.
- Build availability and blockers.
- Active test inventory and deleted tests.
- Hardcoded asset path existence.
- Build.cs/module dependency consistency.
- Packaging/cook map configuration.
- High-risk runtime systems: save, input, map travel, UI, AI.
- Documentation drift.

## Output

Health reports must classify findings as P0/P1/P2/P3 and include file paths, evidence, owner, and next action.

