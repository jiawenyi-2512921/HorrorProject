# SM-13 War Studio Operating Design

Date: 2026-04-24
Status: Approved by user directive
Scope: Current SM-13 project only

## Mission

Run SM-13 as an autonomous war studio focused only on shipping the current deep-sea VHS investigation horror game with maximum quality, depth, and speed.

The studio does not optimize for reusable company infrastructure, generic agent platforms, live-service tooling, or future projects. Every department, agent, packet, review, and external research task must serve the current SM-13 delivery.

## Authority Model

The assistant acts as CEO / Executive Producer.

The user has authorized autonomous operation for day-to-day project decisions. The CEO should not repeatedly ask for operational approval when a decision is reversible, local, and clearly serves the SM-13 north star.

Safety and repository integrity still apply:

- destructive actions require explicit confirmation
- externally visible actions require explicit confirmation
- local commits require explicit user request under the active harness rules
- model policy and system safety instructions override studio convenience

## Model and Effort Rule

All agent work is specified to use `gpt-5.5` with `xhigh` effort.

No department may intentionally downgrade reasoning effort for speed. Speed comes from parallelism, scoped packets, and tight review loops, not from weaker reasoning.

## Dynamic Capacity

The studio uses two operating levels.

### Daily War Mode: 96 agents

Used for continuous production, review, QA, documentation, and context hygiene.

Recommended allocation:

- 15% command, dispatch, scope, context, and architecture control
- 35% reconnaissance, asset indexing, code reading, and external research
- 30% production implementation
- 20% QA, review, red-team, and build validation

### Burst War Mode: 192 agents

Used only for bounded windows:

- full project reconnaissance
- milestone readiness review
- large risk audit
- build lockdown
- Day 7 / Day 14 / Day 21 review
- final red-team sweep

Burst mode is not a permanent writing mode. Most burst agents read, compare, test, review, and summarize. Only tightly scoped owners write to files.

## Command Layer

### CEO / Executive Producer

Owns final project direction, packet priority, scope cuts, department coordination, and delivery judgment.

### ADMIRAL

Owns product north star, milestone discipline, and final scope interpretation.

### DISPATCHER

Turns goals into packets, assigns owners, prevents duplicate work, and tracks dependencies.

### CURATOR

Maintains the active working set, compresses stale context, and prevents context overload.

### WATCHDOG

Monitors deadline risk, scope creep, blockers, and milestone health.

### ORACLE

Owns architecture review, technical risk assessment, Unreal route selection, and dependency policy.

### ARBITER

Runs the independent quality court. Authors do not self-approve.

### SCRIBE

Records ADRs, EventBus updates, verified evidence, and concise packet outcomes.

## Production Departments

### FORGE — Core Engineering

Builds the Unreal C++ / Blueprint foundation for player, interaction, evidence, objectives, save/load, and build stability.

### LABYRINTH — Level and Flow

Owns DeepWaterStation segmentation, route clarity, gates, puzzles, beats, and the 45-75 minute flow.

### SPECTER — Threat AI

Owns the single primary threat: patrol, investigate, chase, kill/fail, tuning, and director-triggered pressure.

### MUSE — VHS, Atmosphere, Audio

Owns VHS language, camera corruption, post-process intensity, lighting mood, environmental audio, threat audio, and sensory pacing.

### HIVEMIND — Narrative and Evidence

Owns evidence chain, archive text, environmental storytelling, endings, and clue readability.

### WEAVER — UI/UX and Diegetic Interface

Owns archive UI, objective UI, camera review, interaction feedback, and readable but immersive interface behavior.

## Support and Review Departments

### REAPER — QA, Regression, Build

Runs build checks, smoke tests, flow validation, packaging risk review, and regression tracking.

### RED TEAM

Actively attempts to break the experience:

- player confusion
- puzzle dead ends
- AI unfairness or stupidity
- VHS readability failures
- save/load corruption
- pacing collapse
- performance drops
- false completion claims

### OPENWATER — External Research and Open Source Intelligence

Prevents closed-door development by continuously checking mature public references and open-source projects when relevant.

OPENWATER may research:

- Unreal Engine horror implementation patterns
- save systems
- interaction systems
- StateTree / AI examples
- VHS and camera effect approaches
- build automation
- QA tooling
- audio implementation references

External code may only be adopted when ORACLE approves:

- license is compatible
- dependency is maintained or small enough to vendor safely
- integration surface is narrow
- removal path is clear
- it serves SM-13 directly

## Packet Types

### Recon Packet

A read-only investigation task. Safe to run at high concurrency. Produces facts, risks, file paths, asset paths, and recommendations.

### Execution Packet

A scoped implementation task with explicit owner, files, acceptance criteria, verification command or in-editor check, and review path.

### Court Review Packet

An independent review task. It checks correctness, integration, UX impact, regression risk, and evidence quality.

### Red-Team Packet

A hostile validation task. It looks for breakage, confusion, bad assumptions, exploit paths, pacing failures, and false confidence.

### Hygiene Packet

A context cleanup task. It closes stale notes, updates concise memory, removes duplicate summaries, and preserves only validated facts.

## Concurrency Rules

- Read-only reconnaissance can scale aggressively.
- Write work is limited by ownership boundaries, not by available tokens.
- No two write packets may edit the same file set simultaneously.
- Every non-trivial write packet must have an independent reviewer.
- Critical systems require at least two independent review perspectives.
- Production speed cannot bypass REAPER validation or ARBITER review.
- If a packet cannot define acceptance criteria, it is not ready to execute.

## Interface Stability

The studio keeps the same packet interface across departments:

- objective
- owner department
- allowed file or asset scope
- dependencies
- acceptance criteria
- verification method
- review owner
- evidence output
- context cleanup rule

Department internals may scale up or down, but the packet interface remains stable so the company can keep running without coordination collapse.

## Continuous Operation

The studio uses recurring control loops to keep work moving while the session is alive.

Loop responsibilities:

1. inspect current git/project state
2. identify active blockers
3. dispatch safe read-only reconnaissance where useful
4. advance one bounded implementation or planning packet
5. run or queue verification when possible
6. update concise evidence
7. prune stale context
8. produce the next packet

Recurring tasks are bounded by the harness scheduler. Session-only loops stop when the session exits; durable loops can survive restart when explicitly created. Recurring scheduled tasks auto-expire after 7 days unless renewed.

## Context Hygiene

CURATOR and SCRIBE must aggressively prevent context overload.

Keep warm:

- current milestone objective
- active blockers
- validated architecture decisions
- current packet state
- verified evidence

Discard or summarize:

- stale exploration logs
- duplicate summaries
- abandoned alternatives
- speculative notes
- raw agent transcripts after evidence extraction

Memory is for durable project facts and user preferences, not transient task chatter.

## Quality Bar

A packet is complete only when:

- implementation or validated decision exists
- verification evidence exists
- independent review has passed
- user-facing or project-facing docs are updated when needed
- stale context has been cleared

No department can mark its own work as final.

## Immediate Operating Directive

The first War Studio wave should focus on:

1. verify project build/open state
2. inventory HorrorProject code, maps, plugins, and assets
3. identify Unreal blockers
4. establish architecture boundaries for player, interaction, evidence, save, VHS, threat, and UI
5. produce Day 0 risk map
6. dispatch the first implementation packet only after the relevant boundary is known

The project remains locked to SM-13. Any task that does not directly improve the current game is rejected by default.
