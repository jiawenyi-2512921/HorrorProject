# SM-13 Final Production Plan: Deep Water Station — Last Tape

## Mission

Build the most polished possible 21-day Unreal Engine 5.6 horror vertical slice from the current project. The target is not a large game. The target is a tiny, dense, premium 10-15 minute found-footage experience with enough controlled material for a 3-minute trailer.

The final piece should feel expensive because every meter, interaction, sound, camera beat, and encounter is curated.

## Non-negotiable target

By the end of the 21-day push, the project must deliver:

1. A playable first-person found-footage route set in a deep-water industrial station.
2. A bodycam/VHS presentation layer with strong camera feedback.
3. A clear objective chain from entry to final escape.
4. A scripted stone golem encounter that feels authored, not systemic.
5. Evidence capture and archive review as the core interaction fantasy.
6. Strong horror audio, lighting, and VFX polish.
7. A trailer capture route with repeatable moments.
8. A packaged Windows build or a stable editor-playable build if packaging blocks.

## Scope lock

Do not build an open-ended survival game. Do not build inventory complexity, combat, multiple monsters, procedural levels, full AI simulation, multiplayer, crafting, quests, or large narrative branches.

The slice is one route, one monster, one visual identity, one polished gameplay loop.

## Core fantasy

The player is reviewing/recording the final bodycam tape inside an abandoned underwater station. The camera is both a survival tool and evidence tool. The monster is not fought. It is documented, avoided, and survived.

## Project title

Working title: **Deep Water Station: Last Tape**.

## Asset policy

Use what already exists. Do not import more UE Marketplace/Fab packs unless a specific missing role cannot be filled.

### Keep and use

- `/Game/DeepWaterStation`: primary station environment.
- `/Game/SD_Art`: industrial architecture library; use selectively only if it clearly improves the station.
- `/Game/IndustrialPipesM`: industrial pipe dressing; use selectively.
- `/Game/Bodycam_VHS_Effect`: VHS/bodycam visual language.
- `/Game/SoundsOfHorror`: core sound palette.
- `/Game/Stone_Golem`: core encounter creature.
- `/Game/Input`: input actions used by native code.
- `/Game/Horror`, `/Game/Blueprints`, `/Game/Variant_Horror`: current game mode/player/map bridge.
- `/Game/Characters`: keep because `BP_HorrorPlayerCharacter` currently references `/Game/Characters`.
- `/Game/Grimytheus_Vol_2`: optional music source; use at most 1-2 cues.
- `/Game/LevelPrototyping`: tiny, keep only if useful for greybox interactables.

### Removed in cleanup

The following broad or off-theme packs were deleted from Content during planning cleanup:

- `/Game/AdvancedUniversalSpawner`
- `/Game/Fantastic_Dungeon_Pack`
- `/Game/ForestOfSpikes`
- `/Game/Free_Magic`
- `/Game/M5VFXVOL2`
- `/Game/NiagaraExamples`
- `/Game/Scrapopolis`
- `/Game/ModernBridges`

`/Game/FirstPerson` was mostly deleted, but three files were locked by another process and remain until the editor/process releases them:

- `/Game/FirstPerson/Anims/ABP_FP_Copy.uasset`
- `/Game/FirstPerson/Anims/CtrlRig_FPWarp.uasset`
- `/Game/FirstPerson/MI_FirstPersonColorway.uasset`

Delete these after closing the locking process, then fix redirectors in Unreal Editor.

## Asset organization

Vendor/imported packs stay at top level. Project-authored and curated slice assets go under `/Game/_SM13`.

Created target folders:

- `/Game/_SM13/Maps`
- `/Game/_SM13/Blueprints`
- `/Game/_SM13/Route`
- `/Game/_SM13/UI`
- `/Game/_SM13/Audio`
- `/Game/_SM13/VFX`
- `/Game/_SM13/Materials`
- `/Game/_SM13/Cinematics`

Rules:

1. Do not move marketplace pack internals manually in the filesystem.
2. Duplicate or migrate chosen assets into `_SM13` from inside Unreal Editor when they become part of the final slice.
3. After any in-editor move/delete, run **Fix Up Redirectors in Folder** on `/Game`.
4. Do not manually delete `__ExternalActors__` or `__ExternalObjects__` except pack-specific external actor folders that belong to already-deleted maps.

## Current hard asset references

Do not move these without updating code/config:

- `/Game/Variant_Horror/Lvl_Horror.Lvl_Horror`
- `/Game/Horror/BP_HorrorGameModeBase.BP_HorrorGameModeBase_C`
- `/Game/Input/Actions/IA_Interact.IA_Interact`
- `/Game/Input/Actions/IA_Record.IA_Record`
- `/Game/Input/Actions/IA_TakePhoto.IA_TakePhoto`
- `/Game/Input/Actions/IA_Rewind.IA_Rewind`
- `/Game/Input/Actions/IA_OpenArchive.IA_OpenArchive`
- `/Game/Bodycam_VHS_Effect/Materials/Instances/PostProcess/MI_OldVHS.MI_OldVHS`

## Architecture

### Game authority

`AHorrorGameModeBase` owns the objective state and decides whether the player can advance. It should remain the single source of truth for objective progression.

Responsibilities:

- Track current objective.
- Accept objective completion requests.
- Validate special requirements like camera recording.
- Publish state changes to UI and route systems.
- Unlock final escape after archive review.

### Route kit

`ADeepWaterStationRouteKit` places the route's objective interactables and controlled beats.

Upgrade direction:

- Replace arbitrary linear default positions with authored transforms from the final map.
- Keep route nodes data-driven through editable structs.
- Support tags/IDs for trailer capture beats.
- Avoid full quest system complexity.

### Interactable contract

`UInteractableInterface` stays the standard interaction entry point. All objective props, doors, archive terminals, recorders, notes, and special station controls should implement or route through this interface.

### Objective interactables

`AFoundFootageObjectiveInteractable` is the native objective prop. It should handle simple objective handoff, metadata, and player interaction.

Required fix:

- First anomaly recording must validate actual player recording/bodycam state through `AHorrorGameModeBase::IsLeadPlayerRecording`, not only a local boolean.

### Camera/evidence loop

The bodycam is the identity of the project. The player should feel that the camera changes the world.

Core loop:

1. Find evidence.
2. Raise/enable bodycam.
3. Record or photograph anomaly.
4. Archive updates with evidence metadata.
5. Objective chain advances.
6. Monster/station reacts.

Minimum systems:

- Recording state from player camera component.
- Evidence capture validation window.
- UI feedback for recording/evidence acquired.
- Archive terminal review step.

### Encounter director

Add a lightweight scripted encounter director instead of building complex AI.

Responsibilities:

- Trigger golem reveal.
- Control golem staging position.
- Fire audio/lighting/camera shake events.
- Gate route progression during encounter.
- End encounter cleanly and return control.

The stone golem should be treated like a cinematic horror set piece. It does not need open-world AI. It needs timing, framing, sound, and believable motion.

### UI

The UI should be minimal and diegetic:

- REC indicator.
- Battery/noise/timecode feel.
- Evidence acquired toast.
- Archive review screen.
- Objective hint text only when needed.

Avoid gamey HUD clutter.

### Save

For the 21-day slice, save only what matters:

- Current objective.
- Evidence collected.
- Notes collected.
- Archive reviewed.
- Bodycam acquired.

No complex checkpoint system unless packaging tests show it is needed.

## Agent team operating model

Use the ContextVault/agent team aggressively. The main coordinator owns product direction and integration. Agents own bounded work packets.

### Default agent rules

Every agent must follow these rules:

1. Build for the 21-day target, not future hypothetical scope.
2. Use the highest-quality implementation that still fits the schedule.
3. Prefer small, testable, native C++ systems with Blueprint-facing hooks.
4. Do not create broad frameworks unless directly required.
5. Do not import or invent new assets when existing assets can solve the beat.
6. Keep all project-authored curated assets under `/Game/_SM13`.
7. Respect Unreal reference safety and fix redirectors after asset moves.
8. Report exact files changed and validation performed.

### Parallel work streams

Run these in parallel where possible:

#### Stream A — Core gameplay/code

- Fix first anomaly recording validation.
- Harden objective progression.
- Add evidence capture component if needed.
- Add/upgrade encounter director.
- Add automation tests for progression gates.

#### Stream B — Map/route/layout

- Duplicate active map into `/Game/_SM13/Maps` in Unreal Editor.
- Build a tight station route with 6-8 rooms/beats.
- Place objective nodes.
- Place golem reveal and escape route.
- Remove unused visual noise.

#### Stream C — Visual identity

- Tune VHS/bodycam post process.
- Add water pressure/flicker/fog atmosphere.
- Build 3-5 hero shots for trailer.
- Use DeepWaterStation and SD_Art selectively.

#### Stream D — Audio

- Choose ambient loop from SoundsOfHorror.
- Choose stingers for anomaly, golem reveal, archive, final escape.
- Add localized station groans, metal impacts, distant movement.
- Avoid music overuse; silence and pressure matter.

#### Stream E — UI/archive

- Polish REC/bodycam overlay.
- Build archive review widget.
- Add evidence metadata presentation.
- Add final route unlock feedback.

#### Stream F — QA/package/trailer

- Run editor playthrough every day.
- Maintain a known-good trailer capture route.
- Package early and often.
- Track blockers in ContextVault.

## 21-day schedule

### Days 1-2: Stabilize and freeze direction

- Finish asset cleanup.
- Fix redirectors in Unreal Editor.
- Confirm active map and player BP still load.
- Commit baseline if source control is ready.
- Freeze the exact slice route and no-scope-creep rules.

### Days 3-5: Core route and objective progression

- Implement reliable objective chain.
- Place first pass route objectives.
- Fix anomaly recording validation.
- Make start-to-exit playable in ugly form.

Exit criteria: one full route can be completed without editor intervention.

### Days 6-8: Camera, evidence, archive

- Polish bodycam state and VHS feedback.
- Implement evidence capture feel.
- Implement archive review step.
- Add UI feedback.

Exit criteria: the camera loop feels like the core mechanic, not a debug state.

### Days 9-12: Stone golem set piece

- Build scripted reveal.
- Add chase/avoidance or pressure sequence.
- Add lighting/audio/camera shake timing.
- Gate and release route progression.

Exit criteria: one memorable monster moment is trailer-worthy.

### Days 13-15: Visual/audio polish pass

- Dress only visible route areas.
- Cut all unused spaces from player path.
- Tune fog, flicker, post-process, sound levels.
- Create 3-5 repeatable trailer shots.

Exit criteria: screenshots and short clips look intentional.

### Days 16-18: Packaging and stability

- Package Windows build.
- Fix missing references and cook failures.
- Run full playthroughs.
- Remove dead debug content from route.

Exit criteria: stable packaged or stable editor-playable build.

### Days 19-20: Final polish

- Tighten pacing.
- Reduce confusing UI/objectives.
- Improve final escape beat.
- Record trailer footage.

Exit criteria: the experience feels complete even if small.

### Day 21: Freeze

- No new features.
- Only crash/blocker fixes.
- Final build.
- Final trailer capture.
- Final notes for continuation.

## Implementation priorities

Priority 0:

- Project opens.
- Active map loads.
- Player spawns.
- Input works.
- Route can complete.

Priority 1:

- Evidence/camera loop.
- Golem reveal.
- Archive review.
- Escape endpoint.

Priority 2:

- Audio polish.
- Visual polish.
- Trailer capture points.

Priority 3:

- Extra props, optional music, secondary anomalies.

## Quality bar

Every feature must pass three questions:

1. Does it make the 10-15 minute slice more intense?
2. Does it improve the 3-minute trailer?
3. Can it be finished and tested inside 21 days?

If not, cut it.

## Immediate next work packets

1. Close Unreal Editor or any process locking `/Game/FirstPerson`, delete the remaining three FirstPerson files, then fix redirectors.
2. Open the project and verify the active map still loads.
3. Duplicate `/Game/Variant_Horror/Lvl_Horror` into `/Game/_SM13/Maps` from inside Unreal Editor.
4. Update `DefaultEngine.ini` to point to the `_SM13` map after duplication succeeds.
5. Fix first anomaly recording validation in C++.
6. Build the route blockout in the `_SM13` map.
7. Add the encounter director and wire the stone golem reveal.
8. Package a Windows build as early as possible.

## Final instruction to all future agents

This project wins by concentration. Do less, but make it feel authored. Do not chase systems. Build the tape.
