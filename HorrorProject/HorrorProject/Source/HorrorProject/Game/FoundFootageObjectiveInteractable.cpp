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

FText AFoundFootageObjectiveInteractable::GetInteractionPromptText(AActor* InstigatorActor) const
{
	AHorrorGameModeBase* GameMode = ResolveObjectiveGameMode();
	if (CanCompleteObjectiveForInstigator(GameMode, InstigatorActor))
	{
		return BuildObjectiveActionPrompt();
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

FText AFoundFootageObjectiveInteractable::BuildBlockedObjectivePrompt(AHorrorGameModeBase* GameMode, AActor* InstigatorActor) const
{
	if (!GameMode)
	{
		return FText::FromString(TEXT("目标不可用。"));
	}

	switch (Objective)
	{
	case EFoundFootageInteractableObjective::Bodycam:
		return GameMode->HasBodycamAcquired()
			? FText::FromString(TEXT("随身摄像机已经找回。"))
			: FText::FromString(TEXT("找回随身摄像机。"));

	case EFoundFootageInteractableObjective::FirstNote:
		if (!GameMode->HasBodycamAcquired())
		{
			return FText::FromString(TEXT("先找回随身摄像机。"));
		}
		return GameMode->HasCollectedFirstNote()
			? FText::FromString(TEXT("笔记已经记录。"))
			: FText::FromString(TEXT("阅读站内笔记。"));

	case EFoundFootageInteractableObjective::FirstAnomalyCandidate:
		if (!GameMode->HasCollectedFirstNote())
		{
			return FText::FromString(TEXT("先找到第一份站内笔记。"));
		}
		if (GameMode->HasPendingFirstAnomalyCandidate())
		{
			return FText::FromString(TEXT("异常点已对准，开始录制。"));
		}
		return GameMode->HasRecordedFirstAnomaly()
			? FText::FromString(TEXT("异常已经记录。"))
			: FText::FromString(TEXT("对准异常点。"));

	case EFoundFootageInteractableObjective::FirstAnomalyRecord:
		if (!GameMode->HasPendingFirstAnomalyCandidate())
		{
			return FText::FromString(TEXT("先对准异常点。"));
		}
		if (!GameMode->IsLeadPlayerRecording())
		{
	return FText::FromString(TEXT("先开始录制，再按 E键记录异常点。"));
		}
		return GameMode->HasRecordedFirstAnomaly()
			? FText::FromString(TEXT("异常已经记录。"))
			: FText::FromString(TEXT("捕获异常。"));

	case EFoundFootageInteractableObjective::ArchiveReview:
		if (!GameMode->HasRecordedFirstAnomaly())
		{
			return FText::FromString(TEXT("先记录异常点，再使用档案终端。"));
		}
		if (!InstigatorActor)
		{
			return FText::FromString(TEXT("靠近档案终端。"));
		}
		return GameMode->HasReviewedArchive()
			? FText::FromString(TEXT("档案已经审查。"))
			: FText::FromString(TEXT("缺少所需录像证据。"));

	case EFoundFootageInteractableObjective::ExitRouteGate:
		if (GameMode->IsDay1Complete())
		{
			return FText::FromString(TEXT("第 1 天完成。"));
		}
		if (!GameMode->IsExitUnlocked())
		{
			return FText::FromString(TEXT("出口已锁定。先审查档案。"));
		}
		if (const ADeepWaterStationRouteKit* OwningRouteKit = ResolveOwningRouteKit())
		{
			if (OwningRouteKit->IsRouteGatedByEncounter() && !OwningRouteKit->CanResolveEncounterAtExit())
			{
				return FText::FromString(TEXT("路线被封锁。先撑过这次遭遇。"));
			}
		}
		return FText::FromString(TEXT("从勤务闸门离开。"));
	}

	return FText::FromString(TEXT("目标不可用。"));
}

FText AFoundFootageObjectiveInteractable::BuildObjectiveActionPrompt() const
{
	const FText Label = !DebugLabel.IsEmpty()
		? DebugLabel
		: (!ObjectiveHint.IsEmpty() ? ObjectiveHint : FText::FromString(TEXT("目标")));

	return FText::Format(FText::FromString(TEXT("E键  {0}")), Label);
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
		: (!EvidenceMetadata.DisplayName.IsEmpty() ? EvidenceMetadata.DisplayName : FText::FromString(TEXT("目标提示")));
	HintNote.Body = !ObjectiveHint.IsEmpty()
		? ObjectiveHint
		: FText::FromString(TEXT("这条线索已记录到目标日志。"));

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
	ArchiveSummary.Title = FText::FromString(TEXT("档案终端摘要"));
	ArchiveSummary.Body = FText::FromString(FString::Printf(
		TEXT("档案审查已确认一号异常录像，并恢复出口闸门指令。勤务闸门密码：%s。"),
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
