// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorGameModeBase.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Engine/World.h"
#include "Audio/HorrorAudioSubsystem.h"
#include "Game/HorrorEncounterDirector.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSettings.h"
#include "Interaction/DoorInteractable.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/HorrorPlayerController.h"
#include "Save/HorrorSaveSubsystem.h"
#include "Tests/AutomationCommon.h"
#include "UI/Day1SliceHUD.h"

namespace
{
	const FString Day1FailureRecoveryAutosaveSlotName(TEXT("SM13_Day1_Autosave"));
	constexpr int32 Day1FailureRecoveryAutosaveUserIndex = 0;

	AHorrorGameModeBase* CreateDay1FailureRecoveryGameMode(FAutomationTestBase& Test, FTestWorldWrapper& TestWorld, UWorld*& OutWorld)
	{
		OutWorld = nullptr;
		Test.TestTrue(TEXT("Transient game world should be created for Day 1 failure recovery coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
		OutWorld = TestWorld.GetTestWorld();
		if (!OutWorld)
		{
			return nullptr;
		}

		OutWorld->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
		Test.TestTrue(TEXT("Transient world should create the Day 1 failure recovery game mode."), OutWorld->SetGameMode(FURL()));

		AHorrorGameModeBase* GameMode = OutWorld->GetAuthGameMode<AHorrorGameModeBase>();
		Test.TestNotNull(TEXT("Transient world should expose the Day 1 failure recovery game mode."), GameMode);
		if (!GameMode)
		{
			TestWorld.DestroyTestWorld(false);
		}

		return GameMode;
	}

	AHorrorPlayerCharacter* SpawnDay1FailureRecoveryPlayer(FAutomationTestBase& Test, UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		APlayerController* PlayerController = World->SpawnActor<APlayerController>();
		AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
		Test.TestNotNull(TEXT("Failure recovery test should spawn a player controller."), PlayerController);
		Test.TestNotNull(TEXT("Failure recovery test should spawn the horror player character."), PlayerCharacter);
		if (PlayerController)
		{
			PlayerController->PlayerState = World->SpawnActor<APlayerState>();
			World->AddController(PlayerController);
		}
		if (PlayerController && PlayerCharacter)
		{
			PlayerController->Possess(PlayerCharacter);
		}
		return PlayerCharacter;
	}

	bool FindLastFailureRecoveryEvent(
		const UHorrorEventBusSubsystem& EventBus,
		const FGameplayTag& FailureEventTag,
		FHorrorEventMessage& OutMessage)
	{
		const TArray<FHorrorEventMessage>& History = EventBus.GetHistory();
		for (int32 Index = History.Num() - 1; Index >= 0; --Index)
		{
			if (History[Index].EventTag == FailureEventTag)
			{
				OutMessage = History[Index];
				return true;
			}
		}
		return false;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerFailureWithoutCheckpointClearsModalStateTest,
	"HorrorProject.Game.PlayerFailureRecovery.NoCheckpointClearsModalState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerFailureWithoutCheckpointClearsModalStateTest::RunTest(const FString& Parameters)
{
	UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);

	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateDay1FailureRecoveryGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	TestNotNull(TEXT("No checkpoint failure test should spawn a horror player controller."), PlayerController);
	TestNotNull(TEXT("No checkpoint failure test should spawn a Day1 HUD."), HUD);
	TestNotNull(TEXT("No checkpoint failure test should spawn a horror player character."), PlayerCharacter);
	TestNotNull(TEXT("No checkpoint failure test should spawn a password door."), Door);
	if (!PlayerController || !HUD || !PlayerCharacter || !Door)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	TestTrue(TEXT("Escape should open pause before no-checkpoint failure."), PlayerController->HandleInputKeyForTests(EKeys::Escape));
	TestTrue(TEXT("Pause should be open before no-checkpoint failure."), PlayerController->IsDay1PauseMenuOpenForTests());
	TestTrue(TEXT("Escape should close pause before opening notes."), PlayerController->HandleInputKeyForTests(EKeys::Escape));
	TestTrue(TEXT("J should open notes before no-checkpoint failure."), PlayerController->HandleInputKeyForTests(EKeys::J));
	TestTrue(TEXT("Notes should be open before no-checkpoint failure."), PlayerController->IsDay1NotesJournalOpenForTests());

	Door->ConfigurePassword(TEXT("1234"), FText::FromString(TEXT("无检查点测试密码。")));
	PlayerController->BeginDoorPasswordEntry(Door);
	TestTrue(TEXT("Password entry should be active before no-checkpoint failure."), PlayerController->IsAwaitingDoorPassword());
	TestTrue(TEXT("Password prompt should be visible before no-checkpoint failure."), HUD->IsPasswordPromptVisibleForTests());

	TestFalse(TEXT("Death without a save should report no checkpoint recovery."), GameMode->RequestPlayerDeath(TEXT("Death.NoCheckpoint")));
	TestFalse(TEXT("No-checkpoint failure should leave pause closed."), PlayerController->IsDay1PauseMenuOpenForTests());
	TestFalse(TEXT("No-checkpoint failure should leave notes closed."), PlayerController->IsDay1NotesJournalOpenForTests());
	TestFalse(TEXT("No-checkpoint failure should cancel password entry."), PlayerController->IsAwaitingDoorPassword());
	TestFalse(TEXT("No-checkpoint failure should clear password prompt."), HUD->IsPasswordPromptVisibleForTests());
	TestFalse(TEXT("No-checkpoint failure should report no checkpoint recovery."), GameMode->DidLastPlayerFailureRecoverFromCheckpoint());
	TestEqual(TEXT("No-checkpoint failure should record the cause."), GameMode->GetLastPlayerFailureCause(), FName(TEXT("Death.NoCheckpoint")));

	UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);
	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerFailureWithoutCheckpointPublishesMissingEventTest,
	"HorrorProject.Game.PlayerFailureRecovery.PublishesMissingCheckpointEvent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerFailureWithoutCheckpointPublishesMissingEventTest::RunTest(const FString& Parameters)
{
	UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);

	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateDay1FailureRecoveryGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);
		return false;
	}

	TestNotNull(
		TEXT("Missing-checkpoint failure event test should spawn a player character."),
		SpawnDay1FailureRecoveryPlayer(*this, World));

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Missing-checkpoint failure event test should expose the event bus."), EventBus);
	if (!EventBus)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);
		return false;
	}

	const FGameplayTag FailureEventTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Day1.PlayerFailure")), false);
	const FGameplayTag MissingStateTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Day1.CheckpointMissing")), false);
	TestTrue(TEXT("Player failure event tag should be registered."), FailureEventTag.IsValid());
	TestTrue(TEXT("Missing-checkpoint state tag should be registered."), MissingStateTag.IsValid());

	TestFalse(TEXT("Death without a save should still publish a missing-checkpoint event."), GameMode->RequestPlayerDeath(TEXT("Death.NoCheckpointEvent")));
	FHorrorEventMessage FailureMessage;
	TestTrue(
		TEXT("Death without a save should publish the Day1 player failure event."),
		FindLastFailureRecoveryEvent(*EventBus, FailureEventTag, FailureMessage));
	TestEqual(TEXT("The failure event should preserve the failure cause as source id."), FailureMessage.SourceId, FName(TEXT("Death.NoCheckpointEvent")));
	TestEqual(TEXT("The failure event should report missing checkpoint state."), FailureMessage.StateTag, MissingStateTag);

	UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);
	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerFailureRecoveryLoadsLastDay1CheckpointTest,
	"HorrorProject.Game.PlayerFailureRecovery.LoadsLastDay1Checkpoint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerFailureRecoveryLoadsLastDay1CheckpointTest::RunTest(const FString& Parameters)
{
	UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);

	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateDay1FailureRecoveryGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = SpawnDay1FailureRecoveryPlayer(*this, World);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);
		return false;
	}

	const FTransform CheckpointTransform(FRotator(0.0f, 45.0f, 0.0f), FVector(100.0f, 200.0f, 300.0f));
	PlayerCharacter->SetActorTransform(CheckpointTransform);
	TestTrue(TEXT("Failure recovery test should save a Day1 checkpoint before the lethal event."), GameMode->SaveDay1Checkpoint(TEXT("Checkpoint.Day1.FailureRecovery")));

	const FTransform FailedTransform(FRotator(0.0f, -90.0f, 0.0f), FVector(900.0f, -800.0f, 120.0f));
	PlayerCharacter->SetActorTransform(FailedTransform);

	TestTrue(
		TEXT("Player death API should record the failure and load the last Day1 checkpoint."),
		GameMode->RequestPlayerDeath(TEXT("Death.TestHazard")));
	TestEqual(TEXT("The last failure cause should be recorded for feedback and telemetry."), GameMode->GetLastPlayerFailureCause(), FName(TEXT("Death.TestHazard")));
	TestTrue(TEXT("The death flow should report that checkpoint recovery was loaded."), GameMode->DidLastPlayerFailureRecoverFromCheckpoint());
	TestTrue(
		TEXT("Player should be restored to the saved checkpoint transform after death."),
		PlayerCharacter->GetActorLocation().Equals(CheckpointTransform.GetLocation(), KINDA_SMALL_NUMBER));
	if (UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>())
	{
		TestEqual(TEXT("Player failure should push Day1 audio into the failure stage."), AudioSubsystem->GetDay1AudioStage(), EHorrorDay1AudioStage::Failure);
		TestEqual(TEXT("Player failure audio should retain the failure cause as source id."), AudioSubsystem->GetLastDay1AudioSourceId(), FName(TEXT("Death.TestHazard")));
	}

	UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);
	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerFailureRecoveryPublishesRestoredEventTest,
	"HorrorProject.Game.PlayerFailureRecovery.PublishesCheckpointRestoredEvent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerFailureRecoveryPublishesRestoredEventTest::RunTest(const FString& Parameters)
{
	UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);

	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateDay1FailureRecoveryGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = SpawnDay1FailureRecoveryPlayer(*this, World);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);
		return false;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	TestNotNull(TEXT("Checkpoint-restored failure event test should expose the event bus."), EventBus);
	if (!EventBus)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);
		return false;
	}

	PlayerCharacter->SetActorLocation(FVector(50.0f, 0.0f, 0.0f));
	TestTrue(TEXT("Checkpoint-restored failure event test should save a checkpoint."), GameMode->SaveDay1Checkpoint(TEXT("Checkpoint.Day1.EventRecovery")));

	const FGameplayTag FailureEventTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Day1.PlayerFailure")), false);
	const FGameplayTag RestoredStateTag = FGameplayTag::RequestGameplayTag(FName(TEXT("State.Day1.CheckpointRestored")), false);
	TestTrue(TEXT("Player failure event tag should be registered."), FailureEventTag.IsValid());
	TestTrue(TEXT("Checkpoint-restored state tag should be registered."), RestoredStateTag.IsValid());

	TestTrue(TEXT("Death with a save should publish a checkpoint-restored event."), GameMode->RequestPlayerDeath(TEXT("Death.RestoredEvent")));
	FHorrorEventMessage FailureMessage;
	TestTrue(
		TEXT("Death with a save should publish the Day1 player failure event."),
		FindLastFailureRecoveryEvent(*EventBus, FailureEventTag, FailureMessage));
	TestEqual(TEXT("The restored failure event should preserve the failure cause as source id."), FailureMessage.SourceId, FName(TEXT("Death.RestoredEvent")));
	TestEqual(TEXT("The restored failure event should report checkpoint restored state."), FailureMessage.StateTag, RestoredStateTag);

	UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);
	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerFailureRecoveryRestoresEncounterPhaseTest,
	"HorrorProject.Game.PlayerFailureRecovery.RestoresEncounterPhase",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerFailureRecoveryRestoresEncounterPhaseTest::RunTest(const FString& Parameters)
{
	UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);

	FTestWorldWrapper TestWorld;
	UWorld* World = nullptr;
	AHorrorGameModeBase* GameMode = CreateDay1FailureRecoveryGameMode(*this, TestWorld, World);
	if (!GameMode || !World)
	{
		UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = SpawnDay1FailureRecoveryPlayer(*this, World);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);
		return false;
	}

	TestTrue(TEXT("Bodycam should record before encounter phase checkpoint."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	TestTrue(TEXT("First note should prime the encounter before checkpoint."), GameMode->TryCollectFirstNote(TEXT("Note.Intro")));
	AHorrorEncounterDirector* Director = GameMode->GetRuntimeEncounterDirector();
	TestNotNull(TEXT("First note should create a runtime encounter director."), Director);
	if (!Director)
	{
		TestWorld.DestroyTestWorld(false);
		UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);
		return false;
	}

	TestEqual(TEXT("First note checkpoint should leave encounter primed."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Primed);
	TestTrue(TEXT("Primed encounter checkpoint should save."), GameMode->SaveDay1Checkpoint(TEXT("Checkpoint.Day1.PrimedEncounter")));

	Director->RevealRadius = 0.0f;
	TestTrue(TEXT("Anomaly candidate should register after primed checkpoint."), GameMode->BeginFirstAnomalyCandidate(TEXT("Evidence.Anomaly01")));
	TestTrue(TEXT("Recording the anomaly should complete evidence capture."), GameMode->TryRecordFirstAnomaly(true));
	TestEqual(TEXT("Recording the anomaly should keep the encounter primed until the archive route reveals it."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Primed);
	TestTrue(TEXT("Anomaly-recorded primed encounter checkpoint should save."), GameMode->SaveDay1Checkpoint(TEXT("Checkpoint.Day1.AnomalyRecordedPrimedEncounter")));
	TestTrue(TEXT("Route-owned encounter reveal should succeed before death recovery."), Director->TriggerReveal(PlayerCharacter));
	TestEqual(TEXT("Encounter should be revealed before death recovery."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Revealed);
	TestTrue(TEXT("Revealed encounter should gate the route before death recovery."), Director->IsRouteGated());

	TestTrue(TEXT("Death should load the anomaly-recorded primed encounter checkpoint."), GameMode->RequestPlayerDeath(TEXT("Death.TestEncounter")));
	TestTrue(TEXT("Death should report checkpoint recovery."), GameMode->DidLastPlayerFailureRecoverFromCheckpoint());
	TestTrue(TEXT("First note objective should restore from the checkpoint."), GameMode->HasCollectedFirstNote());
	TestTrue(TEXT("Anomaly objective should restore without forcing an encounter reveal."), GameMode->HasRecordedFirstAnomaly());
	TestEqual(TEXT("Encounter phase should restore to primed instead of staying revealed."), Director->GetEncounterPhase(), EHorrorEncounterPhase::Primed);
	TestFalse(TEXT("Restored primed encounter should not keep the route gated."), Director->IsRouteGated());

	UGameplayStatics::DeleteGameInSlot(Day1FailureRecoveryAutosaveSlotName, Day1FailureRecoveryAutosaveUserIndex);
	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
