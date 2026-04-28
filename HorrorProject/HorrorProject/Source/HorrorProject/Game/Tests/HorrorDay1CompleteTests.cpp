// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/FoundFootageObjectiveInteractable.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Game/HorrorGameModeBase.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorDay1CompleteExitGatePublishesCompletionTest,
	"HorrorProject.Game.Day1Complete.ExitGatePublishesCompletion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorDay1CompleteExitGatePublishesCompletionTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for Day1 completion coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Day1 game mode."), World->SetGameMode(FURL()));

	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Day1 completion test requires a game mode."), GameMode);
	TestNotNull(TEXT("Day1 completion test requires the event bus."), EventBus);
	if (!GameMode || !EventBus)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	EventBus->ResetForTests();
	int32 Day1CompleteEventCount = 0;
	FHorrorEventMessage LastDay1CompleteMessage;
	const FDelegateHandle EventHandle = EventBus->GetOnEventPublishedNative().AddLambda(
		[&Day1CompleteEventCount, &LastDay1CompleteMessage](const FHorrorEventMessage& Message)
		{
			if (Message.EventTag == HorrorDay1Tags::Day1CompletedEvent())
			{
				++Day1CompleteEventCount;
				LastDay1CompleteMessage = Message;
			}
		});

	TestTrue(TEXT("Bodycam acquisition should record."), GameMode->TryAcquireBodycam(TEXT("BodycamPickup"), true));
	TestTrue(TEXT("First note should record."), GameMode->TryCollectFirstNote(TEXT("NoteIntro")));
	TestTrue(TEXT("First anomaly candidate should register."), GameMode->BeginFirstAnomalyCandidate(TEXT("Anomaly01")));
	TestTrue(TEXT("First anomaly should record."), GameMode->TryRecordFirstAnomaly(true));
	TestTrue(TEXT("Archive review should unlock the exit."), GameMode->TryReviewArchive(TEXT("ArchiveTerminal")));
	TestTrue(TEXT("Exit should be unlocked before completion interaction."), GameMode->IsExitUnlocked());
	TestFalse(TEXT("Day1 should not be complete until the player uses the exit."), GameMode->IsDay1Complete());

	AFoundFootageObjectiveInteractable* ExitGate = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	TestNotNull(TEXT("Exit gate interactable should spawn."), ExitGate);
	if (!ExitGate)
	{
		EventBus->GetOnEventPublishedNative().Remove(EventHandle);
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	ExitGate->Objective = EFoundFootageInteractableObjective::ExitRouteGate;
	ExitGate->SourceId = TEXT("Exit.ServiceDoor");
	ExitGate->TrailerBeatId = TEXT("Beat.Day1Complete");
	ExitGate->ObjectiveHint = FText::FromString(TEXT("第 1 天完成。"));
	ExitGate->DebugLabel = FText::FromString(TEXT("第 1 天完成"));

	const FHitResult EmptyHit;
	TestTrue(TEXT("Unlocked exit gate should complete Day1 on interaction."), ExitGate->Interact_Implementation(nullptr, EmptyHit));
	TestTrue(TEXT("GameMode should expose Day1 complete state after exit interaction."), GameMode->IsDay1Complete());
	TestEqual(TEXT("Exit interaction should publish exactly one Day1 complete event."), Day1CompleteEventCount, 1);
	TestEqual(TEXT("Day1 complete event should keep the exit source id."), LastDay1CompleteMessage.SourceId, FName(TEXT("Exit.ServiceDoor")));
	TestEqual(TEXT("Day1 complete event should expose a completion state."), LastDay1CompleteMessage.StateTag, HorrorDay1Tags::Day1CompletedState());
	TestEqual(TEXT("Day1 complete event should carry exit HUD metadata."), LastDay1CompleteMessage.DebugLabel.ToString(), FString(TEXT("第 1 天完成")));

	TestFalse(TEXT("Completed Day1 exit gate should not complete again."), ExitGate->Interact_Implementation(nullptr, EmptyHit));
	TestEqual(TEXT("Duplicate exit interaction should not publish another Day1 complete event."), Day1CompleteEventCount, 1);

	EventBus->GetOnEventPublishedNative().Remove(EventHandle);
	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
