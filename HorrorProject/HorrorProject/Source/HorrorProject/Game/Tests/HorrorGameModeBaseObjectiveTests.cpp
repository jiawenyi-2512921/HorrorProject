// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Game/HorrorGameModeBase.h"
#include "Game/DeepWaterStationRouteKit.h"
#include "Game/HorrorEncounterDirector.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Player/HorrorPlayerCharacter.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGameModeBaseObjectiveBridgeTest,
	"HorrorProject.Game.GameModeBase.ObjectiveBridgeCompletesFirstLoop",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHorrorGameModeBaseObjectiveBridgeTest::RunTest(const FString& Parameters)
{
	AHorrorGameModeBase* GameMode = NewObject<AHorrorGameModeBase>();

	TestFalse(TEXT("Exit should start locked."), GameMode->IsExitUnlocked());
	FHorrorFoundFootageProgressSnapshot Snapshot = GameMode->BuildFoundFootageProgressSnapshot();
	TestEqual(TEXT("Initial GameMode snapshot should expose all milestones."), Snapshot.Milestones.Num(), 5);
	TestEqual(TEXT("Initial GameMode snapshot should have no completed milestones."), Snapshot.CompletedMilestoneCount, 0);
	TestFalse(TEXT("First note should be blocked before bodycam acquisition."), GameMode->CanCollectFirstNote());
	TestFalse(TEXT("First note objective should not record before bodycam acquisition."), GameMode->TryCollectFirstNote(TEXT("Note01")));
	TestFalse(TEXT("First anomaly candidate should be blocked before first note."), GameMode->CanBeginFirstAnomalyCandidate());
	TestFalse(TEXT("First anomaly candidate should not register before first note."), GameMode->BeginFirstAnomalyCandidate(TEXT("Anomaly01")));
	TestFalse(TEXT("Archive review should be blocked before first anomaly recording."), GameMode->CanReviewArchive());
	TestFalse(TEXT("Archive review should not record before first anomaly recording."), GameMode->TryReviewArchive(TEXT("ArchiveTerminal")));
	TestTrue(TEXT("Bodycam objective should record through GameMode."), GameMode->TryAcquireBodycam(TEXT("BodycamPickup"), true));
	TestTrue(TEXT("Bodycam objective should be queryable."), GameMode->HasBodycamAcquired());
	Snapshot = GameMode->BuildFoundFootageProgressSnapshot();
	TestEqual(TEXT("GameMode snapshot should count bodycam progress."), Snapshot.CompletedMilestoneCount, 1);
	TestFalse(TEXT("Duplicate bodycam acquisition should be ignored."), GameMode->TryAcquireBodycam(TEXT("BodycamPickup"), true));

	TestTrue(TEXT("First note should become available after bodycam acquisition."), GameMode->CanCollectFirstNote());
	TestTrue(TEXT("First note objective should record through GameMode."), GameMode->TryCollectFirstNote(TEXT("Note01")));
	TestTrue(TEXT("First note objective should be queryable."), GameMode->HasCollectedFirstNote());
	TestTrue(TEXT("First anomaly candidate should become available after first note."), GameMode->CanBeginFirstAnomalyCandidate());
	TestTrue(TEXT("First anomaly candidate should register through GameMode."), GameMode->BeginFirstAnomalyCandidate(TEXT("Anomaly01")));
	TestFalse(TEXT("Non-recording anomaly attempt should not complete the objective."), GameMode->TryRecordFirstAnomaly(false));
	TestTrue(TEXT("Recording anomaly attempt should complete the objective."), GameMode->TryRecordFirstAnomaly(true));
	TestTrue(TEXT("First anomaly objective should be queryable."), GameMode->HasRecordedFirstAnomaly());
	TestTrue(TEXT("Archive review should become available after first anomaly recording."), GameMode->CanReviewArchive());
	TestTrue(TEXT("Archive review should record through GameMode."), GameMode->TryReviewArchive(TEXT("ArchiveTerminal")));
	TestTrue(TEXT("Archive review objective should be queryable."), GameMode->HasReviewedArchive());

	TestTrue(TEXT("Exit should unlock after all first-loop milestones."), GameMode->IsExitUnlocked());
	TestTrue(
		TEXT("Exit unlocked state should be exposed through generic state query."),
		GameMode->HasFoundFootageCompletedState(HorrorFoundFootageTags::ExitUnlockedState()));
	Snapshot = GameMode->BuildFoundFootageProgressSnapshot();
	TestEqual(TEXT("GameMode snapshot should count full first-loop completion."), Snapshot.CompletedMilestoneCount, 5);
	TestTrue(TEXT("GameMode snapshot should expose unlocked exit."), Snapshot.bExitUnlocked);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGameModeBaseAutoSpawnsRouteKitTest,
	"HorrorProject.Game.GameModeBase.AutoSpawnsRouteKit",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHorrorGameModeBaseAutoSpawnsRouteKitTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for GameMode route kit bootstrap coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	TestNotNull(TEXT("GameMode route kit bootstrap test should spawn GameMode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	int32 RouteKitCountBeforeBeginPlay = 0;
	for (TActorIterator<ADeepWaterStationRouteKit> It(World); It; ++It)
	{
		++RouteKitCountBeforeBeginPlay;
	}
	TestEqual(TEXT("No route kit should exist before bootstrap."), RouteKitCountBeforeBeginPlay, 0);

	GameMode->DispatchBeginPlay();

	int32 RouteKitCountAfterBeginPlay = 0;
	for (TActorIterator<ADeepWaterStationRouteKit> It(World); It; ++It)
	{
		++RouteKitCountAfterBeginPlay;
		if (It->ObjectiveNodes.IsEmpty())
		{
			It->ConfigureDefaultFirstLoopObjectiveNodes();
		}
		TArray<FText> ValidationErrors;
		TestTrue(TEXT("Runtime route kit should validate its default objective nodes."), It->ValidateObjectiveNodes(ValidationErrors));
	}
	TestEqual(TEXT("GameMode BeginPlay should spawn one runtime route kit when the map lacks one."), RouteKitCountAfterBeginPlay, 1);

	GameMode->DispatchBeginPlay();
	int32 RouteKitCountAfterRepeatedBeginPlay = 0;
	for (TActorIterator<ADeepWaterStationRouteKit> It(World); It; ++It)
	{
		++RouteKitCountAfterRepeatedBeginPlay;
	}
	TestEqual(TEXT("Repeated BeginPlay dispatch should not duplicate runtime route kits."), RouteKitCountAfterRepeatedBeginPlay, 1);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGameModeBaseAutoSpawnsEncounterDirectorTest,
	"HorrorProject.Game.GameModeBase.AutoSpawnsEncounterDirector",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHorrorGameModeBaseAutoSpawnsEncounterDirectorTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for encounter director bootstrap coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	TestNotNull(TEXT("Encounter director bootstrap test should spawn GameMode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	int32 EncounterCountBefore = 0;
	for (TActorIterator<AHorrorEncounterDirector> It(World); It; ++It)
	{
		++EncounterCountBefore;
	}
	TestEqual(TEXT("No encounter director should exist before bootstrap."), EncounterCountBefore, 0);

	GameMode->DispatchBeginPlay();

	int32 EncounterCountAfter = 0;
	for (TActorIterator<AHorrorEncounterDirector> It(World); It; ++It)
	{
		++EncounterCountAfter;
	}
	TestEqual(TEXT("BeginPlay should spawn one runtime encounter director."), EncounterCountAfter, 1);

	AHorrorEncounterDirector* Director = GameMode->GetRuntimeEncounterDirector();
	TestNotNull(TEXT("Runtime encounter director should be retained on the GameMode."), Director);
	if (Director)
	{
		TestEqual(TEXT("Runtime encounter director should start dormant."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Dormant);
		TestEqual(TEXT("Runtime encounter director should default to unlimited reveal radius for scripted milestones."), Director->RevealRadius, 0.0f);
	}

	GameMode->DispatchBeginPlay();
	int32 EncounterCountAfterRepeat = 0;
	for (TActorIterator<AHorrorEncounterDirector> It(World); It; ++It)
	{
		++EncounterCountAfterRepeat;
	}
	TestEqual(TEXT("Repeated BeginPlay should not duplicate the encounter director."), EncounterCountAfterRepeat, 1);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGameModeBaseDrivesEncounterFromMilestonesTest,
	"HorrorProject.Game.GameModeBase.DrivesEncounterFromMilestones",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHorrorGameModeBaseDrivesEncounterFromMilestonesTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for milestone-driven encounter coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	TestNotNull(TEXT("Milestone encounter test should spawn GameMode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->DispatchBeginPlay();
	AHorrorEncounterDirector* Director = GameMode->GetRuntimeEncounterDirector();
	TestNotNull(TEXT("Milestone encounter test requires a runtime encounter director."), Director);
	if (!Director)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	AActor* PlayerStandIn = World->SpawnActor<AHorrorPlayerCharacter>(FVector(50.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Milestone encounter test should spawn a player stand-in."), PlayerStandIn);

	TestTrue(TEXT("Bodycam acquisition should record."), GameMode->TryAcquireBodycam(TEXT("BodycamPickup"), true));
	TestEqual(TEXT("Bodycam should not yet prime the encounter."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Dormant);

	TestTrue(TEXT("First note collection should record."), GameMode->TryCollectFirstNote(TEXT("Note01")));
	TestEqual(TEXT("First note should prime the encounter director."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Primed);

	TestTrue(TEXT("First anomaly candidate should register."), GameMode->BeginFirstAnomalyCandidate(TEXT("Anomaly01")));
	TestTrue(TEXT("First anomaly should record under recording state."), GameMode->TryRecordFirstAnomaly(true));
	TestEqual(
		TEXT("First anomaly should reveal the encounter director after a player exists in the world."),
		Director->GetEncounterPhase(),
		EHorrorEncounterPhase::Revealed);

	TestTrue(TEXT("Archive review should record."), GameMode->TryReviewArchive(TEXT("ArchiveTerminal")));
	TestEqual(TEXT("Archive review should resolve the encounter."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Resolved);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGameModeBaseReusesPlacedEncounterDirectorTest,
	"HorrorProject.Game.GameModeBase.ReusesPlacedEncounterDirector",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHorrorGameModeBaseReusesPlacedEncounterDirectorTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for placed encounter director coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorEncounterDirector* PlacedDirector = World->SpawnActor<AHorrorEncounterDirector>(FVector(123.0f, 0.0f, 0.0f), FRotator::ZeroRotator);
	TestNotNull(TEXT("Placed encounter director should spawn before GameMode bootstrap."), PlacedDirector);
	if (!PlacedDirector)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}
	const float PlacedRevealRadius = PlacedDirector->RevealRadius;

	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	TestNotNull(TEXT("Placed encounter director test should spawn GameMode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->DispatchBeginPlay();

	int32 EncounterCount = 0;
	for (TActorIterator<AHorrorEncounterDirector> It(World); It; ++It)
	{
		++EncounterCount;
	}
	TestEqual(TEXT("Pre-placed encounter director should be reused, not duplicated."), EncounterCount, 1);
	TestEqual(TEXT("GameMode should expose the pre-placed encounter director."), GameMode->GetRuntimeEncounterDirector(), PlacedDirector);
	TestEqual(TEXT("Pre-placed encounter director RevealRadius should not be overwritten by bootstrap."), PlacedDirector->RevealRadius, PlacedRevealRadius);

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorGameModeBaseRegistersDefaultObjectiveMetadataTest,
	"HorrorProject.Game.GameModeBase.RegistersDefaultObjectiveMetadata",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FHorrorGameModeBaseRegistersDefaultObjectiveMetadataTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for default metadata coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	TestNotNull(TEXT("Default metadata test should spawn GameMode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->DispatchBeginPlay();

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Default metadata test requires an event bus subsystem."), EventBus);
	if (!EventBus)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	struct FExpectedMetadataEntry
	{
		FGameplayTag EventTag;
		FName SourceId;
		FName TrailerBeatId;
	};

	const FExpectedMetadataEntry Expected[] = {
		{ HorrorFoundFootageTags::BodycamAcquiredEvent(), TEXT("Bodycam"), TEXT("Trailer.Beat.BodycamAcquired") },
		{ HorrorFoundFootageTags::FirstNoteCollectedEvent(), TEXT("FirstNote"), TEXT("Trailer.Beat.FirstNote") },
		{ HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), TEXT("FirstAnomaly"), TEXT("Trailer.Beat.FirstAnomaly") },
		{ HorrorFoundFootageTags::ArchiveReviewedEvent(), TEXT("Archive"), TEXT("Trailer.Beat.ArchiveReviewed") },
		{ HorrorFoundFootageTags::ExitUnlockedEvent(), TEXT("Exit"), TEXT("Trailer.Beat.ExitUnlocked") },
	};

	for (const FExpectedMetadataEntry& Entry : Expected)
	{
		FHorrorObjectiveMessageMetadata Metadata;
		const bool bFound = EventBus->GetObjectiveMetadataForTests(Entry.EventTag, Entry.SourceId, Metadata);
		TestTrue(*FString::Printf(TEXT("Default metadata should be registered for %s."), *Entry.EventTag.ToString()), bFound);
		if (bFound)
		{
			TestEqual(*FString::Printf(TEXT("Trailer beat id should match for %s."), *Entry.EventTag.ToString()), Metadata.TrailerBeatId, Entry.TrailerBeatId);
			TestFalse(*FString::Printf(TEXT("Objective hint should not be empty for %s."), *Entry.EventTag.ToString()), Metadata.ObjectiveHint.IsEmpty());
			TestFalse(*FString::Printf(TEXT("Debug label should not be empty for %s."), *Entry.EventTag.ToString()), Metadata.DebugLabel.IsEmpty());
		}
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
