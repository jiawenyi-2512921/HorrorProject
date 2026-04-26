// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Game/HorrorGameModeBase.h"
#include "Game/DeepWaterStationRouteKit.h"
#include "Game/HorrorFoundFootageContract.h"

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

#endif
