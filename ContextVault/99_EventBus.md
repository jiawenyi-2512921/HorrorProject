# Event Bus

## Event Schema & SLA

Required fields for new events:

```yaml
- timestamp: YYYY-MM-DDTHH:MM:SS
  event: Event.<Domain>.<ProjectOrPacket>.<State>
  from: ROLE
  payload:
    subject: concise outcome
  subscribers_expected: [ROLE]
```

Naming:

- `Event.Packet.*.Started|Passed|Failed|Blocked` for execution packets.
- `Event.Validation.*.Passed|Failed` for build/test evidence.
- `Event.Decision.*.Approved|Rejected` for direction or architecture decisions.
- `Event.Risk.*.Raised|Mitigated` for RiskMatrix changes.
- `Event.Ops.*` for operating-model changes.

SLA:

- P0 build/test failure: classify before new implementation work in the same territory.
- P0 binary/editor blocker: record exact asset paths and required UE Editor action.
- Packet closeout: include validation evidence or explicit blocker.
- Retention: keep EventBus compact; long rationale belongs in DecisionLog or `07_Operations/*`.

```yaml
- timestamp: 2026-04-23T12:00:00
  event: Event.Arch.Proposal.Approved
  from: ORACLE
  payload:
    subject: Fleet OS bootstrap
    adr: ADR-000
    vault_ref: 00_Admiral/DecisionLog.md
  subscribers_expected: [ADMIRAL, FORGE, REAPER]

- timestamp: 2026-04-23T12:05:00
  event: Event.Milestone.M0.Reached
  from: ADMIRAL
  payload:
    subject: Workspace scaffolding established
    scope:
      - .fleet
      - ContextVault
      - docs
  subscribers_expected: [ORACLE, FORGE, WEAVER, HIVEMIND, LABYRINTH, SPECTER, MUSE, REAPER]

- timestamp: 2026-04-24T09:30:00
  event: Event.Direction.SM13.Locked
  from: ADMIRAL
  payload:
    subject: DeepWaterStation VHS horror production direction approved
    adr: ADR-003
    docs:
      - docs/superpowers/specs/2026-04-24-sm13-production-design.md
      - docs/superpowers/specs/2026-04-24-sm13-execution-plan.md
  subscribers_expected: [DISPATCHER, CURATOR, ORACLE, FORGE, WEAVER, HIVEMIND, MUSE, REAPER]

- timestamp: 2026-04-24T09:35:00
  event: Event.Schedule.HardLock.Set
  from: ADMIRAL
  payload:
    subject: Hard M3 remains 2026-05-14 with contingency through 2026-05-17
    adr: ADR-004
  subscribers_expected: [WATCHDOG, ORACLE, REAPER, MUSE]

- timestamp: 2026-04-25T00:00:00
  event: Event.Ops.PermissionMode.DefaultAllowSet
  from: ADMIRAL
  payload:
    subject: User approved uninterrupted 24h agent-cluster development; project-local Claude Code permissions set to default dontAsk
    settings_ref: .claude/settings.local.json
    guardrail: destructive or external shared-state actions still require policy-level caution
  subscribers_expected: [WATCHDOG, DISPATCHER, FORGE, REAPER]

- timestamp: 2026-04-25T00:05:00
  event: Event.Packet.SM13.Day1Infra.CloseoutStarted
  from: ADMIRAL
  payload:
    subject: EventBus and Save v0 quality closeout in progress
    focus:
      - EventBus history object-lifetime safety
      - guarded EventBus automation test indexing
      - SaveCheckpoint partial-save prevention
      - LoadCheckpoint restore-recipient validation
      - real subsystem-path save tests
  subscribers_expected: [FORGE, REAPER, WATCHDOG, CURATOR]

- timestamp: 2026-04-25T00:20:00
  event: Event.Validation.SM13.Day1Infra.Passed
  from: REAPER
  payload:
    subject: EventBus and Save v0 closeout passed build and automation
    build: HorrorProjectEditor Win64 Development exit 0
    tests: Automation RunTests HorrorProject exit 0
    fixes:
      - EventBus history no longer retains transient source objects
      - EventBus objective sequence tests guard history indexing
      - Save snapshots require complete Day 1 restore targets
      - Save restore rejects unsupported save versions
      - Save subsystem tests cover preflight and snapshot round-trip
  subscribers_expected: [ADMIRAL, WATCHDOG, FORGE, CURATOR]
- timestamp: 2026-04-25T00:35:00
  event: Event.Packet.SM13.InteractionProgressBridge.Started
  from: FORGE
  payload:
    subject: Objective interactables now bridge successful player interactions into saveable inventory/note progress
    scope:
      - FoundFootageObjectiveInteractable writes Bodycam and FirstAnomalyRecord evidence IDs to instigator inventory
      - FoundFootageObjectiveInteractable writes FirstNote IDs to instigator note recorder
      - Rejected interactions do not mutate player progress
    validation_pending:
      - HorrorProjectEditor build
      - HorrorProject.Game.FoundFootage automation tests
  subscribers_expected: [REAPER, WATCHDOG, CURATOR]
- timestamp: 2026-04-25T00:45:00
  event: Event.Validation.SM13.InteractionProgressBridge.Passed
  from: REAPER
  payload:
    subject: Interaction progress bridge passed build and targeted automation
    build: HorrorProjectEditor Win64 Development exit 0
    tests: HorrorProject.Game.FoundFootage + HorrorProject.Player + HorrorProject.Save exit 0
  subscribers_expected: [ADMIRAL, WATCHDOG, FORGE, CURATOR]
- timestamp: 2026-04-25T00:50:00
  event: Event.Validation.SM13.FullRegression.Passed
  from: REAPER
  payload:
    subject: Full HorrorProject automation regression passed after EventBus, Save, and interaction progress bridge changes
    tests: Automation RunTests HorrorProject exit 0
  subscribers_expected: [ADMIRAL, WATCHDOG, FORGE, CURATOR]
- timestamp: 2026-04-25T01:05:00
  event: Event.Validation.SM13.MetadataLayer.Passed
  from: REAPER
  payload:
    subject: Evidence and note metadata layer passed build, targeted automation, and full regression
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Player.Inventory exit 0
      - HorrorProject.Player.NoteRecorder exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - Inventory evidence metadata registered and queried by evidence id
      - Note metadata registered and queried by note id
      - Collected/recorded metadata views only include registered collected IDs
      - Save-facing evidence/note ID arrays and acquisition order remain unchanged
  subscribers_expected: [ADMIRAL, WATCHDOG, FORGE, CURATOR]
- timestamp: 2026-04-25T01:20:00
  event: Event.Validation.SM13.MetadataBridge.Passed
  from: REAPER
  payload:
    subject: Objective interactables register runtime evidence/note metadata for successful player progress writes
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests: HorrorProject.Game.FoundFootage exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - Bodycam interactions register matching evidence metadata on inventory
      - First note interactions register matching note metadata on note recorder
      - First anomaly candidates register metadata before the later recording stores the candidate evidence id
      - Rejected anomaly recordings do not write evidence or metadata
  subscribers_expected: [ADMIRAL, WATCHDOG, FORGE, CURATOR]
- timestamp: 2026-04-25T01:35:00
  event: Event.Validation.SM13.ArchiveUIMetadata.Passed
  from: REAPER
  payload:
    subject: Archive UI snapshot now exposes metadata-rich evidence and note rows while preserving the legacy ID/count Blueprint event
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.UI.Archive exit 0
      - HorrorProject.Game.FoundFootage exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - FHorrorArchiveSnapshot carries evidence IDs, note IDs, counts, and registered metadata arrays
      - UHorrorUI keeps BP_ArchiveSnapshotUpdated for existing Blueprint compatibility
      - UHorrorUI adds BP_ArchiveMetadataSnapshotUpdated for archive/evidence UI enrichment
      - Objective metadata registration now occurs before evidence/note progress broadcasts
  subscribers_expected: [ADMIRAL, WATCHDOG, FORGE, CURATOR]
- timestamp: 2026-04-25T01:50:00
  event: Event.Validation.SM13.ExitRouteGate.Passed
  from: REAPER
  payload:
    subject: ExitRouteGate objective interactable gives Blueprint/placeable route gates an exit-unlocked precondition
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.FoundFootage exit 0
      - HorrorProject.UI.Archive exit 0
      - HorrorProject.Game.GameModeBase + HorrorProject.Game.EventBus exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - ExitRouteGate stays locked before first-loop objectives complete
      - ExitRouteGate unlocks after the found-footage contract unlocks the exit
      - ExitRouteGate completes without writing inventory or note progress
      - Existing objective interactable behavior remains covered by FoundFootage regression
  subscribers_expected: [ADMIRAL, WATCHDOG, FORGE, CURATOR]
- timestamp: 2026-04-25T03:55:00
  event: Event.Validation.SM13.VHSFeedbackBridge.Passed
  from: REAPER
  payload:
    subject: VHS feedback bridge now exposes bodycam-active feedback state and follows QuantumCamera mode changes
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Player.VHS exit 0
      - HorrorProject.Player.QuantumCamera exit 0
      - HorrorProject.Game.GameModeBase exit 0
      - HorrorProject.Game.EventBus exit 0 after native found-footage gameplay tags
      - HorrorProject.Game.FoundFootage.FirstAnomaly exit 0
      - HorrorProject.Game.FoundFootage.ObjectiveInteractable.WritesInstigatorProgress exit 0
      - HorrorProject.Save.Subsystem exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - UVHSEffectComponent exposes feedback active state, feedback camera mode, and OnVHSFeedbackChanged for Blueprint/audio/post-process hooks
      - VHS feedback activates on enabled bodycam, starts in Viewfinder, follows Recording/Photo/Rewind style QuantumCamera modes, and resets to Disabled when bodycam is lost
      - AHorrorPlayerCharacter bridges QuantumCamera OnCameraModeChanged into VHSEffect feedback mode updates
      - SetFeedbackCameraMode now returns false for no-op duplicate mode requests, matching existing component setter semantics
      - Found-footage objective tags are native gameplay tags so commandlet automation no longer depends on config tag load timing
      - Regression tests now avoid invalid subsystem construction and verify exit gate completion only after archive review unlocks the route
  subscribers_expected: [ADMIRAL, WATCHDOG, FORGE, CURATOR]
- timestamp: 2026-04-25T04:20:00
  event: Event.Validation.SM13.VHSUIFeedbackHook.Passed
  from: REAPER
  payload:
    subject: Horror UI now exposes VHS feedback state changes to Blueprint widgets
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.UI exit 0
      - HorrorProject.Player.VHS exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - UHorrorUI observes the player VHSEffect component alongside inventory and note recorder components
      - UHorrorUI now removes VHS feedback delegate bindings during teardown or character swaps
      - BP_VHSFeedbackChanged gives UI Blueprints an active flag plus QuantumCamera mode for bodycam/VHS overlays
      - RefreshVHSFeedbackState pushes the current state immediately after SetupCharacter and falls back to inactive/Disabled without an observed VHS component
  subscribers_expected: [ADMIRAL, WATCHDOG, FORGE, CURATOR]

- timestamp: 2026-04-26T00:45:00
  event: Event.Ops.SM13.AgentCompanyOS.Expanded
  from: CURATOR
  payload:
    subject: ContextVault upgraded into explicit high-concurrency agent-company operating system
    docs:
      - ContextVault/00_Admiral/OrgChartAndRACI.md
      - ContextVault/07_Operations/AgentCompanyOS.md
      - ContextVault/07_Operations/DispatchProtocol.md
      - ContextVault/07_Operations/ParallelExecutionLanes.md
      - ContextVault/07_Operations/QualityGateProtocol.md
      - ContextVault/07_Operations/MissionHandoffPacket.md
      - ContextVault/07_Operations/ConcurrencyBoard.md
      - ContextVault/07_Operations/IncidentPlaybook.md
    policies:
      - one writer per file territory
      - high-concurrency read-only reconnaissance encouraged
      - G0-G5 quality gates required before packet closeout
      - binary assets remain UE Editor-owned
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, REAPER, CURATOR]

- timestamp: 2026-04-26T01:20:00
  event: Event.Validation.SM13.ObjectiveOrderRouteGate.Passed
  from: REAPER
  payload:
    subject: Objective order and route-owned encounter exit gating passed build, focused automation, and full regression
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.FoundFootage.ContractTracksMilestones exit 0
      - HorrorProject.Game.GameModeBase.ObjectiveBridgeCompletesFirstLoop exit 0
      - HorrorProject.Game.FoundFootage.ObjectiveInteractable exit 0
      - HorrorProject.Game.DeepWaterStation.RouteKit.SpawnsObjectiveNodes exit 0
      - HorrorProject.Game.FoundFootage.FirstAnomaly.RequiresRecording exit 0
      - HorrorProject.Player.Interaction exit 0
      - HorrorProject.Player.Input.DoesNotBypassRouteObjectives exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - AHorrorGameModeBase exposes and enforces first-loop objective prerequisites
      - FHorrorFoundFootageContract rejects direct out-of-order objective event recording
      - FHorrorAnomalyDirector rejects first-anomaly candidates before the first note is collected
      - AFoundFootageObjectiveInteractable Can/Try paths share prerequisite gates
      - route-owned ExitRouteGate blocks while ADeepWaterStationRouteKit reports an active encounter gate
      - route kit validation rejects out-of-order first-loop objective nodes
    next_candidates:
      - encounter director authored threat spawn bridge
      - native archive review evidence loop
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, CURATOR]

- timestamp: 2026-04-26T01:40:00
  event: Event.Validation.SM13.EncounterThreatSpawnBridge.Passed
  from: REAPER
  payload:
    subject: Encounter director can author and spawn the reveal threat without manual preassignment
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.Encounter.Director exit 0
      - HorrorProject.Game.DeepWaterStation.RouteKit exit 0
      - HorrorProject.AI.Threat exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - AHorrorEncounterDirector exposes ThreatClass and ThreatRelativeTransform authoring fields
      - AHorrorEncounterDirector spawns and reuses an authored AHorrorThreatCharacter on reveal when ThreatActor is not preassigned
      - spawned reveal threats activate, detect the reveal target, deactivate on resolve, and remain route-gated through the encounter phase
      - ADeepWaterStationRouteKit passes encounter threat authoring defaults into spawned encounter directors
      - AHorrorEncounterDirector and ADeepWaterStationRouteKit now have stable scene roots for transform-authored placement
    next_candidates:
      - native archive review evidence loop
      - UE Editor binary redirector cleanup
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, CURATOR]

- timestamp: 2026-04-25T04:40:00
  event: Event.Validation.SM13.SaveBodycamRuntimeSync.Passed
  from: REAPER
  payload:
    subject: Save/load restore now rehydrates live bodycam, VHS feedback, and QuantumCamera runtime state
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Save.Subsystem exit 0
      - HorrorProject.Player.VHS exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - AHorrorGameModeBase::ImportFoundFootageSaveState now syncs restored found-footage state back onto the lead player
      - Restored BodycamAcquired state re-acquires and enables UVHSEffectComponent feedback for UI/Blueprint hooks
      - Restored BodycamAcquired state re-acquires and enables UQuantumCameraComponent in Viewfinder mode
      - Save subsystem automation now proves runtime VHS and QuantumCamera state clear before restore and reactivate after snapshot application
    recon:
      - MUSE recommends Blueprint UI_Horror implementation of BP_VHSFeedbackChanged before camera post-process C++ hooks
      - LABYRINTH recommends a code-only route kit of placeable objective presets for DeepWaterStation route assembly
      - SPECTER confirms single-threat AI source/assets are absent; next AI packet should add minimal threat pawn/controller plus later StateTree/Blueprint/map assets
  subscribers_expected: [ADMIRAL, WATCHDOG, FORGE, CURATOR]
- timestamp: 2026-04-25T05:05:00
  event: Event.Validation.SM13.DeepWaterStationRouteKit.Passed
  from: REAPER
  payload:
    subject: Code-only DeepWaterStation route kit can spawn Day 1 objective interactable nodes without binary map edits
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.DeepWaterStation.RouteKit exit 0
      - HorrorProject.Game.FoundFootage exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - ADeepWaterStationRouteKit owns editable route objective nodes and spawns AFoundFootageObjectiveInteractable instances at runtime
      - Route nodes preserve objective type, source id, bodycam enable flag, anomaly recording flag, evidence metadata, and note metadata
      - Route automation covers the first-loop sequence through Bodycam, FirstNote, FirstAnomalyCandidate, FirstAnomalyRecord, ArchiveReview, and ExitRouteGate
      - Packet stays code-only so later Blueprint/map placement can bind DeepWaterStation meshes without text-editing binary assets
    next_candidates:
      - minimal single-threat AI foundation
      - Blueprint-visible VHS asset binding in UI_Horror
  subscribers_expected: [ADMIRAL, WATCHDOG, LABYRINTH, REAPER]
- timestamp: 2026-04-25T05:40:00
  event: Event.Validation.SM13.P0P1ArchitectureInteraction.Passed
  from: REAPER
  payload:
    subject: P0/P1 anomaly architecture, route idempotency, and interaction automation closeout passed full regression
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.FoundFootage.FirstAnomaly exit 0
      - HorrorProject.Game.DeepWaterStation.RouteKit exit 0
      - HorrorProject.Player.Interaction exit 0
    full_regression: Automation RunTests HorrorProject exit 0; 22 tests found and passed
    scope:
      - FHorrorAnomalyDirector is the tested authoritative first-anomaly state machine after removing duplicate runtime coverage
      - ADeepWaterStationRouteKit duplicate SpawnObjectiveNodes calls no-op after initial spawn
      - UInteractionComponent TryInteractWithHit covers objective completion and missing-candidate rejection in automation
      - Root cause isolated to native interface dispatch behavior; native interactable path is now covered through UInteractionComponent
    next_candidates:
      - delete safe empty Variant_Horror AHorrorGameMode shell
      - implement Blueprint-visible VHS post-process material binding
      - add EventBus history capacity limit
  subscribers_expected: [ADMIRAL, WATCHDOG, FORGE, REAPER]
- timestamp: 2026-04-25T06:00:00
  event: Event.Validation.SM13.EventBusCapacity.Passed
  from: REAPER
  payload:
    subject: EventBus history now has bounded retention with oldest-entry eviction
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.EventBus exit 0
    full_regression: Automation RunTests HorrorProject exit 0; 23 tests found and passed
    scope:
      - UHorrorEventBusSubsystem exposes editable HistoryCapacity with minimum clamp
      - Publish retains only the newest HistoryCapacity messages and drops oldest entries first
      - Dev automation hook SetHistoryCapacityForTests verifies capacity-two eviction behavior
    recon:
      - VHS post-process binding should add editor-visible material/weight properties to UVHSEffectComponent and apply them to the first-person camera
      - Minimal threat AI foundation has no existing AI source; smallest code-only packet is AHorrorThreatCharacter plus AHorrorThreatAIController and tests
  subscribers_expected: [ADMIRAL, WATCHDOG, FORGE, REAPER]
- timestamp: 2026-04-25T09:20:00
  event: Event.Validation.SM13.VHSPostProcessBinding.Passed
  from: REAPER
  payload:
    subject: VHS feedback can now drive an editor-assigned post-process material on the first-person camera
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Player.VHS exit 0
    full_regression: Automation RunTests HorrorProject exit 0; 24 tests found and passed
    scope:
      - UVHSEffectComponent exposes VHSPostProcessMaterial and VHSPostProcessBlendWeight for Blueprint/editor assignment
      - UVHSEffectComponent can bind a UCameraComponent and refresh its post-process blendable weight from feedback active state
      - AHorrorPlayerCharacter binds VHSEffect to GetFirstPersonCameraComponent during BeginPlay
      - Automation proves inactive feedback binds zero blend weight and active feedback applies the configured weight
    next_candidates:
      - minimal single-threat AI foundation
      - delete safe empty Variant_Horror AHorrorGameMode shell after asset-reference risk review
  subscribers_expected: [ADMIRAL, WATCHDOG, MUSE, FORGE, REAPER]
- timestamp: 2026-04-25T10:10:00
  event: Event.Validation.SM13.MinimalThreatAI.Passed
  from: REAPER
  payload:
    subject: Minimal single-threat AI foundation passed build, targeted automation, and full regression
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.AI exit 0
    full_regression: Automation RunTests HorrorProject exit 0; 26 tests found and passed
    scope:
      - AHorrorThreatCharacter provides a code-only active/inactive threat shell with ThreatId, DetectionRadius, and CanDetectActor
      - AHorrorThreatAIController exposes the currently possessed AHorrorThreatCharacter without BehaviorTree, StateTree, or Blueprint asset dependencies
      - Automation covers threat activation, duplicate activation rejection, radius-based detection, deactivation, and controller possession/unpossession
    root_cause:
      - Early detection test failures came from transient actor placement/collision adjustment in the test world; AlwaysSpawn threat actors with explicit spawn transforms made the radius assertions deterministic
    next_candidates:
      - delete safe empty Variant_Horror AHorrorGameMode shell after asset-reference risk review
      - Blueprint/map placement pass for DeepWaterStation route kit and VHS post-process asset assignment
  subscribers_expected: [ADMIRAL, WATCHDOG, SPECTER, FORGE, REAPER]
- timestamp: 2026-04-25T10:15:00
  event: Event.Validation.SM13.SafeStubCleanup.Passed
  from: REAPER
  payload:
    subject: Empty Variant_Horror AHorrorGameMode shell removed after reference audit
    build: HorrorProjectEditor Win64 Development exit 0
    full_regression: Automation RunTests HorrorProject exit 0; 26 tests found and passed
    scope:
      - Removed Source/HorrorProject/Variant_Horror/HorrorGameMode.h
      - Removed Source/HorrorProject/Variant_Horror/HorrorGameMode.cpp
      - Reference audit found AHorrorGameMode only in its own shell; active config and code use AHorrorGameModeBase instead
    next_candidates:
      - Blueprint/map placement pass for DeepWaterStation route kit and VHS post-process asset assignment
      - further template class cleanup only after asset-reference review
  subscribers_expected: [ADMIRAL, WATCHDOG, FORGE, REAPER]
- timestamp: 2026-04-25T10:25:00
  event: Event.Validation.SM13.VHSDefaultAssetResolve.Passed
  from: REAPER
  payload:
    subject: VHS post-process material now resolves the project default MI_OldVHS asset in code
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Player.VHS exit 0
    full_regression: Automation RunTests HorrorProject exit 0; 26 tests found and passed
    scope:
      - UVHSEffectComponent exposes ResolveDefaultPostProcessMaterial for Blueprint/code callers
      - AHorrorPlayerCharacter resolves /Game/Bodycam_VHS_Effect/Materials/Instances/PostProcess/MI_OldVHS before binding the first-person camera
      - VHS automation verifies default asset resolution, duplicate-resolution no-op behavior, and feedback-driven blend weight remains intact
  subscribers_expected: [ADMIRAL, WATCHDOG, MUSE, FORGE, REAPER]
- timestamp: 2026-04-25T11:25:00
  event: Event.Validation.SM13.RouteKitSpatialLayout.Passed
  from: REAPER
  payload:
    subject: DeepWaterStation route kit default first-loop nodes now have deterministic corridor placements
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.DeepWaterStation.RouteKit exit 0
    full_regression: Automation RunTests HorrorProject exit 0; 26 tests found and passed
    scope:
      - ConfigureDefaultFirstLoopObjectiveNodes assigns Bodycam, FirstNote, FirstAnomalyCandidate, FirstAnomalyRecord, ArchiveReview, and ExitRouteGate along a linear X-axis route
      - Route kit automation verifies configured relative transforms and spawned interactable world locations
      - Packet stays code-only and avoids binary map edits while giving Blueprint/map placement a deterministic Day 1 spine
  subscribers_expected: [ADMIRAL, WATCHDOG, LABYRINTH, FORGE, REAPER]
- timestamp: 2026-04-26T02:10:00
  event: Event.Validation.SM13.NativeArchiveReviewEvidenceLoop.Passed
  from: REAPER
  payload:
    subject: Archive review is now a native evidence-loop gameplay beat instead of only a BP/UI opening
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.FoundFootage.ObjectiveInteractable exit 0
      - HorrorProject.UI.Archive.SnapshotIncludesMetadata exit 0
      - HorrorProject.Game.GameModeBase.ObjectiveBridgeCompletesFirstLoop exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - AFoundFootageObjectiveInteractable routes world interaction through instigator-aware completion checks
      - ArchiveReview requires objective readiness plus native player evidence and note progress when invoked by a player instigator
      - ArchiveReview writes its own evidence id and metadata to the player inventory on successful interaction
      - Existing direct objective APIs remain usable for low-level GameMode/contract sequencing tests
      - ExitRouteGate remains progress-neutral after archive review unlocks the first-loop exit
    next_candidates:
      - UE Editor binary redirector cleanup after source/content reference audit
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, CURATOR]
- timestamp: 2026-04-26T02:18:00
  event: Event.Audit.SM13.FirstPersonRedirectorCleanup.ReadyForEditor
  from: ORACLE
  payload:
    subject: FirstPerson redirector cleanup has a read-only handoff and is blocked only on UE Editor-owned asset operations
    findings:
      - `Content/FirstPerson` is absent on disk
      - no plain-string `.uasset` or `.umap` references found for `/Game/FirstPerson`, `ABP_FP_Copy`, `CtrlRig_FPWarp`, or `MI_FirstPersonColorway`
      - `Config/DefaultEditorPerProjectUserSettings.ini` still has `ContentBrowserTab1.SelectedPaths=/Game/FirstPerson` and can be text-cleaned after editor verification
      - `Config/DefaultEngine.ini` still has `TP_FirstPerson` game/class redirects; keep until editor resave proves no old serialized class references remain
      - Bodycam demo `Content/Bodycam_VHS_Effect/DEMO/Blueprints/BP_FirstPersonCharacter.uasset` is not the deleted `/Game/FirstPerson` folder and should be verified before any demo-content cleanup
    editor_steps:
      - open HorrorProject.uproject in UE Editor
      - enable Show Redirectors and run Fix Up Redirectors on `/Game` plus active gameplay folders
      - open and resave `Content/Variant_Horror/Lvl_Horror.umap`, `Content/Horror/BP_HorrorGameModeBase.uasset`, active Horror character/controller Blueprints if present, and kept Bodycam demo Blueprint if referenced
      - use Reference Viewer for `/Game/FirstPerson`, `ABP_FP_Copy`, `CtrlRig_FPWarp`, `MI_FirstPersonColorway`, `TP_FirstPerson`, and `BP_FirstPersonCharacter`
    next_candidates:
      - editor-owned redirector fix-up/resave pass
      - code-only gameplay polish outside binary asset territory while waiting for editor action
	  subscribers_expected: [ADMIRAL, LABYRINTH, USER, WATCHDOG, CURATOR]
- timestamp: 2026-04-26T02:45:00
  event: Event.Validation.SM13.ObjectiveEncounterBridge.Passed
  from: REAPER
  payload:
    subject: Route-owned archive review now triggers the golem reveal and the exit gate resolves the active encounter
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.FoundFootage.ObjectiveInteractable exit 0
      - HorrorProject.Game.DeepWaterStation.RouteKit exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    independent_audit:
      - bridge audit completed after first validation pass
      - fixed route-owned ArchiveReview precondition so it rejects player interactions that cannot trigger the encounter reveal
      - added coverage that failed reveal leaves the encounter primed and does not complete the archive objective
    scope:
      - AFoundFootageObjectiveInteractable notifies an owning ADeepWaterStationRouteKit after successful world interaction
      - ADeepWaterStationRouteKit handles ArchiveReview by triggering the authored encounter reveal
      - ADeepWaterStationRouteKit handles ExitRouteGate by resolving the active route-gating encounter
      - ExitRouteGate remains progress-neutral and leaves the first-loop exit unlocked after encounter resolution
      - Direct low-level objective APIs remain available for headless objective sequencing without route-kit side effects
    next_candidates:
      - promote route objective hints and trailer beat ids into runtime UI/event messages
      - tighten archive review to authored first-loop evidence/note ids
      - add checkpoint-load UI state restamp without replaying live objective events
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, MUSE, CURATOR]

- timestamp: 2026-04-26T03:20:00
  event: Event.Validation.SM13.RouteBeatMetadataArchiveStrictness.Passed
  from: REAPER
  payload:
    subject: Route-authored objective metadata now reaches runtime objective events and archive review requires authored first-loop ids
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.DeepWaterStation.RouteKit exit 0
      - HorrorProject.Game.FoundFootage.ObjectiveInteractable exit 0
      - HorrorProject.Game.EventBus exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    audit:
      - independent archive strictness audit found direct no-instigator ArchiveReview completion bypass
      - fixed CanCompleteObjective/TryCompleteObjective direct path to reject ArchiveReview without an instigator
      - updated route and objective automation so the bypass stays closed
    scope:
      - route objective nodes copy TrailerBeatId, ObjectiveHint, DebugLabel, evidence metadata, and note metadata into spawned interactables
      - UHorrorEventBusSubsystem enriches published messages from event-and-source metadata before falling back to source-only metadata
      - UHorrorUI preserves the legacy objective toast hook and adds a metadata-rich toast hook for Blueprint widgets
      - ArchiveReview rejects players missing Evidence.Bodycam, Evidence.Anomaly01, or Note.Intro
      - first anomaly recording registers metadata against the pending anomaly source id before the GameMode publishes the event
    next_candidates:
      - checkpoint-load UI state restamp without replaying live objective events
      - UE Editor binary redirector cleanup remains editor-owned
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, MUSE, CURATOR]

- timestamp: 2026-04-26T03:45:00
  event: Event.Validation.SM13.CheckpointUIRestamp.Passed
  from: REAPER
  payload:
    subject: Checkpoint restore refreshes state-derived UI without replaying objective events
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Save.Subsystem exit 0
      - HorrorProject.UI exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - UHorrorUI exposes RestampCheckpointLoadedState for archive and VHS state refresh after restore
      - AHorrorPlayerController forwards checkpoint restamps to an already-created HorrorUI widget without creating UI from the save subsystem
      - UHorrorSaveSubsystem applies found-footage, evidence, and note imports silently, then requests UI restamp
      - save subsystem regression proves ApplyCheckpointSnapshot does not mutate EventBus history or rebroadcast objective events
    next_candidates:
      - continue code-only gameplay polish outside UE Editor binary asset territory
      - UE Editor binary redirector cleanup remains editor-owned
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, MUSE, CURATOR]

- timestamp: 2026-04-26T08:58:00
  event: Event.Validation.SM13.EventBusMetadataPrecedence.Passed
  from: REAPER
  payload:
    subject: EventBus metadata precedence is locked for event-and-source overrides before source fallback
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.EventBus exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - EventBus direct publish coverage now registers source fallback and event-specific metadata for the same source id
      - matching event-and-source metadata overrides source fallback
      - non-matching event metadata continues to use source fallback
      - route objective beat metadata path is protected from future source-id collision regressions
    next_candidates:
      - route validation hardening for authored Day-1 nodes
      - UE Editor binary redirector cleanup remains editor-owned
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, MUSE, CURATOR]

- timestamp: 2026-04-26T09:05:00
  event: Event.Validation.SM13.RouteValidationHardening.Passed
  from: REAPER
  payload:
    subject: Authored Day-1 route validation now rejects missing or duplicate beat/UI metadata
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.DeepWaterStation.RouteKit exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - ValidateObjectiveNodes rejects missing TrailerBeatId on every route node
      - ValidateObjectiveNodes rejects duplicate TrailerBeatId values before runtime interactable spawn
      - ValidateObjectiveNodes rejects missing ObjectiveHint and DebugLabel values needed by UI/event metadata
      - RouteKit automation now mutates archive and exit nodes to prove these authoring regressions are caught
    next_candidates:
      - continue code-only gameplay polish outside UE Editor binary asset territory
      - UE Editor binary redirector cleanup remains editor-owned
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, MUSE, CURATOR]

- timestamp: 2026-04-26T09:35:00
  event: Event.Validation.SM13.CameraVHSCoherence.Passed
  from: REAPER
  payload:
    subject: Quantum camera and VHS feedback state now stay coherent across availability drops and input mode changes
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Player.QuantumCamera exit 0
      - HorrorProject.Player.VHS exit 0
      - HorrorProject.Player.Input exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - QuantumCamera automation covers photo rejection while unavailable, disable while recording, acquisition loss while rewinding, no-op stop calls after forced disable, and reacquire/re-enable returning to Viewfinder
      - VHS automation covers stale Recording/Rewind rejection while disabled, disable/acquisition-loss reset to Disabled, re-enable returning to Viewfinder, and post-process blend weight clearing on acquisition loss
      - AHorrorPlayerCharacter mirrors successful record/photo/rewind camera mode changes into VHS feedback during native input handlers
      - player input automation now verifies record and rewind input propagate VHS feedback modes while still not bypassing objective completion
    next_candidates:
      - interaction fallback precision and safety packet
      - encounter/threat lifecycle invariant packet
      - UE Editor binary redirector cleanup remains editor-owned
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, MUSE, CURATOR]

- timestamp: 2026-04-26T10:02:00
  event: Event.Validation.SM13.InteractionFallbackHardening.Passed
  from: REAPER
  payload:
    subject: Interaction fallback routing is safer and sweep selection is more precise
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Player.Interaction exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - interaction interface routing now preserves native C++ implementer compatibility while allowing Blueprint-scripted interface overrides to dispatch through Execute helpers
      - legacy Door Control timeline fallback now defaults off and must be explicitly enabled before compatibility playback can occur
      - TryPlayDoorTimeline performs its own single lookup rather than pre-scanning twice
      - narrow sweep fallback scores candidates by perpendicular distance to the trace before forward distance, preferring centered targets over closer off-axis hits
      - interaction automation now covers Execute-compatible native objective routing, opt-in legacy timeline playback, unrelated timeline rejection, and centerline sweep scoring
    next_candidates:
      - encounter/threat lifecycle invariant packet
      - component import silent/deterministic packet
      - EventBus empty metadata registration guardrail packet
      - UE Editor binary redirector cleanup remains editor-owned
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, MUSE, CURATOR]

- timestamp: 2026-04-26T10:30:00
  event: Event.Validation.SM13.EncounterLifecycleInvariants.Passed
  from: REAPER
  payload:
    subject: Encounter director lifecycle edge semantics are locked by automation
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.Encounter.Director exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - threatless primed encounters can reveal, gate, remember target, and resolve without spawning a threat actor
      - positive reveal radius rejects out-of-range targets while zero or negative reveal radius remains unlimited
      - threat detection radius can reject a reveal target without blocking encounter reveal or threat activation
      - reset deactivates and clears detected target while preserving spawned threat actor for the next reveal
      - bGateRouteDuringReveal=false allows revealed encounters to avoid route gating while still activating and resolving threats
    validation_notes:
      - fixed pointer comparison assertions in new automation coverage
      - rebuilt after radius semantics edit so focused suite ran against the current test DLL
    next_candidates:
      - component import silent/deterministic packet
      - EventBus empty metadata registration guardrail packet
      - UE Editor binary redirector cleanup remains editor-owned
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, MUSE, CURATOR]

- timestamp: 2026-04-26T10:48:00
  event: Event.Validation.SM13.SilentImportInvariants.Passed
  from: REAPER
  payload:
    subject: Save/import state application is deterministic and does not replay gameplay side effects
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Player.Inventory exit 0
      - HorrorProject.Player.NoteRecorder exit 0
      - HorrorProject.Save.Subsystem exit 0
    full_regression: Automation RunTests HorrorProject found 41 tests exit 0
    scope:
      - Inventory import rejects NAME_None, de-dupes IDs, preserves first-seen order, replaces prior state, and does not broadcast OnEvidenceCollected
      - Note recorder import rejects NAME_None, de-dupes IDs, preserves first-seen order, replaces prior state, and does not broadcast OnNoteRecorded
      - ApplyCheckpointSnapshot imports found-footage, evidence, and note state without mutating EventBus history or rebroadcasting objective events
      - ApplyCheckpointSnapshot also avoids evidence/note delegate broadcasts while still restoring bodycam, archive, evidence, and note state
    validation_notes:
      - dynamic multicast delegate probes now use UHT-generated UObject/UFUNCTION receivers with AddDynamic
      - UHT requires reflected probe classes outside generic preprocessor blocks
    next_candidates:
      - EventBus empty metadata registration guardrail packet
      - UE Editor binary redirector cleanup remains editor-owned
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, MUSE, CURATOR]

- timestamp: 2026-04-26T10:56:00
  event: Event.Validation.SM13.EventBusMetadataGuardrails.Passed
  from: REAPER
  payload:
    subject: EventBus metadata registration guardrails are locked by automation
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.EventBus exit 0
    full_regression: Automation RunTests HorrorProject found 42 tests exit 0
    scope:
      - source metadata registration rejects NAME_None source ids
      - source metadata registration rejects fully empty metadata payloads
      - event-specific metadata registration rejects invalid gameplay tags and NAME_None source ids
      - empty event-specific metadata cannot mask valid source fallback metadata
      - invalid event-specific metadata attempts cannot mask valid source fallback metadata
    validation_notes:
      - first build caught an incomplete automation macro insertion and was fixed before validation
      - new MetadataRegistrationGuardrails automation passes in focused and full regression runs
    next_candidates:
      - continue code-only gameplay polish outside UE Editor binary asset territory
      - UE Editor binary redirector cleanup remains editor-owned
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, MUSE, CURATOR]

- timestamp: 2026-04-26T11:02:00
  event: Event.Validation.SM13.ComponentMetadataGuardrails.Passed
  from: REAPER
  payload:
    subject: Inventory and note metadata guardrails are locked by component automation
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Player.Inventory exit 0
      - HorrorProject.Player.NoteRecorder exit 0
    full_regression: Automation RunTests HorrorProject found 42 tests exit 0
    scope:
      - evidence metadata registration ignores NAME_None evidence ids
      - note metadata registration ignores NAME_None note ids
      - evidence metadata registration overwrites same-id entries deterministically
      - note metadata registration overwrites same-id entries deterministically
      - collected evidence metadata snapshots preserve collected id order while skipping missing metadata
      - recorded note metadata snapshots preserve recorded id order while skipping missing metadata
    next_candidates:
      - found-footage import invariant coverage
      - encounter director reset/resolve edge coverage
      - UE Editor binary redirector cleanup remains editor-owned
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, MUSE, CURATOR]

- timestamp: 2026-04-26T11:08:00
  event: Event.Validation.SM13.FoundFootageImportInvariants.Passed
  from: REAPER
  payload:
    subject: Found-footage save-state import now normalizes objective progress safely
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.FoundFootage exit 0
    full_regression: Automation RunTests HorrorProject found 46 tests exit 0
    scope:
      - ImportSaveState rejects out-of-order milestone progress such as first note without bodycam
      - ImportSaveState rejects terminal exit event/state bypass unless all prerequisite objective event/state pairs are present
      - ImportSaveState ignores invalid, duplicate, and non-objective gameplay tags
      - ImportSaveState exports restored progress in canonical objective order
      - valid partial progress round-trips without auto-unlocking the exit
    next_candidates:
      - encounter director reset/resolve edge coverage
      - continue code-only gameplay polish outside UE Editor binary asset territory
      - UE Editor binary redirector cleanup remains editor-owned
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, MUSE, CURATOR]

- timestamp: 2026-04-26T11:18:00
  event: Event.Validation.SM13.EncounterEdgeInvariants.Passed
  from: REAPER
  payload:
    subject: Encounter director reset, resolve-failure, and failed-reveal invariants are locked by automation
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.Encounter.Director found 11 tests exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - reset from primed clears active encounter state without spawning or gating threats
      - reset from revealed clears route gate state and deactivates the active threat
      - resolve failures from dormant or primed preserve phase, encounter id, route gate state, and threat activity
      - null and out-of-range failed reveals preserve primed state without assigning reveal targets, spawning threats, or gating routes
    validation_notes:
      - failed-reveal test now uses an explicit far threat-character stand-in and verifies CanTriggerReveal before TriggerReveal
    next_candidates:
      - continue code-only gameplay polish outside UE Editor binary asset territory
      - UE Editor binary redirector cleanup remains editor-owned
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, MUSE, CURATOR]

- timestamp: 2026-04-26T11:28:00
  event: Event.Validation.SM13.FoundFootageRuntimeTagGuardrails.Passed
  from: REAPER
  payload:
    subject: Found-footage runtime event recording now rejects valid non-objective tags
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.FoundFootage exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - RecordEvent rejects valid gameplay tags that do not map to canonical objective states
      - state tags cannot be recorded through the runtime event path
      - rejected non-objective tags do not append recorded event output or state-change output
      - runtime recording behavior now matches save import normalization strictness
    next_candidates:
      - route objective node count validation hardening
      - checkpoint restore objective UI restamp
      - UE Editor binary redirector cleanup remains editor-owned
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, MUSE, CURATOR]

- timestamp: 2026-04-26T11:38:00
  event: Event.Validation.SM13.RouteObjectiveCountGuardrail.Passed
  from: REAPER
  payload:
    subject: RouteKit authored Day-1 objective validation now rejects extra route nodes
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.DeepWaterStation.RouteKit exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - ValidateObjectiveNodes requires exactly the canonical six first-loop objectives
      - extra valid-looking objective nodes after the exit gate fail validation
      - spawned objective route remains locked to the authored Day-1 sequence
    next_candidates:
      - checkpoint restore objective UI restamp
      - continue code-only gameplay polish outside UE Editor binary asset territory
      - UE Editor binary redirector cleanup remains editor-owned
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, MUSE, CURATOR]

- timestamp: 2026-04-26T12:05:00
  event: Event.Validation.SM13.LiveStatusDashboard.Passed
  from: REAPER
  payload:
    subject: Local read-only status dashboard is available for Agent Company visibility
    dashboard: http://127.0.0.1:8765/index.html
    scope:
      - status.json operational snapshot
      - index.html five-second polling dashboard
      - server.py localhost-only static server
      - update_status.py atomic status updater
    validation:
      - status JSON parsed successfully
      - Python scripts compiled successfully
      - server returned status.json over localhost with HTTP 200
      - server returned dashboard HTML over localhost
    safety:
      - localhost binding only
      - no POST endpoints
      - no command buttons
      - no LAN exposure
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, CURATOR]

- timestamp: 2026-04-26T12:26:00
  event: Event.Validation.SM13.RouteRecordingFlagGuardrail.Passed
  from: REAPER
  payload:
    subject: RouteKit validation now locks first-anomaly recording flag semantics
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.DeepWaterStation.RouteKit exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - FirstAnomalyRecord must require recording in authored route data
      - non-record objectives cannot carry the first-anomaly recording flag
      - SpawnObjectiveNodes continues to copy only validated recording flag data into interactables
    next_candidates:
      - found-footage import derives exit unlock from complete prerequisites
      - objective interactable metadata registration only after accepted completion
      - checkpoint restore objective UI restamp
      - QuantumCamera acquisition-loss enabled delegate broadcast
      - UE Editor binary redirector cleanup remains editor-owned
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, MUSE, CURATOR]

- timestamp: 2026-04-26T12:36:00
  event: Event.Validation.SM13.FoundFootageImportExitDerivation.Passed
  from: REAPER
  payload:
    subject: Found-footage save import now derives exit unlock from complete prerequisite progress
    build: HorrorProjectEditor Win64 Development exit 0
    targeted_tests:
      - HorrorProject.Game.FoundFootage exit 0
    full_regression: Automation RunTests HorrorProject exit 0
    scope:
      - ImportSaveState imports canonical prerequisite objective event/state pairs in order
      - complete imported prerequisites derive ExitUnlocked event and state even when terminal tags are omitted from the save
      - imported complete progress now matches runtime RecordEvent progression semantics
    next_candidates:
      - objective interactable metadata registration only after accepted completion
      - checkpoint restore objective UI restamp
      - QuantumCamera acquisition-loss enabled delegate broadcast
      - UE Editor binary redirector cleanup remains editor-owned
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, FORGE, REAPER, MUSE, CURATOR]

- timestamp: 2026-04-26T14:30:00
  event: Event.Validation.SM13.SavePendingAnomaly.Passed
  from: REAPER
  payload:
    subject: Pending first-anomaly candidate checkpoint save/restore passed validation
    packet: SM13-20260426-SAVEPENDINGANOMALY
    build: HorrorProjectEditor Win64 Development exit 0
    focused:
      - HorrorProject.Save.Subsystem exit 0
      - HorrorProject.Game.FoundFootage exit 0
    regression: Automation RunTests HorrorProject exit 0
    fixes:
      - Save snapshots capture PendingFirstAnomalySourceId
      - Checkpoint restore rehydrates the pending first-anomaly candidate after found-footage state import
      - Restore keeps EventBus history silent and recording can complete after reload
  subscribers_expected: [ADMIRAL, FORGE, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T14:55:00
  event: Event.Validation.SM13.ThreatTargetLifecycle.Passed
  from: REAPER
  payload:
    subject: Threat detected-target lifecycle hardening passed validation
    packet: SM13-20260426-THREATTARGETLIFE
    build: HorrorProjectEditor Win64 Development exit 0
    focused:
      - HorrorProject.AI.Threat exit 0
    regression: Automation RunTests HorrorProject exit 0
    fixes:
      - DetectedTarget is weak actor state instead of a strong transient object reference
      - active threats reject self-targeting
      - destroyed or stale detected targets are no longer queryable
  subscribers_expected: [ADMIRAL, FORGE, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T15:25:00
  event: Event.Validation.SM13.SaveCrossStream.Passed
  from: REAPER
  payload:
    subject: Save cross-stream restore sanitization passed validation
    packet: SM13-20260426-SAVECROSSSTREAM
    build: HorrorProjectEditor Win64 Development exit 0
    focused:
      - HorrorProject.Save.Subsystem exit 0
      - HorrorProject.Game.FoundFootage exit 0
    regression: Automation RunTests HorrorProject exit 0
    fixes:
      - Checkpoint restore sanitizes found-footage objective tags against evidence and note side streams
      - malformed saves cannot restore anomaly, archive, or exit progress without required records
      - pending first-anomaly snapshots now remain cross-stream consistent in automation
  subscribers_expected: [ADMIRAL, FORGE, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T15:45:00
  event: Event.Validation.SM13.QuantumDelegates.Passed
  from: REAPER
  payload:
    subject: QuantumCamera runtime delegate invariants passed validation
    packet: SM13-20260426-QUANTUMDELEGATES
    build: HorrorProjectEditor Win64 Development exit 0
    focused:
      - HorrorProject.Player.QuantumCamera exit 0
      - HorrorProject.Player exit 0
      - HorrorProject.UI exit 0
    regression: Automation RunTests HorrorProject exit 0
    fixes:
      - runtime delegate probe handlers now record acquired/enabled broadcast counts and payloads
      - acquired/enabled delegates reject no-op rebroadcasts
      - acquisition loss broadcasts acquired false before forced enabled false with deterministic payload order
  subscribers_expected: [ADMIRAL, FORGE, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T16:05:00
  event: Event.Validation.SM13.AnomalyImport.Passed
  from: REAPER
  payload:
    subject: First-anomaly pending candidate import guardrails passed validation
    packet: SM13-20260426-ANOMALYIMPORT
    build: HorrorProjectEditor Win64 Development exit 0
    focused:
      - HorrorProject.Game.FoundFootage.FirstAnomaly exit 0
      - HorrorProject.Game.FoundFootage exit 0
    regression: Automation RunTests HorrorProject exit 0
    fixes:
      - NAME_None and missing-prerequisite pending imports remain empty
      - valid restored pending first-anomaly source can be recorded and consumed
      - stale imports after FirstAnomalyRecorded do not resurrect pending state
  subscribers_expected: [ADMIRAL, FORGE, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T16:30:00
  event: Event.Validation.SM13.ArchiveDelegates.Passed
  from: REAPER
  payload:
    subject: Archive evidence/note delegate payload invariants passed validation
    packet: SM13-20260426-ARCHIVEDELEGATES
    build: HorrorProjectEditor Win64 Development exit 0 after C4458 naming fix
    focused:
      - HorrorProject.Player.Inventory exit 0
      - HorrorProject.Player.NoteRecorder exit 0
      - HorrorProject.UI exit 0
    regression: Automation RunTests HorrorProject exit 0
    fixes:
      - inventory delegate probes record evidence id and total-count payloads
      - note recorder delegate probes record note id and total-count payloads
      - runtime add and import-then-runtime paths lock payload order and totals
  subscribers_expected: [ADMIRAL, FORGE, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T16:50:00
  event: Event.Validation.SM13.RouteExitLock.Passed
  from: REAPER
  payload:
    subject: Route exit-lock gated encounter guardrail passed validation
    packet: SM13-20260426-ROUTEEXITLOCK
    build: HorrorProjectEditor Win64 Development exit 0
    focused:
      - HorrorProject.Game.DeepWaterStation.RouteKit exit 0
    regression: Automation RunTests HorrorProject exit 0
    fixes:
      - locked first-loop exit rejects interaction while encounter is revealed and route-gated
      - rejected locked exit interaction preserves revealed encounter phase and active threat
      - stale RouteKit validation blocker cleared after focused rerun passed
  subscribers_expected: [ADMIRAL, FORGE, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T17:15:00
  event: Event.Validation.SM13.CandidateMeta.Passed
  from: REAPER
  payload:
    subject: First-anomaly candidate metadata visibility passed validation
    packet: SM13-20260426-CANDIDATEMETA
    build: HorrorProjectEditor Win64 Development exit 0 after test-baseline fix
    focused:
      - HorrorProject.Game.FoundFootage.ObjectiveInteractable exit 0
      - HorrorProject.Game.EventBus exit 0
    regression: Automation RunTests HorrorProject exit 0
    fixes:
      - candidate interactions register source-level objective metadata for trailer/UI consumers
      - candidate interactions do not publish objective event history
      - duplicate pending candidates remain silent and do not add history
  subscribers_expected: [ADMIRAL, FORGE, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T17:35:00
  event: Event.Validation.SM13.EventUnregister.Passed
  from: REAPER
  payload:
    subject: EventBus metadata unregister fallback invariant passed validation
    packet: SM13-20260426-EVENTUNREGISTER
    build: HorrorProjectEditor Win64 Development exit 0
    focused:
      - HorrorProject.Game.EventBus exit 0
    regression: Automation RunTests HorrorProject exit 0
    fixes:
      - unregistering event-specific metadata removes only that override
      - source-level fallback metadata resumes on later publishes
      - invalid unregister calls leave fallback metadata intact
  subscribers_expected: [ADMIRAL, FORGE, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T16:23:00
  event: Event.Validation.SM13.InteractionTarget.Passed
  from: REAPER
  payload:
    subject: Interaction component target resolution guardrail passed validation
    packet: SM13-20260426-INTERACTIONTARGET
    build: HorrorProjectEditor Win64 Development exit 0 after test fix
    focused:
      - HorrorProject.Player.Interaction exit 0
    regression: Automation RunTests HorrorProject found 56 tests exit 0
    fixes:
      - exposed ResolveInterfaceTargetForTests for direct target-resolution coverage
      - actor-only hits resolve interactable actor targets
      - non-interactable hit components fall back to interactable actor targets
      - actor-owned component hits preserve actor fallback semantics
  subscribers_expected: [ADMIRAL, FORGE, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T16:29:00
  event: Event.Validation.SM13.VHSBlendClamp.Passed
  from: REAPER
  payload:
    subject: VHS post-process blend clamp guardrail passed validation
    packet: SM13-20260426-VHSBLENDCLAMP
    build: HorrorProjectEditor Win64 Development exit 0
    focused:
      - HorrorProject.Player.VHS.PostProcessBindingFollowsFeedbackState exit 0
    regression: Automation RunTests HorrorProject found 56 tests exit 0
    fixes:
      - active VHS post-process blend weight clamps runtime values above one to 1.0
      - active VHS post-process blend weight clamps negative runtime values to 0.0
      - inactive VHS feedback still clears blend weight to zero
  subscribers_expected: [ADMIRAL, FORGE, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T18:05:00
  event: Event.Dispatch.SM13.ParallelBurst.Degraded
  from: DISPATCHER
  payload:
    subject: Requested 20-agent burst degraded to local integration because runtime agent cap/rate limits blocked parallel fanout
    requested_agents: 20
    observed_limit: 6
    failures:
      - subagent spawn stopped at max 6 active threads
      - five active worker threads failed with 429 Too Many Requests
      - one worker completed without a usable patch summary
    fallback:
      - closed failed worker threads to release capacity
      - continued with main-thread FORGE implementation on low-conflict EventBus territory
  subscribers_expected: [ADMIRAL, DISPATCHER, WATCHDOG, CURATOR]

- timestamp: 2026-04-26T18:08:00
  event: Event.Validation.SM13.EventMetadataClear.Passed
  from: REAPER
  payload:
    subject: EventBus objective metadata registration now clears stale metadata when an empty payload is registered
    packet: SM13-20260426-EVENTMETACLEAR
    build: HorrorProjectEditor Win64 Development exit 0
    focused:
      - HorrorProject.Game.EventBus exit 0
    file_territory:
      - Source/HorrorProject/Game/HorrorEventBusSubsystem.h
      - Source/HorrorProject/Game/HorrorEventBusSubsystem.cpp
      - Source/HorrorProject/Game/Tests/HorrorEventBusSubsystemTests.cpp
    fixes:
      - source-level empty metadata registration removes stale source fallback metadata
      - event-specific empty metadata registration removes stale override metadata
      - repeated EventTag+SourceId metadata key construction now uses a single helper
  subscribers_expected: [ADMIRAL, FORGE, REAPER, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T18:20:00
  event: Event.Validation.SM13.ThreatLifecycleDelegates.Passed
  from: REAPER
  payload:
    subject: Threat characters now expose Blueprint/runtime delegates for active state and detected target changes
    packet: SM13-20260426-THREATDELEGATES
    build: HorrorProjectEditor Win64 Development exit 0
    focused:
      - HorrorProject.AI.Threat exit 0
    file_territory:
      - Source/HorrorProject/AI/HorrorThreatCharacter.h
      - Source/HorrorProject/AI/HorrorThreatCharacter.cpp
      - Source/HorrorProject/AI/Tests/HorrorThreatAITests.cpp
    fixes:
      - activation and deactivation broadcast active-state transitions exactly once
      - first detected target and target clear broadcast target-change payloads
      - duplicate activation and duplicate target updates remain silent
      - deactivation clears detected target before broadcasting inactive state
  subscribers_expected: [ADMIRAL, FORGE, HIVEMIND, REAPER, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T18:32:00
  event: Event.Validation.SM13.RouteTrailerBeatDedupe.Passed
  from: REAPER
  payload:
    subject: RouteKit trailer beat discovery now returns unique cue ids in first-authored order
    packet: SM13-20260426-ROUTEBEATDEDUPE
    build: HorrorProjectEditor Win64 Development exit 0
    focused:
      - HorrorProject.Game.DeepWaterStation.RouteKit exit 0
    file_territory:
      - Source/HorrorProject/Game/DeepWaterStationRouteKit.cpp
      - Source/HorrorProject/Game/Tests/DeepWaterStationRouteKitTests.cpp
    fixes:
      - duplicate TrailerBeatId values are collapsed before cue scheduling consumers read the list
      - first occurrence order is preserved for authored route beats
      - default first-loop route still exposes six unique trailer beat ids
  subscribers_expected: [ADMIRAL, FORGE, MUSE, REAPER, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T18:44:00
  event: Event.Validation.SM13.SaveCheckpointDefault.Passed
  from: REAPER
  payload:
    subject: Empty checkpoint ids now normalize to the Day 1 autosave checkpoint id
    packet: SM13-20260426-SAVECHECKPOINTDEFAULT
    build: HorrorProjectEditor Win64 Development exit 0
    focused:
      - HorrorProject.Save.Subsystem exit 0
    file_territory:
      - Source/HorrorProject/Save/HorrorSaveSubsystem.cpp
      - Source/HorrorProject/Save/Tests/HorrorSaveSubsystemTests.cpp
    fixes:
      - CreateCheckpointSnapshot no longer writes NAME_None into CheckpointId
      - explicit checkpoint ids are still preserved
      - empty Blueprint/C++ callers produce Checkpoint.Day1.Auto for traceable autosaves
  subscribers_expected: [ADMIRAL, FORGE, REAPER, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T18:58:00
  event: Event.Validation.SM13.EncounterPhaseDelegates.Passed
  from: REAPER
  payload:
    subject: Encounter directors now broadcast Blueprint/runtime phase changes for audio, UI, and route consumers
    packet: SM13-20260426-ENCOUNTERPHASEDELEGATES
    build: HorrorProjectEditor Win64 Development exit 0
    focused:
      - HorrorProject.Game.Encounter.Director exit 0
    file_territory:
      - Source/HorrorProject/Game/HorrorEncounterDirector.h
      - Source/HorrorProject/Game/HorrorEncounterDirector.cpp
      - Source/HorrorProject/Game/Tests/HorrorEncounterDirectorTests.cpp
    fixes:
      - primed, revealed, resolved, and reset transitions broadcast ordered phase-change payloads
      - duplicate/failed lifecycle calls remain silent
      - reset broadcasts the encounter id being reset before clearing internal state
  subscribers_expected: [ADMIRAL, FORGE, SPECTER, MUSE, REAPER, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T19:03:00
  event: Event.Validation.SM13.ContinuousCodeBurst.RegressionPassed
  from: REAPER
  payload:
    subject: Continuous post-agent code burst passed full HorrorProject automation regression
    build: HorrorProjectEditor Win64 Development exit 0 after final source edits
    regression: Automation RunTests HorrorProject exit 0
    packets:
      - SM13-20260426-EVENTMETACLEAR
      - SM13-20260426-THREATDELEGATES
      - SM13-20260426-ROUTEBEATDEDUPE
      - SM13-20260426-SAVECHECKPOINTDEFAULT
      - SM13-20260426-ENCOUNTERPHASEDELEGATES
    remaining_blocker: UE Editor-owned redirector cleanup and active map/Blueprint resave still required before alpha-ready claim
  subscribers_expected: [ADMIRAL, FORGE, REAPER, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T19:12:00
  event: Event.Validation.SM13.FoundFootageProgressCount.Passed
  from: REAPER
  payload:
    subject: Found-footage progress snapshots now expose completed milestone count for UI and QA consumers
    packet: SM13-20260426-FOUNDFOOTAGECOUNT
    build: HorrorProjectEditor Win64 Development exit 0
    focused:
      - HorrorProject.Game.FoundFootage exit 0
    file_territory:
      - Source/HorrorProject/Game/HorrorFoundFootageContract.h
      - Source/HorrorProject/Game/HorrorFoundFootageContract.cpp
      - Source/HorrorProject/Game/Tests/HorrorFoundFootageContractTests.cpp
    fixes:
      - BuildSnapshot counts completed milestones while preserving canonical milestone order
      - complete route reports five completed milestones
      - imported partial progress reports three completed milestones and locked exit
  subscribers_expected: [ADMIRAL, FORGE, WEAVER, REAPER, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T19:18:00
  event: Event.Validation.SM13.GameModeProgressSnapshot.Passed
  from: REAPER
  payload:
    subject: GameMode now exposes Blueprint-readable found-footage progress snapshots
    packet: SM13-20260426-GAMEMODESNAPSHOT
    build: HorrorProjectEditor Win64 Development exit 0 after USTRUCT generated-header fix
    focused:
      - HorrorProject.Game.GameModeBase exit 0
      - HorrorProject.Game.FoundFootage exit 0
    file_territory:
      - Source/HorrorProject/Game/HorrorFoundFootageContract.h
      - Source/HorrorProject/Game/HorrorGameModeBase.h
      - Source/HorrorProject/Game/HorrorGameModeBase.cpp
      - Source/HorrorProject/Game/Tests/HorrorGameModeBaseObjectiveTests.cpp
    fixes:
      - progress milestone and snapshot structs are BlueprintType with BlueprintReadOnly fields
      - GameMode exposes BuildFoundFootageProgressSnapshot for UI/Blueprint consumers
      - GameMode tests verify initial, partial, and complete progress counts
  subscribers_expected: [ADMIRAL, FORGE, WEAVER, REAPER, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T19:25:00
  event: Event.Validation.SM13.UIObjectiveProgress.Passed
  from: REAPER
  payload:
    subject: Horror UI now forwards found-footage objective progress snapshots to Blueprint
    packet: SM13-20260426-UIPROGRESS
    build: HorrorProjectEditor Win64 Development exit 0
    focused:
      - HorrorProject.Game.GameModeBase exit 0
    file_territory:
      - Source/HorrorProject/Variant_Horror/UI/HorrorUI.h
      - Source/HorrorProject/Variant_Horror/UI/HorrorUI.cpp
    fixes:
      - RefreshObjectiveProgressSnapshot pushes GameMode progress or an empty fallback snapshot
      - SetupCharacter and checkpoint restamp refresh objective progress alongside archive/VHS state
      - objective event bus messages refresh progress before Blueprint toast requests
  subscribers_expected: [ADMIRAL, WEAVER, MUSE, REAPER, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T19:29:00
  event: Event.Validation.SM13.ProgressUIBurst.RegressionPassed
  from: REAPER
  payload:
    subject: Progress snapshot and UI forwarding burst passed full HorrorProject automation regression
    build: HorrorProjectEditor Win64 Development exit 0
    regression: Automation RunTests HorrorProject exit 0
    packets:
      - SM13-20260426-FOUNDFOOTAGECOUNT
      - SM13-20260426-GAMEMODESNAPSHOT
      - SM13-20260426-UIPROGRESS
    remaining_blocker: UE Editor-owned redirector cleanup and active map/Blueprint resave still required before alpha-ready claim
  subscribers_expected: [ADMIRAL, FORGE, WEAVER, REAPER, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T19:40:00
  event: Event.Validation.SM13.QuantumCameraModeDelegates.Passed
  from: REAPER
  payload:
    subject: Quantum camera tests now lock mode-change delegate sequencing for UI/VHS consumers
    packet: SM13-20260426-QUANTUMMODEDELEGATES
    build: HorrorProjectEditor Win64 Development exit 0
    focused:
      - HorrorProject.Player.QuantumCamera exit 0
    file_territory:
      - Source/HorrorProject/Player/Components/QuantumCameraComponent.h
      - Source/HorrorProject/Player/Components/QuantumCameraComponent.cpp
    fixes:
      - runtime delegate probe now records camera mode broadcasts
      - enabling an acquired camera broadcasts Viewfinder exactly once
      - one-shot photo capture broadcasts Photo then Viewfinder
      - repeated enable remains silent for mode delegates
  subscribers_expected: [ADMIRAL, FORGE, SPECTER, REAPER, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T19:43:00
  event: Event.Validation.SM13.ContinuousDevelopmentRound2.RegressionPassed
  from: REAPER
  payload:
    subject: Second continuous development round passed full HorrorProject automation regression
    build: HorrorProjectEditor Win64 Development exit 0
    regression: Automation RunTests HorrorProject exit 0
    packets:
      - SM13-20260426-FOUNDFOOTAGECOUNT
      - SM13-20260426-GAMEMODESNAPSHOT
      - SM13-20260426-UIPROGRESS
      - SM13-20260426-QUANTUMMODEDELEGATES
    remaining_blocker: UE Editor-owned redirector cleanup and active map/Blueprint resave still required before alpha-ready claim
  subscribers_expected: [ADMIRAL, FORGE, WEAVER, SPECTER, REAPER, CURATOR, WATCHDOG]

- timestamp: 2026-04-26T18:50:00
  event: Event.Validation.SM13.EditorRefsAndPlayableLoop.Passed
  from: REAPER
  payload:
    subject: UE Editor reference blocker cleared by commandlet fixup/resave and target-map automation smoke
    packet: SM13-20260426-EDITORREFS-PLAYABLELOOP
    editor_cleanup:
      - ResavePackages -FixupRedirectors -ProjectOnly -PackageFolder=/Game exit 0
      - targeted ResavePackages for BP_HorrorPlayerCharacter, BP_HorrorGameModeBase, and Lvl_Horror exit 0
      - targeted resave reported 83/83 packages resaved, 0 errors, 0 warnings
    code_fallback:
      - GameMode now auto-spawns a runtime DeepWaterStationRouteKit when a map has none
      - AutoSpawnsRouteKit automation verifies no duplicate runtime route kits
    validation:
      - HorrorProjectEditor Win64 Development exit 0
      - Automation RunTests HorrorProject.Game.GameModeBase exit 0
      - UnrealEditor-Cmd /Game/Variant_Horror/Lvl_Horror + Automation RunTests HorrorProject exit 0
    log_audit:
      - latest target-map automation log contains no /Game/FirstPerson missing-package warnings
      - latest target-map automation log contains no LoadErrors entries
    caveat:
      - direct UE Python actor placement in commandlet crashed UnrealEditor-Cmd with EXCEPTION_INT_DIVIDE_BY_ZERO, so permanent map actor placement should still be done in the interactive editor when convenient
  subscribers_expected: [ADMIRAL, LABYRINTH, FORGE, WEAVER, REAPER, CURATOR, WATCHDOG]
```
