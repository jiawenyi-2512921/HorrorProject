// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorCampaignBossActor.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorGameModeBase.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
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
	BossName = InBossName.IsEmpty() ? NSLOCTEXT("HorrorCampaignBoss", "DefaultBossNameFallback", "石像巨人") : InBossName;
	RequiredWeakPointCount = FMath::Max(0, InRequiredWeakPointCount);
	ResolvedWeakPointCount = 0;
	bDefeated = false;
	bAwake = false;
	LastBossAttackWorldSeconds = -100000.0f;
	ApplyBossVisuals();
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
	BossMoveSpeed = FMath::Max(0.0f, InMoveSpeed);
	BossEngageRadius = FMath::Max(0.0f, InEngageRadius);
	BossAttackRadius = FMath::Max(0.0f, InAttackRadius);
	FearPressureRadius = FMath::Max(0.0f, InFearPressureRadius);
	const float SafeActorScale = FMath::Max(0.1f, InActorScale);
	SetActorScale3D(FVector(SafeActorScale));
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

	const FVector MoveDirection = ToTarget / DistanceToTarget;
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

	return FVector::DistSquared2D(GetActorLocation(), TargetActor->GetActorLocation()) <= FMath::Square(BossAttackRadius);
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
			GameMode->RequestPlayerFailure(
				BossAttackFailureCause,
				NSLOCTEXT("HorrorCampaignBoss", "BossAttackFailure", "石像巨人的重击打断了你的行动。"));
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
	const FColor BossColor = bDefeated ? FColor(100, 255, 150) : bAwake ? FColor(255, 70, 35) : FColor(115, 150, 180);
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
