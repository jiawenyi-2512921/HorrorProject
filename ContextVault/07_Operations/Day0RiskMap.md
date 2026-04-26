# Day 0 Risk Map

Date: 2026-04-24
Project Root: HorrorProject
Status: Reconnaissance in progress

## Confirmed Facts

- Project path is `HorrorProject/`.
- Unreal project file is `HorrorProject/HorrorProject/HorrorProject.uproject`.
- Engine association is `5.6`.
- Runtime module is `HorrorProject`.
- Target game direction is deep-sea VHS investigation horror.
- War Studio operates at 96-agent daily mode with 192-agent burst mode.
- Enabled project plugins include `ModelingToolsEditorMode`, `StateTree`, and `GameplayStateTree`.
- C++ dependencies include `EnhancedInput`, `GameplayTags`, `AIModule`, `StateTreeModule`, `GameplayStateTreeModule`, `UMG`, and `Slate` in `HorrorProject/HorrorProject/Source/HorrorProject/HorrorProject.Build.cs`.
- Core horror player stack exists under `HorrorProject/HorrorProject/Source/HorrorProject/Player/`.
- `AHorrorPlayerCharacter` owns `Interaction`, `Inventory`, `VHSEffect`, `NoteRecorder`, and `QuantumCamera` components.
- Input assets exist for horror semantics: `IA_Interact`, `IA_Flashlight`, `IA_OpenArchive`, `IA_Record`, `IA_Rewind`, and `IA_TakePhoto` under `HorrorProject/HorrorProject/Content/Input/Actions/`.
- Main candidate maps include DeepWaterStation maps under `HorrorProject/HorrorProject/Content/DeepWaterStation/Maps/`, Bodycam VHS maps under `HorrorProject/HorrorProject/Content/Bodycam_VHS_Effect/Maps/`, and `HorrorProject/HorrorProject/Content/Variant_Horror/Lvl_Horror.umap`.

## Build/Open Blockers

- 2026-04-25 validation: `HorrorProjectEditor Win64 Development` build completed with exit code 0 using UE 5.6 Build.bat.
- 2026-04-25 validation: `Automation RunTests HorrorProject` completed with exit code 0 under `UnrealEditor-Cmd.exe -nullrhi`.
- Project contains a project plugin at `HorrorProject/HorrorProject/Plugins/VisualStudioTools/`; compatibility with UE 5.6 should still be watched during editor-open validation, but it is not blocking the current commandlet build/test pass.

## Code Architecture Risks

- 2026-04-25: Day 1 EventBus v0 now publishes gameplay-tagged messages from GameMode objective transitions, retains broadcast source objects only for immediate delegates, stores history without retaining transient source objects, and guards automation-test history indexing.
- 2026-04-25: Day 1 Save v0 now requires a complete GameMode + player + inventory + note-recorder target set before snapshot/save/load, rejects unsupported save versions, and prevents partial saves / false-positive loads.
- 2026-04-25: Interaction progress bridge passed build and full `Automation RunTests HorrorProject` regression; successful objective interactions now populate saveable player inventory/note progress for Bodycam, FirstNote, and FirstAnomalyRecord while rejected interactions do not mutate progress.
- 2026-04-25: Existing `UHorrorUI` already observes `UInventoryComponent::OnEvidenceCollected` and `UNoteRecorderComponent::OnNoteRecorded`, refreshes archive snapshots, and exposes Blueprint events for archive/evidence/note feedback, so the next UI packet should focus on Blueprint/asset verification or metadata richness rather than duplicating delegate plumbing.
- 2026-04-25: Evidence/note metadata layer passed `HorrorProjectEditor Win64 Development`, targeted player automation, and full `Automation RunTests HorrorProject`; inventory and note components now preserve save-facing ID arrays while exposing transient runtime metadata queries for archive/UI enrichment. Objective interactables now register matching evidence/note metadata during successful progress writes before progress broadcasts, including first-anomaly candidate metadata before final recording stores the candidate evidence ID; rejected interactions do not register metadata. `UHorrorUI` now exposes an additive metadata-rich archive snapshot while preserving the legacy ID/count Blueprint event.
- 2026-04-25: ExitRouteGate objective interactable passed build, FoundFootage/UI/GameMode/EventBus targeted tests, and full `Automation RunTests HorrorProject`; route gate Blueprints can now query the found-footage exit unlock state through the same interactable contract without mutating inventory/note progress.
- 2026-04-25: VHS feedback bridge passed `HorrorProjectEditor Win64 Development`, targeted VHS/QuantumCamera/GameMode/EventBus/FoundFootage/Save tests, and full `Automation RunTests HorrorProject`; `UVHSEffectComponent` now exposes feedback-active state, feedback camera mode, and `OnVHSFeedbackChanged`, while `AHorrorPlayerCharacter` bridges `UQuantumCameraComponent::OnCameraModeChanged` into VHS feedback mode updates. Full-regression hardening also moved found-footage objective tags to native gameplay tags, guarded FirstAnomaly test indexing, fixed ExitRouteGate test preconditions, and made Save subsystem tests use the real GameInstance subsystem path.
- 2026-04-25: VHS UI feedback hook passed `HorrorProjectEditor Win64 Development`, targeted UI/VHS automation, and full `Automation RunTests HorrorProject`; `UHorrorUI` now observes the player `UVHSEffectComponent`, releases VHS delegates during teardown/character swaps, immediately refreshes current VHS feedback state after setup, and exposes `BP_VHSFeedbackChanged(bool, EQuantumCameraMode)` for Blueprint bodycam/VHS overlays.
- 2026-04-25: Save bodycam runtime sync passed `HorrorProjectEditor Win64 Development`, targeted Save/VHS automation, and full `Automation RunTests HorrorProject`; `AHorrorGameModeBase::ImportFoundFootageSaveState` now syncs restored BodycamAcquired state back to the lead player's `UVHSEffectComponent` and `UQuantumCameraComponent`, so load restores live VHS feedback and Viewfinder runtime state instead of only restoring objective tags.
- 2026-04-25: DeepWaterStation code-only route kit passed `HorrorProjectEditor Win64 Development`, targeted route-kit/found-footage automation, and full `Automation RunTests HorrorProject`; `ADeepWaterStationRouteKit` can now spawn configured `AFoundFootageObjectiveInteractable` nodes for Bodycam, FirstNote, FirstAnomalyCandidate, FirstAnomalyRecord, ArchiveReview, and ExitRouteGate without binary map edits, preserving metadata and objective preconditions for later Blueprint/map placement.
- 2026-04-25: P0/P1 architecture and interaction closeout passed `HorrorProjectEditor Win64 Development` plus full `Automation RunTests HorrorProject` across 22 tests; `FHorrorAnomalyDirector` is now the sole first-anomaly runtime under test, `ADeepWaterStationRouteKit::SpawnObjectiveNodes` is idempotent, and `UInteractionComponent::TryInteractWithHit` now has automation coverage for objective completion and rejected anomaly-record interactions.
- 2026-04-25 root cause: Unreal interface `Execute_*` dispatch for native C++ interactable implementations returned the interface default when a generated `UFunction` existed, so `UInteractionComponent::TryInvokeInteractableInterface` correctly uses the native interface path for native targets before Blueprint dispatch; automation now covers the route through `AFoundFootageObjectiveInteractable`.
- 2026-04-25: EventBus history capacity passed `HorrorProjectEditor Win64 Development`, targeted `HorrorProject.Game.EventBus`, and full `Automation RunTests HorrorProject` across 23 tests; `UHorrorEventBusSubsystem` now retains only the newest configured history messages and drops oldest entries first, reducing long-session memory growth risk.
- 2026-04-25: VHS post-process binding passed `HorrorProjectEditor Win64 Development`, targeted `HorrorProject.Player.VHS`, and full `Automation RunTests HorrorProject` across 24 tests; `UVHSEffectComponent` now exposes editor/Blueprint-assigned `VHSPostProcessMaterial` and `VHSPostProcessBlendWeight`, binds to the first-person camera during `AHorrorPlayerCharacter::BeginPlay`, and drives blendable weight from feedback active state without binary asset edits.
- 2026-04-25: VHS default asset resolve passed `HorrorProjectEditor Win64 Development`, targeted `HorrorProject.Player.VHS`, and full `Automation RunTests HorrorProject` across 26 tests; `UVHSEffectComponent::ResolveDefaultPostProcessMaterial` now loads `/Game/Bodycam_VHS_Effect/Materials/Instances/PostProcess/MI_OldVHS`, and `AHorrorPlayerCharacter::BeginPlay` resolves it before first-person camera binding so the primary VHS look no longer depends solely on manual Blueprint assignment.
- 2026-04-25: DeepWaterStation route kit spatial layout passed `HorrorProjectEditor Win64 Development`, targeted `HorrorProject.Game.DeepWaterStation.RouteKit`, and full `Automation RunTests HorrorProject` across 26 tests; `ADeepWaterStationRouteKit::ConfigureDefaultFirstLoopObjectiveNodes` now places the default first-loop objective sequence along a deterministic linear corridor spine without binary map edits, and automation verifies both configured relative transforms and spawned interactable world positions.
- 2026-04-25: Minimal single-threat AI foundation passed `HorrorProjectEditor Win64 Development`, targeted `HorrorProject.AI`, and full `Automation RunTests HorrorProject` across 26 tests; `AHorrorThreatCharacter` now provides a code-only active/inactive threat shell with `ThreatId`, `DetectionRadius`, and radius-based detection, while `AHorrorThreatAIController` exposes its possessed threat without BehaviorTree/StateTree/Blueprint asset dependencies.
- 2026-04-25: Safe stub cleanup passed `HorrorProjectEditor Win64 Development` and full `Automation RunTests HorrorProject` across 26 tests after deleting the empty `Variant_Horror/AHorrorGameMode` shell; reference audit found `AHorrorGameMode` only in its own header/cpp, while active config and runtime code use `AHorrorGameModeBase`.
- There are still template-style classes (`HorrorProjectCharacter`, `HorrorProjectGameMode`, `HorrorProjectPlayerController`) alongside SM-13-specific classes (`HorrorPlayerCharacter`, `HorrorGameModeBase`, `HorrorPlayerController`). Risk: template drift unless startup maps and Blueprint parents are aligned to the SM-13 stack; further cleanup needs asset-reference review.
- `DefaultGameplayTags.ini` contains mojibake in some Chinese comments, while later English tags are readable. Risk: documentation/readability issue, likely not runtime-breaking.
- `UInteractionComponent` still has legacy compatibility paths for old interact functions and door timelines. Risk: useful bridge for assets, but should not become permanent architecture sprawl.

## Asset and Level Risks

- LABYRINTH confirmed 26 `.umap` files under `HorrorProject/HorrorProject/Content/`.
- DeepWaterStation core maps are `HorrorProject/HorrorProject/Content/DeepWaterStation/Maps/Overview.umap`, `DemoMapScalabilityEpic.umap`, and `DemoMapScalabilityCinematic.umap`.
- DeepWaterStation contains modular station geometry under `Content/DeepWaterStation/Meshes/`, including `SM_Mod02Int`, `SM_Mod02OUT01`, `SM_ModA02`, `SM_ModA03`, `SM_ModA05`, `SM_FloorBase4x4`, `SM_FloorGrate01`, `SM_MetalDoor01`, `SM_MetalGate01`, `SM_Ladder01-03`, `SM_Hatch01`, `SM_Bridge01`, and `SM_Transition01`.
- DeepWaterStation atmosphere assets include `Content/DeepWaterStation/Niagara/NS_Dust`, `NS_Fish01-04`, `Content/DeepWaterStation/Particles/P_Ambient_Dust`, and materials for water, glass, emissive elements, and caustics.
- Strong route opportunity: build a linear undersea station route using modular corridor segments, gates, hatches, ladders, bridge pieces, terminals, screens, lab tables, oxygen tanks, and containers.
- Risk: DeepWaterStation does not appear to provide interaction Blueprints; interaction beats likely need project-side wrappers.
- Risk: collision, scale, navmesh behavior, Virtual Texture dependencies, and migrated sample-map stability still require in-editor verification.
- Multiple asset packs are present: DeepWaterStation, Bodycam_VHS_Effect, Fantastic_Dungeon_Pack, Scrapopolis, ForestOfSpikes, Stone_Golem, ModernBridges, and AdvancedUniversalSpawner. Risk: scope drift and content noise unless SM-13 ignores non-core packs by default.
- DeepWaterStation and Bodycam_VHS_Effect are the primary aligned asset roots; other packs should be considered non-core unless explicitly needed.

## AI and Threat Risks

- 2026-04-25 SPECTER recon: StateTree/GamePlayStateTree plugins and modules are enabled, and the first code-only AI foundation now exists as `AHorrorThreatCharacter` plus `AHorrorThreatAIController`. Remaining AI risk is asset/behavior integration: no BehaviorTree/StateTree assets, perception tuning, navmesh validation, or placed threat content have been verified yet.

## VHS, Audio, and Presentation Risks

- `UVHSEffectComponent` and `UQuantumCameraComponent` exist under `HorrorProject/HorrorProject/Source/HorrorProject/Player/Components/`.
- Bodycam VHS assets exist under `HorrorProject/HorrorProject/Content/Bodycam_VHS_Effect/`.
- MUSE confirmed core VHS post-process assets: `Content/Bodycam_VHS_Effect/Materials/Masters/PostProcess/M_OldVHS.uasset`, `Content/Bodycam_VHS_Effect/Materials/Masters/PostProcess/MI_OldVHS.uasset`, `Content/Bodycam_VHS_Effect/Textures/Default/T_VHS_LUT.uasset`, and `Content/Bodycam_VHS_Effect/Materials/Masters/PostProcess/M_CameraHole.uasset`.
- `VHSEffectComponent` now manages bodycam acquired/enabled state plus feedback-active and feedback camera mode state for Blueprint/audio/post-process hooks; `HorrorPlayerCharacter` bridges `QuantumCameraComponent` mode changes into VHS feedback updates.
- `QuantumCameraComponent` contains viewfinder, recording, photo, and rewind state logic with automation tests.
- Bodycam camera assets include `Content/Bodycam_VHS_Effect/Blueprints/BP_CameraComponent.uasset` and Idle/Walk/Run CameraShake assets.
- Player flashlight logic exists in `Source/HorrorProject/Variant_Horror/HorrorCharacter.cpp`; related flicker/light assets exist under `Content/Variant_Horror/Blueprints/Light/` and `Content/Bodycam_VHS_Effect/Materials/Masters/Lights/M_LightFlicker.uasset`.
- Confirmed audio assets include `Content/Bodycam_VHS_Effect/Sounds/S_VHS_Ambient.uasset`, `Content/Bodycam_VHS_Effect/Sounds/FootSteps/S_FootStep_Carpet_Cue.uasset`, and `Content/Bodycam_VHS_Effect/Sounds/Shouts/S_Shouts_Cue.uasset`.
- Risk: MetaSounds were not found; audio implementation may rely on SoundCue/simple assets.
- Risk: VHS integration point may live in Blueprint assets, requiring in-editor verification.
- 2026-04-25 MUSE recon: `Content/Variant_Horror/UI/UI_Horror.uasset` appears not yet to implement `BP_VHSFeedbackChanged`; `Content/Bodycam_VHS_Effect/Blueprints/Widgets/WBP_Screen_Blur.uasset` and post-process material instances are candidate assets for the next Blueprint-visible VHS packet.
- Risk: DX12/SM6, RayTracing, and Virtual Shadow settings may create performance pressure.
- Risk: `DefaultEngine.ini` contains AndroidFileServer SecurityToken with `bIncludeInShipping=False`; still requires security/config review.

## UI, Interaction, Evidence Risks

- `UInteractionComponent`, `UInventoryComponent`, `UNoteRecorderComponent`, `UHorrorUI`, and archive/evidence events exist in C++.
- `UHorrorUI` is implemented in `Source/HorrorProject/Variant_Horror/UI/HorrorUI.h/.cpp` and backed by `Content/Variant_Horror/UI/UI_Horror.uasset`.
- VHS widgets include `Content/Bodycam_VHS_Effect/UI/WBP_TimeStamp.uasset` and `Content/Bodycam_VHS_Effect/Widgets/WBP_Screen_Blur.uasset`.
- `UInteractionComponent` uses view trace/sweep, prioritizes `IInteractableInterface`, and includes legacy `Interact/Use/OpenDoor` and door timeline fallback paths.
- `UInventoryComponent` stores unique `FName` evidence IDs, supports SaveGame, and broadcasts `OnEvidenceCollected`.
- `UNoteRecorderComponent` stores unique note IDs, supports SaveGame, and broadcasts `OnNoteRecorded`.
- `UHorrorUI` listens to inventory, note recorder, and VHS feedback component events, then calls Blueprint events `BP_ArchiveSnapshotUpdated`, `BP_ArchiveMetadataSnapshotUpdated`, `BP_EvidenceCollected`, `BP_NoteRecorded`, and `BP_VHSFeedbackChanged`.
- `AHorrorPlayerCharacter::DoOpenArchive()` currently delegates to Blueprint via `BP_OpenArchive()`.
- `HorrorFoundFootageContract` manages Bodycam, FirstNote, FirstAnomaly, ArchiveReviewed, and ExitUnlocked objective state.
- Archive-related gameplay tags exist: `Event.Archive.Reviewed`, `State.Objective.ArchiveReviewed`, and evidence/note tags in `DefaultGameplayTags.ini`.
- Risk: dedicated Objective/Archive/Evidence assets were not found in first pass.
- Risk: C++ has no guaranteed `ArchiveReviewed` recording path if Blueprint open/archive flow does not call back into the contract.
- Risk: evidence and notes currently store IDs but no metadata/text content, so the evidence loop may need a small data table or asset-backed content layer.
- Risk: `DefaultGameplayTags.ini` contains mojibake comments in early lines.

## External Dependency Opportunities

Pending OPENWATER reconnaissance.

## Next Packets

1. Wait for FORGE, LABYRINTH, SPECTER, MUSE, WEAVER/HIVEMIND, and REAPER recon reports.
2. Verify editor/build command path for UE 5.6.
3. Decide first bounded execution packet from evidence: likely template-drift cleanup, build/open blocker fix, or SM-13 player/controller/map alignment.
4. Research mature Unreal references only where they reduce risk.
