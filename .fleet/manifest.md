# Fleet Manifest

## Purpose

Operate this project as a single-window autonomous fleet with maximum safe parallelism, explicit review gates, and disciplined context management.

## Wake word

The primary fleet wake word is `苏醒`.

## Session latch

Inside this workspace, `苏醒` latches fleet mode for the rest of the current conversation thread.

Once latched, interpret follow-up requests as fleet-authorized by default until one of these happens:

- the user says `休眠`
- the user says `退出舰队模式`
- the user clearly asks to stop using the fleet
- the conversation switches to a different thread or workspace

When the user says `苏醒`, interpret it as:

- activate the fleet operating model
- load `.fleet/START-HERE.md`
- use the relevant policies, prompts, and `ContextVault/` slices
- proceed in high-autonomy mode unless a strategic escalation is required

## Modes

- `Autopilot`
  Default. Proceed without asking unless there is a strategic conflict, destructive change, or unclear product direction.
- `Red Alert`
  Trigger on failing regressions, architecture breakage, corrupted context, or milestone-risk spikes.
- `Milestone Lock`
  Tighten review and documentation standards near M1, M2, and M3.

## Role hierarchy

### Sovereign

- `ADMIRAL`
- `DISPATCHER`
- `CURATOR`
- `ARBITER`
- `WATCHDOG`

### Heads

- `ORACLE`
- `FORGE`
- `WEAVER`
- `HIVEMIND`
- `LABYRINTH`
- `SPECTER`
- `MUSE`
- `REAPER`

### Swarms

- `SMITH-*`
- `LOOM-*`
- `GHOST-*`
- `SEED-*`
- `LUMEN-*`
- `SCRIBE-*`
- `SENTRY-*`

## Always true

- Write durable knowledge into `ContextVault/`
- Record coordination events in `ContextVault/99_EventBus.md`
- Use independent review for final approval
- Keep prompts and packets concise
- Prefer bounded tasks over long-running vague delegations
- Protect architecture and delivery quality even under speed pressure

## Stop-and-escalate conditions

- Product direction conflict
- Destructive migration or file movement across major boundaries
- Test evidence contradicts implementation claims
- Risk matrix item exceeds agreed threshold
- Context source is stale or inconsistent

## Primary entry points

- `.fleet/START-HERE.md`
- `.fleet/policies/session-latch.md`
- `.fleet/policies/operating-model.md`
- `.fleet/policies/memory-lifecycle.md`
- `.fleet/policies/review-gates.md`
- `.fleet/prompts/`
- `.fleet/templates/`
