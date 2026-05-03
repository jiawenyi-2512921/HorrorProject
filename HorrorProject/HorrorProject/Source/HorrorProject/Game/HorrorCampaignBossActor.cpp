// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorCampaignBossActor.h"

#include "Animation/AnimationAsset.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "CollisionQueryParams.h"
#include "Materials/MaterialInterface.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorGameModeBase.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Player/Components/FearComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FVector BossMeshRelativeLocation(0.0f, 0.0f, -95.0f);
	const FRotator BossMeshRelativeRotation(0.0f, -90.0f, 0.0f);
	const FVector BossMeshRelativeScale(1.35f, 1.35f, 1.35f);
	const TCHAR* BossVFXPath = TEXT("/Game/Fantastic_Dungeon_Pack/effects/PS_FX_fire_dungeon_03_Niagara.PS_FX_fire_dungeon_03_Niagara");
	const FName BossAttackFailureCause(TEXT("Failure.Boss.StoneGolemAttack"));
	constexpr float BossRunAnimationReferenceSpeedCmPerSecond = 180.0f;
	constexpr float BossRunAnimationMinPlayRate = 0.72f;
	constexpr float BossRunAnimationMaxPlayRate = 1.85f;
	constexpr float BossAttackMaxVerticalDeltaCm = 180.0f;
	constexpr float BossMovementLineTraceHeightCm = 85.0f;
	constexpr float BossNavigationLookAheadCm = 650.0f;
	constexpr float BossCornerSidestepMinLateralCm = 180.0f;
	constexpr float BossCornerSidestepProbeSlackCm = 20.0f;
	constexpr float BossFairChaseMoveSpeedMaxCmPerSecond = 180.0f;
	constexpr float BossFairAttackRadiusMaxCm = 180.0f;
	constexpr float BossFairFearPressureRadiusMaxCm = 1400.0f;
	constexpr float BossFairFearPressurePerSecondMax = 1.4f;
	constexpr float BossFairActorScaleMax = 1.35f;

	FGameplayTag BossAttackEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.BossAttack")), false);
	}
}

AHorrorCampaignBossActor::AHorrorCampaignBossActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.25f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	BossMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BossMesh"));
	BossMesh->SetupAttachment(SceneRoot);
	BossMesh->SetRelativeLocation(BossMeshRelativeLocation);
	BossMesh->SetRelativeRotation(BossMeshRelativeRotation);
	BossMesh->SetRelativeScale3D(BossMeshRelativeScale);
	BossMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> GolemMesh(
		TEXT("/Game/Stone_Golem/mesh/SKM_Stone_Golem.SKM_Stone_Golem"));
	static ConstructorHelpers::FObjectFinder<UAnimationAsset> GolemIdleAnimation(
		TEXT("/Game/Stone_Golem/demo/animations/ThirdPersonIdle.ThirdPersonIdle"));
	static ConstructorHelpers::FObjectFinder<UAnimationAsset> GolemRunAnimation(
		TEXT("/Game/Stone_Golem/demo/animations/ThirdPersonRun.ThirdPersonRun"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultGolemMaterialAsset(
		TEXT("/Game/Stone_Golem/materials/M_Stone_Golem.M_Stone_Golem"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DeepWaterGolemMaterialAsset(
		TEXT("/Game/Stone_Golem/materials/MI_Stone_Golem_Inst.MI_Stone_Golem_Inst"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ForestGolemMaterialAsset(
		TEXT("/Game/Stone_Golem/materials/MI_Stone_Golem_Inst1.MI_Stone_Golem_Inst1"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DungeonGolemMaterialAsset(
		TEXT("/Game/Stone_Golem/materials/MI_Stone_Golem_Inst2.MI_Stone_Golem_Inst2"));
	IdleAnimation = GolemIdleAnimation.Object;
	RunAnimation = GolemRunAnimation.Object;
	DefaultGolemMaterial = DefaultGolemMaterialAsset.Object;
	DeepWaterGolemMaterial = DeepWaterGolemMaterialAsset.Object;
	ForestGolemMaterial = ForestGolemMaterialAsset.Object;
	DungeonGolemMaterial = DungeonGolemMaterialAsset.Object;
	if (GolemMesh.Succeeded())
	{
		BossMesh->SetSkeletalMesh(GolemMesh.Object);
	}

	BossLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("BossLabel"));
	BossLabel->SetupAttachment(SceneRoot);
	BossLabel->SetHorizontalAlignment(EHTA_Center);
	BossLabel->SetVerticalAlignment(EVRTA_TextCenter);
	BossLabel->SetWorldSize(42.0f);
	BossLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 230.0f));
	BossLabel->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));

	BossLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BossLight"));
	BossLight->SetupAttachment(SceneRoot);
	BossLight->SetIntensity(2600.0f);
	BossLight->SetAttenuationRadius(700.0f);
	BossLight->SetRelativeLocation(FVector(0.0f, 0.0f, 140.0f));

	BossVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("BossVFX"));
	BossVFX->SetupAttachment(SceneRoot);
	BossVFX->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f));
	BossVFX->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
	BossVFX->SetAutoActivate(true);
	if (UNiagaraSystem* VFX = LoadObject<UNiagaraSystem>(nullptr, BossVFXPath))
	{
		BossVFX->SetAsset(VFX);
	}

	BossName = NSLOCTEXT("HorrorCampaignBoss", "DefaultBossName", "石像巨人");
	ApplyBossVisuals();
	ApplyBossAnimation();
}

void AHorrorCampaignBossActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateBossMovementAndAttacks(DeltaTime);
	ApplyBossPressureToPlayerPawns(DeltaTime);
}

void AHorrorCampaignBossActor::BeginPlay()
{
	Super::BeginPlay();
	ApplyBossVisuals();
}

void AHorrorCampaignBossActor::ConfigureBoss(FName InChapterId, FText InBossName, int32 InRequiredWeakPointCount)
{
	ChapterId = InChapterId;
	BossName = ResolveProfiledBossName(InChapterId, InBossName);
	RequiredWeakPointCount = FMath::Max(0, InRequiredWeakPointCount);
	ResolvedWeakPointCount = 0;
	bDefeated = false;
	bAwake = false;
	LastBossAttackWorldSeconds = -100000.0f;
	ApplyBossVisuals();
	ApplyBossAnimation();
}

void AHorrorCampaignBossActor::SetBossDefeated(bool bInDefeated)
{
	const int32 TargetResolvedWeakPointCount = bInDefeated ? RequiredWeakPointCount : 0;
	const bool bTargetAwake = bInDefeated ? false : bAwake;
	if (bDefeated == bInDefeated && bAwake == bTargetAwake && ResolvedWeakPointCount == TargetResolvedWeakPointCount)
	{
		return;
	}

	bDefeated = bInDefeated;
	bAwake = bTargetAwake;
	ResolvedWeakPointCount = TargetResolvedWeakPointCount;
	ApplyBossVisuals();
	ApplyBossAnimation();
}

void AHorrorCampaignBossActor::SetBossAwake(bool bInAwake)
{
	const bool bTargetAwake = bInAwake && !bDefeated;
	if (bAwake == bTargetAwake)
	{
		return;
	}

	bAwake = bTargetAwake;
	LastBossAttackWorldSeconds = -100000.0f;
	ApplyBossVisuals();
	ApplyBossAnimation();
	if (bAwake)
	{
		BP_OnBossAwake();
	}
}

void AHorrorCampaignBossActor::ConfigureChasePressure(
	float InMoveSpeed,
	float InEngageRadius,
	float InAttackRadius,
	float InFearPressureRadius,
	float InActorScale)
{
	ConfigureChasePressureWithFearRate(
		InMoveSpeed,
		InEngageRadius,
		InAttackRadius,
		InFearPressureRadius,
		FearPressurePerSecond,
		InActorScale);
}

void AHorrorCampaignBossActor::ConfigureChasePressureWithFearRate(
	float InMoveSpeed,
	float InEngageRadius,
	float InAttackRadius,
	float InFearPressureRadius,
	float InFearPressurePerSecond,
	float InActorScale)
{
	BossMoveSpeed = FMath::Clamp(InMoveSpeed, 0.0f, BossFairChaseMoveSpeedMaxCmPerSecond);
	BossEngageRadius = FMath::Max(0.0f, InEngageRadius);
	BossAttackRadius = FMath::Clamp(InAttackRadius, 0.0f, BossFairAttackRadiusMaxCm);
	FearPressureRadius = FMath::Clamp(InFearPressureRadius, 0.0f, BossFairFearPressureRadiusMaxCm);
	FearPressurePerSecond = FMath::Clamp(InFearPressurePerSecond, 0.0f, BossFairFearPressurePerSecondMax);
	const float SafeActorScale = FMath::Clamp(InActorScale, 0.1f, BossFairActorScaleMax);
	SetActorScale3D(FVector(SafeActorScale));
	ApplyBossAnimation();
}

bool AHorrorCampaignBossActor::RegisterWeakPointResolved()
{
	if (bDefeated || RequiredWeakPointCount <= 0 || ResolvedWeakPointCount >= RequiredWeakPointCount)
	{
		return false;
	}

	++ResolvedWeakPointCount;
	bDefeated = ResolvedWeakPointCount >= RequiredWeakPointCount;
	if (bDefeated)
	{
		bAwake = false;
	}
	ApplyBossVisuals();
	ApplyBossAnimation();
	return true;
}

bool AHorrorCampaignBossActor::ApplyBossPressureToActor(AActor* TargetActor, float DeltaTime)
{
	if (!TargetActor)
	{
		return false;
	}

	UFearComponent* FearComponent = TargetActor->FindComponentByClass<UFearComponent>();
	if (!FearComponent)
	{
		return false;
	}

	const float PressureAmount = CalculateFearPressureAmount(FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation()), DeltaTime);
	if (PressureAmount <= 0.0f)
	{
		return false;
	}

	return FearComponent->AddFear(PressureAmount, ChapterId.IsNone() ? FName(TEXT("CampaignBoss")) : ChapterId);
}

bool AHorrorCampaignBossActor::MoveBossTowardActor(AActor* TargetActor, float DeltaTime)
{
	if (!bEnableBossMovement || !IsBossAwake() || !TargetActor || DeltaTime <= 0.0f || BossMoveSpeed <= 0.0f)
	{
		return false;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();
	FVector ToTarget = TargetLocation - CurrentLocation;
	ToTarget.Z = 0.0f;

	const float DistanceToTarget = ToTarget.Size();
	if (DistanceToTarget <= BossAttackRadius || DistanceToTarget > BossEngageRadius || DistanceToTarget <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const float DesiredStopDistance = FMath::Max(BossAttackRadius * 0.85f, 1.0f);
	const float StepDistance = FMath::Min(DistanceToTarget - DesiredStopDistance, BossMoveSpeed * DeltaTime);
	if (StepDistance <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	FVector MoveTargetLocation = TargetLocation;
	TryResolveNavigableMoveTarget(TargetLocation, MoveTargetLocation);
	FHitResult MovementBlockHit;
	if (TryGetMovementLineBlockHit(MoveTargetLocation, TargetActor, MovementBlockHit))
	{
		if (!TryResolveCornerSidestepMoveTarget(TargetLocation, TargetActor, MovementBlockHit, StepDistance, MoveTargetLocation))
		{
			return false;
		}
	}

	FVector MoveVector = MoveTargetLocation - CurrentLocation;
	MoveVector.Z = 0.0f;
	if (MoveVector.SizeSquared() <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const FVector MoveDirection = MoveVector.GetSafeNormal();
	FHitResult SweepHit;
	SetActorLocation(CurrentLocation + MoveDirection * StepDistance, true, &SweepHit);
	SetActorRotation(FRotator(0.0f, MoveDirection.Rotation().Yaw, 0.0f));
	return true;
}

bool AHorrorCampaignBossActor::CanBossAttackActor(AActor* TargetActor) const
{
	if (!IsBossAwake() || !TargetActor || BossAttackRadius <= 0.0f)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const float WorldSeconds = World ? World->GetTimeSeconds() : 0.0f;
	if (WorldSeconds - LastBossAttackWorldSeconds < BossAttackCooldownSeconds)
	{
		return false;
	}

	return FVector::DistSquared2D(GetActorLocation(), TargetActor->GetActorLocation()) <= FMath::Square(BossAttackRadius)
		&& FMath::Abs(GetActorLocation().Z - TargetActor->GetActorLocation().Z) <= BossAttackMaxVerticalDeltaCm
		&& HasClearAttackLineToActor(TargetActor);
}

bool AHorrorCampaignBossActor::TryTriggerBossAttack(AActor* TargetActor)
{
	if (!CanBossAttackActor(TargetActor))
	{
		return false;
	}

	const UWorld* World = GetWorld();
	LastBossAttackWorldSeconds = World ? World->GetTimeSeconds() : LastBossAttackWorldSeconds + BossAttackCooldownSeconds;
	BP_OnBossAttack(TargetActor);

	if (UWorld* MutableWorld = GetWorld())
	{
		if (UHorrorEventBusSubsystem* EventBus = MutableWorld->GetSubsystem<UHorrorEventBusSubsystem>())
		{
			const FGameplayTag EventTag = BossAttackEventTag();
			EventBus->Publish(EventTag, ChapterId.IsNone() ? BossAttackFailureCause : ChapterId, EventTag, this);
		}
	}

	if (bBossAttackRequestsPlayerFailure)
	{
		if (AHorrorGameModeBase* GameMode = World ? World->GetAuthGameMode<AHorrorGameModeBase>() : nullptr)
		{
			if (!GameMode->AbortActiveCampaignPursuitForRecovery(BossAttackFailureCause, TargetActor))
			{
				GameMode->RequestPlayerFailure(
					BossAttackFailureCause,
					NSLOCTEXT("HorrorCampaignBoss", "BossAttackFailure", "石像巨人的重击打断了你的行动。"));
			}
		}
	}

	return true;
}

float AHorrorCampaignBossActor::CalculateFearPressureAmount(float DistanceCm, float DeltaTime) const
{
	if (!bEnableFearPressure || !IsBossAwake() || DeltaTime <= 0.0f || FearPressureRadius <= 0.0f || FearPressurePerSecond <= 0.0f)
	{
		return 0.0f;
	}

	const float DistanceAlpha = 1.0f - FMath::Clamp(DistanceCm / FearPressureRadius, 0.0f, 1.0f);
	if (DistanceAlpha <= 0.0f)
	{
		return 0.0f;
	}

	const float ProximityMultiplier = FMath::Lerp(0.35f, 1.0f, DistanceAlpha);
	return FearPressurePerSecond * DeltaTime * ProximityMultiplier * GetWeakPointPressureMultiplier();
}

void AHorrorCampaignBossActor::ApplyBossVisuals()
{
	const FColor BossColor = ResolveBossStateColor();
	const float PressureMultiplier = GetWeakPointPressureMultiplier();

	if (BossLabel)
	{
		BossLabel->SetText(bDefeated
			? NSLOCTEXT("HorrorCampaignBoss", "BossDefeated", "巨人已击败")
			: !bAwake
				? FText::Format(
					NSLOCTEXT("HorrorCampaignBoss", "BossDormant", "{0}  休眠中"),
					BossName)
			: FText::Format(
				NSLOCTEXT("HorrorCampaignBoss", "BossWeakPoints", "{0}  弱点 {1}/{2}"),
				BossName,
				FText::AsNumber(ResolvedWeakPointCount),
				FText::AsNumber(RequiredWeakPointCount)));
		BossLabel->SetTextRenderColor(BossColor);
	}

	if (BossLight)
	{
		BossLight->SetLightColor(FLinearColor(BossColor));
		BossLight->SetIntensity(bDefeated ? 900.0f : bAwake ? 2600.0f * PressureMultiplier : 650.0f);
	}

	if (BossMesh)
	{
		if (UMaterialInterface* ProfileMaterial = ResolveBossMaterial())
		{
			BossMesh->SetMaterial(0, ProfileMaterial);
		}
		BossMesh->SetVisibility(true);
	}

	if (BossVFX)
	{
		if (bDefeated || !bAwake)
		{
			BossVFX->Deactivate();
			BossVFX->SetVisibility(!bDefeated);
		}
		else
		{
			const float VFXScale = 2.5f * PressureMultiplier;
			BossVFX->SetRelativeScale3D(FVector(VFXScale, VFXScale, VFXScale));
			BossVFX->SetVisibility(true);
			BossVFX->Activate(true);
		}
	}
}

bool AHorrorCampaignBossActor::HasClearAttackLineToActor(AActor* TargetActor) const
{
	const UWorld* World = GetWorld();
	if (!World || !TargetActor)
	{
		return false;
	}

	const FVector TraceStart = GetActorLocation() + FVector(0.0f, 0.0f, 110.0f);
	const FVector TraceEnd = TargetActor->GetActorLocation() + FVector(0.0f, 0.0f, 80.0f);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HorrorCampaignBossAttackLine), false);
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(TargetActor);

	FHitResult Hit;
	if (!World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		return true;
	}

	return !Hit.bBlockingHit;
}

bool AHorrorCampaignBossActor::HasClearMovementLineToLocation(const FVector& TargetLocation, const AActor* TargetActor) const
{
	FHitResult IgnoredHit;
	return !TryGetMovementLineBlockHit(TargetLocation, TargetActor, IgnoredHit);
}

bool AHorrorCampaignBossActor::TryGetMovementLineBlockHit(const FVector& TargetLocation, const AActor* TargetActor, FHitResult& OutHit) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return true;
	}

	const FVector TraceStart = GetActorLocation() + FVector(0.0f, 0.0f, BossMovementLineTraceHeightCm);
	const FVector TraceEnd = TargetLocation + FVector(0.0f, 0.0f, BossMovementLineTraceHeightCm);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HorrorCampaignBossMovementLine), false);
	QueryParams.AddIgnoredActor(this);
	if (TargetActor)
	{
		QueryParams.AddIgnoredActor(TargetActor);
	}

	FHitResult Hit;
	if (!World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		return false;
	}

	if (Hit.bBlockingHit)
	{
		OutHit = Hit;
		return true;
	}
	return false;
}

bool AHorrorCampaignBossActor::TryResolveNavigableMoveTarget(const FVector& TargetLocation, FVector& OutMoveTarget) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	UNavigationPath* NavigationPath = UNavigationSystemV1::FindPathToLocationSynchronously(
		World,
		GetActorLocation(),
		TargetLocation,
		const_cast<AHorrorCampaignBossActor*>(this));
	if (!NavigationPath || !NavigationPath->IsValid() || NavigationPath->IsPartial() || NavigationPath->PathPoints.Num() < 2)
	{
		return false;
	}

	float RemainingLookAhead = BossNavigationLookAheadCm;
	FVector MoveTarget = NavigationPath->PathPoints[1];
	for (int32 PointIndex = 1; PointIndex < NavigationPath->PathPoints.Num(); ++PointIndex)
	{
		const FVector PreviousPoint = PointIndex == 1 ? GetActorLocation() : NavigationPath->PathPoints[PointIndex - 1];
		const FVector CurrentPoint = NavigationPath->PathPoints[PointIndex];
		const float SegmentLength = FVector::Dist2D(PreviousPoint, CurrentPoint);
		MoveTarget = CurrentPoint;
		if (SegmentLength >= RemainingLookAhead)
		{
			const FVector SegmentDirection = (CurrentPoint - PreviousPoint).GetSafeNormal2D();
			MoveTarget = PreviousPoint + SegmentDirection * RemainingLookAhead;
			break;
		}
		RemainingLookAhead -= SegmentLength;
		if (RemainingLookAhead <= 0.0f)
		{
			break;
		}
	}

	OutMoveTarget = MoveTarget;
	return true;
}

bool AHorrorCampaignBossActor::TryResolveCornerSidestepMoveTarget(
	const FVector& TargetLocation,
	const AActor* TargetActor,
	const FHitResult& BlockHit,
	float StepDistance,
	FVector& OutMoveTarget) const
{
	UWorld* World = GetWorld();
	if (!World || !BlockHit.bBlockingHit || StepDistance <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const FVector CurrentLocation = GetActorLocation();
	FVector ToTarget = TargetLocation - CurrentLocation;
	ToTarget.Z = 0.0f;
	if (ToTarget.SizeSquared() <= KINDA_SMALL_NUMBER)
	{
		return false;
	}

	FVector WallNormal = BlockHit.ImpactNormal;
	WallNormal.Z = 0.0f;
	if (WallNormal.SizeSquared() <= UE_SMALL_NUMBER)
	{
		WallNormal = -ToTarget.GetSafeNormal();
	}
	WallNormal.Normalize();

	const FVector WallTangent(-WallNormal.Y, WallNormal.X, 0.0f);
	const float TargetLateralOffset = FVector::DotProduct(ToTarget, WallTangent);
	if (FMath::Abs(TargetLateralOffset) < BossCornerSidestepMinLateralCm)
	{
		return false;
	}

	const FVector PreferredSideDirection = WallTangent * FMath::Sign(TargetLateralOffset);
	const FVector CandidateDirections[] = {
		PreferredSideDirection,
		-PreferredSideDirection
	};

	for (const FVector& CandidateDirection : CandidateDirections)
	{
		if (FVector::DotProduct(ToTarget, CandidateDirection) <= 0.0f)
		{
			continue;
		}

		const FVector CandidateLocation = CurrentLocation + CandidateDirection * (StepDistance + BossCornerSidestepProbeSlackCm);
		if (!HasClearMovementLineToLocation(CandidateLocation, TargetActor))
		{
			continue;
		}

		FVector CandidateToTarget = TargetLocation - CandidateLocation;
		CandidateToTarget.Z = 0.0f;
		if (CandidateToTarget.SizeSquared() >= ToTarget.SizeSquared())
		{
			continue;
		}

		OutMoveTarget = CandidateLocation;
		return true;
	}

	return false;
}

void AHorrorCampaignBossActor::ApplyBossAnimation()
{
	if (!BossMesh)
	{
		return;
	}

	UAnimationAsset* TargetAnimation = IsBossAwake() ? RunAnimation.Get() : IdleAnimation.Get();
	if (!TargetAnimation)
	{
		return;
	}

	const float PlayRate = CalculateBossAnimationPlayRate();
	BossMesh->OverrideAnimationData(TargetAnimation, true, true, 0.0f, PlayRate);
	BossMesh->SetPlayRate(PlayRate);
}

float AHorrorCampaignBossActor::CalculateBossAnimationPlayRate() const
{
	if (!IsBossAwake())
	{
		return 1.0f;
	}

	const float ReferenceSpeed = FMath::Max(1.0f, BossRunAnimationReferenceSpeedCmPerSecond);
	return FMath::Clamp(BossMoveSpeed / ReferenceSpeed, BossRunAnimationMinPlayRate, BossRunAnimationMaxPlayRate);
}

UMaterialInterface* AHorrorCampaignBossActor::ResolveBossMaterial() const
{
	const FString ProfileId = ChapterId.ToString();
	if (ProfileId.Contains(TEXT("DeepWater")) || ProfileId.Contains(TEXT("Dock")) || ProfileId.Contains(TEXT("Signal")))
	{
		return DeepWaterGolemMaterial ? DeepWaterGolemMaterial.Get() : DefaultGolemMaterial.Get();
	}

	if (ProfileId.Contains(TEXT("Forest")) || ProfileId.Contains(TEXT("Spike")))
	{
		return ForestGolemMaterial ? ForestGolemMaterial.Get() : DefaultGolemMaterial.Get();
	}

	if (ProfileId.Contains(TEXT("Dungeon")) || ProfileId.Contains(TEXT("Depths")) || ProfileId.Contains(TEXT("Hall")) || ProfileId.Contains(TEXT("Temple")) || ProfileId.Contains(TEXT("Boss")))
	{
		return DungeonGolemMaterial ? DungeonGolemMaterial.Get() : DefaultGolemMaterial.Get();
	}

	return DefaultGolemMaterial.Get();
}

FText AHorrorCampaignBossActor::ResolveProfiledBossName(FName InChapterId, const FText& RequestedName) const
{
	if (!RequestedName.IsEmpty() && !RequestedName.EqualTo(NSLOCTEXT("HorrorGameMode", "Chaser", "追猎者")))
	{
		return RequestedName;
	}

	const FString ProfileId = InChapterId.ToString();
	if (ProfileId.Contains(TEXT("DeepWater")) || ProfileId.Contains(TEXT("Dock")))
	{
		return NSLOCTEXT("HorrorCampaignBoss", "DeepWaterChaserName", "干船坞溺影");
	}

	if (ProfileId.Contains(TEXT("Forest")) || ProfileId.Contains(TEXT("Spike")))
	{
		return NSLOCTEXT("HorrorCampaignBoss", "ForestChaserName", "荆棘石像");
	}

	if (ProfileId.Contains(TEXT("Signal")))
	{
		return NSLOCTEXT("HorrorCampaignBoss", "SignalChaserName", "录像带残响");
	}

	if (ProfileId.Contains(TEXT("Dungeon")) || ProfileId.Contains(TEXT("Depths")) || ProfileId.Contains(TEXT("Hall")) || ProfileId.Contains(TEXT("Temple")) || ProfileId.Contains(TEXT("Boss")))
	{
		return NSLOCTEXT("HorrorCampaignBoss", "DungeonChaserName", "地牢守像");
	}

	return RequestedName.IsEmpty()
		? NSLOCTEXT("HorrorCampaignBoss", "DefaultBossNameFallback", "石像巨人")
		: RequestedName;
}

FColor AHorrorCampaignBossActor::ResolveBossStateColor() const
{
	if (bDefeated)
	{
		return FColor(100, 255, 150);
	}

	if (!bAwake)
	{
		return FColor(115, 150, 180);
	}

	const FString ProfileId = ChapterId.ToString();
	if (ProfileId.Contains(TEXT("DeepWater")) || ProfileId.Contains(TEXT("Dock")))
	{
		return FColor(55, 185, 255);
	}

	if (ProfileId.Contains(TEXT("Forest")) || ProfileId.Contains(TEXT("Spike")))
	{
		return FColor(105, 255, 95);
	}

	if (ProfileId.Contains(TEXT("Signal")))
	{
		return FColor(210, 80, 255);
	}

	if (ProfileId.Contains(TEXT("Dungeon")) || ProfileId.Contains(TEXT("Depths")) || ProfileId.Contains(TEXT("Hall")) || ProfileId.Contains(TEXT("Temple")) || ProfileId.Contains(TEXT("Boss")))
	{
		return FColor(255, 95, 45);
	}

	return FColor(255, 70, 35);
}

void AHorrorCampaignBossActor::ApplyBossPressureToPlayerPawns(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		if (!PlayerController)
		{
			continue;
		}

		ApplyBossPressureToActor(PlayerController->GetPawn(), DeltaTime);
	}
}

void AHorrorCampaignBossActor::UpdateBossMovementAndAttacks(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!World || !IsBossAwake())
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		APawn* TargetPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
		if (!TargetPawn)
		{
			continue;
		}

		if (TryTriggerBossAttack(TargetPawn))
		{
			continue;
		}

		MoveBossTowardActor(TargetPawn, DeltaTime);
	}
}

float AHorrorCampaignBossActor::GetWeakPointPressureMultiplier() const
{
	if (RequiredWeakPointCount <= 0)
	{
		return 1.0f;
	}

	const float ResolvedAlpha = static_cast<float>(ResolvedWeakPointCount) / static_cast<float>(RequiredWeakPointCount);
	return 1.0f + FMath::Clamp(ResolvedAlpha, 0.0f, 1.0f) * WeakPointFearPressureBonus;
}
