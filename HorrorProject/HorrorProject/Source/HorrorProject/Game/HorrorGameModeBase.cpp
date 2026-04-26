// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorGameModeBase.h"

#include "EngineUtils.h"
#include "Game/DeepWaterStationRouteKit.h"
#include "Game/HorrorEncounterDirector.h"
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

namespace HorrorObjectiveMilestoneCheckpoints
{
	const FName FirstNoteCollected(TEXT("Checkpoint.Day1.FirstNoteCollected"));
	const FName FirstAnomalyRecorded(TEXT("Checkpoint.Day1.FirstAnomalyRecorded"));
	const FName ArchiveReviewed(TEXT("Checkpoint.Day1.ArchiveReviewed"));
	const FName ExitUnlocked(TEXT("Checkpoint.Day1.ExitUnlocked"));
}

namespace HorrorObjectiveMilestoneSources
{
	const FName Bodycam(TEXT("Bodycam"));
	const FName FirstNote(TEXT("FirstNote"));
	const FName FirstAnomaly(TEXT("FirstAnomaly"));
	const FName Archive(TEXT("Archive"));
	const FName Exit(TEXT("Exit"));
}

AHorrorGameModeBase::AHorrorGameModeBase()
{
	DefaultPawnClass = AHorrorPlayerCharacter::StaticClass();
	PlayerControllerClass = AHorrorPlayerController::StaticClass();
	RuntimeEncounterDirectorClass = AHorrorEncounterDirector::StaticClass();
}

void AHorrorGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	RegisterDefaultObjectiveMetadata();

	if (bAutoSpawnRouteKitOnBeginPlay)
	{
		EnsureRouteKit();
	}

	if (bAutoSpawnEncounterDirectorOnBeginPlay)
	{
		EnsureEncounterDirector();
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
		for (const FHorrorFoundFootageStateChange& StateChange : StateChanges)
		{
			if (StateChange.bCompleted)
			{
				HandleObjectiveStateChange(StateChange.StateTag);
			}
		}
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
		for (const FHorrorFoundFootageStateChange& StateChange : StateChanges)
		{
			if (StateChange.bCompleted)
			{
				HandleObjectiveStateChange(StateChange.StateTag);
			}
		}
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

AHorrorEncounterDirector* AHorrorGameModeBase::EnsureEncounterDirector()
{
	if (RuntimeEncounterDirector)
	{
		return RuntimeEncounterDirector.Get();
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AHorrorEncounterDirector> It(World); It; ++It)
	{
		RuntimeEncounterDirector = *It;
		return RuntimeEncounterDirector.Get();
	}

	const TSubclassOf<AHorrorEncounterDirector> SpawnClass = RuntimeEncounterDirectorClass
		? RuntimeEncounterDirectorClass
		: TSubclassOf<AHorrorEncounterDirector>(AHorrorEncounterDirector::StaticClass());

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	RuntimeEncounterDirector = World->SpawnActor<AHorrorEncounterDirector>(SpawnClass, RuntimeEncounterDirectorTransform, SpawnParameters);
	if (RuntimeEncounterDirector)
	{
		RuntimeEncounterDirector->RevealRadius = 0.0f;
	}
	return RuntimeEncounterDirector.Get();
}

void AHorrorGameModeBase::RegisterDefaultObjectiveMetadata()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	if (!EventBus)
	{
		return;
	}

	const struct FDefaultMilestoneMetadata
	{
		FGameplayTag EventTag;
		FName SourceId;
		FName TrailerBeatId;
		const TCHAR* HintFallback;
		const TCHAR* DebugLabel;
	} Defaults[] = {
		{
			HorrorFoundFootageTags::BodycamAcquiredEvent(),
			HorrorObjectiveMilestoneSources::Bodycam,
			TEXT("Trailer.Beat.BodycamAcquired"),
			TEXT("Bodycam online — keep it raised."),
			TEXT("Bodycam Acquired")
		},
		{
			HorrorFoundFootageTags::FirstNoteCollectedEvent(),
			HorrorObjectiveMilestoneSources::FirstNote,
			TEXT("Trailer.Beat.FirstNote"),
			TEXT("Note logged — find the first anomaly."),
			TEXT("First Note Collected")
		},
		{
			HorrorFoundFootageTags::FirstAnomalyRecordedEvent(),
			HorrorObjectiveMilestoneSources::FirstAnomaly,
			TEXT("Trailer.Beat.FirstAnomaly"),
			TEXT("Anomaly captured — head to the archive terminal."),
			TEXT("First Anomaly Recorded")
		},
		{
			HorrorFoundFootageTags::ArchiveReviewedEvent(),
			HorrorObjectiveMilestoneSources::Archive,
			TEXT("Trailer.Beat.ArchiveReviewed"),
			TEXT("Archive review complete — exit unlocking."),
			TEXT("Archive Reviewed")
		},
		{
			HorrorFoundFootageTags::ExitUnlockedEvent(),
			HorrorObjectiveMilestoneSources::Exit,
			TEXT("Trailer.Beat.ExitUnlocked"),
			TEXT("Exit unlocked — get out."),
			TEXT("Exit Unlocked")
		}
	};

	for (const FDefaultMilestoneMetadata& Default : Defaults)
	{
		FHorrorObjectiveMessageMetadata Metadata;
		Metadata.TrailerBeatId = Default.TrailerBeatId;
		Metadata.ObjectiveHint = FText::AsCultureInvariant(Default.HintFallback);
		Metadata.DebugLabel = FText::AsCultureInvariant(Default.DebugLabel);
		EventBus->RegisterObjectiveMetadata(Default.EventTag, Default.SourceId, Metadata);
	}
}

void AHorrorGameModeBase::HandleObjectiveStateChange(FGameplayTag StateTag)
{
	if (!StateTag.IsValid())
	{
		return;
	}

	if (AHorrorEncounterDirector* EncounterDirector = EnsureEncounterDirector())
	{
		if (StateTag == HorrorFoundFootageTags::FirstNoteCollectedState())
		{
			EncounterDirector->PrimeEncounter(NAME_None);
		}
		else if (StateTag == HorrorFoundFootageTags::FirstAnomalyRecordedState())
		{
			if (AHorrorPlayerCharacter* PlayerCharacter = ResolveLeadPlayerCharacter())
			{
				EncounterDirector->TriggerReveal(PlayerCharacter);
			}
		}
		else if (StateTag == HorrorFoundFootageTags::ArchiveReviewedState())
		{
			EncounterDirector->ResolveEncounter();
		}
	}

	if (StateTag == HorrorFoundFootageTags::FirstNoteCollectedState())
	{
		TryAutosaveOnMilestone(HorrorObjectiveMilestoneCheckpoints::FirstNoteCollected);
	}
	else if (StateTag == HorrorFoundFootageTags::FirstAnomalyRecordedState())
	{
		TryAutosaveOnMilestone(HorrorObjectiveMilestoneCheckpoints::FirstAnomalyRecorded);
	}
	else if (StateTag == HorrorFoundFootageTags::ArchiveReviewedState())
	{
		TryAutosaveOnMilestone(HorrorObjectiveMilestoneCheckpoints::ArchiveReviewed);
	}
	else if (StateTag == HorrorFoundFootageTags::ExitUnlockedState())
	{
		TryAutosaveOnMilestone(HorrorObjectiveMilestoneCheckpoints::ExitUnlocked);
	}
}

void AHorrorGameModeBase::TryAutosaveOnMilestone(FName CheckpointId)
{
	if (!bAutosaveOnObjectiveMilestone || CheckpointId.IsNone())
	{
		return;
	}

	SaveDay1Checkpoint(CheckpointId);
}
