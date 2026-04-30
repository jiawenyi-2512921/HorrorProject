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
	float YOffsetCm;
	const TCHAR* TrailerBeatId;
	const TCHAR* ObjectiveHint;
	const TCHAR* DebugLabel;
	bool bEnableBodycamOnInteract;
	bool bIsRecordingForFirstAnomalyRecord;
	const TCHAR* EvidenceId;
	const TCHAR* EvidenceDisplayName;
	const TCHAR* NoteId;
	const TCHAR* NoteTitle;
	const TCHAR* NoteBody;
};

constexpr float BodycamXOffsetCm = 200.0f;
constexpr float FirstNoteXOffsetCm = 600.0f;
constexpr float FirstAnomalyCandidateXOffsetCm = 1000.0f;
constexpr float FirstAnomalyRecordXOffsetCm = 1400.0f;
constexpr float ArchiveReviewXOffsetCm = 1650.0f;
constexpr float ExitRouteGateXOffsetCm = 2050.0f;
constexpr float MainCorridorYOffsetCm = 0.0f;
constexpr float ServiceRouteYOffsetCm = 300.0f;

constexpr FFirstLoopObjectiveSpec FirstLoopObjectiveSpecs[] = {
	{
		EFoundFootageInteractableObjective::Bodycam,
		TEXT("Evidence.Bodycam"),
		BodycamXOffsetCm,
		MainCorridorYOffsetCm,
		TEXT("Beat.BodycamAcquire"),
		TEXT("取回随身摄像机。"),
		TEXT("取得随身摄像机"),
		true,
		false,
		TEXT("Evidence.Bodycam"),
		TEXT("随身摄像机"),
		nullptr,
		nullptr,
		nullptr
	},
	{
		EFoundFootageInteractableObjective::FirstNote,
		TEXT("Note.Intro"),
		FirstNoteXOffsetCm,
		MainCorridorYOffsetCm,
		TEXT("Beat.FirstNote"),
		TEXT("阅读第一份站内备忘录。"),
		TEXT("站内备忘录"),
		true,
		false,
		nullptr,
		nullptr,
		TEXT("Note.Intro"),
		TEXT("维修日志：路线门禁码"),
		TEXT("0417 可以打开第一道维修门。1831 可以打开异常室门。1799 是后续维修舱门密码。1697 是出口通道门禁码。标记 1939 的上层维修舱暂时封死，不需要攀爬；第一个异常在舱门下方的地面层。")
	},
	{
		EFoundFootageInteractableObjective::FirstAnomalyCandidate,
		TEXT("Evidence.Anomaly01"),
		FirstAnomalyCandidateXOffsetCm,
		MainCorridorYOffsetCm,
		TEXT("Beat.FirstAnomalyCandidate"),
		TEXT("对准第一个异常。"),
		TEXT("异常目标"),
		true,
		false,
		TEXT("Evidence.Anomaly01"),
		TEXT("第一异常"),
		nullptr,
		nullptr,
		nullptr
	},
	{
		EFoundFootageInteractableObjective::FirstAnomalyRecord,
		TEXT("Evidence.Recorder"),
		FirstAnomalyRecordXOffsetCm,
		MainCorridorYOffsetCm,
		TEXT("Beat.FirstAnomalyRecord"),
		TEXT("异常可见时开始录像。"),
		TEXT("异常录像窗口"),
		true,
		true,
		TEXT("Evidence.Anomaly01"),
		TEXT("第一异常"),
		nullptr,
		nullptr,
		nullptr
	},
	{
		EFoundFootageInteractableObjective::ArchiveReview,
		TEXT("Archive.Terminal"),
		ArchiveReviewXOffsetCm,
		ServiceRouteYOffsetCm,
		TEXT("Beat.ArchiveReview"),
		TEXT("在档案终端复查录像。"),
		TEXT("档案终端"),
		true,
		false,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},
	{
		EFoundFootageInteractableObjective::ExitRouteGate,
		TEXT("Exit.Gate"),
		ExitRouteGateXOffsetCm,
		ServiceRouteYOffsetCm,
		TEXT("Beat.ExitGate"),
		TEXT("穿过已解锁的维修出口。"),
		TEXT("出口闸门"),
		true,
		false,
		nullptr,
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

FVector MakeFirstLoopRelativeLocation(float XOffsetCm, float YOffsetCm)
{
	return FVector(XOffsetCm, YOffsetCm, HorrorRouteKitDefaults::ObjectiveHeightCm);
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

FDeepWaterStationObjectiveNode MakeFirstLoopNode(const FFirstLoopObjectiveSpec& Spec)
{
	FDeepWaterStationObjectiveNode Node = MakeFirstLoopNode(
		Spec.Objective,
		Spec.SourceId,
		MakeFirstLoopRelativeLocation(Spec.XOffsetCm, Spec.YOffsetCm),
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
		if (Spec.NoteBody)
		{
			Node.NoteMetadata.Body = FText::FromString(Spec.NoteBody);
		}
	}

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

bool IsLegacyStraightLineLateObjectivePlacement(const FDeepWaterStationObjectiveNode& Node)
{
	const FVector Location = Node.RelativeTransform.GetLocation();
	if (Node.Objective == EFoundFootageInteractableObjective::ArchiveReview)
	{
		return Location.Equals(FVector(1800.0, 0.0, HorrorRouteKitDefaults::ObjectiveHeightCm), KINDA_SMALL_NUMBER);
	}
	if (Node.Objective == EFoundFootageInteractableObjective::ExitRouteGate)
	{
		return Location.Equals(FVector(2200.0, 0.0, HorrorRouteKitDefaults::ObjectiveHeightCm), KINDA_SMALL_NUMBER);
	}
	return false;
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
	else
	{
		EnsureDefaultFirstLoopObjectiveNodes();
	}
	SpawnObjectiveNodes();
	SpawnEncounterDirector();
}

void ADeepWaterStationRouteKit::ConfigureDefaultFirstLoopObjectiveNodes()
{
	ObjectiveNodes.Reset(UE_ARRAY_COUNT(FirstLoopObjectiveSpecs));
	for (const FFirstLoopObjectiveSpec& Spec : FirstLoopObjectiveSpecs)
	{
		ObjectiveNodes.Add(MakeFirstLoopNode(Spec));
	}
}

bool ADeepWaterStationRouteKit::EnsureDefaultFirstLoopObjectiveNodes()
{
	bool bChanged = false;

	if (ObjectiveNodes.Num() != UE_ARRAY_COUNT(DeepWaterExpectedObjectiveOrder))
	{
		TArray<FDeepWaterStationObjectiveNode> RepairedNodes;
		RepairedNodes.Reserve(UE_ARRAY_COUNT(FirstLoopObjectiveSpecs));
		for (const FFirstLoopObjectiveSpec& Spec : FirstLoopObjectiveSpecs)
		{
			const FDeepWaterStationObjectiveNode* ExistingNode = ObjectiveNodes.FindByPredicate(
				[&Spec](const FDeepWaterStationObjectiveNode& Node)
				{
					return Node.Objective == Spec.Objective;
				});
			RepairedNodes.Add(ExistingNode ? *ExistingNode : MakeFirstLoopNode(Spec));
		}
		ObjectiveNodes = MoveTemp(RepairedNodes);
		bChanged = true;
	}

	for (int32 ObjectiveIndex = 0; ObjectiveIndex < UE_ARRAY_COUNT(DeepWaterExpectedObjectiveOrder); ++ObjectiveIndex)
	{
		if (!ObjectiveNodes.IsValidIndex(ObjectiveIndex) || ObjectiveNodes[ObjectiveIndex].Objective != DeepWaterExpectedObjectiveOrder[ObjectiveIndex])
		{
			ConfigureDefaultFirstLoopObjectiveNodes();
			return true;
		}

		const FFirstLoopObjectiveSpec& Spec = FirstLoopObjectiveSpecs[ObjectiveIndex];
		FDeepWaterStationObjectiveNode& Node = ObjectiveNodes[ObjectiveIndex];
		const FDeepWaterStationObjectiveNode DefaultNode = MakeFirstLoopNode(Spec);
		if (Node.SourceId.IsNone())
		{
			Node.SourceId = DefaultNode.SourceId;
			bChanged = true;
		}
		if (Node.TrailerBeatId.IsNone())
		{
			Node.TrailerBeatId = DefaultNode.TrailerBeatId;
			bChanged = true;
		}
		if (Node.ObjectiveHint.IsEmpty())
		{
			Node.ObjectiveHint = DefaultNode.ObjectiveHint;
			bChanged = true;
		}
		if (Node.DebugLabel.IsEmpty())
		{
			Node.DebugLabel = DefaultNode.DebugLabel;
			bChanged = true;
		}
		if (Node.bIsRecordingForFirstAnomalyRecord != DefaultNode.bIsRecordingForFirstAnomalyRecord)
		{
			Node.bIsRecordingForFirstAnomalyRecord = DefaultNode.bIsRecordingForFirstAnomalyRecord;
			bChanged = true;
		}
		if (IsLegacyStraightLineLateObjectivePlacement(Node))
		{
			Node.RelativeTransform = DefaultNode.RelativeTransform;
			bChanged = true;
		}
		if (DeepWaterObjectiveRequiresEvidenceMetadata(Node.Objective) && Node.EvidenceMetadata.EvidenceId.IsNone())
		{
			Node.EvidenceMetadata = DefaultNode.EvidenceMetadata;
			bChanged = true;
		}
		if (DeepWaterObjectiveRequiresNoteMetadata(Node.Objective) && Node.NoteMetadata.NoteId.IsNone())
		{
			Node.NoteMetadata = DefaultNode.NoteMetadata;
			bChanged = true;
		}
	}

	return bChanged;
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
		ValidationErrors.Add(FText::AsCultureInvariant(TEXT("ObjectiveInteractableClass is not set.")));
	}

	if (!EncounterDirectorClass)
	{
		ValidationErrors.Add(FText::AsCultureInvariant(TEXT("EncounterDirectorClass is not set.")));
	}

	if (EncounterId.IsNone())
	{
		ValidationErrors.Add(FText::AsCultureInvariant(TEXT("EncounterId is not set.")));
	}
}

void ADeepWaterStationRouteKit::ValidateObjectiveRouteOrder(TArray<FText>& ValidationErrors) const
{
	if (ObjectiveNodes.Num() != UE_ARRAY_COUNT(DeepWaterExpectedObjectiveOrder))
	{
		ValidationErrors.Add(FText::Format(
			FText::AsCultureInvariant(TEXT("Objective route must contain exactly {0} first-loop nodes.")),
			FText::AsNumber(UE_ARRAY_COUNT(DeepWaterExpectedObjectiveOrder))));
	}

	for (int32 ObjectiveIndex = 0; ObjectiveIndex < UE_ARRAY_COUNT(DeepWaterExpectedObjectiveOrder); ++ObjectiveIndex)
	{
		if (!ObjectiveNodes.IsValidIndex(ObjectiveIndex) || ObjectiveNodes[ObjectiveIndex].Objective != DeepWaterExpectedObjectiveOrder[ObjectiveIndex])
		{
			ValidationErrors.Add(FText::Format(
				FText::AsCultureInvariant(TEXT("Objective node {0} does not match the first-loop order.")),
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
			FText::AsCultureInvariant(TEXT("Objective node {0} is missing SourceId.")),
			FText::AsNumber(NodeIndex)));
	}
	else if (SeenSourceIds.Contains(ObjectiveNode.SourceId))
	{
		ValidationErrors.Add(FText::Format(
			FText::AsCultureInvariant(TEXT("Objective node {0} has duplicate SourceId {1}.")),
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
			FText::AsCultureInvariant(TEXT("Objective node {0} is missing TrailerBeatId.")),
			FText::AsNumber(NodeIndex)));
	}
	else if (SeenTrailerBeatIds.Contains(ObjectiveNode.TrailerBeatId))
	{
		ValidationErrors.Add(FText::Format(
			FText::AsCultureInvariant(TEXT("Objective node {0} has duplicate TrailerBeatId {1}.")),
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
			FText::AsCultureInvariant(TEXT("Objective node {0} is missing ObjectiveHint.")),
			FText::AsNumber(NodeIndex)));
	}

	if (ObjectiveNode.DebugLabel.IsEmpty())
	{
		ValidationErrors.Add(FText::Format(
			FText::AsCultureInvariant(TEXT("Objective node {0} is missing DebugLabel.")),
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
			FText::AsCultureInvariant(TEXT("Objective node {0} has invalid first anomaly recording flag.")),
			FText::AsNumber(NodeIndex)));
	}

	if (DeepWaterObjectiveRequiresEvidenceMetadata(ObjectiveNode.Objective) && ObjectiveNode.EvidenceMetadata.EvidenceId.IsNone())
	{
		ValidationErrors.Add(FText::Format(
			FText::AsCultureInvariant(TEXT("Objective node {0} requires evidence metadata.")),
			FText::AsNumber(NodeIndex)));
	}

	if (DeepWaterObjectiveRequiresNoteMetadata(ObjectiveNode.Objective) && ObjectiveNode.NoteMetadata.NoteId.IsNone())
	{
		ValidationErrors.Add(FText::Format(
			FText::AsCultureInvariant(TEXT("Objective node {0} requires note metadata.")),
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
		Interactable->RefreshVisualDefaults();
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

bool ADeepWaterStationRouteKit::TryGetObjectiveWorldLocation(EFoundFootageInteractableObjective Objective, FVector& OutWorldLocation) const
{
	for (const TObjectPtr<AFoundFootageObjectiveInteractable>& Interactable : SpawnedObjectiveInteractables)
	{
		if (Interactable && Interactable->Objective == Objective)
		{
			OutWorldLocation = Interactable->GetActorLocation();
			return true;
		}
	}

	for (const FDeepWaterStationObjectiveNode& ObjectiveNode : ObjectiveNodes)
	{
		if (ObjectiveNode.Objective == Objective)
		{
			OutWorldLocation = (ObjectiveNode.RelativeTransform * GetActorTransform()).GetLocation();
			return true;
		}
	}

	return false;
}
