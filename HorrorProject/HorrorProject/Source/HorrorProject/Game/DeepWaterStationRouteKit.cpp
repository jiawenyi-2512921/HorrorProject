// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/DeepWaterStationRouteKit.h"

#include "AI/HorrorThreatCharacter.h"
#include "Components/SceneComponent.h"
#include "Containers/Set.h"
#include "Engine/World.h"
#include "Game/HorrorEncounterDirector.h"

ADeepWaterStationRouteKit::ADeepWaterStationRouteKit()
{
	PrimaryActorTick.bCanEverTick = false;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
	ObjectiveInteractableClass = AFoundFootageObjectiveInteractable::StaticClass();
	EncounterDirectorClass = AHorrorEncounterDirector::StaticClass();
	EncounterThreatClass = AHorrorThreatCharacter::StaticClass();
}

void ADeepWaterStationRouteKit::BeginPlay()
{
	Super::BeginPlay();
	if (ObjectiveNodes.IsEmpty())
	{
		ConfigureDefaultFirstLoopObjectiveNodes();
	}
	SpawnObjectiveNodes();
	SpawnEncounterDirector();
}

void ADeepWaterStationRouteKit::ConfigureDefaultFirstLoopObjectiveNodes()
{
	ObjectiveNodes = {
		{ EFoundFootageInteractableObjective::Bodycam, TEXT("Evidence.Bodycam"), FTransform(FRotator::ZeroRotator, FVector(200.0f, 0.0f, 80.0f)) },
		{ EFoundFootageInteractableObjective::FirstNote, TEXT("Note.Intro"), FTransform(FRotator::ZeroRotator, FVector(600.0f, 0.0f, 80.0f)) },
		{ EFoundFootageInteractableObjective::FirstAnomalyCandidate, TEXT("Evidence.Anomaly01"), FTransform(FRotator::ZeroRotator, FVector(1000.0f, 0.0f, 80.0f)) },
		{ EFoundFootageInteractableObjective::FirstAnomalyRecord, TEXT("Evidence.Recorder"), FTransform(FRotator::ZeroRotator, FVector(1400.0f, 0.0f, 80.0f)), true, true },
		{ EFoundFootageInteractableObjective::ArchiveReview, TEXT("Archive.Terminal"), FTransform(FRotator::ZeroRotator, FVector(1800.0f, 0.0f, 80.0f)) },
		{ EFoundFootageInteractableObjective::ExitRouteGate, TEXT("Exit.Gate"), FTransform(FRotator::ZeroRotator, FVector(2200.0f, 0.0f, 80.0f)) }
	};
	ObjectiveNodes[0].EvidenceMetadata.EvidenceId = TEXT("Evidence.Bodycam");
	ObjectiveNodes[0].EvidenceMetadata.DisplayName = FText::FromString(TEXT("Bodycam"));
	ObjectiveNodes[0].TrailerBeatId = TEXT("Beat.BodycamAcquire");
	ObjectiveNodes[0].ObjectiveHint = FText::FromString(TEXT("Recover the bodycam."));
	ObjectiveNodes[0].DebugLabel = FText::FromString(TEXT("Bodycam Pickup"));
	ObjectiveNodes[1].NoteMetadata.NoteId = TEXT("Note.Intro");
	ObjectiveNodes[1].NoteMetadata.Title = FText::FromString(TEXT("Intro"));
	ObjectiveNodes[1].TrailerBeatId = TEXT("Beat.FirstNote");
	ObjectiveNodes[1].ObjectiveHint = FText::FromString(TEXT("Read the first station note."));
	ObjectiveNodes[1].DebugLabel = FText::FromString(TEXT("Intro Note"));
	ObjectiveNodes[2].EvidenceMetadata.EvidenceId = TEXT("Evidence.Anomaly01");
	ObjectiveNodes[2].EvidenceMetadata.DisplayName = FText::FromString(TEXT("First Anomaly"));
	ObjectiveNodes[2].TrailerBeatId = TEXT("Beat.FirstAnomalyCandidate");
	ObjectiveNodes[2].ObjectiveHint = FText::FromString(TEXT("Frame the first anomaly."));
	ObjectiveNodes[2].DebugLabel = FText::FromString(TEXT("Anomaly Candidate"));
	ObjectiveNodes[3].EvidenceMetadata.EvidenceId = TEXT("Evidence.Anomaly01");
	ObjectiveNodes[3].EvidenceMetadata.DisplayName = FText::FromString(TEXT("First Anomaly"));
	ObjectiveNodes[3].TrailerBeatId = TEXT("Beat.FirstAnomalyRecord");
	ObjectiveNodes[3].ObjectiveHint = FText::FromString(TEXT("Record while the anomaly is visible."));
	ObjectiveNodes[3].DebugLabel = FText::FromString(TEXT("Anomaly Recording Window"));
	ObjectiveNodes[4].TrailerBeatId = TEXT("Beat.ArchiveReview");
	ObjectiveNodes[4].ObjectiveHint = FText::FromString(TEXT("Review the tape at the archive terminal."));
	ObjectiveNodes[4].DebugLabel = FText::FromString(TEXT("Archive Terminal"));
	ObjectiveNodes[5].TrailerBeatId = TEXT("Beat.ExitGate");
	ObjectiveNodes[5].ObjectiveHint = FText::FromString(TEXT("Leave through the unlocked service gate."));
	ObjectiveNodes[5].DebugLabel = FText::FromString(TEXT("Exit Gate"));
}

bool ADeepWaterStationRouteKit::ValidateObjectiveNodes(TArray<FText>& ValidationErrors) const
{
	ValidationErrors.Reset();

	if (!ObjectiveInteractableClass)
	{
		ValidationErrors.Add(FText::FromString(TEXT("ObjectiveInteractableClass is not set.")));
	}

	if (!EncounterDirectorClass)
	{
		ValidationErrors.Add(FText::FromString(TEXT("EncounterDirectorClass is not set.")));
	}

	if (EncounterId.IsNone())
	{
		ValidationErrors.Add(FText::FromString(TEXT("EncounterId is not set.")));
	}

	const EFoundFootageInteractableObjective ExpectedObjectiveOrder[] = {
		EFoundFootageInteractableObjective::Bodycam,
		EFoundFootageInteractableObjective::FirstNote,
		EFoundFootageInteractableObjective::FirstAnomalyCandidate,
		EFoundFootageInteractableObjective::FirstAnomalyRecord,
		EFoundFootageInteractableObjective::ArchiveReview,
		EFoundFootageInteractableObjective::ExitRouteGate
	};

	if (ObjectiveNodes.Num() != UE_ARRAY_COUNT(ExpectedObjectiveOrder))
	{
		ValidationErrors.Add(FText::Format(
			FText::FromString(TEXT("Objective route must contain exactly {0} first-loop nodes.")),
			FText::AsNumber(UE_ARRAY_COUNT(ExpectedObjectiveOrder))));
	}

	for (int32 ObjectiveIndex = 0; ObjectiveIndex < UE_ARRAY_COUNT(ExpectedObjectiveOrder); ++ObjectiveIndex)
	{
		if (!ObjectiveNodes.IsValidIndex(ObjectiveIndex) || ObjectiveNodes[ObjectiveIndex].Objective != ExpectedObjectiveOrder[ObjectiveIndex])
		{
			ValidationErrors.Add(FText::Format(
				FText::FromString(TEXT("Objective node {0} does not match the required first-loop order.")),
				FText::AsNumber(ObjectiveIndex)));
		}
	}

	TSet<FName> SeenSourceIds;
	TSet<FName> SeenTrailerBeatIds;
	for (int32 NodeIndex = 0; NodeIndex < ObjectiveNodes.Num(); ++NodeIndex)
	{
		const FDeepWaterStationObjectiveNode& ObjectiveNode = ObjectiveNodes[NodeIndex];
		if (ObjectiveNode.SourceId.IsNone())
		{
			ValidationErrors.Add(FText::Format(
				FText::FromString(TEXT("Objective node {0} has no SourceId.")),
				FText::AsNumber(NodeIndex)));
		}
		else if (SeenSourceIds.Contains(ObjectiveNode.SourceId))
		{
			ValidationErrors.Add(FText::Format(
				FText::FromString(TEXT("Objective node {0} duplicates SourceId {1}.")),
				FText::AsNumber(NodeIndex),
				FText::FromName(ObjectiveNode.SourceId)));
		}
		else
		{
			SeenSourceIds.Add(ObjectiveNode.SourceId);
		}

		if (ObjectiveNode.TrailerBeatId.IsNone())
		{
			ValidationErrors.Add(FText::Format(
				FText::FromString(TEXT("Objective node {0} has no TrailerBeatId.")),
				FText::AsNumber(NodeIndex)));
		}
		else if (SeenTrailerBeatIds.Contains(ObjectiveNode.TrailerBeatId))
		{
			ValidationErrors.Add(FText::Format(
				FText::FromString(TEXT("Objective node {0} duplicates TrailerBeatId {1}.")),
				FText::AsNumber(NodeIndex),
				FText::FromName(ObjectiveNode.TrailerBeatId)));
		}
		else
		{
			SeenTrailerBeatIds.Add(ObjectiveNode.TrailerBeatId);
		}

		if (ObjectiveNode.ObjectiveHint.IsEmpty())
		{
			ValidationErrors.Add(FText::Format(
				FText::FromString(TEXT("Objective node {0} has no ObjectiveHint.")),
				FText::AsNumber(NodeIndex)));
		}

		if (ObjectiveNode.DebugLabel.IsEmpty())
		{
			ValidationErrors.Add(FText::Format(
				FText::FromString(TEXT("Objective node {0} has no DebugLabel.")),
				FText::AsNumber(NodeIndex)));
		}

		if (ObjectiveNode.bIsRecordingForFirstAnomalyRecord
			!= (ObjectiveNode.Objective == EFoundFootageInteractableObjective::FirstAnomalyRecord))
		{
			ValidationErrors.Add(FText::Format(
				FText::FromString(TEXT("Objective node {0} has an invalid first-anomaly recording flag.")),
				FText::AsNumber(NodeIndex)));
		}

		if ((ObjectiveNode.Objective == EFoundFootageInteractableObjective::Bodycam
			|| ObjectiveNode.Objective == EFoundFootageInteractableObjective::FirstAnomalyCandidate
			|| ObjectiveNode.Objective == EFoundFootageInteractableObjective::FirstAnomalyRecord)
			&& ObjectiveNode.EvidenceMetadata.EvidenceId.IsNone())
		{
			ValidationErrors.Add(FText::Format(
				FText::FromString(TEXT("Objective node {0} needs evidence metadata.")),
				FText::AsNumber(NodeIndex)));
		}

		if (ObjectiveNode.Objective == EFoundFootageInteractableObjective::FirstNote
			&& ObjectiveNode.NoteMetadata.NoteId.IsNone())
		{
			ValidationErrors.Add(FText::Format(
				FText::FromString(TEXT("Objective node {0} needs note metadata.")),
				FText::AsNumber(NodeIndex)));
		}
	}

	return ValidationErrors.IsEmpty();
}

int32 ADeepWaterStationRouteKit::SpawnObjectiveNodes()
{
	if (!SpawnedObjectiveInteractables.IsEmpty())
	{
		return 0;
	}

	if (!ObjectiveInteractableClass)
	{
		return 0;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return 0;
	}

	int32 SpawnCount = 0;
	for (const FDeepWaterStationObjectiveNode& ObjectiveNode : ObjectiveNodes)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AFoundFootageObjectiveInteractable* Interactable = World->SpawnActor<AFoundFootageObjectiveInteractable>(
			ObjectiveInteractableClass,
			ObjectiveNode.RelativeTransform * GetActorTransform(),
			SpawnParameters);
		if (!Interactable)
		{
			continue;
		}

		Interactable->Objective = ObjectiveNode.Objective;
		Interactable->SourceId = ObjectiveNode.SourceId;
		Interactable->bEnableBodycamOnInteract = ObjectiveNode.bEnableBodycamOnInteract;
		Interactable->bIsRecordingForFirstAnomalyRecord = ObjectiveNode.bIsRecordingForFirstAnomalyRecord;
		Interactable->EvidenceMetadata = ObjectiveNode.EvidenceMetadata;
		Interactable->NoteMetadata = ObjectiveNode.NoteMetadata;
		Interactable->TrailerBeatId = ObjectiveNode.TrailerBeatId;
		Interactable->ObjectiveHint = ObjectiveNode.ObjectiveHint;
		Interactable->DebugLabel = ObjectiveNode.DebugLabel;
		SpawnedObjectiveInteractables.Add(Interactable);
		SpawnedObjectiveInteractableViews.Add(Interactable);
		++SpawnCount;
	}

	return SpawnCount;
}

AHorrorEncounterDirector* ADeepWaterStationRouteKit::SpawnEncounterDirector()
{
	if (SpawnedEncounterDirector)
	{
		return SpawnedEncounterDirector.Get();
	}

	if (!EncounterDirectorClass)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	SpawnedEncounterDirector = World->SpawnActor<AHorrorEncounterDirector>(
		EncounterDirectorClass,
		EncounterDirectorRelativeTransform * GetActorTransform(),
		SpawnParameters);

	if (SpawnedEncounterDirector)
	{
		SpawnedEncounterDirector->DefaultEncounterId = EncounterId;
		SpawnedEncounterDirector->RevealRadius = EncounterRevealRadius;
		SpawnedEncounterDirector->ThreatClass = EncounterThreatClass;
		SpawnedEncounterDirector->ThreatRelativeTransform = EncounterThreatRelativeTransform;
		if (bAutoPrimeEncounter)
		{
			SpawnedEncounterDirector->PrimeEncounter(EncounterId);
		}
	}

	return SpawnedEncounterDirector.Get();
}

AHorrorEncounterDirector* ADeepWaterStationRouteKit::GetSpawnedEncounterDirector() const
{
	return SpawnedEncounterDirector.Get();
}

bool ADeepWaterStationRouteKit::TriggerEncounterReveal(AActor* PlayerActor)
{
	AHorrorEncounterDirector* EncounterDirector = SpawnEncounterDirector();
	return EncounterDirector && EncounterDirector->TriggerReveal(PlayerActor);
}

bool ADeepWaterStationRouteKit::CanTriggerEncounterReveal(AActor* PlayerActor) const
{
	return SpawnedEncounterDirector && SpawnedEncounterDirector->CanTriggerReveal(PlayerActor);
}

bool ADeepWaterStationRouteKit::ResolveEncounter()
{
	return SpawnedEncounterDirector && SpawnedEncounterDirector->ResolveEncounter();
}

bool ADeepWaterStationRouteKit::HandleObjectiveCompleted(EFoundFootageInteractableObjective Objective, AActor* InstigatorActor)
{
	switch (Objective)
	{
	case EFoundFootageInteractableObjective::ArchiveReview:
		return TriggerEncounterReveal(InstigatorActor);

	case EFoundFootageInteractableObjective::ExitRouteGate:
		return CanResolveEncounterAtExit() && ResolveEncounter();

	default:
		return false;
	}
}

bool ADeepWaterStationRouteKit::IsRouteGatedByEncounter() const
{
	return SpawnedEncounterDirector && SpawnedEncounterDirector->IsRouteGated();
}

bool ADeepWaterStationRouteKit::CanResolveEncounterAtExit() const
{
	return IsRouteGatedByEncounter();
}

FName ADeepWaterStationRouteKit::GetTrailerBeatIdForSourceId(FName SourceId) const
{
	for (const FDeepWaterStationObjectiveNode& ObjectiveNode : ObjectiveNodes)
	{
		if (ObjectiveNode.SourceId == SourceId)
		{
			return ObjectiveNode.TrailerBeatId;
		}
	}

	return NAME_None;
}

TArray<FName> ADeepWaterStationRouteKit::GetTrailerBeatIds() const
{
	TArray<FName> TrailerBeatIds;
	TrailerBeatIds.Reserve(ObjectiveNodes.Num());
	for (const FDeepWaterStationObjectiveNode& ObjectiveNode : ObjectiveNodes)
	{
		if (!ObjectiveNode.TrailerBeatId.IsNone())
		{
			TrailerBeatIds.AddUnique(ObjectiveNode.TrailerBeatId);
		}
	}

	return TrailerBeatIds;
}
