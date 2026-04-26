# SM-13 War Studio Wave 0 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Start the SM-13 War Studio operating loop around `HorrorProject/`, produce the first project risk map, and prepare bounded implementation packets without closed-door assumptions.

**Architecture:** Wave 0 is mostly read-only reconnaissance plus governance. High concurrency is used for code, asset, build, and external reference discovery; write work is limited to concise project evidence, packet plans, and context hygiene documents.

**Tech Stack:** Unreal Engine project files, C++/Blueprint asset inventory, Claude Code scheduled tasks, project memory, git status, docs under `docs/superpowers/`, and external open-source research gated by ORACLE.

---

## File Structure

- Modify: `docs/superpowers/specs/2026-04-24-sm13-war-studio-operating-design.md`
  - Source of truth for the 96/192-agent War Studio model.
- Modify: `docs/superpowers/specs/2026-04-24-sm13-execution-plan.md`
  - Milestone and execution source of truth.
- Create: `docs/superpowers/plans/2026-04-24-sm13-war-studio-wave-0.md`
  - This implementation plan.
- Create: `ContextVault/07_Operations/WarStudioStatus.md`
  - Current operating status, blockers, active packets, and next dispatch.
- Create: `ContextVault/07_Operations/Day0RiskMap.md`
  - Verified Day 0 risk map for `HorrorProject/`.
- Modify: `ContextVault/99_EventBus.md`
  - Add concise events when War Studio loop and Day 0 reconnaissance start.
- Memory: `C:\Users\。\.claude\projects\d--gptzuo\memory\MEMORY.md`
  - Keep only durable SM-13 operating memories.

---

### Task 1: Confirm durable War Studio operating loop

**Files:**
- Modify: `.claude/scheduled_tasks.json`
- Read: `C:\Users\。\.claude\projects\d--gptzuo\memory\MEMORY.md`

- [ ] **Step 1: List scheduled tasks**

Run: use `CronList`.

Expected: A recurring job exists with prompt containing `SM-13 War Studio control loop` and cadence `*/10 * * * *`.

- [ ] **Step 2: Verify memory index is concise**

Read: `C:\Users\。\.claude\projects\d--gptzuo\memory\MEMORY.md`

Expected content remains only:

```markdown
- [SM-13 War Studio Operating Model](sm13_war_studio_operating_model.md) — autonomous 96/192 agent operating model for current project.
- [Autonomous SM-13 Execution Preference](autonomous_sm13_execution_preference.md) — minimize questions and use needed tools/skills directly.
```

- [ ] **Step 3: If memory contains non-SM-13 or stale operational chatter, remove it**

Write the memory index back with only the two lines above unless a newer durable SM-13 fact has been explicitly approved.

Expected: no transient task details remain in memory.

---

### Task 2: Create operations status files

**Files:**
- Create: `ContextVault/07_Operations/WarStudioStatus.md`
- Create: `ContextVault/07_Operations/Day0RiskMap.md`

- [ ] **Step 1: Ensure operations directory exists**

Run:

```bash
mkdir -p "ContextVault/07_Operations"
```

Expected: command exits `0`.

- [ ] **Step 2: Create WarStudioStatus.md**

Write this content:

```markdown
# War Studio Status

Date: 2026-04-24
Project Root: HorrorProject
Operating Mode: Daily War Mode, 96-agent baseline with 192-agent burst windows
Deadline: 2026-05-14 hard delivery; 2026-05-15 to 2026-05-17 contingency only

## Current Objective

Complete Wave 0 reconnaissance for `HorrorProject/`, identify build/open blockers, map existing systems and assets, and prepare the first bounded implementation packets.

## Active Control Loop

- cadence: every 10 minutes while scheduler is active
- durable: true
- expiration: scheduler auto-expires after 7 days unless renewed
- behavior: inspect state, prune stale context, dispatch safe reconnaissance, update evidence, prepare next packet

## Active Packets

1. Build/Open Recon
2. Code and Module Recon
3. Map and Asset Recon
4. AI and Threat Recon
5. VHS/PostProcess/Audio Recon
6. UI/Interaction/Evidence Recon
7. External Open-Source Reference Recon
8. Day 0 Risk Map

## Context Hygiene

Keep warm:

- current milestone
- active blockers
- verified file and asset paths
- validated architecture decisions
- next packet

Discard:

- duplicate summaries
- stale exploration logs
- speculative implementation ideas without evidence
```

Expected: file exists and contains no placeholders.

- [ ] **Step 3: Create Day0RiskMap.md skeleton**

Write this content:

```markdown
# Day 0 Risk Map

Date: 2026-04-24
Project Root: HorrorProject
Status: Reconnaissance in progress

## Confirmed Facts

- Project path is `HorrorProject/`.
- Target game direction is deep-sea VHS investigation horror.
- War Studio operates at 96-agent daily mode with 192-agent burst mode.

## Build/Open Blockers

Pending reconnaissance.

## Code Architecture Risks

Pending reconnaissance.

## Asset and Level Risks

Pending reconnaissance.

## AI and Threat Risks

Pending reconnaissance.

## VHS, Audio, and Presentation Risks

Pending reconnaissance.

## UI, Interaction, Evidence Risks

Pending reconnaissance.

## External Dependency Opportunities

Pending OPENWATER reconnaissance.

## Next Packets

1. Verify `.uproject`, modules, plugins, and engine version.
2. Inventory source files and existing gameplay classes.
3. Inventory maps, post-process assets, audio assets, UI assets, and AI assets.
4. Research mature Unreal references only where they reduce risk.
```

Expected: file exists and marks pending sections explicitly for reconnaissance.

---

### Task 3: Run first local project reconnaissance

**Files:**
- Read-only: `HorrorProject/**`
- Modify: `ContextVault/07_Operations/Day0RiskMap.md`

- [ ] **Step 1: Locate Unreal project files**

Use Glob:

```text
path: d:\gptzuo\HorrorProject
pattern: **/*.uproject
```

Expected: at least one `.uproject` path or an explicit blocker if none exists.

- [ ] **Step 2: Locate source files**

Use Glob:

```text
path: d:\gptzuo\HorrorProject
pattern: **/*.{h,cpp,cs}
```

Expected: C++ module and Build.cs files are listed or the project is Blueprint-only.

- [ ] **Step 3: Locate maps and core assets**

Use Glob for each pattern:

```text
**/*.umap
**/*.uasset
```

Expected: map and asset paths are available for classification.

- [ ] **Step 4: Search for core system names**

Use Grep over `HorrorProject/` for:

```text
EnhancedInput|StateTree|EQS|SaveGame|Interaction|Inventory|Archive|VHS|PostProcess|AIController|BehaviorTree
```

Expected: matches are grouped into Code Architecture, AI/Threat, VHS/Audio, UI/Interaction/Evidence sections.

- [ ] **Step 5: Update Day0RiskMap.md with verified facts**

Replace each `Pending reconnaissance.` section only when facts exist.

Expected: every added claim has a file path or command evidence.

---

### Task 4: Dispatch parallel reconnaissance agents

**Files:**
- Read-only: `HorrorProject/**`
- Modify after synthesis: `ContextVault/07_Operations/Day0RiskMap.md`

- [ ] **Step 1: Dispatch Code/Module Recon**

Agent prompt:

```text
You are FORGE Code/Module Recon for SM-13. Read only. Inspect d:\gptzuo\HorrorProject for Unreal modules, C++ classes, Build.cs files, plugins, input setup, GameMode/Character/Controller classes, and obvious build blockers. Report concise facts with paths and risks. Do not edit files.
```

Expected: concise report of modules, classes, build blockers, and recommended first implementation boundary.

- [ ] **Step 2: Dispatch Level/Asset Recon**

Agent prompt:

```text
You are LABYRINTH Level/Asset Recon for SM-13. Read only. Inspect d:\gptzuo\HorrorProject for maps, DeepWaterStation assets, level organization, content folders, and route-building opportunities. Report concise facts with paths and risks. Do not edit files.
```

Expected: concise report of maps, candidate starting area, missing level blockers, and path risks.

- [ ] **Step 3: Dispatch AI/Threat Recon**

Agent prompt:

```text
You are SPECTER AI/Threat Recon for SM-13. Read only. Inspect d:\gptzuo\HorrorProject for AIController, StateTree, BehaviorTree, EQS, perception, enemy, chase, damage, and failure assets/code. Report concise facts with paths and risks. Do not edit files.
```

Expected: concise report of existing AI resources and whether a single-threat loop can reuse them.

- [ ] **Step 4: Dispatch VHS/Audio/Presentation Recon**

Agent prompt:

```text
You are MUSE VHS/Audio/Presentation Recon for SM-13. Read only. Inspect d:\gptzuo\HorrorProject for VHS, post-process, camera, lighting, audio, sound cues, MetaSounds, and presentation assets. Report concise facts with paths and risks. Do not edit files.
```

Expected: concise report of sensory assets and presentation blockers.

- [ ] **Step 5: Dispatch UI/Interaction/Evidence Recon**

Agent prompt:

```text
You are WEAVER/HIVEMIND UI Interaction Evidence Recon for SM-13. Read only. Inspect d:\gptzuo\HorrorProject for UI widgets, interaction Blueprints/C++, objective systems, notes, archive, inventory, and evidence-like assets. Report concise facts with paths and risks. Do not edit files.
```

Expected: concise report of interaction/UI/evidence foundations.

- [ ] **Step 6: Dispatch Build/QA Recon**

Agent prompt:

```text
You are REAPER Build/QA Recon for SM-13. Read only unless explicitly running safe local inspection commands. Inspect d:\gptzuo\HorrorProject for .uproject settings, plugin dependencies, generated files, build scripts, Visual Studio integration, and likely Unreal open/build blockers. Report concise facts with paths and safe next verification commands. Do not edit files.
```

Expected: concise report of build/open risks and recommended next command.

---

### Task 5: Run OPENWATER external reference reconnaissance

**Files:**
- Modify: `ContextVault/07_Operations/Day0RiskMap.md`

- [ ] **Step 1: Research Unreal save system candidates**

Search the web for mature Unreal Engine save systems and evaluate against SM-13 dependency policy.

Expected output section:

```markdown
## External Dependency Opportunities

### Save Persistence

- Candidate: SPUD
- Use: possible narrow save persistence dependency
- Risk: engine version compatibility must be verified before adoption
- Decision: research-only until ORACLE approves
```

- [ ] **Step 2: Research Unreal interaction/evidence references**

Search mature Unreal interaction system examples, but prefer implementation patterns over dependencies.

Expected: list patterns, not copied code, unless license and integration path are clear.

- [ ] **Step 3: Research StateTree single-threat examples**

Search Unreal StateTree AI examples relevant to patrol/investigate/chase.

Expected: concise references and lessons for SPECTER, no adoption decision yet.

---

### Task 6: Update EventBus for War Studio start

**Files:**
- Modify: `ContextVault/99_EventBus.md`

- [ ] **Step 1: Add War Studio loop event**

Append this event before the closing code fence:

```yaml
- timestamp: 2026-04-24T10:00:00
  event: Event.WarStudio.ControlLoop.Enabled
  from: CEO
  payload:
    subject: Durable SM-13 War Studio control loop enabled
    cadence: every_10_minutes
    mode: daily_96_burst_192
    project_root: HorrorProject
  subscribers_expected: [ADMIRAL, DISPATCHER, CURATOR, WATCHDOG, ORACLE, ARBITER, REAPER, SCRIBE]
```

Expected: YAML remains inside the existing code fence.

- [ ] **Step 2: Add Day 0 recon event**

Append this event after the control loop event:

```yaml
- timestamp: 2026-04-24T10:05:00
  event: Event.Recon.Day0.Started
  from: DISPATCHER
  payload:
    subject: HorrorProject Day 0 reconnaissance started
    docs:
      - ContextVault/07_Operations/WarStudioStatus.md
      - ContextVault/07_Operations/Day0RiskMap.md
  subscribers_expected: [FORGE, LABYRINTH, SPECTER, MUSE, HIVEMIND, WEAVER, REAPER, OPENWATER]
```

Expected: file remains valid markdown with a single closing code fence.

---

### Task 7: Self-review and handoff

**Files:**
- Read: `docs/superpowers/plans/2026-04-24-sm13-war-studio-wave-0.md`
- Read: `ContextVault/07_Operations/WarStudioStatus.md`
- Read: `ContextVault/07_Operations/Day0RiskMap.md`

- [ ] **Step 1: Placeholder scan**

Search the plan and operations docs for incomplete-marker terms from the writing-plans skill, excluding this self-review instruction line.

Expected: no matches outside the self-review instruction itself.

- [ ] **Step 2: Scope scan**

Confirm all tasks directly serve `HorrorProject/` and SM-13.

Expected: no generic platform or unrelated future-project tasks.

- [ ] **Step 3: Context hygiene scan**

Confirm memory remains concise and transient details are only in operations docs.

Expected: memory contains durable operating rules only.

- [ ] **Step 4: Next packet decision**

If reconnaissance has enough facts, create the first Execution Packet for the highest-blocking item. If facts are insufficient, continue Recon Packets instead of guessing.

Expected: next action is evidence-driven.
