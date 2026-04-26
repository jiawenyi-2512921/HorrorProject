# Dispatch Protocol

Date: 2026-04-26
Scope: SM-13 high-concurrency execution

## Intake

Every request becomes one of:

- **implementation packet** — edits source/docs/config;
- **recon packet** — read-only audit or plan;
- **validation packet** — build/test/package/playtest evidence;
- **editor-action packet** — UE Editor work on binary assets/maps;
- **decision packet** — ADR, scope call, or risk escalation.

## Triage Questions

1. Does it serve NorthStar, milestone, risk, or validation?
2. Is it code-only, docs-only, validation-only, or editor/binary?
3. Which file territory does it own?
4. What can run in parallel without touching that territory?
5. What evidence proves completion?

## Dispatch Loop

1. ADMIRAL selects priority.
2. DISPATCHER assigns lane and file ownership.
3. CURATOR checks existing ContextVault state.
4. FORGE/role owner executes or scouts.
5. REAPER validates.
6. CURATOR records EventBus/status.
7. ADMIRAL selects next packet.

## WIP Limits

| Work Type | Max Active |
|---|---:|
| Write packet touching source | 1 per file territory |
| ContextVault writer | 1 |
| Validation command | 1 heavy build at a time |
| Read-only scout | many, as useful |
| Editor binary action | 1 coordinated session |

## Packet States

| State | Meaning |
|---|---|
| `queued` | selected but not started |
| `active` | owner is working |
| `blocked` | cannot proceed without named condition |
| `validating` | implementation done, evidence pending |
| `passed` | evidence accepted |
| `failed` | evidence failed, triage required |
| `deferred` | intentionally postponed |

## Merge / Integration Order

1. Finish current writer packet.
2. Review diff for owned files.
3. Run required focused validation.
4. Update evidence.
5. Only then start another writer in overlapping territory.

## Agent Prompt Requirements

A delegated agent must receive:

- objective and why it matters;
- exact mode: read-only, write, validation, or editor-action;
- paths/topics in and out of scope;
- known recent state or validation;
- expected return format;
- instruction not to duplicate active main-thread work.

## Rejection Criteria

Do not dispatch a packet if:

- success criteria are vague;
- it requires binary asset edits without Editor action ownership;
- it overlaps an active writer;
- it is broad cleanup unrelated to SM-13 delivery;
- it has no plausible validation path.
