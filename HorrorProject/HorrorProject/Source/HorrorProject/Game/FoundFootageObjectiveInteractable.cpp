// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/FoundFootageObjectiveInteractable.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
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
	const FName ArchiveSummaryNoteId(TEXT("Note.ArchiveSummary"));
	const TCHAR* ArchiveExitGateCode = TEXT("1697");
	const FVector DefaultInteractionBoundsExtent(85.0f, 85.0f, 85.0f);
	const TCHAR* FallbackVisualMeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");

	struct FFoundFootageObjectiveVisualSpec
	{
		const TCHAR* MeshPath = FallbackVisualMeshPath;
		FVector RelativeScale = FVector(0.25f);
		FVector RelativeLocation = FVector::ZeroVector;
		FRotator RelativeRotation = FRotator::ZeroRotator;
		FVector BoundsExtent = DefaultInteractionBoundsExtent;
	};

	FFoundFootageObjectiveVisualSpec GetObjectiveVisualSpec(EFoundFootageInteractableObjective Objective)
	{
		switch (Objective)
		{
		case EFoundFootageInteractableObjective::Bodycam:
			return {
				TEXT("/Game/DeepWaterStation/Meshes/SM_Device01.SM_Device01"),
				FVector(0.28f),
				FVector(0.0f, 0.0f, -30.0f),
				FRotator(0.0f, 20.0f, 0.0f),
				FVector(90.0f, 90.0f, 80.0f)
			};

		case EFoundFootageInteractableObjective::FirstNote:
			return {
				TEXT("/Game/DeepWaterStation/Meshes/SM_Props07.SM_Props07"),
				FVector(0.18f),
				FVector(0.0f, 0.0f, -35.0f),
				FRotator(0.0f, -25.0f, 0.0f),
				FVector(85.0f, 85.0f, 70.0f)
			};

		case EFoundFootageInteractableObjective::FirstAnomalyCandidate:
			return {
				TEXT("/Game/DeepWaterStation/Meshes/SM_ScreensA01.SM_ScreensA01"),
				FVector(0.22f),
				FVector(0.0f, 0.0f, -20.0f),
				FRotator(0.0f, 180.0f, 0.0f),
				FVector(100.0f, 100.0f, 95.0f)
			};

		case EFoundFootageInteractableObjective::FirstAnomalyRecord:
			return {
				TEXT("/Game/DeepWaterStation/Meshes/SM_ScreensA02.SM_ScreensA02"),
				FVector(0.22f),
				FVector(0.0f, 0.0f, -20.0f),
				FRotator(0.0f, 180.0f, 0.0f),
				FVector(100.0f, 100.0f, 95.0f)
			};

		case EFoundFootageInteractableObjective::ArchiveReview:
			return {
				TEXT("/Game/DeepWaterStation/Meshes/SM_Terminal01.SM_Terminal01"),
				FVector(0.30f),
				FVector(0.0f, 0.0f, -35.0f),
				FRotator(0.0f, 180.0f, 0.0f),
				FVector(120.0f, 120.0f, 110.0f)
			};

		case EFoundFootageInteractableObjective::ExitRouteGate:
			return {
				TEXT("/Game/DeepWaterStation/Meshes/SM_MetalGate01.SM_MetalGate01"),
				FVector(0.18f),
				FVector(0.0f, 0.0f, -70.0f),
				FRotator::ZeroRotator,
				FVector(130.0f, 130.0f, 140.0f)
			};
		}

		return {};
	}

	UStaticMesh* LoadObjectiveVisualMesh(const TCHAR* MeshPath)
	{
		if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, MeshPath))
		{
			return Mesh;
		}

		return LoadObject<UStaticMesh>(nullptr, FallbackVisualMeshPath);
	}
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

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(InteractionBounds);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFoundFootageObjectiveInteractable::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshVisualDefaults();
}

void AFoundFootageObjectiveInteractable::RefreshVisualDefaults()
{
	if (!InteractionBounds || !VisualMesh)
	{
		return;
	}

	const FFoundFootageObjectiveVisualSpec VisualSpec = GetObjectiveVisualSpec(Objective);
	if (UStaticMesh* ResolvedMesh = LoadObjectiveVisualMesh(VisualSpec.MeshPath))
	{
		VisualMesh->SetStaticMesh(ResolvedMesh);
	}

	VisualMesh->SetRelativeLocation(VisualSpec.RelativeLocation);
	VisualMesh->SetRelativeRotation(VisualSpec.RelativeRotation);
	VisualMesh->SetRelativeScale3D(VisualSpec.RelativeScale);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	InteractionBounds->SetBoxExtent(VisualSpec.BoundsExtent);
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
	const bool bRecordingPendingAnomalyFromCandidate =
		Objective == EFoundFootageInteractableObjective::FirstAnomalyCandidate
		&& GameMode
		&& CanRecordFirstAnomalyFromCandidate(*GameMode);
	if (bRecordingPendingAnomalyFromCandidate)
	{
		ProgressId = GameMode->GetPendingFirstAnomalySourceId();
	}

	const bool bCompleted = TryCompleteObjectiveForInstigator(GameMode, InstigatorActor);
	if (bCompleted)
	{
		if (bRecordingPendingAnomalyFromCandidate)
		{
			if (AHorrorPlayerCharacter* PlayerCharacter = Cast<AHorrorPlayerCharacter>(InstigatorActor))
			{
				RecordEvidenceProgress(*PlayerCharacter, ProgressId, true);
			}
		}
		else
		{
			RecordInstigatorProgress(InstigatorActor, ProgressId);
		}
		if (ADeepWaterStationRouteKit* OwningRouteKit = Cast<ADeepWaterStationRouteKit>(GetOwner()))
		{
			const EFoundFootageInteractableObjective CompletedObjective = bRecordingPendingAnomalyFromCandidate
				? EFoundFootageInteractableObjective::FirstAnomalyRecord
				: Objective;
			OwningRouteKit->HandleObjectiveCompleted(CompletedObjective, InstigatorActor);
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
		return (!ResolveSourceId().IsNone() && GameMode->CanBeginFirstAnomalyCandidate())
			|| CanRecordFirstAnomalyFromCandidate(*GameMode);

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
		if (CanRecordFirstAnomalyFromCandidate(*GameMode))
		{
			return TryCompleteFirstAnomalyRecordObjective(*GameMode);
		}
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

FText AFoundFootageObjectiveInteractable::GetInteractionPromptText(AActor* InstigatorActor) const
{
	AHorrorGameModeBase* GameMode = ResolveObjectiveGameMode();
	if (CanCompleteObjectiveForInstigator(GameMode, InstigatorActor))
	{
		return BuildObjectiveActionPrompt(GameMode);
	}

	return BuildBlockedObjectivePrompt(GameMode, InstigatorActor);
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
		RecordObjectiveHintNoteProgress(*PlayerCharacter, ProgressId);
		break;

	case EFoundFootageInteractableObjective::ArchiveReview:
		RecordEvidenceProgress(*PlayerCharacter, ProgressId, true);
		RecordArchiveSummaryProgress(*PlayerCharacter, ProgressId);
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
	if (GameMode.IsDay1Complete())
	{
		return false;
	}

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
	const FName ProgressId = ResolveSourceId();
	RegisterObjectiveEventMetadataForCompletedObjective(&GameMode, ProgressId);

	if (const ADeepWaterStationRouteKit* OwningRouteKit = ResolveOwningRouteKit())
	{
		if (OwningRouteKit->CanResolveEncounterAtExit())
		{
			const bool bCompleted = GameMode.TryCompleteDay1(ProgressId);
			if (!bCompleted)
			{
				UnregisterObjectiveEventMetadataForCompletedObjective(&GameMode, ProgressId);
			}
			return bCompleted;
		}
	}

	const bool bCompleted = GameMode.TryCompleteDay1(ProgressId);
	if (!bCompleted)
	{
		UnregisterObjectiveEventMetadataForCompletedObjective(&GameMode, ProgressId);
	}
	return bCompleted;
}

bool AFoundFootageObjectiveInteractable::CanRecordFirstAnomalyFromCandidate(AHorrorGameModeBase& GameMode) const
{
	const FName ProgressId = ResolveSourceId();
	return !ProgressId.IsNone()
		&& GameMode.GetPendingFirstAnomalySourceId() == ProgressId
		&& GameMode.CanRecordFirstAnomaly(GameMode.IsLeadPlayerRecording());
}

FText AFoundFootageObjectiveInteractable::BuildBlockedObjectivePrompt(AHorrorGameModeBase* GameMode, AActor* InstigatorActor) const
{
	if (!GameMode)
	{
		return NSLOCTEXT("FoundFootageObjective", "ObjectiveUnavailable", "目标暂不可用。");
	}

	switch (Objective)
	{
	case EFoundFootageInteractableObjective::Bodycam:
		return GameMode->HasBodycamAcquired()
			? NSLOCTEXT("FoundFootageObjective", "BodycamAlreadyAcquired", "随身摄像机已取回。")
			: NSLOCTEXT("FoundFootageObjective", "RecoverBodycam", "取回随身摄像机。");

	case EFoundFootageInteractableObjective::FirstNote:
		if (!GameMode->HasBodycamAcquired())
		{
			return NSLOCTEXT("FoundFootageObjective", "RecoverBodycamFirst", "先取回随身摄像机。");
		}
		return GameMode->HasCollectedFirstNote()
			? NSLOCTEXT("FoundFootageObjective", "NoteAlreadyRecorded", "备忘录已记录。")
			: NSLOCTEXT("FoundFootageObjective", "ReadStationNote", "阅读站内备忘录。");

	case EFoundFootageInteractableObjective::FirstAnomalyCandidate:
		if (!GameMode->HasCollectedFirstNote())
		{
			return NSLOCTEXT("FoundFootageObjective", "FindFirstNoteFirst", "先找到第一份站内备忘录。");
		}
		if (GameMode->HasPendingFirstAnomalyCandidate())
		{
			return NSLOCTEXT("FoundFootageObjective", "AnomalyAlignedRecord", "异常已对准，开始录像。");
		}
		return GameMode->HasRecordedFirstAnomaly()
			? NSLOCTEXT("FoundFootageObjective", "AnomalyAlreadyRecorded", "异常已记录。")
			: NSLOCTEXT("FoundFootageObjective", "AimAtAnomaly", "对准异常。");

	case EFoundFootageInteractableObjective::FirstAnomalyRecord:
		if (!GameMode->HasPendingFirstAnomalyCandidate())
		{
			return NSLOCTEXT("FoundFootageObjective", "AimAtAnomalyFirst", "先对准异常。");
		}
		if (!GameMode->IsLeadPlayerRecording())
		{
			return NSLOCTEXT("FoundFootageObjective", "StartRecordingFirst", "先开启录像，再按互动键记录异常。");
		}
		return GameMode->HasRecordedFirstAnomaly()
			? NSLOCTEXT("FoundFootageObjective", "AnomalyAlreadyCaptured", "异常已记录。")
			: NSLOCTEXT("FoundFootageObjective", "CaptureAnomaly", "捕捉异常。");

	case EFoundFootageInteractableObjective::ArchiveReview:
		if (!GameMode->HasRecordedFirstAnomaly())
		{
			return NSLOCTEXT("FoundFootageObjective", "RecordAnomalyFirst", "先记录异常，再使用档案终端。");
		}
		if (!InstigatorActor)
		{
			return NSLOCTEXT("FoundFootageObjective", "ApproachArchiveTerminal", "靠近档案终端。");
		}
		return GameMode->HasReviewedArchive()
			? NSLOCTEXT("FoundFootageObjective", "ArchiveAlreadyReviewed", "档案已复查。")
			: NSLOCTEXT("FoundFootageObjective", "MissingVideoEvidence", "缺少必要录像证据。");

	case EFoundFootageInteractableObjective::ExitRouteGate:
		if (GameMode->IsDay1Complete())
		{
			return NSLOCTEXT("FoundFootageObjective", "Day1Complete", "第一天已完成。");
		}
		if (!GameMode->IsExitUnlocked())
		{
			return NSLOCTEXT("FoundFootageObjective", "ExitLocked", "出口已锁定，请先复查档案。");
		}
		if (const ADeepWaterStationRouteKit* OwningRouteKit = ResolveOwningRouteKit())
		{
			if (OwningRouteKit->IsRouteGatedByEncounter() && !OwningRouteKit->CanResolveEncounterAtExit())
			{
				return NSLOCTEXT("FoundFootageObjective", "RouteBlocked", "路线受阻，先撑过遭遇。");
			}
		}
		return NSLOCTEXT("FoundFootageObjective", "ExitThroughGate", "穿过维修闸门离开。");
	}

	return NSLOCTEXT("FoundFootageObjective", "ObjectiveUnavailableFallback", "目标暂不可用。");
}

FText AFoundFootageObjectiveInteractable::BuildObjectiveActionPrompt(AHorrorGameModeBase* GameMode) const
{
	if (Objective == EFoundFootageInteractableObjective::FirstAnomalyCandidate
		&& GameMode
		&& CanRecordFirstAnomalyFromCandidate(*GameMode))
	{
		return NSLOCTEXT("FoundFootageObjective", "InteractionPromptCaptureAnomaly", "互动键：捕捉异常");
	}

	const FText Label = !DebugLabel.IsEmpty()
		? DebugLabel
		: (!ObjectiveHint.IsEmpty() ? ObjectiveHint : NSLOCTEXT("FoundFootageObjective", "DefaultObjectiveLabel", "目标"));

	return FText::Format(NSLOCTEXT("FoundFootageObjective", "InteractionPromptFormat", "互动键：{0}"), Label);
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

void AFoundFootageObjectiveInteractable::RecordObjectiveHintNoteProgress(AHorrorPlayerCharacter& PlayerCharacter, FName ProgressId) const
{
	UNoteRecorderComponent* NoteRecorder = PlayerCharacter.GetNoteRecorderComponent();
	if (!NoteRecorder || ProgressId.IsNone())
	{
		return;
	}

	const FName HintNoteId(*FString::Printf(TEXT("Note.ObjectiveHint.%s"), *ProgressId.ToString()));

	FHorrorNoteMetadata HintNote;
	HintNote.NoteId = HintNoteId;
	HintNote.Title = !DebugLabel.IsEmpty()
		? DebugLabel
		: (!EvidenceMetadata.DisplayName.IsEmpty() ? EvidenceMetadata.DisplayName : NSLOCTEXT("FoundFootageObjective", "ObjectiveHintTitle", "目标线索"));
	HintNote.Body = !ObjectiveHint.IsEmpty()
		? ObjectiveHint
		: NSLOCTEXT("FoundFootageObjective", "ObjectiveHintBody", "这条线索已记录到目标日志。");

	NoteRecorder->RegisterNoteMetadata(HintNote);
	NoteRecorder->AddRecordedNoteId(HintNoteId);
}

void AFoundFootageObjectiveInteractable::RecordArchiveSummaryProgress(AHorrorPlayerCharacter& PlayerCharacter, FName ArchiveEvidenceId) const
{
	(void)ArchiveEvidenceId;

	UNoteRecorderComponent* NoteRecorder = PlayerCharacter.GetNoteRecorderComponent();
	if (!NoteRecorder)
	{
		return;
	}

	FHorrorNoteMetadata ArchiveSummary;
	ArchiveSummary.NoteId = ArchiveSummaryNoteId;
	ArchiveSummary.Title = NSLOCTEXT("FoundFootageObjective", "ArchiveSummaryTitle", "档案终端摘要");
	ArchiveSummary.Body = FText::AsCultureInvariant(FString::Printf(
		TEXT("档案复查确认了第一异常录像，并恢复了出口闸门指令。维修闸门门禁码：%s。"),
		ArchiveExitGateCode));

	NoteRecorder->RegisterNoteMetadata(ArchiveSummary);
	NoteRecorder->AddRecordedNoteId(ArchiveSummaryNoteId);
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
	case EFoundFootageInteractableObjective::ExitRouteGate:
		return HorrorDay1Tags::Day1CompletedEvent();
	default:
		break;
	}

	return FGameplayTag();
}
