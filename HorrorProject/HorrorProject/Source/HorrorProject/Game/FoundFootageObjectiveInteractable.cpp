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
	const FVector DefaultInteractionBoundsExtent(32.0f, 32.0f, 32.0f);
}

AFoundFootageObjectiveInteractable::AFoundFootageObjectiveInteractable()
{
	PrimaryActorTick.bCanEverTick = false;

	InteractionBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBounds"));
	InteractionBounds->SetBoxExtent(DefaultInteractionBoundsExtent);
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
		return CanCompleteArchiveReview(*GameMode, InstigatorActor);

	case EFoundFootageInteractableObjective::ExitRouteGate:
		return CanCompleteExitRouteGate(*GameMode, InstigatorActor);
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
		return TryCompleteBodycamObjective(*GameMode);

	case EFoundFootageInteractableObjective::FirstNote:
		return TryCompleteFirstNoteObjective(*GameMode);

	case EFoundFootageInteractableObjective::FirstAnomalyCandidate:
		return TryCompleteFirstAnomalyCandidateObjective(*GameMode);

	case EFoundFootageInteractableObjective::FirstAnomalyRecord:
		return TryCompleteFirstAnomalyRecordObjective(*GameMode);

	case EFoundFootageInteractableObjective::ArchiveReview:
		return TryCompleteArchiveReviewObjective(*GameMode);

	case EFoundFootageInteractableObjective::ExitRouteGate:
		return TryCompleteExitRouteGateObjective(*GameMode);
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
		RecordEvidenceProgress(*PlayerCharacter, ProgressId, true);
		break;

	case EFoundFootageInteractableObjective::FirstNote:
		RecordNoteProgress(*PlayerCharacter, ProgressId);
		break;

	case EFoundFootageInteractableObjective::FirstAnomalyCandidate:
		RecordEvidenceProgress(*PlayerCharacter, ProgressId, false);
		break;

	case EFoundFootageInteractableObjective::ArchiveReview:
		RecordEvidenceProgress(*PlayerCharacter, ProgressId, true);
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

	const FGameplayTag EventTag = ResolveCompletedObjectiveEventTag();
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

	const FGameplayTag EventTag = ResolveCompletedObjectiveEventTag();
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

ADeepWaterStationRouteKit* AFoundFootageObjectiveInteractable::ResolveOwningRouteKit() const
{
	return Cast<ADeepWaterStationRouteKit>(GetOwner());
}

bool AFoundFootageObjectiveInteractable::CanCompleteArchiveReview(AHorrorGameModeBase& GameMode, AActor* InstigatorActor) const
{
	if (!GameMode.CanReviewArchive() || !InstigatorActor)
	{
		return false;
	}

	const ADeepWaterStationRouteKit* OwningRouteKit = ResolveOwningRouteKit();
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

bool AFoundFootageObjectiveInteractable::CanCompleteExitRouteGate(AHorrorGameModeBase& GameMode, AActor* InstigatorActor) const
{
	const ADeepWaterStationRouteKit* OwningRouteKit = ResolveOwningRouteKit();
	if (OwningRouteKit && OwningRouteKit->IsRouteGatedByEncounter())
	{
		return GameMode.IsExitUnlocked() && InstigatorActor && OwningRouteKit->CanResolveEncounterAtExit();
	}

	return GameMode.IsExitUnlocked();
}

bool AFoundFootageObjectiveInteractable::TryCompleteBodycamObjective(AHorrorGameModeBase& GameMode) const
{
	const FName ProgressId = ResolveSourceId();
	RegisterObjectiveEventMetadataForCompletedObjective(&GameMode, ProgressId);
	const bool bCompleted = GameMode.TryAcquireBodycam(ProgressId, bEnableBodycamOnInteract);
	if (!bCompleted)
	{
		UnregisterObjectiveEventMetadataForCompletedObjective(&GameMode, ProgressId);
	}
	return bCompleted;
}

bool AFoundFootageObjectiveInteractable::TryCompleteFirstNoteObjective(AHorrorGameModeBase& GameMode) const
{
	const FName ProgressId = ResolveSourceId();
	RegisterObjectiveEventMetadataForCompletedObjective(&GameMode, ProgressId);
	const bool bCompleted = GameMode.TryCollectFirstNote(ProgressId);
	if (!bCompleted)
	{
		UnregisterObjectiveEventMetadataForCompletedObjective(&GameMode, ProgressId);
	}
	return bCompleted;
}

bool AFoundFootageObjectiveInteractable::TryCompleteFirstAnomalyCandidateObjective(AHorrorGameModeBase& GameMode) const
{
	const FName ProgressId = ResolveSourceId();
	const bool bCompleted = GameMode.BeginFirstAnomalyCandidate(ProgressId);
	if (bCompleted)
	{
		RegisterObjectiveEventMetadata(ProgressId);
	}
	return bCompleted;
}

bool AFoundFootageObjectiveInteractable::TryCompleteFirstAnomalyRecordObjective(AHorrorGameModeBase& GameMode) const
{
	const FName ProgressId = GameMode.GetPendingFirstAnomalySourceId();
	RegisterObjectiveEventMetadataForCompletedObjective(&GameMode, ProgressId);
	const bool bCompleted = GameMode.TryRecordFirstAnomaly(GameMode.IsLeadPlayerRecording());
	if (!bCompleted)
	{
		UnregisterObjectiveEventMetadataForCompletedObjective(&GameMode, ProgressId);
	}
	return bCompleted;
}

bool AFoundFootageObjectiveInteractable::TryCompleteArchiveReviewObjective(AHorrorGameModeBase& GameMode) const
{
	const FName ProgressId = ResolveSourceId();
	RegisterObjectiveEventMetadataForCompletedObjective(&GameMode, ProgressId);
	const bool bCompleted = GameMode.TryReviewArchive(ProgressId);
	if (!bCompleted)
	{
		UnregisterObjectiveEventMetadataForCompletedObjective(&GameMode, ProgressId);
	}
	return bCompleted;
}

bool AFoundFootageObjectiveInteractable::TryCompleteExitRouteGateObjective(AHorrorGameModeBase& GameMode) const
{
	if (const ADeepWaterStationRouteKit* OwningRouteKit = ResolveOwningRouteKit())
	{
		if (OwningRouteKit->CanResolveEncounterAtExit())
		{
			return GameMode.IsExitUnlocked();
		}
	}

	return GameMode.IsExitUnlocked();
}

void AFoundFootageObjectiveInteractable::RecordEvidenceProgress(AHorrorPlayerCharacter& PlayerCharacter, FName ProgressId, bool bMarkCollected) const
{
	UInventoryComponent* Inventory = PlayerCharacter.GetInventoryComponent();
	if (!Inventory)
	{
		return;
	}

	if (!EvidenceMetadata.EvidenceId.IsNone() && EvidenceMetadata.EvidenceId == ProgressId)
	{
		Inventory->RegisterEvidenceMetadata(EvidenceMetadata);
	}

	if (bMarkCollected)
	{
		Inventory->AddCollectedEvidenceId(ProgressId);
	}
}

void AFoundFootageObjectiveInteractable::RecordNoteProgress(AHorrorPlayerCharacter& PlayerCharacter, FName ProgressId) const
{
	UNoteRecorderComponent* NoteRecorder = PlayerCharacter.GetNoteRecorderComponent();
	if (!NoteRecorder)
	{
		return;
	}

	if (!NoteMetadata.NoteId.IsNone() && NoteMetadata.NoteId == ProgressId)
	{
		NoteRecorder->RegisterNoteMetadata(NoteMetadata);
	}
	NoteRecorder->AddRecordedNoteId(ProgressId);
}

FGameplayTag AFoundFootageObjectiveInteractable::ResolveCompletedObjectiveEventTag() const
{
	switch (Objective)
	{
	case EFoundFootageInteractableObjective::Bodycam:
		return HorrorFoundFootageTags::BodycamAcquiredEvent();
	case EFoundFootageInteractableObjective::FirstNote:
		return HorrorFoundFootageTags::FirstNoteCollectedEvent();
	case EFoundFootageInteractableObjective::FirstAnomalyRecord:
		return HorrorFoundFootageTags::FirstAnomalyRecordedEvent();
	case EFoundFootageInteractableObjective::ArchiveReview:
		return HorrorFoundFootageTags::ArchiveReviewedEvent();
	default:
		break;
	}

	return FGameplayTag();
}
