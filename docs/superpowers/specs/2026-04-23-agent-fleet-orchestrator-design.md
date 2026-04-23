# Agent Fleet Orchestrator Design

Date: 2026-04-23  
Status: Approved for implementation  
Mode: Aggressive autonomous delivery

## Objective

Create a single-window operating system for project delivery where the user gives one instruction and an agent fleet automatically decomposes, executes, reviews, verifies, and documents the result with industrial-grade quality and maximum parallel throughput.

## Design Summary

The fleet uses a layered model:

1. Sovereign layer
   `USER`, `ADMIRAL`, `DISPATCHER`, `CURATOR`, `ARBITER`, `WATCHDOG`
2. Head layer
   `ORACLE`, `FORGE`, `WEAVER`, `HIVEMIND`, `LABYRINTH`, `SPECTER`, `MUSE`, `REAPER`
3. Swarm layer
   Elastic worker swarms such as `SMITH-*`, `LOOM-*`, `GHOST-*`, `SEED-*`, `LUMEN-*`, `SCRIBE-*`, `SENTRY-*`
4. Artifact layer
   Code, tests, prompts, ADRs, event logs, inventories, and milestone evidence

The fleet is not modeled as 24 permanently live threads. Instead, stable roles define responsibility and review authority, while worker swarms expand and contract dynamically based on task graph shape.

## Core Principles

### 1. Single source of truth

All durable knowledge is written into `ContextVault/`. Agents do not maintain separate long-lived notes outside approved artifacts.

### 2. Event-driven collaboration

Cross-role coordination happens through `ContextVault/99_EventBus.md` and structured mission packets, not ad hoc conversational memory.

### 3. Role/model decoupling

Roles define responsibilities. Specific models are replaceable.

### 4. Independent review

The implementation chain and the review chain must be separate. A writer cannot be the final approver of the same artifact.

### 5. Memory hygiene

The system optimizes for precise, current context, not maximal remembered context.

## Memory Architecture

The fleet uses six memory layers:

1. Constitution memory
   North star, milestones, quality bars, architecture laws
2. Domain memory
   Role-specific guidance and standards
3. Mission memory
   Current task goals, scope, acceptance criteria, risks
4. Working set
   Minimal files and evidence needed for the active subtask
5. Ephemeral scratch
   Temporary reasoning and failed hypotheses; destroyed after task completion
6. Evidence memory
   Verified facts promoted from tests, decisions, or approved deliverables

### Automatic cleanup rules

- Destroy ephemeral scratch by default after task completion
- Invalidate working sets when source files materially change
- Deduplicate repeated facts
- Compress long logs into evidence summaries plus references
- Do not promote unverified speculation into durable memory
- Demote stale summaries that are not reused

## Control Flow

1. User issues a task in the main thread
2. `DISPATCHER` produces a mission brief and dependency graph
3. `CURATOR` compiles minimal context packets
4. Relevant heads receive execution packets
5. Heads create worker swarms for bounded subtasks
6. Workers implement or produce artifacts
7. Head-level review runs first
8. `ARBITER` assigns independent review
9. `REAPER` validates tests, performance, regression, and delivery evidence
10. `ADMIRAL` arbitrates only when risk, scope, or direction requires final judgment

## Review Gates

### Architecture gate

`ORACLE` must approve new boundaries, interfaces, patterns, class introductions, and coupling tradeoffs.

### Department gate

Each head reviews domain correctness before cross-functional approval.

### Independent review gate

`ARBITER` assigns a separate reviewer chain.

### Delivery gate

`REAPER` verifies acceptance criteria, tests, performance, and release readiness.

## Workspace Artifacts

### `.fleet/`

The fleet operating system. Contains manifest, policies, prompts, and reusable packet templates.

### `ContextVault/`

Project source of truth. Holds north star, milestones, risks, architecture registry, AI docs, content docs, delivery evidence, and event history.

### `agent-fleet-os` skill

A Codex skill that teaches future sessions how to use the fleet. It loads workspace policy and uses the same operating model.

## Risks

### Risk: Context bloat

Mitigation: strict working-set compilation and memory lifecycle rules.

### Risk: False confidence from self-review

Mitigation: independent review chain via `ARBITER`.

### Risk: Speed erodes architecture

Mitigation: mandatory `ORACLE` gate for new interfaces and boundaries.

### Risk: Delivery drifts from acceptance criteria

Mitigation: `REAPER` owns evidence-based completion.

## Scope of This Scaffolding

This implementation creates:

- Fleet operating-system docs and prompts
- A workspace bootstrap entrypoint for future sessions
- ContextVault scaffold for the project
- Review and execution templates
- A reusable Codex skill
- Git initialization for repeatable iteration

This implementation does not create the game itself. It creates the control system that will govern delivery.
