// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorCampaignObjectiveActor.h"

#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorGameModeBase.h"
#include "GameFramework/GameModeBase.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

namespace
{
	const FVector ObjectiveBoundsExtent(75.0f, 75.0f, 95.0f);
	const TCHAR* DefaultObjectiveMeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* DefaultObjectiveVFXPath = TEXT("/Game/Fantastic_Dungeon_Pack/effects/PS_FX_particles_dungeon_01_Niagara.PS_FX_particles_dungeon_01_Niagara");
	constexpr float DefaultSurvivalObjectiveDurationSeconds = 14.0f;
	constexpr float MinimumEscapeDestinationOffsetCm = 100.0f;
	constexpr float AdvancedInteractionSuccessProgress = 0.34f;
	constexpr float CircuitFailureProgressPenalty = 0.2f;
	constexpr float GearFailurePauseSeconds = 3.0f;
	constexpr float AdvancedInteractionCueCycleSeconds = 1.6f;
	constexpr float AdvancedInteractionTimingWindowStart = 0.36f;
	constexpr float AdvancedInteractionTimingWindowEnd = 0.72f;

	const FName CircuitInputs[] = {
		TEXT("蓝色端子"),
		TEXT("红色端子"),
		TEXT("黄色端子")
	};

	const FName GearInputs[] = {
		TEXT("齿轮1"),
		TEXT("齿轮2"),
		TEXT("齿轮3")
	};

	FGameplayTag AmbushStartedEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.AmbushStarted")), false);
	}

	int32 GetDefaultInteractionCountForObjective(EHorrorCampaignObjectiveType ObjectiveType)
	{
		switch (ObjectiveType)
		{
			case EHorrorCampaignObjectiveType::RestorePower:
			case EHorrorCampaignObjectiveType::DisableSeal:
				return 3;
			case EHorrorCampaignObjectiveType::PlantBeacon:
			case EHorrorCampaignObjectiveType::BossWeakPoint:
				return 2;
			case EHorrorCampaignObjectiveType::AcquireSignal:
			case EHorrorCampaignObjectiveType::ScanAnomaly:
			case EHorrorCampaignObjectiveType::RecoverRelic:
			case EHorrorCampaignObjectiveType::SurviveAmbush:
			case EHorrorCampaignObjectiveType::FinalTerminal:
			default:
				return 1;
		}
	}

	EHorrorCampaignInteractionMode GetDefaultInteractionModeForObjective(EHorrorCampaignObjectiveType ObjectiveType)
	{
		switch (ObjectiveType)
		{
			case EHorrorCampaignObjectiveType::RestorePower:
				return EHorrorCampaignInteractionMode::CircuitWiring;
			case EHorrorCampaignObjectiveType::DisableSeal:
				return EHorrorCampaignInteractionMode::GearCalibration;
			case EHorrorCampaignObjectiveType::SurviveAmbush:
				return EHorrorCampaignInteractionMode::TimedPursuit;
			case EHorrorCampaignObjectiveType::PlantBeacon:
			case EHorrorCampaignObjectiveType::BossWeakPoint:
				return EHorrorCampaignInteractionMode::MultiStep;
			case EHorrorCampaignObjectiveType::AcquireSignal:
			case EHorrorCampaignObjectiveType::ScanAnomaly:
			case EHorrorCampaignObjectiveType::RecoverRelic:
			case EHorrorCampaignObjectiveType::FinalTerminal:
			default:
				return EHorrorCampaignInteractionMode::Instant;
		}
	}

	float GetDefaultTimedDurationForObjective(EHorrorCampaignObjectiveType ObjectiveType)
	{
		return ObjectiveType == EHorrorCampaignObjectiveType::SurviveAmbush
			? DefaultSurvivalObjectiveDurationSeconds
			: 0.0f;
	}
}

AHorrorCampaignObjectiveActor::AHorrorCampaignObjectiveActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;

	InteractionBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBounds"));
	InteractionBounds->SetBoxExtent(ObjectiveBoundsExtent);
	InteractionBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionBounds->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionBounds->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	InteractionBounds->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionBounds->SetGenerateOverlapEvents(true);
	RootComponent = InteractionBounds;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(InteractionBounds);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -35.0f));
	VisualMesh->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));

	if (UStaticMesh* DefaultMesh = LoadObject<UStaticMesh>(nullptr, DefaultObjectiveMeshPath))
	{
		VisualMesh->SetStaticMesh(DefaultMesh);
	}

	LabelText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("LabelText"));
	LabelText->SetupAttachment(InteractionBounds);
	LabelText->SetHorizontalAlignment(EHTA_Center);
	LabelText->SetVerticalAlignment(EVRTA_TextCenter);
	LabelText->SetWorldSize(28.0f);
	LabelText->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	LabelText->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

	MarkerLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("MarkerLight"));
	MarkerLight->SetupAttachment(InteractionBounds);
	MarkerLight->SetIntensity(900.0f);
	MarkerLight->SetAttenuationRadius(280.0f);
	MarkerLight->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));

	ObjectiveVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ObjectiveVFX"));
	ObjectiveVFX->SetupAttachment(InteractionBounds);
	ObjectiveVFX->SetRelativeLocation(FVector(0.0f, 0.0f, 20.0f));
	ObjectiveVFX->SetAutoActivate(true);
	if (UNiagaraSystem* VFX = LoadObject<UNiagaraSystem>(nullptr, DefaultObjectiveVFXPath))
	{
		ObjectiveVFX->SetAsset(VFX);
	}

	ApplyObjectiveVisuals();
}

void AHorrorCampaignObjectiveActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateAdvancedInteraction(DeltaTime);
	UpdateTimedObjective(DeltaTime);
}

void AHorrorCampaignObjectiveActor::BeginPlay()
{
	Super::BeginPlay();
	RefreshObjectiveState();
}

void AHorrorCampaignObjectiveActor::ConfigureObjective(FName InChapterId, const FHorrorCampaignObjectiveDefinition& InObjectiveDefinition)
{
	ChapterId = InChapterId;
	ObjectiveDefinition = InObjectiveDefinition;
	ObjectiveDefinition.InteractionMode = InObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::Instant
		? GetDefaultInteractionModeForObjective(ObjectiveDefinition.ObjectiveType)
		: InObjectiveDefinition.InteractionMode;
	bCompleted = false;
	RequiredInteractionCount = FMath::Max(1, GetDefaultInteractionCountForObjective(ObjectiveDefinition.ObjectiveType));
	InteractionProgressCount = 0;
	ResetAdvancedInteractionState();
	bTimedObjectiveActive = false;
	TimedObjectiveDurationSeconds = GetDefaultTimedDurationForObjective(ObjectiveDefinition.ObjectiveType);
	TimedObjectiveRemainingSeconds = TimedObjectiveDurationSeconds;
	bTimedObjectiveUsesEscapeDestination = false;
	TimedObjectiveEscapeDestinationLocation = FVector::ZeroVector;
	TimedObjectiveInstigator.Reset();
	RefreshObjectiveState();
}

bool AHorrorCampaignObjectiveActor::CanInteract_Implementation(AActor* InstigatorActor, const FHitResult& Hit) const
{
	(void)Hit;

	return bAvailableForInteraction
		&& !bCompleted
		&& !bTimedObjectiveActive
		&& (bAdvancedInteractionActive || AdvancedInteractionPauseRemainingSeconds <= 0.0f)
		&& InstigatorActor
		&& !ChapterId.IsNone()
		&& !ObjectiveDefinition.ObjectiveId.IsNone();
}

bool AHorrorCampaignObjectiveActor::Interact_Implementation(AActor* InstigatorActor, const FHitResult& Hit)
{
	if (!CanInteract_Implementation(InstigatorActor, Hit))
	{
		return false;
	}

	if (IsTimedSurvivalObjective())
	{
		StartTimedObjective(InstigatorActor);
		return true;
	}

	if (UsesAdvancedInteraction())
	{
		if (!bAdvancedInteractionActive)
		{
			StartAdvancedInteraction();
			return true;
		}

		return SubmitAdvancedInteractionInput(ExpectedAdvancedInputId, InstigatorActor);
	}

	InteractionProgressCount = FMath::Clamp(InteractionProgressCount + 1, 0, RequiredInteractionCount);
	if (InteractionProgressCount < RequiredInteractionCount)
	{
		RefreshObjectiveState();
		return true;
	}

	if (!CompleteObjective(InstigatorActor))
	{
		InteractionProgressCount = FMath::Max(0, InteractionProgressCount - 1);
		RefreshObjectiveState();
		return false;
	}

	return true;
}

void AHorrorCampaignObjectiveActor::RefreshObjectiveState()
{
	if (bCompleted)
	{
		bAvailableForInteraction = false;
		ApplyCompletedVisuals();
		return;
	}

	if (bTimedObjectiveActive)
	{
		bAvailableForInteraction = false;
		ApplyObjectiveVisuals();
		return;
	}

	if (bAdvancedInteractionActive)
	{
		bAvailableForInteraction = true;
		ApplyObjectiveVisuals();
		return;
	}

	const UWorld* World = GetWorld();
	const AHorrorGameModeBase* HorrorGameMode = World ? World->GetAuthGameMode<AHorrorGameModeBase>() : nullptr;
	bAvailableForInteraction = !HorrorGameMode || HorrorGameMode->CanCompleteCampaignObjective(ChapterId, ObjectiveDefinition.ObjectiveId);
	ApplyObjectiveVisuals();
}

FText AHorrorCampaignObjectiveActor::GetInteractionPromptText() const
{
	if (bCompleted)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "Completed", "已完成");
	}

	if (bTimedObjectiveActive)
	{
		if (bTimedObjectiveUsesEscapeDestination)
		{
			const AActor* InstigatorActor = TimedObjectiveInstigator.Get();
			const float DistanceMeters = InstigatorActor
				? FVector::Dist2D(InstigatorActor->GetActorLocation(), TimedObjectiveEscapeDestinationLocation) / 100.0f
				: 0.0f;
			return FText::Format(
				NSLOCTEXT("HorrorCampaignObjective", "EscapeToDestination", "逃向目标点：{0} 米 / {1} 秒"),
				FText::AsNumber(FMath::CeilToInt(FMath::Max(0.0f, DistanceMeters))),
				FText::AsNumber(FMath::CeilToInt(FMath::Max(0.0f, TimedObjectiveRemainingSeconds))));
		}

		return FText::Format(
			NSLOCTEXT("HorrorCampaignObjective", "Surviving", "坚持中：{0} 秒"),
			FText::AsNumber(FMath::CeilToInt(FMath::Max(0.0f, TimedObjectiveRemainingSeconds))));
	}

	if (bAdvancedInteractionActive)
	{
		return BuildAdvancedInteractionPromptText();
	}

	if (!bAvailableForInteraction)
	{
		const UWorld* World = GetWorld();
		const AHorrorGameModeBase* HorrorGameMode = World ? World->GetAuthGameMode<AHorrorGameModeBase>() : nullptr;
		const FText CurrentObjectiveText = HorrorGameMode ? HorrorGameMode->GetCurrentCampaignObjectivePromptText() : FText::GetEmpty();
		return CurrentObjectiveText.IsEmpty()
			? NSLOCTEXT("HorrorCampaignObjective", "LockedCompleteCurrent", "已锁定：请先完成当前目标")
			: FText::Format(
				NSLOCTEXT("HorrorCampaignObjective", "LockedCompleteNamedCurrent", "已锁定：先完成「{0}」"),
				CurrentObjectiveText);
	}

	const FText BasePrompt = ObjectiveDefinition.PromptText.IsEmpty()
		? NSLOCTEXT("HorrorCampaignObjective", "InteractDefault", "互动")
		: ObjectiveDefinition.PromptText;

	return RequiredInteractionCount > 1
		? FText::Format(
			NSLOCTEXT("HorrorCampaignObjective", "InteractMultiFormat", "互动键：{0} ({1}/{2})"),
			BasePrompt,
			FText::AsNumber(InteractionProgressCount),
			FText::AsNumber(RequiredInteractionCount))
		: FText::Format(NSLOCTEXT("HorrorCampaignObjective", "InteractSingleFormat", "互动键：{0}"), BasePrompt);
}

void AHorrorCampaignObjectiveActor::ApplyObjectiveVisuals()
{
	if (VisualMesh)
	{
		UObject* MeshObject = ObjectiveDefinition.VisualMeshPath.IsValid()
			? ObjectiveDefinition.VisualMeshPath.TryLoad()
			: nullptr;
		if (UStaticMesh* Mesh = Cast<UStaticMesh>(MeshObject))
		{
			VisualMesh->SetStaticMesh(Mesh);
		}
		VisualMesh->SetVisibility(true);
	}

	const FColor ObjectiveColor = (bAvailableForInteraction || bTimedObjectiveActive) ? GetObjectiveColor() : FColor(90, 90, 90);
	if (LabelText)
	{
		const FText BaseLabel = ObjectiveDefinition.PromptText.IsEmpty()
			? FText::FromString(TEXT("目标"))
			: ObjectiveDefinition.PromptText;
		LabelText->SetText(!bAvailableForInteraction
			? bTimedObjectiveActive
				? FText::Format(
					bTimedObjectiveUsesEscapeDestination
						? NSLOCTEXT("HorrorCampaignObjective", "EscapeLabelFormat", "{0}\n逃离 {1} 秒")
						: NSLOCTEXT("HorrorCampaignObjective", "SurviveLabelFormat", "{0}\n坚持 {1} 秒"),
					BaseLabel,
					FText::AsNumber(FMath::CeilToInt(FMath::Max(0.0f, TimedObjectiveRemainingSeconds))))
				: NSLOCTEXT("HorrorCampaignObjective", "LockedLabel", "已锁定")
			: bAdvancedInteractionActive
				? FText::Format(
					NSLOCTEXT("HorrorCampaignObjective", "AdvancedLabelFormat", "{0}\n{1}%"),
					BaseLabel,
					FText::AsNumber(FMath::RoundToInt(AdvancedInteractionProgressFraction * 100.0f)))
			: RequiredInteractionCount > 1
				? FText::Format(
					FText::FromString(TEXT("{0}\n{1}/{2}")),
					BaseLabel,
					FText::AsNumber(InteractionProgressCount),
					FText::AsNumber(RequiredInteractionCount))
				: BaseLabel);
		LabelText->SetTextRenderColor(ObjectiveColor);
	}

	if (MarkerLight)
	{
		MarkerLight->SetLightColor(FLinearColor(ObjectiveColor));
		MarkerLight->SetIntensity(bTimedObjectiveActive ? 1350.0f : bAvailableForInteraction ? 900.0f : 120.0f);
	}

	if (ObjectiveVFX)
	{
		const bool bShowActiveVFX = bAvailableForInteraction || bTimedObjectiveActive;
		ObjectiveVFX->SetVisibility(bShowActiveVFX);
		if (bShowActiveVFX)
		{
			ObjectiveVFX->Activate(true);
		}
		else
		{
			ObjectiveVFX->Deactivate();
		}
	}

	if (InteractionBounds)
	{
		InteractionBounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void AHorrorCampaignObjectiveActor::ApplyCompletedVisuals()
{
	if (InteractionBounds)
	{
		InteractionBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (LabelText)
	{
		LabelText->SetText(ObjectiveDefinition.CompletionText.IsEmpty()
			? FText::FromString(TEXT("已完成"))
			: ObjectiveDefinition.CompletionText);
		LabelText->SetTextRenderColor(FColor(120, 255, 160));
	}

	if (MarkerLight)
	{
		MarkerLight->SetLightColor(FLinearColor(FColor(120, 255, 160)));
		MarkerLight->SetIntensity(450.0f);
	}

	if (ObjectiveVFX)
	{
		ObjectiveVFX->Deactivate();
		ObjectiveVFX->SetVisibility(false);
	}
}

FColor AHorrorCampaignObjectiveActor::GetObjectiveColor() const
{
	if (bTimedObjectiveActive)
	{
		return FColor(255, 140, 60);
	}

	switch (ObjectiveDefinition.ObjectiveType)
	{
		case EHorrorCampaignObjectiveType::RestorePower:
			return FColor(255, 190, 75);
		case EHorrorCampaignObjectiveType::RecoverRelic:
			return FColor(180, 110, 255);
		case EHorrorCampaignObjectiveType::PlantBeacon:
			return FColor(75, 210, 255);
		case EHorrorCampaignObjectiveType::DisableSeal:
			return FColor(255, 95, 95);
		case EHorrorCampaignObjectiveType::BossWeakPoint:
			return FColor(255, 70, 45);
		case EHorrorCampaignObjectiveType::FinalTerminal:
			return FColor(120, 255, 220);
		case EHorrorCampaignObjectiveType::ScanAnomaly:
			return FColor(120, 170, 255);
		case EHorrorCampaignObjectiveType::AcquireSignal:
		case EHorrorCampaignObjectiveType::SurviveAmbush:
		default:
			return FColor(110, 230, 180);
	}
}

bool AHorrorCampaignObjectiveActor::IsTimedSurvivalObjective() const
{
	return ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::TimedPursuit
		&& ObjectiveDefinition.ObjectiveType == EHorrorCampaignObjectiveType::SurviveAmbush
		&& TimedObjectiveDurationSeconds > 0.0f;
}

bool AHorrorCampaignObjectiveActor::UsesAdvancedInteraction() const
{
	return ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring
		|| ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration;
}

bool AHorrorCampaignObjectiveActor::HasConfiguredEscapeDestination() const
{
	return ObjectiveDefinition.ObjectiveType == EHorrorCampaignObjectiveType::SurviveAmbush
		&& ObjectiveDefinition.EscapeDestinationOffset.Size2D() >= MinimumEscapeDestinationOffsetCm;
}

FVector AHorrorCampaignObjectiveActor::ResolveEscapeDestinationWorldLocation() const
{
	const FRotator YawRotation(0.0f, GetActorRotation().Yaw, 0.0f);
	const FRotationMatrix RotationMatrix(YawRotation);
	const FVector EscapeOffset =
		RotationMatrix.GetUnitAxis(EAxis::X) * ObjectiveDefinition.EscapeDestinationOffset.X
		+ RotationMatrix.GetUnitAxis(EAxis::Y) * ObjectiveDefinition.EscapeDestinationOffset.Y
		+ FVector(0.0f, 0.0f, ObjectiveDefinition.EscapeDestinationOffset.Z);

	return GetActorLocation() + EscapeOffset;
}

void AHorrorCampaignObjectiveActor::StartAdvancedInteraction()
{
	if (!UsesAdvancedInteraction() || bCompleted)
	{
		return;
	}

	bAdvancedInteractionActive = true;
	AdvancedInteractionProgressFraction = 0.0f;
	AdvancedInteractionStepIndex = 0;
	AdvancedInteractionPauseRemainingSeconds = 0.0f;
	ResetAdvancedInteractionCue();
	AdvancedInteractionFeedbackText = ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring
		? NSLOCTEXT("HorrorCampaignObjective", "CircuitStarted", "接线盘已展开，等端子亮到蓝色窗口再接入。")
		: NSLOCTEXT("HorrorCampaignObjective", "GearStarted", "齿轮组已展开，等停转齿轮进入校准窗口再拨动。");
	AdvanceExpectedAdvancedInput();
	RefreshObjectiveState();
}

void AHorrorCampaignObjectiveActor::ResetAdvancedInteractionState()
{
	bAdvancedInteractionActive = false;
	AdvancedInteractionProgressFraction = 0.0f;
	AdvancedInteractionStepIndex = 0;
	ExpectedAdvancedInputId = NAME_None;
	AdvancedInteractionCueElapsedSeconds = 0.0f;
	AdvancedInteractionTimingFraction = 0.0f;
	bAdvancedInteractionCueResolved = false;
	AdvancedInteractionFeedbackText = FText::GetEmpty();
	AdvancedInteractionPauseRemainingSeconds = 0.0f;
}

void AHorrorCampaignObjectiveActor::ResetAdvancedInteractionCue()
{
	AdvancedInteractionCueElapsedSeconds = 0.0f;
	AdvancedInteractionTimingFraction = 0.0f;
	bAdvancedInteractionCueResolved = false;
}

void AHorrorCampaignObjectiveActor::AdvanceExpectedAdvancedInput()
{
	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring)
	{
		const int32 InputIndex = AdvancedInteractionStepIndex % UE_ARRAY_COUNT(CircuitInputs);
		ExpectedAdvancedInputId = CircuitInputs[InputIndex];
		return;
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration)
	{
		const int32 InputIndex = AdvancedInteractionStepIndex % UE_ARRAY_COUNT(GearInputs);
		ExpectedAdvancedInputId = GearInputs[InputIndex];
		return;
	}

	ExpectedAdvancedInputId = NAME_None;
}

bool AHorrorCampaignObjectiveActor::IsAdvancedInteractionTimingWindowOpen() const
{
	return bAdvancedInteractionActive
		&& !bAdvancedInteractionCueResolved
		&& AdvancedInteractionPauseRemainingSeconds <= 0.0f
		&& AdvancedInteractionTimingFraction >= AdvancedInteractionTimingWindowStart
		&& AdvancedInteractionTimingFraction <= AdvancedInteractionTimingWindowEnd;
}

FHorrorAdvancedInteractionHUDState AHorrorCampaignObjectiveActor::BuildAdvancedInteractionHUDState() const
{
	FHorrorAdvancedInteractionHUDState State;
	State.bVisible = bAdvancedInteractionActive && UsesAdvancedInteraction() && !bCompleted;
	State.Mode = ObjectiveDefinition.InteractionMode;
	State.Title = ObjectiveDefinition.PromptText.IsEmpty()
		? NSLOCTEXT("HorrorCampaignObjective", "AdvancedHUDDefaultTitle", "校准目标")
		: ObjectiveDefinition.PromptText;
	State.ExpectedInputId = ExpectedAdvancedInputId;
	State.FeedbackText = AdvancedInteractionFeedbackText;
	State.ProgressFraction = FMath::Clamp(AdvancedInteractionProgressFraction, 0.0f, 1.0f);
	State.TimingFraction = FMath::Clamp(AdvancedInteractionTimingFraction, 0.0f, 1.0f);
	State.PauseRemainingSeconds = FMath::Max(0.0f, AdvancedInteractionPauseRemainingSeconds);
	State.bTimingWindowOpen = IsAdvancedInteractionTimingWindowOpen();
	State.bPaused = AdvancedInteractionPauseRemainingSeconds > 0.0f;
	State.StepIndex = FMath::Max(0, AdvancedInteractionStepIndex);
	State.RequiredStepCount = FMath::Max(RequiredInteractionCount, 1);
	return State;
}

void AHorrorCampaignObjectiveActor::ApplyAdvancedInteractionTimingFailure()
{
	bAdvancedInteractionCueResolved = true;
	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring)
	{
		AdvancedInteractionProgressFraction = FMath::Clamp(
			AdvancedInteractionProgressFraction - CircuitFailureProgressPenalty,
			0.0f,
			0.99f);
		InteractionProgressCount = FMath::FloorToInt(AdvancedInteractionProgressFraction * RequiredInteractionCount);
		AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "CircuitTimingFailure", "时机过早：红色火花炸开，进度回退。");
		ResetAdvancedInteractionCue();
	}
	else
	{
		AdvancedInteractionPauseRemainingSeconds = GearFailurePauseSeconds;
		AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "GearTimingFailure", "时机不对：齿轮咬死，校准暂停三秒。");
	}
}

void AHorrorCampaignObjectiveActor::ApplyAdvancedInteractionWrongInputFailure()
{
	bAdvancedInteractionCueResolved = true;
	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring)
	{
		AdvancedInteractionProgressFraction = FMath::Clamp(
			AdvancedInteractionProgressFraction - CircuitFailureProgressPenalty,
			0.0f,
			0.99f);
		InteractionProgressCount = FMath::FloorToInt(AdvancedInteractionProgressFraction * RequiredInteractionCount);
		AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "CircuitFailure", "红色火花：接线错误，进度回退。");
		ResetAdvancedInteractionCue();
	}
	else
	{
		AdvancedInteractionPauseRemainingSeconds = GearFailurePauseSeconds;
		AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "GearFailure", "咔哒：齿轮卡死，校准暂停三秒。");
	}
}

bool AHorrorCampaignObjectiveActor::SubmitAdvancedInteractionInput(FName InputId, AActor* InstigatorActor)
{
	if (!bAdvancedInteractionActive || bCompleted || !UsesAdvancedInteraction())
	{
		return false;
	}

	if (AdvancedInteractionPauseRemainingSeconds > 0.0f)
	{
		AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "AdvancedPaused", "机构卡死，等待齿轮重新咬合。");
		RefreshObjectiveState();
		return false;
	}

	if (!IsAdvancedInteractionTimingWindowOpen())
	{
		ApplyAdvancedInteractionTimingFailure();
		RefreshObjectiveState();
		return true;
	}

	const bool bCorrectInput = InputId == ExpectedAdvancedInputId;
	if (!bCorrectInput)
	{
		ApplyAdvancedInteractionWrongInputFailure();
		RefreshObjectiveState();
		return true;
	}

	AdvancedInteractionProgressFraction = FMath::Clamp(
		AdvancedInteractionProgressFraction + AdvancedInteractionSuccessProgress,
		0.0f,
		1.0f);
	InteractionProgressCount = FMath::Clamp(
		FMath::CeilToInt(AdvancedInteractionProgressFraction * RequiredInteractionCount),
		0,
		RequiredInteractionCount);

	if (AdvancedInteractionProgressFraction >= 1.0f)
	{
		return CompleteAdvancedInteraction(InstigatorActor);
	}

	AdvancedInteractionFeedbackText = ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring
		? NSLOCTEXT("HorrorCampaignObjective", "CircuitSuccess", "蓝色电弧：线路接入成功。")
		: NSLOCTEXT("HorrorCampaignObjective", "GearSuccess", "齿轮重新咬合，机械节律恢复。");
	++AdvancedInteractionStepIndex;
	AdvanceExpectedAdvancedInput();
	ResetAdvancedInteractionCue();
	RefreshObjectiveState();
	return true;
}

bool AHorrorCampaignObjectiveActor::CompleteAdvancedInteraction(AActor* InstigatorActor)
{
	bAdvancedInteractionActive = false;
	AdvancedInteractionProgressFraction = 1.0f;
	InteractionProgressCount = RequiredInteractionCount;
	AdvancedInteractionFeedbackText = ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring
		? NSLOCTEXT("HorrorCampaignObjective", "CircuitComplete", "电路闭合，系统恢复。")
		: NSLOCTEXT("HorrorCampaignObjective", "GearComplete", "齿轮校准完成，封锁解除。");
	ExpectedAdvancedInputId = NAME_None;
	AdvancedInteractionPauseRemainingSeconds = 0.0f;
	return CompleteObjective(InstigatorActor);
}

FText AHorrorCampaignObjectiveActor::BuildAdvancedInteractionPromptText() const
{
	const FText BasePrompt = ObjectiveDefinition.PromptText.IsEmpty()
		? NSLOCTEXT("HorrorCampaignObjective", "AdvancedDefault", "校准目标")
		: ObjectiveDefinition.PromptText;
	const int32 ProgressPercent = FMath::RoundToInt(AdvancedInteractionProgressFraction * 100.0f);
	const int32 TimingPercent = FMath::RoundToInt(AdvancedInteractionTimingFraction * 100.0f);

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring)
	{
		return FText::Format(
			NSLOCTEXT("HorrorCampaignObjective", "CircuitPrompt", "{0}\n接线中：端子 {1}  时机 {2}%  进度 {3}%\n{4}"),
			BasePrompt,
			FText::FromName(ExpectedAdvancedInputId),
			FText::AsNumber(TimingPercent),
			FText::AsNumber(ProgressPercent),
			AdvancedInteractionFeedbackText.IsEmpty()
				? NSLOCTEXT("HorrorCampaignObjective", "CircuitHint", "光标进入蓝色窗口时按互动键。")
				: AdvancedInteractionFeedbackText);
	}

	return FText::Format(
		NSLOCTEXT("HorrorCampaignObjective", "GearPrompt", "{0}\n齿轮校准：停转 {1}  时机 {2}%  进度 {3}%  停顿 {4} 秒\n{5}"),
		BasePrompt,
		FText::FromName(ExpectedAdvancedInputId),
		FText::AsNumber(TimingPercent),
		FText::AsNumber(ProgressPercent),
		FText::AsNumber(FMath::CeilToInt(FMath::Max(0.0f, AdvancedInteractionPauseRemainingSeconds))),
		AdvancedInteractionFeedbackText.IsEmpty()
			? NSLOCTEXT("HorrorCampaignObjective", "GearHint", "停转齿轮进入校准窗口时按互动键。")
			: AdvancedInteractionFeedbackText);
}

bool AHorrorCampaignObjectiveActor::HasTimedObjectiveReachedEscapeDestination() const
{
	if (!bTimedObjectiveUsesEscapeDestination)
	{
		return true;
	}

	const AActor* InstigatorActor = TimedObjectiveInstigator.Get();
	if (!InstigatorActor)
	{
		return false;
	}

	return FVector::DistSquared2D(InstigatorActor->GetActorLocation(), TimedObjectiveEscapeDestinationLocation)
		<= FMath::Square(GetEscapeCompletionRadius());
}

void AHorrorCampaignObjectiveActor::StartTimedObjective(AActor* InstigatorActor)
{
	if (!IsTimedSurvivalObjective() || bTimedObjectiveActive || bCompleted)
	{
		return;
	}

	TimedObjectiveInstigator = InstigatorActor;
	TimedObjectiveRemainingSeconds = TimedObjectiveDurationSeconds;
	bTimedObjectiveUsesEscapeDestination = HasConfiguredEscapeDestination();
	TimedObjectiveEscapeDestinationLocation = bTimedObjectiveUsesEscapeDestination
		? ResolveEscapeDestinationWorldLocation()
		: GetActorLocation();
	bTimedObjectiveActive = true;
	InteractionProgressCount = 0;
	PublishAmbushStartedEvent(InstigatorActor);
	if (UWorld* World = GetWorld())
	{
		if (AHorrorGameModeBase* HorrorGameMode = World->GetAuthGameMode<AHorrorGameModeBase>())
		{
			HorrorGameMode->StartCampaignAmbushThreat(
				ObjectiveDefinition.ObjectiveId,
				InstigatorActor ? InstigatorActor : this);
		}
	}
	if (bTimedObjectiveUsesEscapeDestination)
	{
		SetActorLocation(TimedObjectiveEscapeDestinationLocation, false);
	}
	RefreshObjectiveState();
}

void AHorrorCampaignObjectiveActor::UpdateAdvancedInteraction(float DeltaTime)
{
	if (!bAdvancedInteractionActive || DeltaTime <= 0.0f)
	{
		return;
	}

	if (AdvancedInteractionPauseRemainingSeconds > 0.0f)
	{
		AdvancedInteractionPauseRemainingSeconds = FMath::Max(0.0f, AdvancedInteractionPauseRemainingSeconds - DeltaTime);
		if (AdvancedInteractionPauseRemainingSeconds <= 0.0f)
		{
			ResetAdvancedInteractionCue();
			AdvancedInteractionFeedbackText = ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration
				? NSLOCTEXT("HorrorCampaignObjective", "GearPauseCleared", "齿轮重新松动，继续校准。")
				: NSLOCTEXT("HorrorCampaignObjective", "AdvancedPauseCleared", "系统复位，继续操作。");
		}
		RefreshObjectiveState();
		return;
	}

	if (!bAdvancedInteractionCueResolved)
	{
		AdvancedInteractionCueElapsedSeconds += DeltaTime;
		AdvancedInteractionTimingFraction = FMath::Clamp(
			AdvancedInteractionCueElapsedSeconds / AdvancedInteractionCueCycleSeconds,
			0.0f,
			1.0f);

		if (AdvancedInteractionTimingFraction >= 1.0f)
		{
			ApplyAdvancedInteractionTimingFailure();
		}
	}
	RefreshObjectiveState();
}

void AHorrorCampaignObjectiveActor::UpdateTimedObjective(float DeltaTime)
{
	if (!bTimedObjectiveActive || bCompleted || DeltaTime <= 0.0f)
	{
		return;
	}

	TimedObjectiveRemainingSeconds = FMath::Max(0.0f, TimedObjectiveRemainingSeconds - DeltaTime);
	if (TimedObjectiveRemainingSeconds > 0.0f || !HasTimedObjectiveReachedEscapeDestination())
	{
		ApplyObjectiveVisuals();
		return;
	}

	bTimedObjectiveActive = false;
	const bool bCompletedObjective = CompleteObjective(TimedObjectiveInstigator.Get());
	if (UWorld* World = GetWorld())
	{
		if (AHorrorGameModeBase* HorrorGameMode = World->GetAuthGameMode<AHorrorGameModeBase>())
		{
			HorrorGameMode->StopCampaignAmbushThreat(ObjectiveDefinition.ObjectiveId);
		}
	}

	if (!bCompletedObjective)
	{
		TimedObjectiveRemainingSeconds = TimedObjectiveDurationSeconds;
		bTimedObjectiveUsesEscapeDestination = false;
		TimedObjectiveEscapeDestinationLocation = FVector::ZeroVector;
		TimedObjectiveInstigator.Reset();
		RefreshObjectiveState();
	}
}

bool AHorrorCampaignObjectiveActor::CompleteObjective(AActor* InstigatorActor)
{
	UWorld* World = GetWorld();
	AHorrorGameModeBase* HorrorGameMode = World ? World->GetAuthGameMode<AHorrorGameModeBase>() : nullptr;
	if (HorrorGameMode && !HorrorGameMode->TryCompleteCampaignObjective(ChapterId, ObjectiveDefinition.ObjectiveId, InstigatorActor))
	{
		return false;
	}

	bCompleted = true;
	bTimedObjectiveActive = false;
	bAdvancedInteractionActive = false;
	TimedObjectiveRemainingSeconds = 0.0f;
	bTimedObjectiveUsesEscapeDestination = false;
	TimedObjectiveInstigator.Reset();
	InteractionProgressCount = RequiredInteractionCount;
	RefreshObjectiveState();
	return true;
}

void AHorrorCampaignObjectiveActor::PublishAmbushStartedEvent(AActor* InstigatorActor) const
{
	UWorld* World = GetWorld();
	UHorrorEventBusSubsystem* EventBus = World ? World->GetSubsystem<UHorrorEventBusSubsystem>() : nullptr;
	const FGameplayTag EventTag = AmbushStartedEventTag();
	if (EventBus && EventTag.IsValid())
	{
		EventBus->Publish(
			EventTag,
			ObjectiveDefinition.ObjectiveId,
			EventTag,
			InstigatorActor ? InstigatorActor : const_cast<AHorrorCampaignObjectiveActor*>(this));
	}
}
