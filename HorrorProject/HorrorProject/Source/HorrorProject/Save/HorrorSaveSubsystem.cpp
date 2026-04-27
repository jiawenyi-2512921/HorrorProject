// Copyright Epic Games, Inc. All Rights Reserved.

#include "Save/HorrorSaveSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Game/HorrorGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/HorrorPlayerController.h"
#include "Player/Components/InventoryComponent.h"
#include "Player/Components/NoteRecorderComponent.h"
#include "Save/HorrorSaveGame.h"

namespace HorrorSaveSubsystemConstants
{
	const FString Day1AutosaveSlotName(TEXT("SM13_Day1_Autosave"));
	const FName Day1DefaultCheckpointId(TEXT("Checkpoint.Day1.Auto"));
	constexpr int32 Day1AutosaveUserIndex = 0;
	constexpr int32 Day1SaveVersion = 1;
}

namespace
{
	bool HasName(const TArray<FName>& Values, FName Query)
	{
		return Values.Contains(Query);
	}

	void RemoveObjectivePair(FHorrorFoundFootageSaveState& OutSaveState, FGameplayTag EventTag, FGameplayTag StateTag)
	{
		OutSaveState.RecordedObjectiveEvents.Remove(EventTag);
		OutSaveState.CompletedObjectiveStates.Remove(StateTag);
	}

	void SanitizeFoundFootageSaveStateForDay1Restore(
		FHorrorFoundFootageSaveState& SaveState,
		const TArray<FName>& CollectedEvidenceIds,
		const TArray<FName>& RecordedNoteIds)
	{
		const bool bHasBodycamEvidence = HasName(CollectedEvidenceIds, TEXT("Evidence.Bodycam"));
		const bool bHasIntroNote = HasName(RecordedNoteIds, TEXT("Note.Intro"));
		const bool bHasAnomalyEvidence = HasName(CollectedEvidenceIds, TEXT("Evidence.Anomaly01"));

		if (!bHasBodycamEvidence)
		{
			RemoveObjectivePair(SaveState, HorrorFoundFootageTags::BodycamAcquiredEvent(), HorrorFoundFootageTags::BodycamAcquiredState());
		}

		if (!bHasBodycamEvidence || !bHasIntroNote)
		{
			RemoveObjectivePair(SaveState, HorrorFoundFootageTags::FirstNoteCollectedEvent(), HorrorFoundFootageTags::FirstNoteCollectedState());
		}

		if (!bHasBodycamEvidence || !bHasIntroNote || !bHasAnomalyEvidence)
		{
			RemoveObjectivePair(SaveState, HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), HorrorFoundFootageTags::FirstAnomalyRecordedState());
			RemoveObjectivePair(SaveState, HorrorFoundFootageTags::ArchiveReviewedEvent(), HorrorFoundFootageTags::ArchiveReviewedState());
			RemoveObjectivePair(SaveState, HorrorFoundFootageTags::ExitUnlockedEvent(), HorrorFoundFootageTags::ExitUnlockedState());
		}
	}

	bool ResolveDay1SaveTargets(
		UWorld* World,
		AHorrorGameModeBase*& OutHorrorGameMode,
		AHorrorPlayerCharacter*& OutPlayerCharacter,
		UInventoryComponent*& OutInventory,
		UNoteRecorderComponent*& OutNoteRecorder)
	{
		OutHorrorGameMode = nullptr;
		OutPlayerCharacter = nullptr;
		OutInventory = nullptr;
		OutNoteRecorder = nullptr;

		if (!World)
		{
			return false;
		}

		OutHorrorGameMode = Cast<AHorrorGameModeBase>(UGameplayStatics::GetGameMode(World));
		OutPlayerCharacter = Cast<AHorrorPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
		OutInventory = OutPlayerCharacter ? OutPlayerCharacter->GetInventoryComponent() : nullptr;
		OutNoteRecorder = OutPlayerCharacter ? OutPlayerCharacter->GetNoteRecorderComponent() : nullptr;
		return OutHorrorGameMode && OutPlayerCharacter && OutInventory && OutNoteRecorder;
	}
}

bool UHorrorSaveSubsystem::SaveCheckpoint(UObject* WorldContextObject, FName CheckpointId)
{
	UHorrorSaveGame* SaveGame = CreateCheckpointSnapshot(WorldContextObject, CheckpointId);
	if (!SaveGame)
	{
		return false;
	}

	const bool bSaved = UGameplayStatics::SaveGameToSlot(
		SaveGame,
		HorrorSaveSubsystemConstants::Day1AutosaveSlotName,
		HorrorSaveSubsystemConstants::Day1AutosaveUserIndex);

	if (bSaved)
	{
		CachedSave = SaveGame;
	}

	return bSaved;
}

bool UHorrorSaveSubsystem::LoadCheckpoint(UObject* WorldContextObject)
{
	UWorld* World = ResolveWorld(WorldContextObject);
	if (!World)
	{
		return false;
	}

	UHorrorSaveGame* SaveGame = CachedSave.Get();
	if (!SaveGame)
	{
		SaveGame = Cast<UHorrorSaveGame>(UGameplayStatics::LoadGameFromSlot(
			HorrorSaveSubsystemConstants::Day1AutosaveSlotName,
			HorrorSaveSubsystemConstants::Day1AutosaveUserIndex));
	}

	if (!SaveGame || SaveGame->SaveVersion != HorrorSaveSubsystemConstants::Day1SaveVersion)
	{
		return false;
	}

	if (!ApplyCheckpointSnapshot(WorldContextObject, SaveGame))
	{
		return false;
	}

	CachedSave = SaveGame;
	return true;
}

UHorrorSaveGame* UHorrorSaveSubsystem::CreateCheckpointSnapshot(UObject* WorldContextObject, FName CheckpointId) const
{
	UWorld* World = ResolveWorld(WorldContextObject);
	AHorrorGameModeBase* HorrorGameMode = nullptr;
	AHorrorPlayerCharacter* PlayerCharacter = nullptr;
	UInventoryComponent* Inventory = nullptr;
	UNoteRecorderComponent* NoteRecorder = nullptr;
	if (!ResolveDay1SaveTargets(World, HorrorGameMode, PlayerCharacter, Inventory, NoteRecorder))
	{
		return nullptr;
	}

	UHorrorSaveGame* SaveGame = Cast<UHorrorSaveGame>(UGameplayStatics::CreateSaveGameObject(UHorrorSaveGame::StaticClass()));
	if (!SaveGame)
	{
		return nullptr;
	}

	SaveGame->SaveVersion = HorrorSaveSubsystemConstants::Day1SaveVersion;
	SaveGame->CheckpointId = CheckpointId.IsNone()
		? HorrorSaveSubsystemConstants::Day1DefaultCheckpointId
		: CheckpointId;

	const FHorrorFoundFootageSaveState FoundFootageSaveState = HorrorGameMode->ExportFoundFootageSaveState();
	SaveGame->RecordedObjectiveEvents = FoundFootageSaveState.RecordedObjectiveEvents;
	SaveGame->CompletedObjectiveStates = FoundFootageSaveState.CompletedObjectiveStates;
	SaveGame->PendingFirstAnomalySourceId = HorrorGameMode->GetPendingFirstAnomalySourceId();
	SaveGame->CollectedEvidenceIds = Inventory->ExportCollectedEvidenceIds();
	SaveGame->RecordedNoteIds = NoteRecorder->ExportRecordedNoteIds();
	SaveGame->PlayerTransform = PlayerCharacter->GetActorTransform();
	if (APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController()))
	{
		SaveGame->PlayerControlRotation = PC->GetControlRotation();
	}
	return SaveGame;
}

bool UHorrorSaveSubsystem::ApplyCheckpointSnapshot(UObject* WorldContextObject, const UHorrorSaveGame* SaveGame) const
{
	if (!SaveGame || SaveGame->SaveVersion != HorrorSaveSubsystemConstants::Day1SaveVersion)
	{
		return false;
	}

	UWorld* World = ResolveWorld(WorldContextObject);
	AHorrorGameModeBase* HorrorGameMode = nullptr;
	AHorrorPlayerCharacter* PlayerCharacter = nullptr;
	UInventoryComponent* Inventory = nullptr;
	UNoteRecorderComponent* NoteRecorder = nullptr;
	if (!ResolveDay1SaveTargets(World, HorrorGameMode, PlayerCharacter, Inventory, NoteRecorder))
	{
		return false;
	}

	FHorrorFoundFootageSaveState FoundFootageSaveState;
	FoundFootageSaveState.RecordedObjectiveEvents = SaveGame->RecordedObjectiveEvents;
	FoundFootageSaveState.CompletedObjectiveStates = SaveGame->CompletedObjectiveStates;
	SanitizeFoundFootageSaveStateForDay1Restore(FoundFootageSaveState, SaveGame->CollectedEvidenceIds, SaveGame->RecordedNoteIds);
	HorrorGameMode->ImportFoundFootageSaveState(FoundFootageSaveState);
	HorrorGameMode->ImportPendingFirstAnomalyCandidate(SaveGame->PendingFirstAnomalySourceId);

	Inventory->ImportCollectedEvidenceIds(SaveGame->CollectedEvidenceIds);
	NoteRecorder->ImportRecordedNoteIds(SaveGame->RecordedNoteIds);

	PlayerCharacter->SetActorTransform(SaveGame->PlayerTransform);
	if (APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController()))
	{
		PC->SetControlRotation(SaveGame->PlayerControlRotation);
	}

	if (AHorrorPlayerController* PlayerController = Cast<AHorrorPlayerController>(UGameplayStatics::GetPlayerController(World, 0)))
	{
		PlayerController->RestampCheckpointLoadedUIState();
	}

	return true;
}

bool UHorrorSaveSubsystem::HasSave() const
{
	return CachedSave != nullptr
		|| UGameplayStatics::DoesSaveGameExist(
			HorrorSaveSubsystemConstants::Day1AutosaveSlotName,
			HorrorSaveSubsystemConstants::Day1AutosaveUserIndex);
}

void UHorrorSaveSubsystem::ClearCachedSaveOnly()
{
	CachedSave = nullptr;
}

UWorld* UHorrorSaveSubsystem::ResolveWorld(const UObject* WorldContextObject) const
{
	if (WorldContextObject)
	{
		if (UWorld* World = WorldContextObject->GetWorld())
		{
			return World;
		}

		if (GEngine)
		{
			return GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
		}
	}

	return GetWorld();
}
