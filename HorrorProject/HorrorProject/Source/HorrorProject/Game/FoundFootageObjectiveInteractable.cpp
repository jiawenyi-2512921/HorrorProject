// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/FoundFootageObjectiveInteractable.h"

#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Game/DeepWaterStationRouteKit.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorFoundFootageContract.h"
#include "Game/HorrorGameModeBase.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/InventoryComponent.h"
#include "Player/Components/NoteRecorderComponent.h"

namespace
{
	const FName RequiredBodycamEvidenceId(TEXT("Evidence.Bodycam"));
	const FName RequiredFirstAnomalyEvidenceId(TEXT("Evidence.Anomaly01"));
	const FName RequiredFirstNoteId(TEXT("Note.Intro"));
}

AFoundFootageObjectiveInteractable::AFoundFootageObjectiveInteractable()
{
	PrimaryActorTick.bCanEverTick = false;

	InteractionBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBounds"));
	InteractionBounds->SetBoxExtent(FVector(32.0f, 32.0f, 32.0f));
	InteractionBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBounds->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBounds->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	RootComponent = InteractionBounds;
}

bool AFoundFootageObjectiveInteractable::CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const
{
	(void)Hit;

	return CanCompleteObjectiveForInstigator(ResolveObjectiveGameMode(), InstigatorActor);
}

bool AFoundFootageObjectiveInteractable::Interact_Implementation(AActor* InstigatorActor, const FHitResult& Hit)
{
	(void)Hit;

	AHorrorGameModeBase* GameMode = ResolveObjectiveGameMode();
	FName ProgressId = ResolveSourceId();
	if (Objective == EFoundFootageInteractableObjective::FirstAnomalyRecord && GameMode)
	{
		ProgressId = GameMode->GetPendingFirstAnomalySourceId();
	}

	const bool bCompleted = TryCompleteObjectiveForInstigator(GameMode, InstigatorActor);
	if (bCompleted)
	{
		RecordInstigatorProgress(InstigatorActor, ProgressId);
		if (ADeepWaterStationRouteKit* OwningRouteKit = Cast<ADeepWaterStationRouteKit>(GetOwner()))
		{
			OwningRouteKit->HandleObjectiveCompleted(Objective, InstigatorActor);
		}
	}
	return bCompleted;
}

bool AFoundFootageObjectiveInteractable::CanCompleteObjective(AHorrorGameModeBase* GameMode) const
{
	return CanCompleteObjectiveForInstigator(GameMode, nullptr);
}

bool AFoundFootageObjectiveInteractable::CanCompleteObjectiveForInstigator(AHorrorGameModeBase* GameMode, AActor* InstigatorActor) const
{
	if (!GameMode)
	{
		return false;
	}

	const ADeepWaterStationRouteKit* OwningRouteKit = Cast<ADeepWaterStationRouteKit>(GetOwner());
	const bool bRouteGatedByEncounter = OwningRouteKit && OwningRouteKit->IsRouteGatedByEncounter();

	switch (Objective)
	{
	case EFoundFootageInteractableObjective::Bodycam:
		return !GameMode->HasBodycamAcquired();

	case EFoundFootageInteractableObjective::FirstNote:
		return GameMode->CanCollectFirstNote();

	case EFoundFootageInteractableObjective::FirstAnomalyCandidate:
		return !ResolveSourceId().IsNone() && GameMode->CanBeginFirstAnomalyCandidate();

	case EFoundFootageInteractableObjective::FirstAnomalyRecord:
		return GameMode->CanRecordFirstAnomaly(GameMode->IsLeadPlayerRecording());

	case EFoundFootageInteractableObjective::ArchiveReview:
	{
		if (!GameMode->CanReviewArchive())
		{
			return false;
		}

		if (!InstigatorActor)
		{
			return false;
		}

		if (OwningRouteKit && !OwningRouteKit->CanTriggerEncounterReveal(InstigatorActor))
		{
			return false;
		}

		const AHorrorPlayerCharacter* PlayerCharacter = Cast<AHorrorPlayerCharacter>(InstigatorActor);
		const UInventoryComponent* Inventory = PlayerCharacter ? PlayerCharacter->GetInventoryComponent() : nullptr;
		const UNoteRecorderComponent* NoteRecorder = PlayerCharacter ? PlayerCharacter->GetNoteRecorderComponent() : nullptr;
		return Inventory
			&& NoteRecorder
			&& Inventory->HasCollectedEvidenceId(RequiredBodycamEvidenceId)
			&& Inventory->HasCollectedEvidenceId(RequiredFirstAnomalyEvidenceId)
			&& NoteRecorder->HasRecordedNoteId(RequiredFirstNoteId);
	}

	case EFoundFootageInteractableObjective::ExitRouteGate:
	{
		if (bRouteGatedByEncounter)
		{
			return GameMode->IsExitUnlocked() && InstigatorActor && OwningRouteKit->CanResolveEncounterAtExit();
		}
		return GameMode->IsExitUnlocked();
	}
	}

	return false;
}

bool AFoundFootageObjectiveInteractable::TryCompleteObjective(AHorrorGameModeBase* GameMode) const
{
	return TryCompleteObjectiveForInstigator(GameMode, nullptr);
}

bool AFoundFootageObjectiveInteractable::TryCompleteObjectiveForInstigator(AHorrorGameModeBase* GameMode, AActor* InstigatorActor) const
{
	if (!CanCompleteObjectiveForInstigator(GameMode, InstigatorActor))
	{
		return false;
	}

	switch (Objective)
	{
	case EFoundFootageInteractableObjective::Bodycam:
	{
		const FName ProgressId = ResolveSourceId();
		RegisterObjectiveEventMetadataForCompletedObjective(GameMode, ProgressId);
		const bool bCompleted = GameMode->TryAcquireBodycam(ProgressId, bEnableBodycamOnInteract);
		if (!bCompleted)
		{
			UnregisterObjectiveEventMetadataForCompletedObjective(GameMode, ProgressId);
		}
		return bCompleted;
	}

	case EFoundFootageInteractableObjective::FirstNote:
	{
		const FName ProgressId = ResolveSourceId();
		RegisterObjectiveEventMetadataForCompletedObjective(GameMode, ProgressId);
		const bool bCompleted = GameMode->TryCollectFirstNote(ProgressId);
		if (!bCompleted)
		{
			UnregisterObjectiveEventMetadataForCompletedObjective(GameMode, ProgressId);
		}
		return bCompleted;
	}

	case EFoundFootageInteractableObjective::FirstAnomalyCandidate:
	{
		const FName ProgressId = ResolveSourceId();
		const bool bCompleted = GameMode->BeginFirstAnomalyCandidate(ProgressId);
		if (bCompleted)
		{
			RegisterObjectiveEventMetadata(ProgressId);
		}
		return bCompleted;
	}

	case EFoundFootageInteractableObjective::FirstAnomalyRecord:
	{
		const FName ProgressId = GameMode->GetPendingFirstAnomalySourceId();
		RegisterObjectiveEventMetadataForCompletedObjective(GameMode, ProgressId);
		const bool bCompleted = GameMode->TryRecordFirstAnomaly(GameMode->IsLeadPlayerRecording());
		if (!bCompleted)
		{
			UnregisterObjectiveEventMetadataForCompletedObjective(GameMode, ProgressId);
		}
		return bCompleted;
	}

	case EFoundFootageInteractableObjective::ArchiveReview:
	{
		const FName ProgressId = ResolveSourceId();
		RegisterObjectiveEventMetadataForCompletedObjective(GameMode, ProgressId);
		const bool bCompleted = GameMode->TryReviewArchive(ProgressId);
		if (!bCompleted)
		{
			UnregisterObjectiveEventMetadataForCompletedObjective(GameMode, ProgressId);
		}
		return bCompleted;
	}

	case EFoundFootageInteractableObjective::ExitRouteGate:
	{
		ADeepWaterStationRouteKit* OwningRouteKit = Cast<ADeepWaterStationRouteKit>(GetOwner());
		if (OwningRouteKit && OwningRouteKit->CanResolveEncounterAtExit())
		{
			return GameMode->IsExitUnlocked();
		}
		return GameMode->IsExitUnlocked();
	}
	}

	return false;
}

void AFoundFootageObjectiveInteractable::RecordInstigatorProgress(AActor* InstigatorActor, FName ProgressId) const
{
	AHorrorPlayerCharacter* PlayerCharacter = Cast<AHorrorPlayerCharacter>(InstigatorActor);
	if (!PlayerCharacter)
	{
		return;
	}

	if (ProgressId.IsNone())
	{
		return;
	}

	switch (Objective)
	{
	case EFoundFootageInteractableObjective::Bodycam:
	case EFoundFootageInteractableObjective::FirstAnomalyRecord:
		if (UInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent())
		{
			if (!EvidenceMetadata.EvidenceId.IsNone() && EvidenceMetadata.EvidenceId == ProgressId)
			{
				Inventory->RegisterEvidenceMetadata(EvidenceMetadata);
			}
			Inventory->AddCollectedEvidenceId(ProgressId);
		}
		break;

	case EFoundFootageInteractableObjective::FirstNote:
		if (UNoteRecorderComponent* NoteRecorder = PlayerCharacter->GetNoteRecorderComponent())
		{
			if (!NoteMetadata.NoteId.IsNone() && NoteMetadata.NoteId == ProgressId)
			{
				NoteRecorder->RegisterNoteMetadata(NoteMetadata);
			}
			NoteRecorder->AddRecordedNoteId(ProgressId);
		}
		break;

	case EFoundFootageInteractableObjective::FirstAnomalyCandidate:
		if (UInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent())
		{
			if (!EvidenceMetadata.EvidenceId.IsNone() && EvidenceMetadata.EvidenceId == ProgressId)
			{
				Inventory->RegisterEvidenceMetadata(EvidenceMetadata);
			}
		}
		break;

	case EFoundFootageInteractableObjective::ArchiveReview:
		if (UInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent())
		{
			if (!EvidenceMetadata.EvidenceId.IsNone() && EvidenceMetadata.EvidenceId == ProgressId)
			{
				Inventory->RegisterEvidenceMetadata(EvidenceMetadata);
			}
			Inventory->AddCollectedEvidenceId(ProgressId);
		}
		break;

	case EFoundFootageInteractableObjective::ExitRouteGate:
		break;
	}
}

void AFoundFootageObjectiveInteractable::RegisterObjectiveEventMetadata(FName ProgressId) const
{
	if (ProgressId.IsNone() || (TrailerBeatId.IsNone() && ObjectiveHint.IsEmpty() && DebugLabel.IsEmpty()))
	{
		return;
	}

	UWorld* World = GetWorld();
	UHorrorEventBusSubsystem* EventBus = World ? World->GetSubsystem<UHorrorEventBusSubsystem>() : nullptr;
	if (!EventBus)
	{
		return;
	}

	FHorrorObjectiveMessageMetadata Metadata;
	Metadata.TrailerBeatId = TrailerBeatId;
	Metadata.ObjectiveHint = ObjectiveHint;
	Metadata.DebugLabel = DebugLabel;
	EventBus->RegisterObjectiveMetadata(ProgressId, Metadata);
}

void AFoundFootageObjectiveInteractable::RegisterObjectiveEventMetadataForCompletedObjective(AHorrorGameModeBase* GameMode, FName ProgressId) const
{
	UWorld* World = GetWorld();
	UHorrorEventBusSubsystem* EventBus = World ? World->GetSubsystem<UHorrorEventBusSubsystem>() : nullptr;
	if (!GameMode || !EventBus || ProgressId.IsNone() || (TrailerBeatId.IsNone() && ObjectiveHint.IsEmpty() && DebugLabel.IsEmpty()))
	{
		return;
	}

	FHorrorObjectiveMessageMetadata Metadata;
	Metadata.TrailerBeatId = TrailerBeatId;
	Metadata.ObjectiveHint = ObjectiveHint;
	Metadata.DebugLabel = DebugLabel;

	FGameplayTag EventTag;
	switch (Objective)
	{
	case EFoundFootageInteractableObjective::Bodycam:
		EventTag = HorrorFoundFootageTags::BodycamAcquiredEvent();
		break;
	case EFoundFootageInteractableObjective::FirstNote:
		EventTag = HorrorFoundFootageTags::FirstNoteCollectedEvent();
		break;
	case EFoundFootageInteractableObjective::FirstAnomalyRecord:
		EventTag = HorrorFoundFootageTags::FirstAnomalyRecordedEvent();
		break;
	case EFoundFootageInteractableObjective::ArchiveReview:
		EventTag = HorrorFoundFootageTags::ArchiveReviewedEvent();
		break;
	default:
		break;
	}

	if (EventTag.IsValid())
	{
		EventBus->RegisterObjectiveMetadata(EventTag, ProgressId, Metadata);
	}
}

void AFoundFootageObjectiveInteractable::UnregisterObjectiveEventMetadataForCompletedObjective(AHorrorGameModeBase* GameMode, FName ProgressId) const
{
	UWorld* World = GetWorld();
	UHorrorEventBusSubsystem* EventBus = World ? World->GetSubsystem<UHorrorEventBusSubsystem>() : nullptr;
	if (!GameMode || !EventBus || ProgressId.IsNone())
	{
		return;
	}

	FGameplayTag EventTag;
	switch (Objective)
	{
	case EFoundFootageInteractableObjective::Bodycam:
		EventTag = HorrorFoundFootageTags::BodycamAcquiredEvent();
		break;
	case EFoundFootageInteractableObjective::FirstNote:
		EventTag = HorrorFoundFootageTags::FirstNoteCollectedEvent();
		break;
	case EFoundFootageInteractableObjective::FirstAnomalyRecord:
		EventTag = HorrorFoundFootageTags::FirstAnomalyRecordedEvent();
		break;
	case EFoundFootageInteractableObjective::ArchiveReview:
		EventTag = HorrorFoundFootageTags::ArchiveReviewedEvent();
		break;
	default:
		break;
	}

	if (EventTag.IsValid())
	{
		EventBus->UnregisterObjectiveMetadata(EventTag, ProgressId);
	}
}

AHorrorGameModeBase* AFoundFootageObjectiveInteractable::ResolveObjectiveGameMode() const
{
	UWorld* World = GetWorld();
	return World ? World->GetAuthGameMode<AHorrorGameModeBase>() : nullptr;
}

FName AFoundFootageObjectiveInteractable::ResolveSourceId() const
{
	return SourceId.IsNone() ? GetFName() : SourceId;
}
