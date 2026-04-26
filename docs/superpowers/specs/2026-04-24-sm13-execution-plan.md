# SM-13 Execution Plan

Date: 2026-04-24
Status: Active

## Mission

Ship a polished vertical-slice-plus short game by `2026-05-14`, with `2026-05-15` to `2026-05-17` reserved for contingency and polish only.

## Operating Model

The project runs as an autonomous SM-13 War Studio, not as a generic multi-agent platform and not as unlimited concurrent writers.

The active operating model is dynamic:

- daily war mode: `96` agents
- burst war mode: `192` agents for bounded reconnaissance, milestone review, QA lockdown, and red-team sweeps
- all agent work is specified as `gpt-5.5` with `xhigh` effort
- all departments serve the current SM-13 game only

Most high-concurrency work is read-only reconnaissance, review, QA, red-team, and context hygiene. Write work remains file-scoped and review-gated.

### Always-on roles

- `ADMIRAL`: scope, escalation, and final prioritization
- `DISPATCHER`: packet creation and routing
- `CURATOR`: working-set assembly and context hygiene
- `WATCHDOG`: blocker/risk/watch state
- `ORACLE`: architecture gate
- `ARBITER`: independent review assignment
- `REAPER`: validation, perf, and release readiness

### Primary execution lanes

- Player Core
- Interaction and Archive
- Threat AI
- Game Rule and Save
- UI and Presentation
- Level Scripting and Beats
- Audio and Sequencing
- Tools, Build, and Test

### Concurrency rules

- max active write lanes: `8`
- max active review lanes: `5`
- research lanes can burst higher, then close immediately
- no overlapping ownership on the same write set
- target packet duration: `30-120` minutes

## Independent Review Chain

Every non-trivial packet follows:

1. implement
2. local self-check
3. independent code review
4. independent runtime or build validation
5. evidence write-back

No exception for “small” changes if they affect engine setup, gameplay rules, save data, input, AI, or asset automation.

## Memory Hygiene Rules

After each packet:

- close idle agents
- compress logs into short evidence notes
- drop stale exploratory context
- keep only the current milestone, blockers, and validated facts
- replace outdated summaries instead of appending them

After each day:

- prune inactive routes
- archive completed review chains
- refresh the top risk list

## Milestone Breakdown

### Phase 0: Architecture Lock
Dates: `2026-04-24` to `2026-04-26`

Goals:

- remove template shooter drift from the mainline
- create stable player/game mode/controller bases
- establish gameplay tags, event vocabulary, and save boundaries
- unblock Unreal Editor command-line automation
- define the content path for the first playable slice

### Phase 1: First Playable
Dates: `2026-04-27` to `2026-04-30`

Goals:

- one finished start area
- camera-and-archive interaction loop
- checkpoint/save loop
- first pressure beat
- minimal threat prototype

### Phase 2: M1 Vertical Slice
Dates: `2026-05-01` to `2026-05-07`

Goals:

- 5-minute polished core experience
- one complete investigation loop
- one chase sequence
- one puzzle with assist path
- reviewable architecture story and debug evidence

### Phase 3: Full Flow Integration
Dates: `2026-05-08` to `2026-05-11`

Goals:

- all major spaces connected
- full 45-75 minute route reachable
- ending logic integrated
- threat behavior stable across the full route

### Phase 4: Polish and Lockdown
Dates: `2026-05-12` to `2026-05-14`

Goals:

- performance within budget
- packaging stable
- bug burn-down
- trailer/PPT/QA evidence generation

### Phase 5: Contingency Buffer
Dates: `2026-05-15` to `2026-05-17`

Allowed:

- bug fixes
- perf fixes
- audio/presentation polish
- delivery material cleanup

Not allowed:

- new systems
- new endings
- new large level branches
- architectural rewrites

## First Engineering Wave

The first wave is fixed:

1. disable broken project/plugin blockers
2. establish `AHorrorPlayerCharacter`, `AHorrorGameModeBase`, and `AHorrorPlayerController`
3. define initial empty gameplay components for interaction, inventory, VHS, notes, and camera
4. normalize input actions around horror semantics
5. add initial gameplay tag config
6. create Blueprint child classes for designer-facing configuration

## Definition of Done

A packet is done only when:

- code or asset changes exist
- compile or runtime verification exists
- independent review has passed
- affected docs or evidence are updated
- stale packet context is cleared

## Escalation Triggers

Immediate escalation if any of the following happens:

- Unreal automation remains blocked after one bounded fix attempt
- asset pipeline changes require destructive migration
- performance collapses below the target budget
- a feature proposal violates the short-form scope
- AI work starts demanding multiple enemy archetypes or combat escalation
