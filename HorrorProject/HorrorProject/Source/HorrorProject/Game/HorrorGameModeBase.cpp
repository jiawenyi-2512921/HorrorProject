// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorGameModeBase.h"

#include "EngineUtils.h"
#include "Game/DeepWaterStationRouteKit.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Player/Components/VHSEffectComponent.h"
#include "Player/HorrorPlayerController.h"
#include "Save/HorrorSaveSubsystem.h"

namespace
{
	void PublishFoundFootageEvents(
		UWorld* World,
		const FHorrorFoundFootageContract& Contract,
		const TArray<FHorrorFoundFootageRecordedEvent>& RecordedEvents,
		UObject* SourceObject)
	{
		if (!World)
		{
			return;
		}

		UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
		if (!EventBus)
		{
			return;
		}

		for (const FHorrorFoundFootageRecordedEvent& RecordedEvent : RecordedEvents)
		{
			EventBus->Publish(RecordedEvent.EventTag, RecordedEvent.SourceId, Contract.GetStateForEvent(RecordedEvent.EventTag), SourceObject);
		}
	}
}

AHorrorGameModeBase::AHorrorGameModeBase()
{
	DefaultPawnClass = AHorrorPlayerCharacter::StaticClass();
	PlayerControllerClass = AHorrorPlayerController::StaticClass();
}

void AHorrorGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoSpawnRouteKitOnBeginPlay)
	{
		EnsureRouteKit();
	}
}

bool AHorrorGameModeBase::TryAcquireBodycam(FName SourceId, bool bEnableBodycam)
{
	const FName ResolvedSourceId = SourceId.IsNone() ? FName(TEXT("Bodycam")) : SourceId;
	const bool bRecorded = RecordFoundFootageEvent(HorrorFoundFootageTags::BodycamAcquiredEvent(), ResolvedSourceId);

	if (AHorrorPlayerCharacter* PlayerCharacter = ResolveLeadPlayerCharacter())
	{
		if (UVHSEffectComponent* VHSEffect = PlayerCharacter->GetVHSEffectComponent())
		{
			VHSEffect->SetBodycamAcquired(true);
			VHSEffect->SetBodycamEnabled(bEnableBodycam);
		}

		if (UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent())
		{
			QuantumCamera->SetCameraAcquired(true);
			QuantumCamera->SetCameraEnabled(bEnableBodycam);
		}
	}

	return bRecorded;
}

bool AHorrorGameModeBase::TryCollectFirstNote(FName SourceId)
{
	if (!CanCollectFirstNote())
	{
		return false;
	}

	return RecordFoundFootageEvent(
		HorrorFoundFootageTags::FirstNoteCollectedEvent(),
		SourceId.IsNone() ? FName(TEXT("FirstNote")) : SourceId);
}

bool AHorrorGameModeBase::BeginFirstAnomalyCandidate(FName SourceId)
{
	if (!CanBeginFirstAnomalyCandidate())
	{
		return false;
	}

	return AnomalyDirector.BeginFirstAnomalyCandidate(SourceId, FoundFootageContract);
}

bool AHorrorGameModeBase::TryRecordFirstAnomaly(bool bIsRecording)
{
	if (!CanRecordFirstAnomaly(bIsRecording))
	{
		return false;
	}

	TArray<FHorrorFoundFootageRecordedEvent> RecordedEvents;
	TArray<FHorrorFoundFootageStateChange> StateChanges;
	const bool bRecorded = AnomalyDirector.TryRecordFirstAnomaly(bIsRecording, FoundFootageContract, &RecordedEvents, &StateChanges);
	if (bRecorded)
	{
		PublishFoundFootageEvents(GetWorld(), FoundFootageContract, RecordedEvents, this);
	}
	return bRecorded;
}

bool AHorrorGameModeBase::TryReviewArchive(FName SourceId)
{
	if (!CanReviewArchive())
	{
		return false;
	}

	return RecordFoundFootageEvent(
		HorrorFoundFootageTags::ArchiveReviewedEvent(),
		SourceId.IsNone() ? FName(TEXT("Archive")) : SourceId);
}

bool AHorrorGameModeBase::HasBodycamAcquired() const
{
	return FoundFootageContract.HasRecordedEvent(HorrorFoundFootageTags::BodycamAcquiredEvent());
}

bool AHorrorGameModeBase::HasCollectedFirstNote() const
{
	return FoundFootageContract.HasRecordedEvent(HorrorFoundFootageTags::FirstNoteCollectedEvent());
}

bool AHorrorGameModeBase::HasRecordedFirstAnomaly() const
{
	return FoundFootageContract.HasRecordedEvent(HorrorFoundFootageTags::FirstAnomalyRecordedEvent());
}

bool AHorrorGameModeBase::HasPendingFirstAnomalyCandidate() const
{
	return AnomalyDirector.HasPendingFirstAnomalyCandidate();
}

FName AHorrorGameModeBase::GetPendingFirstAnomalySourceId() const
{
	return AnomalyDirector.GetPendingFirstAnomalySourceId();
}

bool AHorrorGameModeBase::HasReviewedArchive() const
{
	return FoundFootageContract.HasRecordedEvent(HorrorFoundFootageTags::ArchiveReviewedEvent());
}

bool AHorrorGameModeBase::HasFoundFootageRecordedEvent(FGameplayTag EventTag) const
{
	return FoundFootageContract.HasRecordedEvent(EventTag);
}

bool AHorrorGameModeBase::HasFoundFootageCompletedState(FGameplayTag StateTag) const
{
	return FoundFootageContract.HasCompletedState(StateTag);
}

bool AHorrorGameModeBase::CanCollectFirstNote() const
{
	return HasBodycamAcquired() && !HasCollectedFirstNote();
}

bool AHorrorGameModeBase::CanBeginFirstAnomalyCandidate() const
{
	return HasCollectedFirstNote()
		&& !HasPendingFirstAnomalyCandidate()
		&& !HasRecordedFirstAnomaly();
}

bool AHorrorGameModeBase::CanRecordFirstAnomaly(bool bIsRecording) const
{
	return bIsRecording
		&& HasPendingFirstAnomalyCandidate()
		&& !HasRecordedFirstAnomaly();
}

bool AHorrorGameModeBase::CanReviewArchive() const
{
	return HasRecordedFirstAnomaly() && !HasReviewedArchive();
}

bool AHorrorGameModeBase::IsExitUnlocked() const
{
	return FoundFootageContract.IsExitUnlocked();
}

FHorrorFoundFootageProgressSnapshot AHorrorGameModeBase::BuildFoundFootageProgressSnapshot() const
{
	return FoundFootageContract.BuildSnapshot();
}

const FHorrorFoundFootageContract& AHorrorGameModeBase::GetFoundFootageContract() const
{
	return FoundFootageContract;
}

FHorrorFoundFootageSaveState AHorrorGameModeBase::ExportFoundFootageSaveState() const
{
	return FoundFootageContract.ExportSaveState();
}

void AHorrorGameModeBase::ImportFoundFootageSaveState(const FHorrorFoundFootageSaveState& SaveState)
{
	FoundFootageContract.ImportSaveState(SaveState);
	AnomalyDirector.ImportPendingFirstAnomalyCandidate(NAME_None, FoundFootageContract);
	SyncFoundFootageRuntimeStateToPlayer();
}

void AHorrorGameModeBase::ImportPendingFirstAnomalyCandidate(FName SourceId)
{
	AnomalyDirector.ImportPendingFirstAnomalyCandidate(SourceId, FoundFootageContract);
}

void AHorrorGameModeBase::SyncFoundFootageRuntimeStateToPlayer()
{
	AHorrorPlayerCharacter* PlayerCharacter = ResolveLeadPlayerCharacter();
	if (!PlayerCharacter)
	{
		return;
	}

	const bool bHasBodycam = HasBodycamAcquired();
	if (UVHSEffectComponent* VHSEffect = PlayerCharacter->GetVHSEffectComponent())
	{
		VHSEffect->SetBodycamAcquired(bHasBodycam);
		VHSEffect->SetBodycamEnabled(bHasBodycam);
	}

	if (UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent())
	{
		QuantumCamera->SetCameraAcquired(bHasBodycam);
		QuantumCamera->SetCameraEnabled(bHasBodycam);
	}
}

bool AHorrorGameModeBase::SaveDay1Checkpoint(FName CheckpointId)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UHorrorSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UHorrorSaveSubsystem>())
		{
			return SaveSubsystem->SaveCheckpoint(this, CheckpointId);
		}
	}

	return false;
}

bool AHorrorGameModeBase::LoadDay1Checkpoint()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UHorrorSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UHorrorSaveSubsystem>())
		{
			return SaveSubsystem->LoadCheckpoint(this);
		}
	}

	return false;
}

bool AHorrorGameModeBase::RecordFoundFootageEvent(FGameplayTag EventTag, FName SourceId)
{
	TArray<FHorrorFoundFootageRecordedEvent> RecordedEvents;
	TArray<FHorrorFoundFootageStateChange> StateChanges;
	const bool bRecorded = FoundFootageContract.RecordEvent(EventTag, SourceId, &RecordedEvents, &StateChanges);
	if (bRecorded)
	{
		PublishFoundFootageEvents(GetWorld(), FoundFootageContract, RecordedEvents, this);
	}
	return bRecorded;
}

AHorrorPlayerCharacter* AHorrorGameModeBase::ResolveLeadPlayerCharacter() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	if (AHorrorPlayerCharacter* PlayerCharacter = Cast<AHorrorPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0)))
	{
		return PlayerCharacter;
	}

	for (TActorIterator<AHorrorPlayerCharacter> PlayerIt(World); PlayerIt; ++PlayerIt)
	{
		return *PlayerIt;
	}

	return nullptr;
}

ADeepWaterStationRouteKit* AHorrorGameModeBase::EnsureRouteKit()
{
	if (RuntimeRouteKit)
	{
		return RuntimeRouteKit.Get();
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<ADeepWaterStationRouteKit> It(World); It; ++It)
	{
		RuntimeRouteKit = *It;
		return RuntimeRouteKit.Get();
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	RuntimeRouteKit = World->SpawnActor<ADeepWaterStationRouteKit>(ADeepWaterStationRouteKit::StaticClass(), RuntimeRouteKitTransform, SpawnParameters);
	return RuntimeRouteKit.Get();
}

bool AHorrorGameModeBase::IsLeadPlayerRecording() const
{
	const AHorrorPlayerCharacter* PlayerCharacter = ResolveLeadPlayerCharacter();
	const UQuantumCameraComponent* QuantumCamera = PlayerCharacter ? PlayerCharacter->GetQuantumCameraComponent() : nullptr;
	return QuantumCamera && QuantumCamera->IsCameraMode(EQuantumCameraMode::Recording);
}
