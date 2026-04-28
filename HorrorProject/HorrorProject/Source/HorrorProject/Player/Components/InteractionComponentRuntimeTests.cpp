// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/InteractionComponent.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Game/FoundFootageObjectiveInteractable.h"
#include "Game/HorrorGameModeBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/AutomationTest.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/HorrorPlayerController.h"
#include "Tests/AutomationCommon.h"
#include "UI/Day1SliceHUD.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentBlockedObjectiveShowsHUDReasonTest,
	"HorrorProject.Player.Interaction.BlockedObjectiveCanInteractFalseShowsHUDReason",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInteractionComponentBlockedObjectiveShowsHUDReasonTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for blocked interaction feedback coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Day1 game mode for interaction feedback."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	AFoundFootageObjectiveInteractable* Recorder = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	TestNotNull(TEXT("Blocked interaction test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Blocked interaction test should spawn a player controller."), PlayerController);
	TestNotNull(TEXT("Blocked interaction test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Blocked interaction test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Blocked interaction test should spawn the blocked objective."), Recorder);
	if (!GameMode || !PlayerController || !HUD || !PlayerCharacter || !Recorder)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	Recorder->Objective = EFoundFootageInteractableObjective::FirstAnomalyRecord;
	Recorder->SourceId = TEXT("Evidence.Recorder");
	Recorder->bIsRecordingForFirstAnomalyRecord = true;

	FHitResult ObjectiveHit;
	ObjectiveHit.HitObjectHandle = FActorInstanceHandle(Recorder);
	ObjectiveHit.Component = Recorder->FindComponentByClass<UBoxComponent>();

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	TestNotNull(TEXT("Player should expose an interaction component."), Interaction);
	if (!Interaction)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestFalse(TEXT("Blocked anomaly recording should reject interaction before a candidate exists."), Interaction->TryInteractWithHit(ObjectiveHit));
	TestFalse(TEXT("Rejected interaction should not record the first anomaly."), GameMode->HasRecordedFirstAnomaly());
	TestTrue(TEXT("Rejected interaction should show a transient HUD reason."), HUD->IsTransientMessageVisibleForTests());
	TestEqual(
		TEXT("Rejected interaction should explain the missing anomaly candidate."),
		HUD->GetTransientMessageForTests().ToString(),
		FString(TEXT("先对准异常点。")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
