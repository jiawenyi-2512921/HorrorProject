# Quality Gate Protocol

Date: 2026-04-26
Scope: SM-13 high-concurrency production quality

## Gate Stack

| Gate | Name | Required Before |
|---|---|---|
| G0 | Scope Gate | dispatching a write packet |
| G1 | Ownership Gate | editing files |
| G2 | Implementation Gate | claiming code is complete |
| G3 | Validation Gate | recording packet as passed |
| G4 | Evidence Gate | moving to the next packet |
| G5 | Release Gate | packaging, trailer capture, submission |

## G0 Scope Gate

A packet must answer:

- Which NorthStar objective or risk does it serve?
- Which files/assets are in scope?
- What is explicitly out of scope?
- What test or observable result proves success?

Reject or split packets that include unrelated refactors, broad framework work, or binary asset edits without Editor ownership.

## G1 Ownership Gate

Before edits:

- verify no other active writer owns the same file set;
- inspect current git status for the target path;
- preserve unrelated user/agent changes;
- record binary asset work as an Editor task, not a code patch.

## G2 Implementation Gate

A code packet must be:

- minimal for the stated packet;
- testable through existing automation or a new focused test;
- Blueprint-safe when exposing UE-facing APIs;
- free of speculative abstractions and broad rewrites.

## G3 Validation Gate

Default validation ladder:

1. `HorrorProjectEditor Win64 Development` build.
2. Focused automation for changed system.
3. Broader targeted automation if contracts changed.
4. Full `Automation RunTests HorrorProject` after cross-system changes or before milestone closeout.

Validation may be deferred only if blocked by UE Editor binary asset work or an explicit user interruption. Deferred validation must be recorded as pending.

## G4 Evidence Gate

Passed packets need one of:

- EventBus validation entry;
- WarStudioStatus active packet update;
- RiskMatrix update if risk changed;
- DecisionLog ADR if operating model or architecture boundary changed.

## G5 Release Gate

Before packaging or trailer capture:

- full regression passes;
- known binary asset redirectors are fixed in UE Editor;
- startup map and game mode are verified in Editor;
- trailer route beats are enumerated and playable;
- PPT / QA evidence links are current.

## Red Flags

Immediate WATCHDOG escalation if:

- build fails twice for unrelated domains;
- a binary asset is modified outside UE Editor;
- a packet changes architecture without an ADR;
- full regression fails after a milestone packet;
- scope expands beyond NorthStar.

## Validation Evidence Format

```yaml
event: Event.Validation.SM13.PacketName.Passed|Failed
from: REAPER
payload:
  subject: concise outcome
  build: command + exit code
  tests:
    - command/name + exit code
  scope:
    - verified behavior
  blockers:
    - remaining issue, if any
```
