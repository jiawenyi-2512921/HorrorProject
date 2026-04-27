// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/DeepWaterStationRouteKit.h"

#include "AI/HorrorThreatCharacter.h"
#include "Components/SceneComponent.h"
#include "Containers/Set.h"
#include "Engine/World.h"
#include "Game/HorrorEncounterDirector.h"

namespace
{
struct FFirstLoopObjectiveSpec
{
	EFoundFootageInteractableObjective Objective;
	const TCHAR* SourceId;
	float XOffsetCm;
	const TCHAR* TrailerBeatId;
	const TCHAR* ObjectiveHint;
	const TCHAR* DebugLabel;
	bool bEnableBodycamOnInteract;
	bool bIsRecordingForFirstAnomalyRecord;
	const TCHAR* EvidenceId;
	const TCHAR* EvidenceDisplayName;
	const TCHAR* NoteId;
	const TCHAR* NoteTitle;
};

constexpr float BodycamXOffsetCm = 200.0f;
constexpr float FirstNoteXOffsetCm = 600.0f;
constexpr float FirstAnomalyCandidateXOffsetCm = 1000.0f;
constexpr float FirstAnomalyRecordXOffsetCm = 1400.0f;
constexpr float ArchiveReviewXOffsetCm = 1800.0f;
constexpr float ExitRouteGateXOffsetCm = 2200.0f;

constexpr FFirstLoopObjectiveSpec FirstLoopObjectiveSpecs[] = {
	{
		EFoundFootageInteractableObjective::Bodycam,
		TEXT("Evidence.Bodycam"),
		BodycamXOffsetCm,
		TEXT("Beat.BodycamAcquire"),
		TEXT("Recover the bodycam."),
		TEXT("Bodycam Pickup"),
		true,
		false,
		TEXT("Evidence.Bodycam"),
		TEXT("Bodycam"),
		nullptr,
		nullptr
	},
	{
		EFoundFootageInteractableObjective::FirstNote,
		TEXT("Note.Intro"),
		FirstNoteXOffsetCm,
		TEXT("Beat.FirstNote"),
		TEXT("Read the first station note."),
		TEXT("Intro Note"),
		true,
		false,
		nullptr,
		nullptr,
		TEXT("Note.Intro"),
		TEXT("Intro")
	},
	{
		EFoundFootageInteractableObjective::FirstAnomalyCandidate,
		TEXT("Evidence.Anomaly01"),
		FirstAnomalyCandidateXOffsetCm,
		TEXT("Beat.FirstAnomalyCandidate"),
		TEXT("Frame the first anomaly."),
		TEXT("Anomaly Candidate"),
		true,
		false,
		TEXT("Evidence.Anomaly01"),
		TEXT("First Anomaly"),
		nullptr,
		nullptr
	},
	{
		EFoundFootageInteractableObjective::FirstAnomalyRecord,
		TEXT("Evidence.Recorder"),
		FirstAnomalyRecordXOffsetCm,
		TEXT("Beat.FirstAnomalyRecord"),
		TEXT("Record while the anomaly is visible."),
		TEXT("Anomaly Recording Window"),
		true,
		true,
		TEXT("Evidence.Anomaly01"),
		TEXT("First Anomaly"),
		nullptr,
		nullptr
	},
	{
		EFoundFootageInteractableObjective::ArchiveReview,
		TEXT("Archive.Terminal"),
		ArchiveReviewXOffsetCm,
		TEXT("Beat.ArchiveReview"),
		TEXT("Review the tape at the archive terminal."),
		TEXT("Archive Terminal"),
		true,
		false,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},
	{
		EFoundFootageInteractableObjective::ExitRouteGate,
		TEXT("Exit.Gate"),
		ExitRouteGateXOffsetCm,
		TEXT("Beat.ExitGate"),
		TEXT("Leave through the unlocked service gate."),
		TEXT("Exit Gate"),
		true,
		false,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	}
};

constexpr EFoundFootageInteractableObjective DeepWaterExpectedObjectiveOrder[] = {
	EFoundFootageInteractableObjective::Bodycam,
	EFoundFootageInteractableObjective::FirstNote,
	EFoundFootageInteractableObjective::FirstAnomalyCandidate,
	EFoundFootageInteractableObjective::FirstAnomalyRecord,
	EFoundFootageInteractableObjective::ArchiveReview,
	EFoundFootageInteractableObjective::ExitRouteGate
};

FVector MakeFirstLoopRelativeLocation(float XOffsetCm)
{
	return FVector(XOffsetCm, 0.0f, HorrorRouteKitDefaults::ObjectiveHeightCm);
}

FDeepWaterStationObjectiveNode MakeFirstLoopNode(
	EFoundFootageInteractableObjective Objective,
	FName SourceId,
	const FVector& RelativeLocation,
	FName TrailerBeatId,
	const TCHAR* ObjectiveHint,
	const TCHAR* DebugLabel,
	bool bEnableBodycamOnInteract = true,
	bool bIsRecordingForFirstAnomalyRecord = false)
{
	FDeepWaterStationObjectiveNode Node;
	Node.Objective = Objective;
	Node.SourceId = SourceId;
	Node.RelativeTransform = FTransform(FRotator::ZeroRotator, RelativeLocation);
	Node.bEnableBodycamOnInteract = bEnableBodycamOnInteract;
	Node.bIsRecordingForFirstAnomalyRecord = bIsRecordingForFirstAnomalyRecord;
	Node.TrailerBeatId = TrailerBeatId;
	Node.ObjectiveHint = FText::FromString(ObjectiveHint);
	Node.DebugLabel = FText::FromString(DebugLabel);
	return Node;
}

bool DeepWaterObjectiveRequiresEvidenceMetadata(EFoundFootageInteractableObjective Objective)
{
	return Objective == EFoundFootageInteractableObjective::Bodycam
		|| Objective == EFoundFootageInteractableObjective::FirstAnomalyCandidate
		|| Objective == EFoundFootageInteractableObjective::FirstAnomalyRecord;
}

bool DeepWaterObjectiveRequiresNoteMetadata(EFoundFootageInteractableObjective Objective)
{
	return Objective == EFoundFootageInteractableObjective::FirstNote;
}
}

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
	ObjectiveNodes.Reset(UE_ARRAY_COUNT(FirstLoopObjectiveSpecs));
	for (const FFirstLoopObjectiveSpec& Spec : FirstLoopObjectiveSpecs)
	{
		FDeepWaterStationObjectiveNode Node = MakeFirstLoopNode(
			Spec.Objective,
			Spec.SourceId,
			MakeFirstLoopRelativeLocation(Spec.XOffsetCm),
			Spec.TrailerBeatId,
			Spec.ObjectiveHint,
			Spec.DebugLabel,
			Spec.bEnableBodycamOnInteract,
			Spec.bIsRecordingForFirstAnomalyRecord);

		if (Spec.EvidenceId && Spec.EvidenceDisplayName)
		{
			Node.EvidenceMetadata.EvidenceId = Spec.EvidenceId;
			Node.EvidenceMetadata.DisplayName = FText::FromString(Spec.EvidenceDisplayName);
		}

		if (Spec.NoteId && Spec.NoteTitle)
		{
			Node.NoteMetadata.NoteId = Spec.NoteId;
			Node.NoteMetadata.Title = FText::FromString(Spec.NoteTitle);
		}

		ObjectiveNodes.Add(MoveTemp(Node));
	}
}

bool ADeepWaterStationRouteKit::ValidateObjectiveNodes(TArray<FText>& ValidationErrors) const
{
	ValidationErrors.Reset();

	ValidateRouteKitClassSettings(ValidationErrors);
	ValidateObjectiveRouteOrder(ValidationErrors);
	ValidateObjectiveNodeDefinitions(ValidationErrors);

	return ValidationErrors.IsEmpty();
}

void ADeepWaterStationRouteKit::ValidateRouteKitClassSettings(TArray<FText>& ValidationErrors) const
{
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
}

void ADeepWaterStationRouteKit::ValidateObjectiveRouteOrder(TArray<FText>& ValidationErrors) const
{
	if (ObjectiveNodes.Num() != UE_ARRAY_COUNT(DeepWaterExpectedObjectiveOrder))
	{
		ValidationErrors.Add(FText::Format(
			FText::FromString(TEXT("Objective route must contain exactly {0} first-loop nodes.")),
			FText::AsNumber(UE_ARRAY_COUNT(DeepWaterExpectedObjectiveOrder))));
	}

	for (int32 ObjectiveIndex = 0; ObjectiveIndex < UE_ARRAY_COUNT(DeepWaterExpectedObjectiveOrder); ++ObjectiveIndex)
	{
		if (!ObjectiveNodes.IsValidIndex(ObjectiveIndex) || ObjectiveNodes[ObjectiveIndex].Objective != DeepWaterExpectedObjectiveOrder[ObjectiveIndex])
		{
			ValidationErrors.Add(FText::Format(
				FText::FromString(TEXT("Objective node {0} does not match the required first-loop order.")),
				FText::AsNumber(ObjectiveIndex)));
		}
	}
}

void ADeepWaterStationRouteKit::ValidateObjectiveNodeDefinitions(TArray<FText>& ValidationErrors) const
{
	TSet<FName> SeenSourceIds;
	TSet<FName> SeenTrailerBeatIds;
	int32 NodeIndex = 0;
	for (const FDeepWaterStationObjectiveNode& ObjectiveNode : ObjectiveNodes)
	{
		ValidateObjectiveNodeIdentity(ObjectiveNode, NodeIndex, SeenSourceIds, SeenTrailerBeatIds, ValidationErrors);
		ValidateObjectiveNodeText(ObjectiveNode, NodeIndex, ValidationErrors);
		ValidateObjectiveNodeRequirements(ObjectiveNode, NodeIndex, ValidationErrors);
		++NodeIndex;
	}
}

void ADeepWaterStationRouteKit::ValidateObjectiveNodeIdentity(
	const FDeepWaterStationObjectiveNode& ObjectiveNode,
	int32 NodeIndex,
	TSet<FName>& SeenSourceIds,
	TSet<FName>& SeenTrailerBeatIds,
	TArray<FText>& ValidationErrors) const
{
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
}

void ADeepWaterStationRouteKit::ValidateObjectiveNodeText(
	const FDeepWaterStationObjectiveNode& ObjectiveNode,
	int32 NodeIndex,
	TArray<FText>& ValidationErrors) const
{
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
}

void ADeepWaterStationRouteKit::ValidateObjectiveNodeRequirements(
	const FDeepWaterStationObjectiveNode& ObjectiveNode,
	int32 NodeIndex,
	TArray<FText>& ValidationErrors) const
{
	if (ObjectiveNode.bIsRecordingForFirstAnomalyRecord
		!= (ObjectiveNode.Objective == EFoundFootageInteractableObjective::FirstAnomalyRecord))
	{
		ValidationErrors.Add(FText::Format(
			FText::FromString(TEXT("Objective node {0} has an invalid first-anomaly recording flag.")),
			FText::AsNumber(NodeIndex)));
	}

	if (DeepWaterObjectiveRequiresEvidenceMetadata(ObjectiveNode.Objective) && ObjectiveNode.EvidenceMetadata.EvidenceId.IsNone())
	{
		ValidationErrors.Add(FText::Format(
			FText::FromString(TEXT("Objective node {0} needs evidence metadata.")),
			FText::AsNumber(NodeIndex)));
	}

	if (DeepWaterObjectiveRequiresNoteMetadata(ObjectiveNode.Objective) && ObjectiveNode.NoteMetadata.NoteId.IsNone())
	{
		ValidationErrors.Add(FText::Format(
			FText::FromString(TEXT("Objective node {0} needs note metadata.")),
			FText::AsNumber(NodeIndex)));
	}
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
