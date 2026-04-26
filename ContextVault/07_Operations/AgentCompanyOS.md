# Agent Company OS

Date: 2026-04-26
Scope: SM-13 / Deep Water Station vertical slice
Mode: Agent-team company, main-thread command deck, high-concurrency support

## Purpose

ContextVault is the company memory and control plane for high-speed SM-13 production. It keeps many agents aligned without turning speed into chaos.

## Operating Principles

1. **One mission, many lanes** — every agent packet must map back to NorthStar, milestone, active risk, or validation evidence.
2. **One writer per file set** — parallel agents may inspect freely, but only one lane owns a file set at a time unless explicitly handed off.
3. **Evidence beats optimism** — build/test output, diff, asset path, or EventBus entry is required before declaring a packet done.
4. **Binary assets are editor-owned** — agents may audit `.uasset` / `.umap` references, but do not move/delete/modify binary assets outside UE Editor.
5. **Fast loops, hard gates** — speed comes from smaller packets and parallel reconnaissance, not skipping validation.

## Company Roles

| Role | Function | Default Output |
|---|---|---|
| ADMIRAL | Decides direction, packet priority, milestone tradeoffs | ADR, WarStudioStatus update |
| DISPATCHER | Breaks work into non-overlapping lanes | mission packets, ownership map |
| ORACLE | Architecture review and boundary decisions | interface/gate recommendations |
| FORGE | Implementation lane owner | code changes + focused tests |
| REAPER | Validation and regression owner | build/test evidence, failure triage |
| WATCHDOG | Risk, scope, destructive-action guard | risk escalation, stop/go call |
| CURATOR | ContextVault hygiene and evidence indexing | EventBus entries, status refresh |
| LABYRINTH | Level/map/asset route reconnaissance | asset-safe route notes |
| MUSE | presentation, UI, trailer, sound, story feel | polish packet recommendations |
| SPECTER | AI/threat/encounter behavior audit | threat/director packet notes |
| WEAVER | gameplay-system integration | handoff and dependency notes |
| HIVEMIND | prompt/runtime/agent intelligence layer | agent protocol improvements |

## Command Deck Loop

1. **Scan**: read active status, risks, current diff, recent validation.
2. **Select**: choose one main write packet and several read-only recon packets.
3. **Dispatch**: assign lanes with explicit file ownership and success criteria.
4. **Implement**: main thread or one writer agent edits; other agents do not touch owned files.
5. **Validate**: build, focused automation, then broader regression when risk justifies it.
6. **Record**: update EventBus, WarStudioStatus, and any risk/ADR affected.
7. **Handoff**: next packet starts only after evidence or explicit blocked state.

## Concurrency Modes

| Mode | Use When | Writer Policy | Validation Policy |
|---|---|---|---|
| Solo | unknown failure, fragile file ownership | main thread only | focused build/test |
| Team | normal SM-13 packets | one writer + read-only agents | focused tests + targeted regression |
| Burst | many independent audits or failures | isolated worktrees or read-only agents | integrate one lane at a time |
| Freeze | release/package/trailer lock | no speculative edits | full regression and package smoke |

## File Ownership Rule

A packet owns files by path prefix and topic. If two packets need the same prefix, DISPATCHER serializes them or gives one packet read-only scope.

Examples:

- `Source/HorrorProject/Game/DeepWaterStationRouteKit.*` — route kit lane only.
- `Source/HorrorProject/Variant_Horror/UI/*` — UI bridge lane only.
- `Content/**/*.uasset` / `Content/**/*.umap` — UE Editor lane only.
- `ContextVault/**` — CURATOR lane; main thread may update status/evidence.

## Done Definition

A packet is done only when it has:

- stated scope and affected files;
- implementation or explicit no-code finding;
- validation evidence or a documented reason validation cannot run;
- EventBus entry for milestone/risk/significant state change;
- no unresolved ownership collision.
