# Org Chart and RACI

Date: 2026-04-26
Scope: SM-13 Agent Company

## Command Structure

```text
USER
  ↓
ADMIRAL  ── WATCHDOG
  ↓
DISPATCHER ── CURATOR
  ↓
Lane Owners: FORGE / REAPER / ORACLE / LABYRINTH / MUSE / SPECTER / WEAVER / HIVEMIND
  ↓
Worker Agents / Read-only Scouts / Validation Runners
```

## Decision Rights

| Decision | Owner | Reviewer | Approver | Notes |
|---|---|---|---|---|
| Product direction / NorthStar | ADMIRAL | ORACLE, MUSE | USER or standing directive | reject scope drift |
| Active packet priority | ADMIRAL | DISPATCHER, WATCHDOG | ADMIRAL | one main write packet at a time |
| Lane split / merge | DISPATCHER | CURATOR | ADMIRAL if high-risk | prevents file ownership collision |
| Architecture boundary | ORACLE | FORGE, WEAVER | ADMIRAL | ADR if persistent |
| Code implementation | FORGE | REAPER | ADMIRAL for milestone packets | validation required |
| Validation pass/fail | REAPER | WATCHDOG | REAPER | evidence over confidence |
| Binary asset/editor action | LABYRINTH | MUSE, WATCHDOG | USER if destructive | UE Editor-owned only |
| Risk escalation | WATCHDOG | ADMIRAL | ADMIRAL | update RiskMatrix/EventBus |
| ContextVault schema | CURATOR | DISPATCHER | ADMIRAL | keep concise and actionable |

## RACI Matrix

| Activity | ADMIRAL | DISPATCHER | FORGE | REAPER | WATCHDOG | CURATOR | ORACLE | MUSE/LABYRINTH/SPECTER |
|---|---|---|---|---|---|---|---|---|
| Choose next packet | A | R | C | C | C | I | C | C |
| Break into lanes | C | A/R | C | C | C | I | C | C |
| Edit code | I | C | A/R | C | I | I | C | C |
| Run build/tests | I | C | C | A/R | C | I | I | I |
| Update EventBus/status | A | C | C | C | C | R | I | I |
| Handle build break | C | C | R | A/R | C | I | C | C |
| Handle scope creep | A/R | C | I | I | C | I | C | C |
| Handle asset corruption risk | A | C | I | C | R | I | C | R |

A = accountable, R = responsible, C = consulted, I = informed.

## Authority Rules

- ADMIRAL may kill, split, merge, or defer packets.
- DISPATCHER may block a packet for ownership collision.
- REAPER may reject done status without evidence.
- WATCHDOG may pause risky actions pending user/editor confirmation.
- CURATOR may prune stale reports that lack file paths, evidence, or next actions.

## Escalation Chain

1. Lane owner resolves local issue.
2. DISPATCHER resolves dependency/ownership issue.
3. REAPER resolves validation classification.
4. WATCHDOG flags risk if quality/scope/destructive action is involved.
5. ADMIRAL decides stop/split/continue.
6. USER is asked only for irreversible/editor/external/shared-state decisions.
