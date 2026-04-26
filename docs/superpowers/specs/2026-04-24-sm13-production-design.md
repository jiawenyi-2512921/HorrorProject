# SM-13 Production Design

Date: 2026-04-24
Status: Approved by autonomous user directive
Project: 深渊档案：SM-13回响

## Objective

Build a premium-feeling, single-player, first-person VHS horror investigation game in Unreal Engine 5.6 using the existing project assets, open-source dependencies where they reduce risk, and a review-heavy autonomous agent workflow.

This is a private personal project. It is not being optimized for monetization, live operations, multiplayer, or external modding.

## Chosen Direction

The project will be a 45-75 minute deep-sea station horror experience built on:

- `DeepWaterStation` for the primary environment
- `Bodycam_VHS_Effect` for camera language, post-processing, and sensory identity
- `Variant_Horror` plus the base first-person stack for movement, stamina, flashlight, and UI foundations

The intended experience is:

1. Enter a compromised undersea research station.
2. Investigate anomalies through a camera-and-archive-driven loop.
3. Survive pressure spikes from a single primary threat.
4. Resolve the incident through an informed ending choice.

## Why This Direction

This direction wins on all four critical axes:

1. Asset leverage
   The strongest environment package and the strongest presentation package already fit each other.
2. Technical fit
   The current codebase already supports first-person locomotion, flashlight, sprint, UI, and reusable AI/shooter fragments.
3. Scope discipline
   A short authored horror game can reach a polished state in the current delivery window.
4. Commercial reference quality
   The right inspirations are high-quality, short-form, atmosphere-first games rather than system-heavy productions.

## Design Anchors

Primary references:

1. `SOMA`
   Borrow: underwater facility dread, environmental storytelling, document-driven discovery.
   Do not borrow: large lore sprawl and long-form narrative mass.
2. `Still Wakes the Deep`
   Borrow: environment as threat, no-power fantasy tension, audio-led pressure.
   Do not borrow: film-scale spectacle or content density.
3. `Iron Lung`
   Borrow: constrained perception, camera as information loop, short-form intensity.
   Do not borrow: total opacity or deliberately harsh readability loss.
4. `Fears to Fathom`
   Borrow: VHS intimacy, mundane spaces turning unstable, episodic scope discipline.
   Do not borrow: intentionally rough production values.

## Non-Negotiable Scope Boundaries

The following are explicitly out of scope:

- multiplayer
- open world or hub-and-spoke expansion
- complex combat systems
- multi-weapon progression
- multiple enemy archetypes as a core pillar
- procedural content as a headline feature
- large branching narrative trees
- LLM-driven freeform NPC dialogue as a main mechanic
- mobile controls or mobile build targets

## Core Experience Pillars

1. Investigation over combat
   The player advances by observing, matching, recording, and interpreting.
2. Pressure through one persistent threat
   Tension comes from a single well-authored pursuer plus environment escalation.
3. VHS as gameplay language
   VHS is not only a filter; it communicates clarity, corruption, and threat state.
4. Elegant short-form pacing
   Quiet reading, observation, and traversal alternate with brief high-pressure beats.
5. Consequence-based endings
   Endings reflect what the player learned and chose to preserve, expose, or sever.

## Core Loop

1. Explore a contained station segment.
2. Collect visual or archival evidence.
3. Match evidence to the correct record, timestamp, or event.
4. Unlock a route, system, or truth fragment.
5. Trigger a pressure escalation, chase beat, or environmental hazard.
6. Repeat at a higher narrative and sensory intensity.

## Gameplay Systems

### Required

- first-person movement, look, sprint, flashlight
- camera-driven observation and evidence capture
- interaction system
- archive/inventory/note tracking
- objective and checkpoint progression
- save/load
- one primary threat with patrol, investigate, chase, and kill states
- VHS/post-process intensity modulation
- ending resolution logic

### Optional but allowed

- minimal self-defense or one-use deterrent
- lightweight diegetic UI for camera or archive review
- one contained rewind-style mechanic if it survives the Day 7 cut

### Explicitly not required for M1

- generalized inventory framework
- large dialogue system
- weapon ecosystem
- broad systemic sandbox behaviors

## Technical Architecture

### Keep

- `Enhanced Input`
- `Gameplay Tags`
- `SaveGame` or a tightly scoped save dependency
- `StateTree` for enemy behavior
- `EQS` only if it simplifies the single-threat loop

### Use selectively

- reusable fragments from `Variant_Shooter` for damage, perception, or death presentation

### Avoid for this delivery window

- `Gameplay Ability System`
- `Game Features` as a core modularity layer
- broad plugin sprawl
- systems that shift the architecture toward a live-service or content-platform model

## Open-Source Dependency Policy

Direct dependencies are allowed only if they satisfy all of the following:

- narrow responsibility
- current maintenance signal
- low engine-coupling risk
- easy to pin to a specific commit or release
- easy to remove later

Current whitelist candidates:

- `sinbad/SPUD` for save persistence
- `ProjectBorealis/UEGitPlugin` if Git LFS workflow pain needs fixing
- `getsentry/sentry-unreal` for crash and runtime observability

Reference-only resources:

- `tranek/GASDocumentation`
- `EpicGames/EOS-Getting-Started`
- `ProjectBorealis/PBCharacterMovement`

## Review and Quality Bar

No implementation is complete unless all of the following are true:

- architecture boundary accepted
- code reviewed by an independent reviewer
- behavior verified in-engine or by automated test when possible
- regression risk called out
- evidence written to project memory

Writers do not self-approve. Review and delivery validation are separate chains.

## Memory and Context Discipline

The fleet will keep only five durable things warm:

- project constitution
- current milestone objective
- active packet working set
- verified evidence
- current blockers

Everything else is disposable. Long traces are summarized, stale packets are closed, and speculative notes are not promoted into durable memory.

## Delivery Dates

There is a date mismatch between the project constitution and the latest user wording.

To remove ambiguity:

- hard delivery target remains `2026-05-14` to match the existing `Day 21 / M3`
- `2026-05-15` through `2026-05-17` are treated as contingency and polish buffer only

## Success Criteria

The final build should be defensible as:

1. technically rigorous
2. visually and tonally coherent
3. tightly scoped
4. architecturally explainable
5. polished enough to feel intentional rather than templated
