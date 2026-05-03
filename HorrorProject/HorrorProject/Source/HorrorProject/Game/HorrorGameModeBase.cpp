// Copyright Epic Games, Inc. All Rights Reserved.

#include "Game/HorrorGameModeBase.h"

#include "Audio/HorrorAudioSubsystem.h"
#include "Camera/PlayerCameraManager.h"
#include "EngineUtils.h"
#include "Game/DeepWaterStationRouteKit.h"
#include "Game/HorrorCampaignBossActor.h"
#include "Game/HorrorCampaignObjectiveActor.h"
#include "Game/HorrorEncounterDirector.h"
#include "Game/HorrorDirectorSubsystem.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Game/HorrorMapChain.h"
#include "Game/HorrorMapChainExit.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/Light.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/PlayerStartPIE.h"
#include "Engine/PointLight.h"
#include "Engine/PostProcessVolume.h"
#include "Components/CapsuleComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/LightComponent.h"
#include "Components/LocalLightComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/PlayerController.h"
#include "HorrorProject.h"
#include "Kismet/GameplayStatics.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/CameraBatteryComponent.h"
#include "Player/Components/FearComponent.h"
#include "Player/Components/InventoryComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Player/Components/VHSEffectComponent.h"
#include "Player/HorrorPlayerController.h"
#include "Save/HorrorSaveSubsystem.h"
#include "TimerManager.h"
#include "UI/Day1SliceHUD.h"

namespace
{
	constexpr float SpawnSearchVerticalStepCm = 50.0f;
	constexpr int32 SpawnSearchVerticalSteps = 8;
	constexpr float SpawnSearchRingStepCm = 100.0f;
	constexpr int32 SpawnSearchRingCount = 8;
	constexpr float CampaignObjectiveGroundTraceUpCm = 1600.0f;
	constexpr float CampaignObjectiveGroundTraceDownCm = 3200.0f;
	constexpr float CampaignObjectiveGroundLiftCm = 95.0f;
	constexpr float CampaignObjectivePlayableRadiusCm = 3600.0f;
	constexpr float CampaignObjectivePlayableMinZCm = 35.0f;
	constexpr float CampaignObjectivePlayableMaxZCm = 450.0f;
	constexpr float CampaignObjectiveClearanceRingStepCm = 140.0f;
	constexpr int32 CampaignObjectiveClearanceRingCount = 8;
	constexpr int32 CampaignObjectiveClearanceDirections = 12;
	constexpr float CampaignObjectiveObstacleTopRejectDeltaCm = 140.0f;
	constexpr float CampaignFailureRecoveryBackstepCm = 360.0f;
	constexpr float CampaignFailureRecoverySideStepCm = 80.0f;
	constexpr float CampaignFailureRecoveryLiftCm = 35.0f;
	constexpr float CampaignFailureCurrentLocationToleranceCm = 160.0f;
	constexpr float CampaignFailureCurrentLocationClearanceSlackCm = 4.0f;
	constexpr float CampaignAmbushThreatMinimumBackstepCm = 900.0f;
	constexpr float CampaignAmbushThreatMaximumBackstepCm = 2200.0f;
	constexpr float CampaignAmbushThreatMinimumSideStepCm = 360.0f;
	const FName CampaignAtmosphereLightTag(TEXT("HorrorCampaignAtmosphereDimmed"));
	const FName CampaignAtmosphereVolumeTag(TEXT("HorrorCampaignAtmosphereVolume"));
	const FName CampaignVisibilityRescueLightTag(TEXT("HorrorCampaignVisibilityRescue"));
	constexpr float ScrapopolisReadableFogDensity = 0.035f;
	constexpr float ScrapopolisReadableFogMaxOpacity = 0.3f;
	constexpr float ScrapopolisRescueLightIntensity = 4200.0f;
	constexpr float ScrapopolisRescueLightRadius = 2200.0f;

	struct FHorrorGameModeDefaultMilestoneMetadata
	{
		FGameplayTag EventTag;
		FName SourceId;
		FName TrailerBeatId;
		const TCHAR* HintFallback;
		const TCHAR* DebugLabel;
	};

	void PublishFoundFootageEvents(
		UWorld* World,
		const FHorrorFoundFootageContract& Contract,
		const TArray<FHorrorFoundFootageRecordedEvent>& RecordedEvents,
		UObject* SourceObject)
	{
		if (!World)
		{
			return;
		}

		UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
		if (!EventBus)
		{
			return;
		}

		for (const FHorrorFoundFootageRecordedEvent& RecordedEvent : RecordedEvents)
		{
			EventBus->Publish(RecordedEvent.EventTag, RecordedEvent.SourceId, Contract.GetStateForEvent(RecordedEvent.EventTag), SourceObject);
		}
	}

	void RegisterHorrorGameModeMilestoneMetadata(
		UHorrorEventBusSubsystem& EventBus,
		const FHorrorGameModeDefaultMilestoneMetadata& Default)
	{
		FHorrorObjectiveMessageMetadata Metadata;
		Metadata.TrailerBeatId = Default.TrailerBeatId;
		Metadata.ObjectiveHint = FText::AsCultureInvariant(Default.HintFallback);
		Metadata.DebugLabel = FText::AsCultureInvariant(Default.DebugLabel);
		EventBus.RegisterObjectiveMetadata(Default.EventTag, Default.SourceId, Metadata);
	}

	FText JoinCampaignObjectiveTextList(const TArray<FText>& ObjectiveTexts)
	{
		if (ObjectiveTexts.IsEmpty())
		{
			return FText::GetEmpty();
		}

		FString JoinedText;
		for (int32 TextIndex = 0; TextIndex < ObjectiveTexts.Num(); ++TextIndex)
		{
			if (TextIndex > 0)
			{
				JoinedText += TEXT("、");
			}
			JoinedText += FString::Printf(TEXT("「%s」"), *ObjectiveTexts[TextIndex].ToString());
		}

		return FText::FromString(JoinedText);
	}

	FText ResolveCampaignNavigationDirectionText(const APawn& ViewerPawn, const FVector& ToObjective, bool bArrived)
	{
		const FVector HorizontalToObjective(ToObjective.X, ToObjective.Y, 0.0f);
		if (bArrived || HorizontalToObjective.SizeSquared() <= FMath::Square(125.0f))
		{
			return NSLOCTEXT("HorrorGameMode", "CampaignNavigationNearby", "附近");
		}

		const FRotator YawRotation(0.0f, ViewerPawn.GetActorRotation().Yaw, 0.0f);
		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		const FVector Direction = HorizontalToObjective.GetSafeNormal();
		const float ForwardDot = FVector::DotProduct(Forward, Direction);
		const float RightDot = FVector::DotProduct(Right, Direction);

		if (ForwardDot >= 0.65f)
		{
			return NSLOCTEXT("HorrorGameMode", "CampaignNavigationForward", "前方");
		}

		if (ForwardDot <= -0.65f)
		{
			return NSLOCTEXT("HorrorGameMode", "CampaignNavigationBehind", "后方");
		}

		return RightDot >= 0.0f
			? NSLOCTEXT("HorrorGameMode", "CampaignNavigationRight", "右侧")
			: NSLOCTEXT("HorrorGameMode", "CampaignNavigationLeft", "左侧");
	}

	bool HasDirectNavigationSightline(
		const UWorld& World,
		const AActor& ViewerActor,
		const FVector& TargetLocation,
		const AActor* TargetActor)
	{
		const FVector TraceStart = ViewerActor.GetActorLocation() + FVector(0.0f, 0.0f, 55.0f);
		const FVector TraceEnd = TargetLocation + FVector(0.0f, 0.0f, 55.0f);
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HorrorCampaignObjectiveNavigationSightline), false);
		QueryParams.AddIgnoredActor(&ViewerActor);
		if (TargetActor)
		{
			QueryParams.AddIgnoredActor(TargetActor);
		}

		FHitResult Hit;
		if (!World.LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
		{
			return true;
		}

		return !Hit.bBlockingHit;
	}

	bool IsCampaignNavigationTargetReachable(const APawn& ViewerPawn, const FVector& TargetLocation, const AActor* TargetActor)
	{
		UWorld* World = ViewerPawn.GetWorld();
		if (!World)
		{
			return false;
		}

		UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
		if (NavigationSystem && NavigationSystem->GetDefaultNavDataInstance(FNavigationSystem::DontCreate))
		{
			constexpr float NavigationProjectionHorizontalExtentCm = 220.0f;
			constexpr float NavigationProjectionVerticalExtentCm = 420.0f;
			const FVector ProjectionExtent(
				NavigationProjectionHorizontalExtentCm,
				NavigationProjectionHorizontalExtentCm,
				NavigationProjectionVerticalExtentCm);
			FNavLocation ProjectedStart;
			FNavLocation ProjectedTarget;
			const bool bProjectedStart = NavigationSystem->ProjectPointToNavigation(
				ViewerPawn.GetActorLocation(),
				ProjectedStart,
				ProjectionExtent);
			const bool bProjectedTarget = NavigationSystem->ProjectPointToNavigation(
				TargetLocation,
				ProjectedTarget,
				ProjectionExtent);
			const FVector PathStartLocation = bProjectedStart ? ProjectedStart.Location : ViewerPawn.GetActorLocation();
			const FVector PathTargetLocation = bProjectedTarget ? ProjectedTarget.Location : TargetLocation;
			const UNavigationPath* NavigationPath = UNavigationSystemV1::FindPathToLocationSynchronously(
				World,
				PathStartLocation,
				PathTargetLocation,
				const_cast<APawn*>(&ViewerPawn));
			return NavigationPath
				&& NavigationPath->IsValid()
				&& !NavigationPath->IsPartial()
				&& NavigationPath->PathPoints.Num() >= 2;
		}

		return HasDirectNavigationSightline(*World, ViewerPawn, TargetLocation, TargetActor);
	}

	bool TryFindClearSpawnLocation(
		UWorld& World,
		const APawn& PawnToFit,
		const FVector& BaseLocation,
		const FRotator& SpawnRotation,
		FVector& OutLocation)
	{
		FVector TestLocation = BaseLocation;
		if (World.FindTeleportSpot(&PawnToFit, TestLocation, SpawnRotation))
		{
			OutLocation = TestLocation;
			return true;
		}

		for (int32 ZStep = 1; ZStep <= SpawnSearchVerticalSteps; ++ZStep)
		{
			TestLocation = BaseLocation + FVector(0.0f, 0.0f, SpawnSearchVerticalStepCm * ZStep);
			if (World.FindTeleportSpot(&PawnToFit, TestLocation, SpawnRotation))
			{
				OutLocation = TestLocation;
				return true;
			}
		}

		for (int32 RingIndex = 1; RingIndex <= SpawnSearchRingCount; ++RingIndex)
		{
			const float Radius = SpawnSearchRingStepCm * RingIndex;
			for (int32 DirectionIndex = 0; DirectionIndex < 8; ++DirectionIndex)
			{
				const float AngleRadians = UE_TWO_PI * static_cast<float>(DirectionIndex) / 8.0f;
				const FVector RingOffset(FMath::Cos(AngleRadians) * Radius, FMath::Sin(AngleRadians) * Radius, 0.0f);

				for (int32 ZStep = 0; ZStep <= SpawnSearchVerticalSteps; ++ZStep)
				{
					TestLocation = BaseLocation + RingOffset + FVector(0.0f, 0.0f, SpawnSearchVerticalStepCm * ZStep);
					if (World.FindTeleportSpot(&PawnToFit, TestLocation, SpawnRotation))
					{
						OutLocation = TestLocation;
						return true;
					}
				}
			}
		}

		return false;
	}

	bool TryProjectCampaignObjectiveToGround(UWorld& World, const FVector& AuthoredLocation, FVector& OutLocation)
	{
		const FVector TraceStart = AuthoredLocation + FVector(0.0f, 0.0f, CampaignObjectiveGroundTraceUpCm);
		const FVector TraceEnd = AuthoredLocation - FVector(0.0f, 0.0f, CampaignObjectiveGroundTraceDownCm);
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HorrorCampaignObjectiveGroundTrace), false);
		FHitResult Hit;
		if (!World.LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams) || !Hit.bBlockingHit)
		{
			return false;
		}

		OutLocation = Hit.ImpactPoint + FVector(0.0f, 0.0f, CampaignObjectiveGroundLiftCm);
		return true;
	}

	bool IsCampaignObjectiveLocationClear(UWorld& World, const FVector& Location)
	{
		const FCollisionShape ObjectiveShape = FCollisionShape::MakeBox(FVector(75.0f, 75.0f, 70.0f));
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HorrorCampaignObjectiveClearance), false);
		return !World.OverlapBlockingTestByChannel(
			Location + FVector(0.0f, 0.0f, 85.0f),
			FQuat::Identity,
			ECC_WorldStatic,
			ObjectiveShape,
			QueryParams);
	}

	bool TryProjectClearCampaignObjectiveLocation(UWorld& World, const FVector& CandidateLocation, FVector& OutLocation)
	{
		FVector GroundedLocation = CandidateLocation;
		if (TryProjectCampaignObjectiveToGround(World, CandidateLocation, GroundedLocation))
		{
			if (GroundedLocation.Z > CandidateLocation.Z + CampaignObjectiveObstacleTopRejectDeltaCm)
			{
				return false;
			}

			if (IsCampaignObjectiveLocationClear(World, GroundedLocation))
			{
				OutLocation = GroundedLocation;
				return true;
			}
			return false;
		}

		if (IsCampaignObjectiveLocationClear(World, CandidateLocation))
		{
			OutLocation = CandidateLocation;
			return true;
		}
		return false;
	}

	bool TryFindClearCampaignObjectiveLocation(UWorld& World, const FVector& DesiredLocation, FVector& OutLocation)
	{
		if (TryProjectClearCampaignObjectiveLocation(World, DesiredLocation, OutLocation))
		{
			return true;
		}

		for (int32 RingIndex = 1; RingIndex <= CampaignObjectiveClearanceRingCount; ++RingIndex)
		{
			const float Radius = CampaignObjectiveClearanceRingStepCm * RingIndex;
			for (int32 DirectionIndex = 0; DirectionIndex < CampaignObjectiveClearanceDirections; ++DirectionIndex)
			{
				const float AngleRadians = UE_TWO_PI * static_cast<float>(DirectionIndex) / static_cast<float>(CampaignObjectiveClearanceDirections);
				const FVector Offset(FMath::Cos(AngleRadians) * Radius, FMath::Sin(AngleRadians) * Radius, 0.0f);
				if (TryProjectClearCampaignObjectiveLocation(World, DesiredLocation + Offset, OutLocation))
				{
					return true;
				}
			}
		}

		return false;
	}

	bool HasBlockingStaticGeometry(const UWorld& World)
	{
		return true;
	}

	bool IsPawnLocationClearIgnoringSelf(UWorld& World, const APawn& PawnToFit, const FVector& CandidateLocation, const FRotator& CandidateRotation)
	{
		const UCapsuleComponent* CapsuleComponent = PawnToFit.FindComponentByClass<UCapsuleComponent>();
		if (!CapsuleComponent)
		{
			return true;
		}

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HorrorCampaignCurrentRecoveryClearance), false);
		QueryParams.AddIgnoredActor(&PawnToFit);
		const FCollisionShape PawnShape = FCollisionShape::MakeCapsule(
			FMath::Max(1.0f, CapsuleComponent->GetScaledCapsuleRadius() - CampaignFailureCurrentLocationClearanceSlackCm),
			FMath::Max(1.0f, CapsuleComponent->GetScaledCapsuleHalfHeight() - CampaignFailureCurrentLocationClearanceSlackCm));
		return !World.OverlapBlockingTestByChannel(
			CandidateLocation,
			CandidateRotation.Quaternion(),
			CapsuleComponent->GetCollisionObjectType(),
			PawnShape,
			QueryParams);
	}

	bool IsNearCurrentPawnLocation(const APawn& PawnToFit, const FVector& CandidateLocation)
	{
		const FVector CurrentPawnLocation = PawnToFit.GetActorLocation();
		return FVector::Dist2D(CurrentPawnLocation, CandidateLocation) <= CampaignFailureCurrentLocationToleranceCm
			&& FMath::Abs(CurrentPawnLocation.Z - CandidateLocation.Z) <= CampaignFailureCurrentLocationToleranceCm;
	}

	bool TryResolveGroundedRecoveryCandidate(
		UWorld& World,
		const APawn& PawnToFit,
		const FVector& CandidateLocation,
		const FRotator& RecoveryRotation,
		bool bRequireGround,
		FVector& OutLocation)
	{
		if (IsNearCurrentPawnLocation(PawnToFit, CandidateLocation)
			&& IsPawnLocationClearIgnoringSelf(World, PawnToFit, PawnToFit.GetActorLocation(), RecoveryRotation))
		{
			OutLocation = PawnToFit.GetActorLocation();
			return true;
		}

		FVector GroundedLocation = CandidateLocation;
		if (!TryProjectCampaignObjectiveToGround(World, CandidateLocation, GroundedLocation) && bRequireGround)
		{
			return false;
		}

		if (IsNearCurrentPawnLocation(PawnToFit, GroundedLocation)
			&& IsPawnLocationClearIgnoringSelf(World, PawnToFit, PawnToFit.GetActorLocation(), RecoveryRotation))
		{
			OutLocation = PawnToFit.GetActorLocation();
			return true;
		}

		return TryFindClearSpawnLocation(World, PawnToFit, GroundedLocation, RecoveryRotation, OutLocation);
	}

	bool TryResolveSafeCampaignFailureRecoveryLocation(
		UWorld& World,
		const APawn& PawnToFit,
		const FRotator& RecoveryRotation,
		TConstArrayView<FVector> CandidateLocations,
		FVector& OutLocation)
	{
		const bool bRequireGround = HasBlockingStaticGeometry(World);
		for (const FVector& CandidateLocation : CandidateLocations)
		{
			if (TryResolveGroundedRecoveryCandidate(World, PawnToFit, CandidateLocation, RecoveryRotation, bRequireGround, OutLocation))
			{
				return true;
			}
		}

		return false;
	}

	FVector ClampCampaignRelativeLocationToPlayableBounds(const FVector& RelativeLocation)
	{
		FVector SafeRelativeLocation = RelativeLocation;
		if (!FMath::IsFinite(SafeRelativeLocation.X) || !FMath::IsFinite(SafeRelativeLocation.Y) || !FMath::IsFinite(SafeRelativeLocation.Z) || SafeRelativeLocation.ContainsNaN())
		{
			return FVector(600.0f, 0.0f, CampaignObjectiveGroundLiftCm);
		}

		FVector HorizontalOffset(SafeRelativeLocation.X, SafeRelativeLocation.Y, 0.0f);
		const float HorizontalDistance = HorizontalOffset.Size();
		if (HorizontalDistance > CampaignObjectivePlayableRadiusCm)
		{
			HorizontalOffset = HorizontalOffset.GetSafeNormal(UE_SMALL_NUMBER, FVector::ForwardVector) * CampaignObjectivePlayableRadiusCm;
			SafeRelativeLocation.X = HorizontalOffset.X;
			SafeRelativeLocation.Y = HorizontalOffset.Y;
		}

		SafeRelativeLocation.Z = FMath::Clamp(
			SafeRelativeLocation.Z,
			CampaignObjectivePlayableMinZCm,
			CampaignObjectivePlayableMaxZCm);
		return SafeRelativeLocation;
	}

	FVector BuildCampaignRelativeOffsetWorldSpace(
		const FVector& RelativeLocation,
		const FRotationMatrix& AnchorRotationMatrix)
	{
		const FVector SafeRelativeLocation = ClampCampaignRelativeLocationToPlayableBounds(RelativeLocation);
		return AnchorRotationMatrix.GetUnitAxis(EAxis::X) * SafeRelativeLocation.X
			+ AnchorRotationMatrix.GetUnitAxis(EAxis::Y) * SafeRelativeLocation.Y
			+ FVector(0.0f, 0.0f, SafeRelativeLocation.Z);
	}

}

namespace HorrorObjectiveMilestoneCheckpoints
{
	const FName BodycamAcquired(TEXT("Checkpoint.Day1.BodycamAcquired"));
	const FName FirstNoteCollected(TEXT("Checkpoint.Day1.FirstNoteCollected"));
	const FName FirstAnomalyRecorded(TEXT("Checkpoint.Day1.FirstAnomalyRecorded"));
	const FName Day1Complete(TEXT("Checkpoint.Day1.Complete"));
}

namespace HorrorObjectiveMilestoneSources
{
	const FName Bodycam(TEXT("Bodycam"));
	const FName FirstNote(TEXT("FirstNote"));
	const FName FirstAnomaly(TEXT("FirstAnomaly"));
	const FName Archive(TEXT("Archive"));
	const FName Exit(TEXT("Exit"));
	const FName Day1Complete(TEXT("Day1Complete"));
}

namespace HorrorPlayerFailureDefaults
{
	const FName UnknownDeathCause(TEXT("Death.Unknown"));
	const FName UnknownFailureCause(TEXT("Failure.Unknown"));
	const FName BossAttackFailureCause(TEXT("Failure.Boss.StoneGolemAttack"));
	const FText DeathFeedback = NSLOCTEXT("HorrorGameMode", "DeathFeedback", "信号丢失，正在恢复到上一个检查点...");
	const FText FailureFeedback = NSLOCTEXT("HorrorGameMode", "FailureFeedback", "任务失败，正在恢复到上一个检查点...");
	const FText MissingCheckpointFeedback = NSLOCTEXT("HorrorGameMode", "MissingCheckpoint", "没有可用检查点。");
}

namespace HorrorCampaignAudioEvents
{
	FGameplayTag ObjectiveCompleted()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ObjectiveCompleted")), false);
	}

	FGameplayTag ChapterCompleted()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ChapterCompleted")), false);
	}

	FGameplayTag BossWeakPoint()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.BossWeakPoint")), false);
	}
}

namespace
{
	void EnsureRouteKitObjectives(ADeepWaterStationRouteKit& RouteKit)
	{
		if (RouteKit.ObjectiveNodes.IsEmpty())
		{
			RouteKit.ConfigureDefaultFirstLoopObjectiveNodes();
		}
		else
		{
			RouteKit.EnsureDefaultFirstLoopObjectiveNodes();
		}
		RouteKit.SpawnObjectiveNodes();
		RouteKit.SpawnEncounterDirector();
	}

	FName GetHorrorGameModeMilestoneCheckpoint(FGameplayTag StateTag)
	{
		if (StateTag == HorrorFoundFootageTags::BodycamAcquiredState())
		{
			return HorrorObjectiveMilestoneCheckpoints::BodycamAcquired;
		}
		if (StateTag == HorrorFoundFootageTags::FirstNoteCollectedState())
		{
			return HorrorObjectiveMilestoneCheckpoints::FirstNoteCollected;
		}
		if (StateTag == HorrorFoundFootageTags::FirstAnomalyRecordedState())
		{
			return HorrorObjectiveMilestoneCheckpoints::FirstAnomalyRecorded;
		}

		return NAME_None;
	}
}

AHorrorGameModeBase::AHorrorGameModeBase()
{
	DefaultPawnClass = AHorrorPlayerCharacter::StaticClass();
	PlayerControllerClass = AHorrorPlayerController::StaticClass();
	HUDClass = ADay1SliceHUD::StaticClass();
	RuntimeEncounterDirectorClass = AHorrorEncounterDirector::StaticClass();
	RuntimeMapChainExitClass = AHorrorMapChainExit::StaticClass();
	RuntimeCampaignObjectiveClass = AHorrorCampaignObjectiveActor::StaticClass();
	RuntimeCampaignBossClass = AHorrorCampaignBossActor::StaticClass();
}

void AHorrorGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	CachedMapPackageName = FHorrorMapChain::NormalizeMapPackageName(MapName);
	Super::InitGame(MapName, Options, ErrorMessage);
}

void AHorrorGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	RegisterDefaultObjectiveMetadata();

	if (bAutoSpawnRouteKitOnBeginPlay && ShouldAutoSpawnLegacyRouteKitInCurrentMap())
	{
		EnsureRouteKit();
	}

	if (bAutoSpawnEncounterDirectorOnBeginPlay)
	{
		EnsureEncounterDirector();
	}

	// Register encounter director with HorrorDirectorSubsystem
	if (RuntimeEncounterDirector)
	{
		if (UWorld* World = GetWorld())
		{
			if (UHorrorDirectorSubsystem* Director = World->GetSubsystem<UHorrorDirectorSubsystem>())
			{
				Director->RegisterEncounter(RuntimeEncounterDirector);
			}
		}
	}

	bool bCampaignRuntimeReady = false;
	if (bAutoSpawnCampaignRuntimeOnBeginPlay)
	{
		bCampaignRuntimeReady = EnsureCampaignRuntime();
	}

	if (bAutoApplyCampaignHorrorAtmosphere && bCampaignRuntimeReady)
	{
		ApplyCampaignHorrorAtmosphere();
	}

	if (bAutoSpawnMapChainExitOnBeginPlay)
	{
		EnsureMapChainExit();
	}

	if (bSanitizeImportedMapShowcaseRuntimeOnBeginPlay)
	{
		SanitizeImportedMapChainRuntime();
	}

	if (bAutoStartDefaultHorrorAmbience)
	{
		if (UWorld* World = GetWorld())
		{
			if (UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>())
			{
				AudioSubsystem->StartDefaultHorrorAmbience();
			}
		}
	}

	// 自动关闭LVL_Showcase_01地图的所有灯光
	const FString CurrentMapName = GetCurrentNormalizedMapPackageName();
	if (CurrentMapName.Contains(TEXT("LVL_Showcase_01")))
	{
		if (UWorld* World = GetWorld())
		{
			int32 DisabledLightCount = 0;
			for (TActorIterator<ALight> It(World); It; ++It)
			{
				ALight* Light = *It;
				if (Light && Light->GetLightComponent())
				{
					Light->GetLightComponent()->SetVisibility(false);
					++DisabledLightCount;
				}
			}
			UE_LOG(LogHorrorProject, Log, TEXT("LVL_Showcase_01: 已关闭 %d 个灯光"), DisabledLightCount);
		}
	}
}

AActor* AHorrorGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	APlayerStart* FirstPlacedPlayerStart = nullptr;
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		if (!PlayerStart)
		{
			continue;
		}

		if (PlayerStart->IsA<APlayerStartPIE>())
		{
			UE_LOG(LogHorrorProject, Log, TEXT("ChoosePlayerStart: using PIE PlayerStart %s."), *GetNameSafe(PlayerStart));
			return PlayerStart;
		}

		if (!FirstPlacedPlayerStart)
		{
			FirstPlacedPlayerStart = PlayerStart;
		}

		if (!PreferredPlayerStartTag.IsNone() && PlayerStart->PlayerStartTag == PreferredPlayerStartTag)
		{
			UE_LOG(LogHorrorProject, Log, TEXT("ChoosePlayerStart: using preferred PlayerStart %s with tag %s."),
				*GetNameSafe(PlayerStart),
				*PreferredPlayerStartTag.ToString());
			return PlayerStart;
		}
	}

	if (FirstPlacedPlayerStart)
	{
		UE_LOG(LogHorrorProject, Log, TEXT("ChoosePlayerStart: using first placed PlayerStart %s because preferred tag %s was not found."),
			*GetNameSafe(FirstPlacedPlayerStart),
			*PreferredPlayerStartTag.ToString());
		return FirstPlacedPlayerStart;
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

APawn* AHorrorGameModeBase::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	UWorld* World = GetWorld();
	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);
	if (!World || !PawnClass)
	{
		return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
	}

	FTransform ResolvedSpawnTransform = SpawnTransform;
	const APawn* PawnToFit = PawnClass->GetDefaultObject<APawn>();
	if (PawnToFit)
	{
		FVector ClearLocation = SpawnTransform.GetLocation();
		const FRotator SpawnRotation = SpawnTransform.Rotator();
		if (TryFindClearSpawnLocation(*World, *PawnToFit, SpawnTransform.GetLocation(), SpawnRotation, ClearLocation))
		{
			ResolvedSpawnTransform.SetLocation(ClearLocation);
			if (!ClearLocation.Equals(SpawnTransform.GetLocation(), KINDA_SMALL_NUMBER))
			{
				UE_LOG(LogHorrorProject, Log, TEXT("SpawnDefaultPawnAtTransform: adjusted pawn spawn from %s to %s."),
					*SpawnTransform.GetLocation().ToCompactString(),
					*ClearLocation.ToCompactString());
			}
		}
		else
		{
			UE_LOG(LogHorrorProject, Warning, TEXT("SpawnDefaultPawnAtTransform: no clear pawn spawn found near %s; trying requested transform."),
				*SpawnTransform.GetLocation().ToCompactString());
		}
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	APawn* ResultPawn = World->SpawnActor<APawn>(PawnClass, ResolvedSpawnTransform, SpawnInfo);
	if (!ResultPawn)
	{
		UE_LOG(LogGameMode, Warning, TEXT("SpawnDefaultPawnAtTransform: Couldn't spawn Pawn of type %s at %s"),
			*GetNameSafe(PawnClass),
			*ResolvedSpawnTransform.ToHumanReadableString());
	}
	return ResultPawn;
}

bool AHorrorGameModeBase::TryAcquireBodycam(FName SourceId, bool bEnableBodycam)
{
	const FName ResolvedSourceId = SourceId.IsNone() ? FName(TEXT("Bodycam")) : SourceId;
	const bool bRecorded = RecordFoundFootageEvent(HorrorFoundFootageTags::BodycamAcquiredEvent(), ResolvedSourceId);

	if (AHorrorPlayerCharacter* PlayerCharacter = ResolveLeadPlayerCharacter())
	{
		if (UVHSEffectComponent* VHSEffect = PlayerCharacter->GetVHSEffectComponent())
		{
			VHSEffect->SetBodycamAcquired(true);
			VHSEffect->SetBodycamEnabled(bEnableBodycam);
		}

		if (UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent())
		{
			QuantumCamera->SetCameraAcquired(true);
			QuantumCamera->SetCameraEnabled(bEnableBodycam);
		}
	}

	return bRecorded;
}

bool AHorrorGameModeBase::TryCollectFirstNote(FName SourceId)
{
	if (!CanCollectFirstNote())
	{
		return false;
	}

	return RecordFoundFootageEvent(
		HorrorFoundFootageTags::FirstNoteCollectedEvent(),
		SourceId.IsNone() ? FName(TEXT("FirstNote")) : SourceId);
}

bool AHorrorGameModeBase::BeginFirstAnomalyCandidate(FName SourceId)
{
	if (!CanBeginFirstAnomalyCandidate())
	{
		return false;
	}

	return AnomalyDirector.BeginFirstAnomalyCandidate(SourceId, FoundFootageContract);
}

bool AHorrorGameModeBase::TryRecordFirstAnomaly(bool bIsRecording)
{
	if (!CanRecordFirstAnomaly(bIsRecording))
	{
		return false;
	}

	TArray<FHorrorFoundFootageRecordedEvent> RecordedEvents;
	TArray<FHorrorFoundFootageStateChange> StateChanges;
	const bool bRecorded = AnomalyDirector.TryRecordFirstAnomaly(bIsRecording, FoundFootageContract, &RecordedEvents, &StateChanges);
	if (bRecorded)
	{
		PublishFoundFootageEvents(GetWorld(), FoundFootageContract, RecordedEvents, this);
		for (const FHorrorFoundFootageStateChange& StateChange : StateChanges)
		{
			if (StateChange.bCompleted)
			{
				HandleObjectiveStateChange(StateChange.StateTag);
			}
		}
	}
	return bRecorded;
}

bool AHorrorGameModeBase::TryReviewArchive(FName SourceId)
{
	if (!CanReviewArchive())
	{
		return false;
	}

	return RecordFoundFootageEvent(
		HorrorFoundFootageTags::ArchiveReviewedEvent(),
		SourceId.IsNone() ? FName(TEXT("Archive")) : SourceId);
}

bool AHorrorGameModeBase::HasBodycamAcquired() const
{
	return FoundFootageContract.HasRecordedEvent(HorrorFoundFootageTags::BodycamAcquiredEvent());
}

bool AHorrorGameModeBase::HasCollectedFirstNote() const
{
	return FoundFootageContract.HasRecordedEvent(HorrorFoundFootageTags::FirstNoteCollectedEvent());
}

bool AHorrorGameModeBase::HasRecordedFirstAnomaly() const
{
	return FoundFootageContract.HasRecordedEvent(HorrorFoundFootageTags::FirstAnomalyRecordedEvent());
}

bool AHorrorGameModeBase::HasPendingFirstAnomalyCandidate() const
{
	return AnomalyDirector.HasPendingFirstAnomalyCandidate();
}

FName AHorrorGameModeBase::GetPendingFirstAnomalySourceId() const
{
	return AnomalyDirector.GetPendingFirstAnomalySourceId();
}

bool AHorrorGameModeBase::HasReviewedArchive() const
{
	return FoundFootageContract.HasRecordedEvent(HorrorFoundFootageTags::ArchiveReviewedEvent());
}

bool AHorrorGameModeBase::HasFoundFootageRecordedEvent(FGameplayTag EventTag) const
{
	return FoundFootageContract.HasRecordedEvent(EventTag);
}

bool AHorrorGameModeBase::HasFoundFootageCompletedState(FGameplayTag StateTag) const
{
	return FoundFootageContract.HasCompletedState(StateTag);
}

bool AHorrorGameModeBase::CanCollectFirstNote() const
{
	return HasBodycamAcquired() && !HasCollectedFirstNote();
}

bool AHorrorGameModeBase::CanBeginFirstAnomalyCandidate() const
{
	return HasCollectedFirstNote()
		&& !HasPendingFirstAnomalyCandidate()
		&& !HasRecordedFirstAnomaly();
}

bool AHorrorGameModeBase::CanRecordFirstAnomaly(bool bIsRecording) const
{
	return bIsRecording
		&& HasPendingFirstAnomalyCandidate()
		&& !HasRecordedFirstAnomaly();
}

bool AHorrorGameModeBase::CanReviewArchive() const
{
	return HasRecordedFirstAnomaly() && !HasReviewedArchive();
}

bool AHorrorGameModeBase::IsExitUnlocked() const
{
	return FoundFootageContract.IsExitUnlocked();
}

bool AHorrorGameModeBase::TryCompleteDay1(FName SourceId)
{
	if (!IsExitUnlocked() || bDay1Complete)
	{
		return false;
	}

	bDay1Complete = true;

	if (UWorld* World = GetWorld())
	{
		if (UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>())
		{
			const FName ResolvedSourceId = SourceId.IsNone() ? HorrorObjectiveMilestoneSources::Day1Complete : SourceId;
			EventBus->Publish(
				HorrorDay1Tags::Day1CompletedEvent(),
				ResolvedSourceId,
				HorrorDay1Tags::Day1CompletedState(),
				this);
		}
	}

	if (bAutoSpawnCampaignRuntimeOnBeginPlay)
	{
		EnsureCampaignRuntime();
	}

	TryAutosaveOnMilestone(HorrorObjectiveMilestoneCheckpoints::Day1Complete);
	return true;
}

void AHorrorGameModeBase::ImportDay1CompleteState(bool bInDay1Complete)
{
	bDay1Complete = bInDay1Complete;
	if (bDay1Complete && bAutoSpawnCampaignRuntimeOnBeginPlay)
	{
		EnsureCampaignRuntime();
	}
}

FHorrorFoundFootageProgressSnapshot AHorrorGameModeBase::BuildFoundFootageProgressSnapshot() const
{
	return FoundFootageContract.BuildSnapshot();
}

FHorrorObjectiveTrackerSnapshot AHorrorGameModeBase::BuildObjectiveTrackerSnapshot() const
{
	FHorrorObjectiveTrackerSnapshot Tracker;
	const auto MakeChecklistItem = [](EHorrorObjectiveTrackerStage Stage, const FText& Label, bool bComplete, bool bActive, bool bRequiresRecording)
	{
		FHorrorObjectiveChecklistItem Item;
		Item.Stage = Stage;
		Item.Label = Label;
		Item.bComplete = bComplete;
		Item.bActive = bActive;
		Item.bRequiresRecording = bRequiresRecording;
		return Item;
	};
	const auto BuildCampaignInteractionLabel = [](const FHorrorCampaignObjectiveDefinition& Objective)
	{
		return Objective.Presentation.MechanicLabel.IsEmpty()
			? NSLOCTEXT("HorrorObjectiveTracker", "BriefInstant", "直接互动：按互动键")
			: Objective.Presentation.MechanicLabel;
	};
	const auto BuildCampaignRuntimeStatusText = [](const FHorrorCampaignObjectiveRuntimeState& RuntimeState)
	{
		switch (RuntimeState.Status)
		{
			case EHorrorCampaignObjectiveRuntimeStatus::AdvancedInteractionActive:
				if (!RuntimeState.AdvancedInteraction.NextActionLabel.IsEmpty())
				{
					if (!RuntimeState.AdvancedInteraction.DeviceStatusLabel.IsEmpty())
					{
						return FText::Format(
							NSLOCTEXT("HorrorObjectiveTracker", "StatusAdvancedActiveWithDiagnostics", "{0}  |  {1}"),
							RuntimeState.AdvancedInteraction.DeviceStatusLabel,
							RuntimeState.AdvancedInteraction.NextActionLabel);
					}
					return RuntimeState.AdvancedInteraction.NextActionLabel;
				}
				return RuntimeState.PhaseText.IsEmpty()
					? NSLOCTEXT("HorrorObjectiveTracker", "StatusAdvancedActive", "交互窗口已展开")
					: RuntimeState.PhaseText;
			case EHorrorCampaignObjectiveRuntimeStatus::TimedPursuitActive:
				return RuntimeState.PhaseText.IsEmpty()
					? NSLOCTEXT("HorrorObjectiveTracker", "StatusPursuitActive", "追逐进行中")
					: RuntimeState.PhaseText;
			case EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable:
				return RuntimeState.PhaseText.IsEmpty()
					? NSLOCTEXT("HorrorObjectiveTracker", "StatusRetryableFailure", "目标失败，可重新互动")
					: RuntimeState.PhaseText;
			case EHorrorCampaignObjectiveRuntimeStatus::Completed:
				return RuntimeState.PhaseText.IsEmpty()
					? NSLOCTEXT("HorrorObjectiveTracker", "StatusCompleted", "目标已完成")
					: RuntimeState.PhaseText;
			case EHorrorCampaignObjectiveRuntimeStatus::Locked:
				return RuntimeState.BlockedReason.IsEmpty()
					? NSLOCTEXT("HorrorObjectiveTracker", "StatusLocked", "目标暂未解锁")
					: RuntimeState.BlockedReason;
			case EHorrorCampaignObjectiveRuntimeStatus::Available:
				return RuntimeState.PhaseText.IsEmpty()
					? NSLOCTEXT("HorrorObjectiveTracker", "StatusAvailable", "等待互动")
					: RuntimeState.PhaseText;
			case EHorrorCampaignObjectiveRuntimeStatus::Hidden:
			default:
				return FText::GetEmpty();
		}
	};
	const auto BuildCampaignGraphStatusText = [](const FHorrorCampaignObjectiveGraphNode& GraphNode, bool bCurrentStoryObjective)
	{
		switch (GraphNode.Status)
		{
			case EHorrorCampaignObjectiveGraphStatus::Completed:
				return NSLOCTEXT("HorrorObjectiveTracker", "GraphStatusCompleted", "目标已完成");
			case EHorrorCampaignObjectiveGraphStatus::Available:
				if (GraphNode.bOptional)
				{
					return NSLOCTEXT("HorrorObjectiveTracker", "GraphStatusOptionalAvailable", "可选线索：靠近主路线时调查");
				}
				if (!bCurrentStoryObjective)
				{
					return NSLOCTEXT("HorrorObjectiveTracker", "GraphStatusParallelMainlineAvailable", "并行主线目标：可切换推进");
				}
				return NSLOCTEXT("HorrorObjectiveTracker", "GraphStatusAvailable", "可执行目标");
			case EHorrorCampaignObjectiveGraphStatus::Locked:
			default:
				return NSLOCTEXT("HorrorObjectiveTracker", "GraphStatusLocked", "目标暂未解锁");
		}
	};
	const auto BuildCampaignGraphLockReasonText = [&BuildCampaignGraphStatusText](const FHorrorCampaignObjectiveGraphNode& GraphNode, const FHorrorCampaignChapterDefinition& Chapter)
	{
		if (GraphNode.MissingPrerequisiteObjectiveIds.IsEmpty())
		{
			return BuildCampaignGraphStatusText(GraphNode, false);
		}

		TArray<FText> MissingPrerequisiteTexts;
		for (const FName MissingObjectiveId : GraphNode.MissingPrerequisiteObjectiveIds)
		{
			const FHorrorCampaignObjectiveDefinition* MissingObjective = FHorrorCampaign::FindObjectiveById(Chapter, MissingObjectiveId);
			MissingPrerequisiteTexts.Add(MissingObjective && !MissingObjective->PromptText.IsEmpty()
				? MissingObjective->PromptText
				: FText::FromName(MissingObjectiveId));
		}

		return FText::Format(
			NSLOCTEXT("HorrorObjectiveTracker", "GraphStatusLockedWithPrerequisites", "锁定：先完成{0}"),
			JoinCampaignObjectiveTextList(MissingPrerequisiteTexts));
	};
	const auto BuildInteractionModeText = [](EHorrorCampaignInteractionMode Mode)
	{
		switch (Mode)
		{
			case EHorrorCampaignInteractionMode::MultiStep:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalModeMultiStep", "多段操作");
			case EHorrorCampaignInteractionMode::CircuitWiring:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalModeCircuitWiring", "电路接线");
			case EHorrorCampaignInteractionMode::GearCalibration:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalModeGearCalibration", "齿轮校准");
			case EHorrorCampaignInteractionMode::SpectralScan:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalModeSpectralScan", "频谱扫描");
			case EHorrorCampaignInteractionMode::SignalTuning:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalModeSignalTuning", "信号调谐");
			case EHorrorCampaignInteractionMode::TimedPursuit:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalModeTimedPursuit", "巨人追逐");
			case EHorrorCampaignInteractionMode::Instant:
			default:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalModeInstant", "直接互动");
		}
	};
	const auto BuildCompletionRuleText = [](EHorrorCampaignObjectiveBeatCompletionRule Rule)
	{
		switch (Rule)
		{
			case EHorrorCampaignObjectiveBeatCompletionRule::HoldInteract:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalRuleHoldInteract", "持续互动");
			case EHorrorCampaignObjectiveBeatCompletionRule::AdvancedWindow:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalRuleAdvancedWindow", "窗口同步");
			case EHorrorCampaignObjectiveBeatCompletionRule::ReachEscapePoint:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalRuleReachEscapePoint", "抵达逃离点");
			case EHorrorCampaignObjectiveBeatCompletionRule::RecordEvidence:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalRuleRecordEvidence", "录像归档");
			case EHorrorCampaignObjectiveBeatCompletionRule::ConfirmResult:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalRuleConfirmResult", "确认结果");
			case EHorrorCampaignObjectiveBeatCompletionRule::InteractOnce:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalRuleInteractOnce", "单次互动");
			case EHorrorCampaignObjectiveBeatCompletionRule::None:
			default:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalRuleNone", "现场确认");
		}
	};
	const auto BuildFailurePolicyText = [](EHorrorCampaignObjectiveBeatFailurePolicy Policy)
	{
		switch (Policy)
		{
			case EHorrorCampaignObjectiveBeatFailurePolicy::RetryBeat:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalFailureRetryBeat", "重试阶段");
			case EHorrorCampaignObjectiveBeatFailurePolicy::ResetObjective:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalFailureResetObjective", "重置目标");
			case EHorrorCampaignObjectiveBeatFailurePolicy::CampaignRecovery:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalFailureCampaignRecovery", "撤回安全点");
			case EHorrorCampaignObjectiveBeatFailurePolicy::None:
			default:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalFailureNone", "无惩罚");
		}
	};
	const auto BuildNavigationRoleText = [](EHorrorCampaignObjectiveBeatNavigationRole NavigationRole)
	{
		switch (NavigationRole)
		{
			case EHorrorCampaignObjectiveBeatNavigationRole::EscapeDestination:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalNavigationEscapeDestination", "逃离点");
			case EHorrorCampaignObjectiveBeatNavigationRole::SearchArea:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalNavigationSearchArea", "搜索区域");
			case EHorrorCampaignObjectiveBeatNavigationRole::ConfirmationPoint:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalNavigationConfirmationPoint", "确认点");
			case EHorrorCampaignObjectiveBeatNavigationRole::ObjectiveActor:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalNavigationObjectiveActor", "目标位置");
			case EHorrorCampaignObjectiveBeatNavigationRole::None:
			default:
				return NSLOCTEXT("HorrorObjectiveTracker", "TacticalNavigationNone", "无导航");
		}
	};
	const auto BuildTacticalLabel = [&BuildInteractionModeText, &BuildCompletionRuleText, &BuildFailurePolicyText, &BuildNavigationRoleText](EHorrorCampaignInteractionMode Mode, EHorrorCampaignObjectiveBeatCompletionRule Rule, EHorrorCampaignObjectiveBeatFailurePolicy Policy, EHorrorCampaignObjectiveBeatNavigationRole NavigationRole)
	{
		return FText::Format(
			NSLOCTEXT("HorrorObjectiveTracker", "TacticalLabelFormat", "玩法：{0}  |  判定：{1}  |  失败：{2}  |  导航：{3}"),
			BuildInteractionModeText(Mode),
			BuildCompletionRuleText(Rule),
			BuildFailurePolicyText(Policy),
			BuildNavigationRoleText(NavigationRole));
	};
	const auto ApplyAdvancedInteractionDiagnostics = [](FHorrorObjectiveChecklistItem& Item, const FHorrorCampaignObjectiveRuntimeState& RuntimeState)
	{
		if (!RuntimeState.AdvancedInteraction.bVisible
			&& !RuntimeState.bAdvancedInteractionActive
			&& RuntimeState.Status != EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable)
		{
			return;
		}

		Item.DeviceStatusLabel = RuntimeState.AdvancedInteraction.DeviceStatusLabel;
		Item.NextActionLabel = RuntimeState.AdvancedInteraction.NextActionLabel;
		Item.FailureRecoveryLabel = RuntimeState.AdvancedInteraction.FailureRecoveryLabel;
		Item.PerformanceGradeFraction = FMath::Clamp(RuntimeState.AdvancedInteraction.PerformanceGradeFraction, 0.0f, 1.0f);
		Item.InputPrecisionFraction = FMath::Clamp(RuntimeState.AdvancedInteraction.InputPrecisionFraction, 0.0f, 1.0f);
		Item.DeviceLoadFraction = FMath::Clamp(RuntimeState.AdvancedInteraction.DeviceLoadFraction, 0.0f, 1.0f);
	};
	const auto ApplyTimedPursuitDiagnostics = [](FHorrorObjectiveChecklistItem& Item, const FHorrorCampaignObjectiveRuntimeState& RuntimeState)
	{
		if (RuntimeState.InteractionMode != EHorrorCampaignInteractionMode::TimedPursuit
			&& RuntimeState.Status != EHorrorCampaignObjectiveRuntimeStatus::TimedPursuitActive)
		{
			return;
		}

		if (!RuntimeState.EscapeBudgetLabel.IsEmpty())
		{
			Item.DeviceStatusLabel = RuntimeState.EscapeBudgetLabel;
		}
		if (!RuntimeState.EscapeActionLabel.IsEmpty())
		{
			Item.NextActionLabel = RuntimeState.EscapeActionLabel;
		}
		Item.FailureRecoveryLabel = NSLOCTEXT(
			"HorrorObjectiveTracker",
			"TimedPursuitRecoveryLabel",
			"被追上或超时后，会拉回追逐起点；重新互动再次开始。");
		Item.RiskLevel = EHorrorCampaignObjectiveRiskLevel::Critical;
		Item.PerformanceGradeFraction = RuntimeState.TimedRemainingSeconds > 0.0f
			? FMath::Clamp(RuntimeState.EscapeTimeBudgetSeconds / FMath::Max(RuntimeState.TimedRemainingSeconds, UE_SMALL_NUMBER), 0.0f, 1.0f)
			: 0.0f;
		if (RuntimeState.EscapeDistanceMeters > 0.0f)
		{
			Item.DistanceMeters = RuntimeState.EscapeDistanceMeters;
		}
	};
	const auto BuildStandardBeatDeviceStatusLabel = [](const FHorrorObjectiveChecklistItem& Item)
	{
		if (Item.bComplete)
		{
			return NSLOCTEXT("HorrorObjectiveTracker", "StandardBeatDeviceComplete", "现场状态：阶段已归档");
		}
		if (Item.bBlocked)
		{
			return NSLOCTEXT("HorrorObjectiveTracker", "StandardBeatDeviceBlocked", "现场状态：等待恢复或前置阶段");
		}
		if (Item.bActive)
		{
			return Item.InteractionMode == EHorrorCampaignInteractionMode::MultiStep
				? NSLOCTEXT("HorrorObjectiveTracker", "StandardBeatDeviceActiveMulti", "现场状态：当前阶段可执行")
				: NSLOCTEXT("HorrorObjectiveTracker", "StandardBeatDeviceActive", "现场状态：目标可互动");
		}
		return NSLOCTEXT("HorrorObjectiveTracker", "StandardBeatDeviceQueued", "现场状态：等待上一阶段");
	};
	const auto BuildStandardBeatNextActionLabel = [](EHorrorCampaignObjectiveBeatCompletionRule Rule, EHorrorCampaignInteractionMode Mode)
	{
		switch (Rule)
		{
			case EHorrorCampaignObjectiveBeatCompletionRule::HoldInteract:
				return NSLOCTEXT("HorrorObjectiveTracker", "StandardBeatNextHoldInteract", "长按互动键，保持到当前阶段完成。");
			case EHorrorCampaignObjectiveBeatCompletionRule::RecordEvidence:
				return NSLOCTEXT("HorrorObjectiveTracker", "StandardBeatNextRecordEvidence", "保持录像并对准证据，直到归档完成。");
			case EHorrorCampaignObjectiveBeatCompletionRule::ConfirmResult:
				return NSLOCTEXT("HorrorObjectiveTracker", "StandardBeatNextConfirmResult", "确认结果反馈后，推进到下一目标。");
			case EHorrorCampaignObjectiveBeatCompletionRule::ReachEscapePoint:
				return NSLOCTEXT("HorrorObjectiveTracker", "StandardBeatNextReachEscapePoint", "沿导航亮点移动到目标点。");
			case EHorrorCampaignObjectiveBeatCompletionRule::AdvancedWindow:
				return NSLOCTEXT("HorrorObjectiveTracker", "StandardBeatNextOpenPanel", "按互动键打开专用操作窗口。");
			case EHorrorCampaignObjectiveBeatCompletionRule::InteractOnce:
			case EHorrorCampaignObjectiveBeatCompletionRule::None:
			default:
				return Mode == EHorrorCampaignInteractionMode::MultiStep
					? NSLOCTEXT("HorrorObjectiveTracker", "StandardBeatNextMultiStep", "按互动键完成当前阶段。")
					: NSLOCTEXT("HorrorObjectiveTracker", "StandardBeatNextInteractOnce", "靠近目标并按互动键。");
		}
	};
	const auto BuildStandardBeatFailureRecoveryLabel = [](EHorrorCampaignObjectiveBeatFailurePolicy Policy)
	{
		switch (Policy)
		{
			case EHorrorCampaignObjectiveBeatFailurePolicy::RetryBeat:
				return NSLOCTEXT("HorrorObjectiveTracker", "StandardBeatRecoveryRetry", "失败后保留目标，重新完成当前阶段。");
			case EHorrorCampaignObjectiveBeatFailurePolicy::ResetObjective:
				return NSLOCTEXT("HorrorObjectiveTracker", "StandardBeatRecoveryReset", "失败后目标进度会重置，需要从第一阶段重来。");
			case EHorrorCampaignObjectiveBeatFailurePolicy::CampaignRecovery:
				return NSLOCTEXT("HorrorObjectiveTracker", "StandardBeatRecoveryCampaign", "失败后撤回安全点，再重新接近目标。");
			case EHorrorCampaignObjectiveBeatFailurePolicy::None:
			default:
				return NSLOCTEXT("HorrorObjectiveTracker", "StandardBeatRecoveryNone", "没有失败惩罚，保持调查节奏即可。");
		}
	};
	const auto ApplyStandardBeatDiagnostics = [&BuildStandardBeatDeviceStatusLabel, &BuildStandardBeatNextActionLabel, &BuildStandardBeatFailureRecoveryLabel](FHorrorObjectiveChecklistItem& Item, const FHorrorCampaignObjectiveRuntimeState& RuntimeState)
	{
		const bool bAdvancedDiagnostics = RuntimeState.Status == EHorrorCampaignObjectiveRuntimeStatus::AdvancedInteractionActive
			|| !RuntimeState.AdvancedInteraction.DeviceStatusLabel.IsEmpty()
			|| !RuntimeState.AdvancedInteraction.NextActionLabel.IsEmpty()
			|| !RuntimeState.AdvancedInteraction.FailureRecoveryLabel.IsEmpty();
		const bool bPursuitDiagnostics = Item.InteractionMode == EHorrorCampaignInteractionMode::TimedPursuit
			|| RuntimeState.Status == EHorrorCampaignObjectiveRuntimeStatus::TimedPursuitActive;
		if (bAdvancedDiagnostics || bPursuitDiagnostics)
		{
			return;
		}

		if (Item.DeviceStatusLabel.IsEmpty())
		{
			Item.DeviceStatusLabel = !RuntimeState.DeviceStatusLabel.IsEmpty()
				? RuntimeState.DeviceStatusLabel
				: BuildStandardBeatDeviceStatusLabel(Item);
		}
		if (Item.NextActionLabel.IsEmpty())
		{
			Item.NextActionLabel = !RuntimeState.NextActionLabel.IsEmpty() && Item.bActive
				? RuntimeState.NextActionLabel
				: BuildStandardBeatNextActionLabel(Item.BeatCompletionRule, Item.InteractionMode);
		}
		if (Item.FailureRecoveryLabel.IsEmpty())
		{
			Item.FailureRecoveryLabel = !RuntimeState.FailureRecoveryLabel.IsEmpty() && Item.bActive
				? RuntimeState.FailureRecoveryLabel
				: BuildStandardBeatFailureRecoveryLabel(Item.BeatFailurePolicy);
		}
		if (Item.PerformanceGradeFraction <= 0.0f)
		{
			Item.PerformanceGradeFraction = Item.bComplete
				? 1.0f
				: (Item.bActive
					? FMath::Clamp(FMath::Max(RuntimeState.ProgressFraction, 0.35f), 0.0f, 1.0f)
					: 0.0f);
		}
	};
	const auto ResolveCampaignObjectiveDistanceMeters = [this](const FHorrorCampaignObjectiveDefinition& Objective)
	{
		const UWorld* World = GetWorld();
		if (!World)
		{
			return 0.0f;
		}

		APawn* ViewerPawn = nullptr;
		if (const APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			ViewerPawn = PlayerController->GetPawn();
		}
		if (!ViewerPawn)
		{
			return 0.0f;
		}

		if (const AHorrorCampaignObjectiveActor* ObjectiveActor = FindRuntimeCampaignObjectiveActor(CampaignProgress.GetActiveChapterId(), Objective.ObjectiveId))
		{
			const FVector ToObjective = ObjectiveActor->GetActorLocation() - ViewerPawn->GetActorLocation();
			return static_cast<float>(FMath::Max(0.0, FVector(ToObjective.X, ToObjective.Y, 0.0).Size() / 100.0));
		}

		const FVector ObjectiveLocation = ResolveCampaignObjectiveTransform(Objective).GetLocation();
		const FVector ToObjective = ObjectiveLocation - ViewerPawn->GetActorLocation();
		return static_cast<float>(FMath::Max(0.0, FVector(ToObjective.X, ToObjective.Y, 0.0).Size() / 100.0));
	};
	const auto ResolveCurrentCampaignRuntimeState = [this](const FHorrorCampaignObjectiveDefinition& CurrentObjective)
	{
		FHorrorCampaignObjectiveRuntimeState RuntimeState;
		if (const AHorrorCampaignObjectiveActor* ObjectiveActor = FindRuntimeCampaignObjectiveActor(CampaignProgress.GetActiveChapterId(), CurrentObjective.ObjectiveId))
		{
			return ObjectiveActor->BuildObjectiveRuntimeState();
		}

		return RuntimeState;
	};
	const auto PopulateDay1Checklist = [this, &Tracker, &MakeChecklistItem]()
	{
		const bool bHasBodycam = HasBodycamAcquired();
		const bool bHasFirstNote = HasCollectedFirstNote();
		const bool bHasPendingAnomaly = HasPendingFirstAnomalyCandidate();
		const bool bHasAnomalyRecording = HasRecordedFirstAnomaly();
		const bool bHasArchiveReview = HasReviewedArchive();
		const bool bHasExit = IsExitUnlocked();
		const bool bHasDay1Complete = IsDay1Complete();

		Tracker.ChecklistItems.Reset();
		Tracker.ChecklistItems.Add(MakeChecklistItem(
			EHorrorObjectiveTrackerStage::RecoverBodycam,
			NSLOCTEXT("HorrorObjectiveChecklist", "RecoverBodycam", "取回随身摄像机"),
			bHasBodycam || bHasDay1Complete,
			Tracker.Stage == EHorrorObjectiveTrackerStage::RecoverBodycam,
			false));
		Tracker.ChecklistItems.Add(MakeChecklistItem(
			EHorrorObjectiveTrackerStage::ReadFirstNote,
			NSLOCTEXT("HorrorObjectiveChecklist", "ReadFirstNote", "阅读第一份站内备忘录"),
			bHasFirstNote || bHasDay1Complete,
			Tracker.Stage == EHorrorObjectiveTrackerStage::ReadFirstNote,
			false));
		Tracker.ChecklistItems.Add(MakeChecklistItem(
			EHorrorObjectiveTrackerStage::FrameFirstAnomaly,
			NSLOCTEXT("HorrorObjectiveChecklist", "FrameFirstAnomaly", "对准第一个异常"),
			bHasPendingAnomaly || bHasAnomalyRecording || bHasDay1Complete,
			Tracker.Stage == EHorrorObjectiveTrackerStage::FrameFirstAnomaly,
			false));
		Tracker.ChecklistItems.Add(MakeChecklistItem(
			EHorrorObjectiveTrackerStage::RecordFirstAnomaly,
			NSLOCTEXT("HorrorObjectiveChecklist", "RecordFirstAnomaly", "录像锁定第一个异常"),
			bHasAnomalyRecording || bHasDay1Complete,
			Tracker.Stage == EHorrorObjectiveTrackerStage::RecordFirstAnomaly,
			true));
		Tracker.ChecklistItems.Add(MakeChecklistItem(
			EHorrorObjectiveTrackerStage::ReviewArchive,
			NSLOCTEXT("HorrorObjectiveChecklist", "ReviewArchive", "复查档案终端"),
			bHasArchiveReview || bHasDay1Complete,
			Tracker.Stage == EHorrorObjectiveTrackerStage::ReviewArchive,
			false));
		Tracker.ChecklistItems.Add(MakeChecklistItem(
			EHorrorObjectiveTrackerStage::Escape,
			NSLOCTEXT("HorrorObjectiveChecklist", "Escape", "穿过维修闸门撤离"),
			bHasDay1Complete,
			Tracker.Stage == EHorrorObjectiveTrackerStage::Escape && bHasExit,
			false));

		const auto ApplyDay1ChecklistSummary = [](FHorrorObjectiveChecklistItem& Item, const FText& Detail, const FText& InteractionLabel, const FText& ActiveAction, const FText& LockedReason, const FText& RecoveryLabel, float ActiveProgressFraction)
		{
			Item.Detail = Detail;
			Item.InteractionLabel = InteractionLabel;

			if (Item.bComplete)
			{
				Item.StatusText = NSLOCTEXT("HorrorObjectiveChecklist", "Day1ItemCompleteStatus", "已完成");
				Item.DeviceStatusLabel = NSLOCTEXT("HorrorObjectiveChecklist", "Day1ItemCompleteDeviceStatus", "现场状态：证据已归档");
				Item.RuntimeProgressFraction = 1.0f;
				Item.PerformanceGradeFraction = 1.0f;
				return;
			}

			if (Item.bActive)
			{
				Item.StatusText = NSLOCTEXT("HorrorObjectiveChecklist", "Day1ItemActiveStatus", "当前目标");
				Item.DeviceStatusLabel = NSLOCTEXT("HorrorObjectiveChecklist", "Day1ItemActiveDeviceStatus", "现场状态：可执行");
				Item.NextActionLabel = ActiveAction;
				Item.FailureRecoveryLabel = RecoveryLabel;
				Item.RuntimeProgressFraction = FMath::Clamp(ActiveProgressFraction, 0.0f, 1.0f);
				Item.PerformanceGradeFraction = FMath::Clamp(ActiveProgressFraction + 0.2f, 0.0f, 1.0f);
				return;
			}

			Item.StatusText = NSLOCTEXT("HorrorObjectiveChecklist", "Day1ItemLockedStatus", "暂未解锁");
			Item.DeviceStatusLabel = NSLOCTEXT("HorrorObjectiveChecklist", "Day1ItemLockedDeviceStatus", "现场状态：等待前置条件");
			Item.LockReason = LockedReason;
			Item.bBlocked = !LockedReason.IsEmpty();
			Item.FailureRecoveryLabel = NSLOCTEXT("HorrorObjectiveChecklist", "Day1ItemLockedRecovery", "完成前置目标后自动解锁。");
			Item.RuntimeProgressFraction = 0.0f;
			Item.PerformanceGradeFraction = 0.0f;
		};

		if (Tracker.ChecklistItems.Num() == 6)
		{
			ApplyDay1ChecklistSummary(
				Tracker.ChecklistItems[0],
				NSLOCTEXT("HorrorObjectiveChecklist", "RecoverBodycamDetail", "建立录像链路，后续证据才会写入档案。"),
				NSLOCTEXT("HorrorObjectiveChecklist", "RecoverBodycamInteraction", "靠近拾取"),
				NSLOCTEXT("HorrorObjectiveChecklist", "RecoverBodycamAction", "靠近随身摄像机并按互动键取回。"),
				FText::GetEmpty(),
				NSLOCTEXT("HorrorObjectiveChecklist", "RecoverBodycamRecovery", "找不到时沿发光路线回到起始调查点。"),
				0.25f);
			ApplyDay1ChecklistSummary(
				Tracker.ChecklistItems[1],
				NSLOCTEXT("HorrorObjectiveChecklist", "ReadFirstNoteDetail", "读取站内备忘录，确认异常和档案终端线索。"),
				NSLOCTEXT("HorrorObjectiveChecklist", "ReadFirstNoteInteraction", "阅读备忘录"),
				NSLOCTEXT("HorrorObjectiveChecklist", "ReadFirstNoteAction", "靠近备忘录并按互动键阅读。"),
				NSLOCTEXT("HorrorObjectiveChecklist", "ReadFirstNoteLockReason", "先取回随身摄像机。"),
				NSLOCTEXT("HorrorObjectiveChecklist", "ReadFirstNoteRecovery", "回到摄像机位置补齐证据链。"),
				0.35f);
			ApplyDay1ChecklistSummary(
				Tracker.ChecklistItems[2],
				NSLOCTEXT("HorrorObjectiveChecklist", "FrameFirstAnomalyDetail", "用镜头搜索异常轮廓，直到系统进入捕捉准备。"),
				NSLOCTEXT("HorrorObjectiveChecklist", "FrameFirstAnomalyInteraction", "镜头对准"),
				NSLOCTEXT("HorrorObjectiveChecklist", "FrameFirstAnomalyAction", "打开摄像机，把异常保持在画面中心。"),
				NSLOCTEXT("HorrorObjectiveChecklist", "FrameFirstAnomalyLockReason", "先阅读第一份站内备忘录。"),
				NSLOCTEXT("HorrorObjectiveChecklist", "FrameFirstAnomalyRecovery", "视角丢失时退回走廊亮点重新取景。"),
				0.45f);
			ApplyDay1ChecklistSummary(
				Tracker.ChecklistItems[3],
				NSLOCTEXT("HorrorObjectiveChecklist", "RecordFirstAnomalyDetail", "保持录像并稳定取景，完成第一个异常的证据锁定。"),
				NSLOCTEXT("HorrorObjectiveChecklist", "RecordFirstAnomalyInteraction", "录像锁定"),
				NSLOCTEXT("HorrorObjectiveChecklist", "RecordFirstAnomalyAction", "保持录像，把异常持续留在镜头内。"),
				NSLOCTEXT("HorrorObjectiveChecklist", "RecordFirstAnomalyLockReason", "先用镜头对准第一个异常。"),
				NSLOCTEXT("HorrorObjectiveChecklist", "RecordFirstAnomalyRecovery", "录像中断时重新对准异常并再次保持录像。"),
				0.55f);
			ApplyDay1ChecklistSummary(
				Tracker.ChecklistItems[4],
				NSLOCTEXT("HorrorObjectiveChecklist", "ReviewArchiveDetail", "在档案终端复查录像，确认出口闸门恢复指令。"),
				NSLOCTEXT("HorrorObjectiveChecklist", "ReviewArchiveInteraction", "档案复查"),
				NSLOCTEXT("HorrorObjectiveChecklist", "ReviewArchiveAction", "前往档案终端并按互动键复查录像。"),
				NSLOCTEXT("HorrorObjectiveChecklist", "ReviewArchiveLockReason", "先录像锁定第一个异常。"),
				NSLOCTEXT("HorrorObjectiveChecklist", "ReviewArchiveRecovery", "找不到终端时回到主走廊，沿终端亮点前进。"),
				0.65f);
			ApplyDay1ChecklistSummary(
				Tracker.ChecklistItems[5],
				NSLOCTEXT("HorrorObjectiveChecklist", "EscapeDetail", "证据链完成后穿过维修闸门，离开失控区域。"),
				NSLOCTEXT("HorrorObjectiveChecklist", "EscapeInteraction", "撤离闸门"),
				NSLOCTEXT("HorrorObjectiveChecklist", "EscapeAction", "沿撤离路线冲向维修闸门并互动离开。"),
				NSLOCTEXT("HorrorObjectiveChecklist", "EscapeLockReason", "先在档案终端复查录像。"),
				NSLOCTEXT("HorrorObjectiveChecklist", "EscapeRecovery", "偏离路线时回到档案终端出口侧重新找门。"),
				0.8f);
		}
	};
	const auto PopulateCampaignChecklist = [this, &Tracker, &MakeChecklistItem, &BuildCampaignInteractionLabel, &BuildCampaignRuntimeStatusText, &BuildCampaignGraphStatusText, &BuildCampaignGraphLockReasonText, &BuildTacticalLabel, &ApplyAdvancedInteractionDiagnostics, &ApplyTimedPursuitDiagnostics, &ApplyStandardBeatDiagnostics, &ResolveCampaignObjectiveDistanceMeters, &ResolveCurrentCampaignRuntimeState](const FHorrorCampaignChapterDefinition& Chapter, const FHorrorCampaignObjectiveDefinition& CurrentObjective)
	{
		const FHorrorCampaignObjectiveRuntimeState RuntimeState = ResolveCurrentCampaignRuntimeState(CurrentObjective);
		const int32 ActiveBeatIndex = RuntimeState.CurrentBeatIndex != INDEX_NONE
			? RuntimeState.CurrentBeatIndex
			: 0;
		Tracker.ChecklistItems.Reset();
		for (int32 BeatIndex = 0; BeatIndex < CurrentObjective.ObjectiveBeats.Num(); ++BeatIndex)
		{
			const FHorrorCampaignObjectiveBeat& Beat = CurrentObjective.ObjectiveBeats[BeatIndex];
			FHorrorObjectiveChecklistItem BeatItem = MakeChecklistItem(
				EHorrorObjectiveTrackerStage::CampaignObjective,
				Beat.Label,
				BeatIndex < ActiveBeatIndex || RuntimeState.Status == EHorrorCampaignObjectiveRuntimeStatus::Completed,
				BeatIndex == ActiveBeatIndex && RuntimeState.Status != EHorrorCampaignObjectiveRuntimeStatus::Completed,
				Beat.bRequiresRecording);
			BeatItem.ObjectiveId = CurrentObjective.ObjectiveId;
			BeatItem.BeatId = Beat.BeatId.IsNone()
				? FName(*FString::Printf(TEXT("%s.Beat%d"), *CurrentObjective.ObjectiveId.ToString(), BeatIndex + 1))
				: Beat.BeatId;
			BeatItem.BeatIndex = BeatIndex;
			BeatItem.Detail = Beat.Detail;
			BeatItem.InteractionLabel = Tracker.InteractionLabel;
			BeatItem.StatusText = BuildCampaignRuntimeStatusText(RuntimeState);
			BeatItem.RuntimeStatus = RuntimeState.Status;
			BeatItem.FailureCause = RuntimeState.FailureCause;
			BeatItem.RecoveryAction = RuntimeState.RecoveryAction;
			BeatItem.InteractionMode = CurrentObjective.InteractionMode;
			BeatItem.RiskLevel = CurrentObjective.Presentation.RiskLevel;
			BeatItem.BeatCompletionRule = Beat.CompletionRule;
			BeatItem.BeatFailurePolicy = Beat.FailurePolicy;
			BeatItem.BeatNavigationRole = Beat.NavigationRole;
			BeatItem.TacticalLabel = BuildTacticalLabel(
				BeatItem.InteractionMode,
				BeatItem.BeatCompletionRule,
				BeatItem.BeatFailurePolicy,
				BeatItem.BeatNavigationRole);
			BeatItem.RewardText = CurrentObjective.Reward.RewardText;
			BeatItem.bOpensInteractionPanel = Beat.bOpensInteractionPanel || Tracker.bOpensInteractionPanel;
			BeatItem.bUrgent = Beat.bUrgent;
			BeatItem.bBlocked = RuntimeState.Status == EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable && BeatIndex > ActiveBeatIndex;
			BeatItem.LockReason = BeatItem.bBlocked
				? NSLOCTEXT("HorrorObjectiveTracker", "BeatBlockedByRetry", "先重新完成失败阶段。")
				: FText::GetEmpty();
			BeatItem.bOptional = CurrentObjective.bOptional || !CurrentObjective.bRequiredForChapterCompletion;
			BeatItem.bRetryable = RuntimeState.bRetryable && BeatIndex == ActiveBeatIndex;
			BeatItem.bMainline = !BeatItem.bOptional;
			BeatItem.RuntimeProgressFraction = RuntimeState.ProgressFraction;
			ApplyAdvancedInteractionDiagnostics(BeatItem, RuntimeState);
			BeatItem.RemainingSeconds = RuntimeState.TimedRemainingSeconds;
			BeatItem.DistanceMeters = ResolveCampaignObjectiveDistanceMeters(CurrentObjective);
			ApplyTimedPursuitDiagnostics(BeatItem, RuntimeState);
			ApplyStandardBeatDiagnostics(BeatItem, RuntimeState);
			Tracker.ChecklistItems.Add(BeatItem);
		}

		if (Tracker.ChecklistItems.IsEmpty())
		{
			FHorrorObjectiveChecklistItem FallbackItem = MakeChecklistItem(
				EHorrorObjectiveTrackerStage::CampaignObjective,
				CurrentObjective.PromptText,
				false,
				true,
				false);
			FallbackItem.ObjectiveId = CurrentObjective.ObjectiveId;
			FallbackItem.BeatId = CurrentObjective.ObjectiveId;
			FallbackItem.BeatIndex = 0;
			FallbackItem.Detail = Tracker.MissionContext;
			FallbackItem.InteractionLabel = Tracker.InteractionLabel;
			FallbackItem.StatusText = BuildCampaignRuntimeStatusText(RuntimeState);
			FallbackItem.RuntimeStatus = RuntimeState.Status;
			FallbackItem.FailureCause = RuntimeState.FailureCause;
			FallbackItem.RecoveryAction = RuntimeState.RecoveryAction;
			FallbackItem.InteractionMode = CurrentObjective.InteractionMode;
			FallbackItem.RiskLevel = CurrentObjective.Presentation.RiskLevel;
			FallbackItem.RewardText = CurrentObjective.Reward.RewardText;
			FallbackItem.BeatCompletionRule = EHorrorCampaignObjectiveBeatCompletionRule::InteractOnce;
			FallbackItem.BeatFailurePolicy = EHorrorCampaignObjectiveBeatFailurePolicy::None;
			FallbackItem.BeatNavigationRole = EHorrorCampaignObjectiveBeatNavigationRole::ObjectiveActor;
			FallbackItem.TacticalLabel = BuildTacticalLabel(
				FallbackItem.InteractionMode,
				FallbackItem.BeatCompletionRule,
				FallbackItem.BeatFailurePolicy,
				FallbackItem.BeatNavigationRole);
			FallbackItem.bOpensInteractionPanel = Tracker.bOpensInteractionPanel;
			FallbackItem.bUrgent = Tracker.bUrgent;
			FallbackItem.bOptional = CurrentObjective.bOptional || !CurrentObjective.bRequiredForChapterCompletion;
			FallbackItem.bRetryable = RuntimeState.bRetryable;
			FallbackItem.bMainline = !FallbackItem.bOptional;
			FallbackItem.RuntimeProgressFraction = RuntimeState.ProgressFraction;
			ApplyAdvancedInteractionDiagnostics(FallbackItem, RuntimeState);
			FallbackItem.RemainingSeconds = RuntimeState.TimedRemainingSeconds;
			FallbackItem.DistanceMeters = ResolveCampaignObjectiveDistanceMeters(CurrentObjective);
			ApplyTimedPursuitDiagnostics(FallbackItem, RuntimeState);
			ApplyStandardBeatDiagnostics(FallbackItem, RuntimeState);
			Tracker.ChecklistItems.Add(FallbackItem);
		}

		Tracker.ObjectiveGraphItems.Reset();
		const TArray<FHorrorCampaignObjectiveGraphNode> GraphNodes = CampaignProgress.BuildObjectiveGraph();
		for (const FHorrorCampaignObjectiveGraphNode& GraphNode : GraphNodes)
		{
			const bool bCurrentStoryObjective = GraphNode.ObjectiveId == CurrentObjective.ObjectiveId;
			const bool bNavigationFocused = !CampaignNavigationFocusObjectiveId.IsNone()
				&& GraphNode.ObjectiveId == CampaignNavigationFocusObjectiveId
				&& IsCampaignNavigationFocusObjectiveValid(CampaignNavigationFocusObjectiveId);
			const bool bAvailableOptionalInvestigation = GraphNode.Status == EHorrorCampaignObjectiveGraphStatus::Available && GraphNode.bOptional;
			const bool bAvailableParallelMainline = GraphNode.Status == EHorrorCampaignObjectiveGraphStatus::Available && GraphNode.bMainline && !bCurrentStoryObjective;
			if (GraphNode.Status == EHorrorCampaignObjectiveGraphStatus::Available
				&& !bCurrentStoryObjective
				&& !bAvailableOptionalInvestigation
				&& !bAvailableParallelMainline)
			{
				continue;
			}

			const FHorrorCampaignObjectiveDefinition* GraphObjective = FHorrorCampaign::FindObjectiveById(Chapter, GraphNode.ObjectiveId);
			if (!GraphObjective)
			{
				continue;
			}

			FHorrorObjectiveChecklistItem GraphItem = MakeChecklistItem(
				EHorrorObjectiveTrackerStage::CampaignObjective,
				GraphObjective->PromptText.IsEmpty() ? FText::FromName(GraphNode.ObjectiveId) : GraphObjective->PromptText,
				GraphNode.Status == EHorrorCampaignObjectiveGraphStatus::Completed,
				GraphNode.Status == EHorrorCampaignObjectiveGraphStatus::Available && bCurrentStoryObjective,
				GraphObjective->ObjectiveBeats.ContainsByPredicate(
					[](const FHorrorCampaignObjectiveBeat& Beat)
					{
						return Beat.bRequiresRecording;
					}));
			GraphItem.ObjectiveId = GraphObjective->ObjectiveId;
			GraphItem.BeatId = GraphObjective->ObjectiveId;
			GraphItem.BeatIndex = GraphNode.ChapterOrder;
			GraphItem.Detail = !GraphObjective->Presentation.MissionContext.IsEmpty()
				? GraphObjective->Presentation.MissionContext
				: (!GraphObjective->ObjectiveBeats.IsEmpty() ? GraphObjective->ObjectiveBeats[0].Detail : FText::GetEmpty());
			GraphItem.InteractionLabel = BuildCampaignInteractionLabel(*GraphObjective);
			GraphItem.StatusText = BuildCampaignGraphStatusText(GraphNode, bCurrentStoryObjective);
			if (bNavigationFocused)
			{
				GraphItem.StatusText = FText::Format(
					NSLOCTEXT("HorrorObjectiveTracker", "GraphStatusNavigationFocused", "导航锁定：{0}"),
					GraphItem.StatusText.IsEmpty()
						? NSLOCTEXT("HorrorObjectiveTracker", "GraphStatusNavigationFocusedFallback", "任务图焦点")
						: GraphItem.StatusText);
			}
			GraphItem.RuntimeStatus = GraphNode.Status == EHorrorCampaignObjectiveGraphStatus::Completed
				? EHorrorCampaignObjectiveRuntimeStatus::Completed
				: GraphNode.Status == EHorrorCampaignObjectiveGraphStatus::Available
					? EHorrorCampaignObjectiveRuntimeStatus::Available
					: EHorrorCampaignObjectiveRuntimeStatus::Locked;
			GraphItem.InteractionMode = GraphObjective->InteractionMode;
			GraphItem.RiskLevel = GraphObjective->Presentation.RiskLevel;
			GraphItem.RewardText = GraphObjective->Reward.RewardText;
			GraphItem.BeatCompletionRule = !GraphObjective->ObjectiveBeats.IsEmpty()
				? GraphObjective->ObjectiveBeats[0].CompletionRule
				: EHorrorCampaignObjectiveBeatCompletionRule::InteractOnce;
			GraphItem.BeatFailurePolicy = !GraphObjective->ObjectiveBeats.IsEmpty()
				? GraphObjective->ObjectiveBeats[0].FailurePolicy
				: EHorrorCampaignObjectiveBeatFailurePolicy::None;
			GraphItem.BeatNavigationRole = !GraphObjective->ObjectiveBeats.IsEmpty()
				? GraphObjective->ObjectiveBeats[0].NavigationRole
				: EHorrorCampaignObjectiveBeatNavigationRole::ObjectiveActor;
			GraphItem.TacticalLabel = BuildTacticalLabel(
				GraphItem.InteractionMode,
				GraphItem.BeatCompletionRule,
				GraphItem.BeatFailurePolicy,
				GraphItem.BeatNavigationRole);
			GraphItem.bBlocked = GraphNode.Status == EHorrorCampaignObjectiveGraphStatus::Locked;
			GraphItem.LockReason = GraphItem.bBlocked
				? BuildCampaignGraphLockReasonText(GraphNode, Chapter)
				: FText::GetEmpty();
			GraphItem.bOptional = GraphNode.bOptional;
			GraphItem.bMainline = GraphNode.bMainline;
			GraphItem.bNavigationFocused = bNavigationFocused;
			GraphItem.bOpensInteractionPanel = GraphObjective->Presentation.bOpensInteractionPanel;
			GraphItem.bUrgent = GraphObjective->InteractionMode == EHorrorCampaignInteractionMode::TimedPursuit
				|| GraphObjective->Presentation.RiskLevel == EHorrorCampaignObjectiveRiskLevel::Critical
				|| GraphObjective->ObjectiveBeats.ContainsByPredicate(
					[](const FHorrorCampaignObjectiveBeat& Beat)
					{
						return Beat.bUrgent;
					});
			GraphItem.DistanceMeters = ResolveCampaignObjectiveDistanceMeters(*GraphObjective);
			GraphItem.RuntimeProgressFraction = GraphItem.bComplete ? 1.0f : 0.0f;
			Tracker.ObjectiveGraphItems.Add(GraphItem);
		}

		Tracker.CompletedMilestoneCount = GetCampaignCompletedObjectiveCount();
		Tracker.RequiredMilestoneCount = GetCampaignRequiredObjectiveCount();
		Tracker.ProgressFraction = Tracker.RequiredMilestoneCount > 0
			? FMath::Clamp(
				static_cast<float>(Tracker.CompletedMilestoneCount) / static_cast<float>(Tracker.RequiredMilestoneCount),
				0.0f,
				1.0f)
			: 0.0f;
		Tracker.ProgressLabel = FText::Format(
			NSLOCTEXT("HorrorObjectiveTracker", "CampaignProgressLabel", "章节任务 {0}/{1}"),
			FText::AsNumber(Tracker.CompletedMilestoneCount),
			FText::AsNumber(Tracker.RequiredMilestoneCount));
		Tracker.bRequiresRecording = CurrentObjective.ObjectiveBeats.ContainsByPredicate(
			[](const FHorrorCampaignObjectiveBeat& Beat)
			{
				return Beat.bRequiresRecording;
			});
		Tracker.bUrgent = CurrentObjective.ObjectiveBeats.ContainsByPredicate(
			[](const FHorrorCampaignObjectiveBeat& Beat)
			{
				return Beat.bUrgent;
			})
			|| CurrentObjective.InteractionMode == EHorrorCampaignInteractionMode::TimedPursuit;
		Tracker.bComplete = CampaignProgress.IsChapterComplete();
		(void)Chapter;
	};

	const FHorrorFoundFootageProgressSnapshot ProgressSnapshot = BuildFoundFootageProgressSnapshot();
	Tracker.CompletedMilestoneCount = ProgressSnapshot.CompletedMilestoneCount;
	Tracker.RequiredMilestoneCount = ProgressSnapshot.Milestones.Num();
	Tracker.ProgressFraction = Tracker.RequiredMilestoneCount > 0
		? FMath::Clamp(
			static_cast<float>(Tracker.CompletedMilestoneCount) / static_cast<float>(Tracker.RequiredMilestoneCount),
			0.0f,
			1.0f)
		: 0.0f;
	Tracker.bExitUnlocked = ProgressSnapshot.bExitUnlocked;
	Tracker.ProgressLabel = FText::Format(
		NSLOCTEXT("HorrorObjectiveTracker", "ProgressLabel", "证据链 {0}/{1}"),
		FText::AsNumber(Tracker.CompletedMilestoneCount),
		FText::AsNumber(Tracker.RequiredMilestoneCount));

	if (ShouldExposeCampaignObjectivesToHUD())
	{
		const FHorrorCampaignChapterDefinition* Chapter = GetCurrentCampaignChapterDefinition();
		const FHorrorCampaignObjectiveDefinition* CurrentObjective = CampaignProgress.GetNextObjective();

		if (Chapter && CurrentObjective)
		{
			const FText CampaignObjectivePrompt = GetCurrentCampaignObjectivePromptText();
			Tracker.Stage = EHorrorObjectiveTrackerStage::CampaignObjective;
			Tracker.Title = Chapter && !Chapter->Title.IsEmpty()
				? Chapter->Title
				: NSLOCTEXT("HorrorObjectiveTracker", "CampaignObjectiveTitle", "当前任务");
			Tracker.PrimaryInstruction = CampaignObjectivePrompt.IsEmpty()
				? FText::Format(NSLOCTEXT("HorrorObjectiveTracker", "CampaignObjectiveFallback", "目标：{0}"),
					FText::FromName(CurrentObjective->ObjectiveId))
				: CampaignObjectivePrompt;
			const FText BeatText = GetCurrentCampaignObjectiveBeatText();
			Tracker.SecondaryInstruction = BeatText.IsEmpty()
				? NSLOCTEXT("HorrorObjectiveTracker", "CampaignObjectiveSecondary", "跟随目标方向推进，留意环境异常。")
				: BeatText;
			Tracker.ActiveChapterId = Chapter->ChapterId;
			Tracker.ActiveObjectiveId = CurrentObjective->ObjectiveId;
			Tracker.InteractionLabel = BuildCampaignInteractionLabel(*CurrentObjective);
			Tracker.MissionContext = !CurrentObjective->Presentation.MissionContext.IsEmpty()
				? CurrentObjective->Presentation.MissionContext
				: (!CurrentObjective->ObjectiveBeats.IsEmpty() && !CurrentObjective->ObjectiveBeats[0].Detail.IsEmpty()
					? CurrentObjective->ObjectiveBeats[0].Detail
					: (!Chapter->StoryBrief.IsEmpty()
						? Chapter->StoryBrief
						: NSLOCTEXT("HorrorObjectiveTracker", "CampaignMissionContextFallback", "完成当前目标以推进章节链路。")));
			Tracker.FailureStakes = CurrentObjective->Presentation.FailureStakes;
			Tracker.LockReason = CampaignProgress.CanCompleteObjective(CurrentObjective->ObjectiveId)
				? FText::GetEmpty()
				: BuildCampaignObjectiveLockReasonText(Chapter->ChapterId, CurrentObjective->ObjectiveId);
			Tracker.bUsesFocusedInteraction = CurrentObjective->Presentation.bUsesFocusedInteraction;
			Tracker.bOpensInteractionPanel = CurrentObjective->Presentation.bOpensInteractionPanel;
			PopulateCampaignChecklist(*Chapter, *CurrentObjective);
			return Tracker;
		}
		else
		{
			UE_LOG(LogHorrorProject, Warning, TEXT("BuildObjectiveTrackerSnapshot: Campaign objectives enabled but no chapter/objective found. Chapter=%s, Objective=%s"),
				Chapter ? *Chapter->ChapterId.ToString() : TEXT("null"),
				CurrentObjective ? *CurrentObjective->ObjectiveId.ToString() : TEXT("null"));
			PopulateDay1Checklist();
		}
	}

	if (IsDay1Complete())
	{
		Tracker.Stage = EHorrorObjectiveTrackerStage::Day1Complete;
		Tracker.Title = NSLOCTEXT("HorrorObjectiveTracker", "Day1CompleteTitle", "第一天完成");
		Tracker.PrimaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "Day1CompletePrimary", "证据已保存。");
		Tracker.SecondaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "Day1CompleteSecondary", "等待信号归档，准备进入下一段记录。");
		Tracker.ProgressFraction = 1.0f;
		Tracker.bComplete = true;
		Tracker.bExitUnlocked = true;
		PopulateDay1Checklist();
		return Tracker;
	}

	if (!HasBodycamAcquired())
	{
		Tracker.Stage = EHorrorObjectiveTrackerStage::RecoverBodycam;
		Tracker.Title = NSLOCTEXT("HorrorObjectiveTracker", "RecoverBodycamTitle", "建立录像链路");
		Tracker.PrimaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "RecoverBodycamPrimary", "取回随身摄像机。");
		Tracker.SecondaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "RecoverBodycamSecondary", "没有摄像机，后续证据无法归档。");
		PopulateDay1Checklist();
		return Tracker;
	}

	if (!HasCollectedFirstNote())
	{
		Tracker.Stage = EHorrorObjectiveTrackerStage::ReadFirstNote;
		Tracker.Title = NSLOCTEXT("HorrorObjectiveTracker", "ReadFirstNoteTitle", "恢复站内线索");
		Tracker.PrimaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "ReadFirstNotePrimary", "找到并阅读第一份站内备忘录。");
		Tracker.SecondaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "ReadFirstNoteSecondary", "备忘录会记录门禁码和异常位置线索。");
		PopulateDay1Checklist();
		return Tracker;
	}

	if (HasPendingFirstAnomalyCandidate() && !HasRecordedFirstAnomaly())
	{
		Tracker.Stage = EHorrorObjectiveTrackerStage::RecordFirstAnomaly;
		Tracker.Title = NSLOCTEXT("HorrorObjectiveTracker", "RecordFirstAnomalyTitle", "捕捉第一个异常");
		Tracker.PrimaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "RecordFirstAnomalyPrimary", "保持随身摄像机录像并捕捉异常。");
		Tracker.SecondaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "RecordFirstAnomalySecondary", "异常已对准，保持录像直到锁定完成。");
		Tracker.bRequiresRecording = true;
		Tracker.bUrgent = true;
		PopulateDay1Checklist();
		return Tracker;
	}

	if (!HasRecordedFirstAnomaly())
	{
		Tracker.Stage = EHorrorObjectiveTrackerStage::FrameFirstAnomaly;
		Tracker.Title = NSLOCTEXT("HorrorObjectiveTracker", "FrameFirstAnomalyTitle", "搜索异常信号");
		Tracker.PrimaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "FrameFirstAnomalyPrimary", "寻找并对准第一个异常。");
		Tracker.SecondaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "FrameFirstAnomalySecondary", "镜头对准异常后，系统会进入捕捉准备状态。");
		PopulateDay1Checklist();
		return Tracker;
	}

	if (!HasReviewedArchive())
	{
		Tracker.Stage = EHorrorObjectiveTrackerStage::ReviewArchive;
		Tracker.Title = NSLOCTEXT("HorrorObjectiveTracker", "ReviewArchiveTitle", "复查证据");
		Tracker.PrimaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "ReviewArchivePrimary", "前往档案终端复查录像。");
		Tracker.SecondaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "ReviewArchiveSecondary", "档案确认后，出口闸门才会恢复指令。");
		PopulateDay1Checklist();
		return Tracker;
	}

	if (IsExitUnlocked())
	{
		Tracker.Stage = EHorrorObjectiveTrackerStage::Escape;
		Tracker.Title = NSLOCTEXT("HorrorObjectiveTracker", "EscapeTitle", "撤离路线已开启");
		Tracker.PrimaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "EscapePrimary", "穿过维修闸门离开。");
		Tracker.SecondaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "EscapeSecondary", "不要停留，站内信号正在恶化。");
		Tracker.bUrgent = true;
		Tracker.bExitUnlocked = true;
		PopulateDay1Checklist();
		return Tracker;
	}

	Tracker.Stage = EHorrorObjectiveTrackerStage::Escape;
	Tracker.Title = NSLOCTEXT("HorrorObjectiveTracker", "FallbackTitle", "继续推进");
	Tracker.PrimaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "FallbackPrimary", "继续深入站内。");
	Tracker.SecondaryInstruction = NSLOCTEXT("HorrorObjectiveTracker", "FallbackSecondary", "寻找下一条可验证线索。");
	PopulateDay1Checklist();
	return Tracker;
}

const FHorrorFoundFootageContract& AHorrorGameModeBase::GetFoundFootageContract() const
{
	return FoundFootageContract;
}

FHorrorFoundFootageSaveState AHorrorGameModeBase::ExportFoundFootageSaveState() const
{
	return FoundFootageContract.ExportSaveState();
}

void AHorrorGameModeBase::ImportFoundFootageSaveState(const FHorrorFoundFootageSaveState& SaveState)
{
	FoundFootageContract.ImportSaveState(SaveState);
	AnomalyDirector.ImportPendingFirstAnomalyCandidate(NAME_None, FoundFootageContract);
	SyncFoundFootageRuntimeStateToPlayer();
	RestoreEncounterStateFromFoundFootageCheckpoint();
}

void AHorrorGameModeBase::ImportPendingFirstAnomalyCandidate(FName SourceId)
{
	AnomalyDirector.ImportPendingFirstAnomalyCandidate(SourceId, FoundFootageContract);
}

FHorrorCampaignSaveState AHorrorGameModeBase::ExportCampaignSaveState() const
{
	FHorrorCampaignSaveState SaveState = CampaignProgress.ExportSaveState();
	for (const AHorrorCampaignObjectiveActor* ObjectiveActor : RuntimeCampaignObjectiveActorViews)
	{
		if (!ObjectiveActor || ObjectiveActor->IsActorBeingDestroyed() || !ObjectiveActor->HasPersistentObjectiveRuntimeState())
		{
			continue;
		}

		FHorrorCampaignObjectiveSaveState ObjectiveSaveState = ObjectiveActor->ExportObjectiveSaveState();
		if (ObjectiveSaveState.HasRuntimeState())
		{
			SaveState.ObjectiveRuntimeStates.Add(ObjectiveSaveState);
		}
	}
	return SaveState;
}

void AHorrorGameModeBase::ImportCampaignSaveState(const FHorrorCampaignSaveState& SaveState)
{
	CampaignProgress.ImportSaveState(SaveState);
	CampaignNavigationFocusObjectiveId = NAME_None;
	RefreshCampaignObjectiveActors();
	for (const FHorrorCampaignObjectiveSaveState& ObjectiveSaveState : SaveState.ObjectiveRuntimeStates)
	{
		if (ObjectiveSaveState.ChapterId != CampaignProgress.GetActiveChapterId())
		{
			continue;
		}

		for (AHorrorCampaignObjectiveActor* ObjectiveActor : RuntimeCampaignObjectiveActorViews)
		{
			if (ObjectiveActor
				&& !ObjectiveActor->IsActorBeingDestroyed()
				&& ObjectiveActor->GetChapterId() == ObjectiveSaveState.ChapterId
				&& ObjectiveActor->GetObjectiveId() == ObjectiveSaveState.ObjectiveId)
			{
				ObjectiveActor->ImportObjectiveSaveState(ObjectiveSaveState, ResolveLeadPlayerCharacter());
				break;
			}
		}
	}
	if (RuntimeCampaignBoss)
	{
		RuntimeCampaignBoss->SetBossDefeated(CampaignProgress.IsBossDefeated());
		RuntimeCampaignBoss->SetBossAwake(false);
	}
}

void AHorrorGameModeBase::SyncFoundFootageRuntimeStateToPlayer()
{
	AHorrorPlayerCharacter* PlayerCharacter = ResolveLeadPlayerCharacter();
	if (!PlayerCharacter)
	{
		return;
	}

	const bool bHasBodycam = HasBodycamAcquired();
	if (UVHSEffectComponent* VHSEffect = PlayerCharacter->GetVHSEffectComponent())
	{
		VHSEffect->SetBodycamAcquired(bHasBodycam);
		VHSEffect->SetBodycamEnabled(bHasBodycam);
	}

	if (UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent())
	{
		QuantumCamera->SetCameraAcquired(bHasBodycam);
		QuantumCamera->SetCameraEnabled(bHasBodycam);
	}
}

bool AHorrorGameModeBase::CanCompleteCampaignObjective(FName ChapterId, FName ObjectiveId) const
{
	if (ChapterId.IsNone() || ObjectiveId.IsNone() || CampaignProgress.GetActiveChapterId() != ChapterId)
	{
		return false;
	}

	return CampaignProgress.CanCompleteObjective(ObjectiveId);
}

bool AHorrorGameModeBase::CanExposeCampaignObjective(FName ChapterId, FName ObjectiveId) const
{
	return ShouldExposeCampaignObjectivesToHUD()
		&& CanCompleteCampaignObjective(ChapterId, ObjectiveId)
		&& IsWorldExposedCampaignObjective(ChapterId, ObjectiveId);
}

bool AHorrorGameModeBase::IsWorldExposedCampaignObjective(FName ChapterId, FName ObjectiveId) const
{
	if (ChapterId.IsNone() || ObjectiveId.IsNone() || CampaignProgress.GetActiveChapterId() != ChapterId)
	{
		return false;
	}

	const FHorrorCampaignChapterDefinition* Chapter = GetCurrentCampaignChapterDefinition();
	const FHorrorCampaignObjectiveDefinition* Objective = Chapter
		? FHorrorCampaign::FindObjectiveById(*Chapter, ObjectiveId)
		: nullptr;
	if (!Objective || Objective->bOptional || !Objective->bRequiredForChapterCompletion)
	{
		return false;
	}

	return CampaignProgress.CanCompleteObjective(ObjectiveId);
}

TArray<const FHorrorCampaignObjectiveDefinition*> AHorrorGameModeBase::GetAvailableCampaignNavigationFocusObjectives() const
{
	TArray<const FHorrorCampaignObjectiveDefinition*> NavigationObjectives;
	const FHorrorCampaignChapterDefinition* Chapter = GetCurrentCampaignChapterDefinition();
	if (!Chapter)
	{
		return NavigationObjectives;
	}

	for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter->Objectives)
	{
		if (!Objective.bOptional
			&& Objective.bRequiredForChapterCompletion
			&& CampaignProgress.CanCompleteObjective(Objective.ObjectiveId))
		{
			NavigationObjectives.Add(&Objective);
		}
	}

	return NavigationObjectives;
}

bool AHorrorGameModeBase::IsCampaignNavigationFocusObjectiveValid(FName ObjectiveId) const
{
	if (ObjectiveId.IsNone())
	{
		return false;
	}

	return GetAvailableCampaignNavigationFocusObjectives().ContainsByPredicate(
		[ObjectiveId](const FHorrorCampaignObjectiveDefinition* Objective)
		{
			return Objective && Objective->ObjectiveId == ObjectiveId;
		});
}

const FHorrorCampaignObjectiveDefinition* AHorrorGameModeBase::ResolveCampaignNavigationObjective() const
{
	if (IsCampaignNavigationFocusObjectiveValid(CampaignNavigationFocusObjectiveId))
	{
		const FHorrorCampaignChapterDefinition* Chapter = GetCurrentCampaignChapterDefinition();
		return Chapter ? FHorrorCampaign::FindObjectiveById(*Chapter, CampaignNavigationFocusObjectiveId) : nullptr;
	}

	return CampaignProgress.GetNextObjective();
}

void AHorrorGameModeBase::RefreshCampaignNavigationFocus()
{
	if (CampaignNavigationFocusObjectiveId.IsNone())
	{
		return;
	}

	if (!IsCampaignNavigationFocusObjectiveValid(CampaignNavigationFocusObjectiveId))
	{
		const FHorrorCampaignObjectiveDefinition* NextObjective = CampaignProgress.GetNextObjective();
		CampaignNavigationFocusObjectiveId = NextObjective
			&& !NextObjective->bOptional
			&& NextObjective->bRequiredForChapterCompletion
			&& CampaignProgress.CanCompleteObjective(NextObjective->ObjectiveId)
			? NextObjective->ObjectiveId
			: NAME_None;
	}
}

bool AHorrorGameModeBase::SetCampaignNavigationFocusObjective(FName ChapterId, FName ObjectiveId)
{
	if (ChapterId.IsNone()
		|| ObjectiveId.IsNone()
		|| CampaignProgress.GetActiveChapterId() != ChapterId
		|| !IsCampaignNavigationFocusObjectiveValid(ObjectiveId))
	{
		return false;
	}

	CampaignNavigationFocusObjectiveId = ObjectiveId;
	return true;
}

bool AHorrorGameModeBase::ClearCampaignNavigationFocusObjective()
{
	CampaignNavigationFocusObjectiveId = NAME_None;
	return true;
}

bool AHorrorGameModeBase::CycleCampaignNavigationFocus(int32 Direction)
{
	TArray<const FHorrorCampaignObjectiveDefinition*> NavigationObjectives = GetAvailableCampaignNavigationFocusObjectives();
	if (NavigationObjectives.IsEmpty())
	{
		CampaignNavigationFocusObjectiveId = NAME_None;
		return false;
	}

	const int32 StepDirection = Direction < 0 ? -1 : 1;
	int32 CurrentIndex = INDEX_NONE;
	if (!CampaignNavigationFocusObjectiveId.IsNone())
	{
		CurrentIndex = NavigationObjectives.IndexOfByPredicate(
			[this](const FHorrorCampaignObjectiveDefinition* Objective)
			{
				return Objective && Objective->ObjectiveId == CampaignNavigationFocusObjectiveId;
			});
	}

	const int32 ObjectiveCount = NavigationObjectives.Num();
	const int32 NextIndex = CurrentIndex == INDEX_NONE
		? (StepDirection > 0 ? 0 : ObjectiveCount - 1)
		: (CurrentIndex + StepDirection + ObjectiveCount) % ObjectiveCount;

	CampaignNavigationFocusObjectiveId = NavigationObjectives[NextIndex]->ObjectiveId;
	return true;
}

FText AHorrorGameModeBase::BuildCampaignObjectiveLockReasonText(FName ChapterId, FName ObjectiveId) const
{
	if (ChapterId.IsNone() || ObjectiveId.IsNone())
	{
		return NSLOCTEXT("HorrorGameMode", "CampaignObjectiveLockedFallback", "已锁定：请先完成当前目标");
	}

	if (!ShouldExposeCampaignObjectivesToHUD())
	{
		return NSLOCTEXT("HorrorGameMode", "CampaignObjectiveLockedStoryGate", "已锁定：先完成当前录像流程。");
	}

	if (CampaignProgress.GetActiveChapterId() != ChapterId)
	{
		return NSLOCTEXT("HorrorGameMode", "CampaignObjectiveLockedWrongChapter", "已锁定：当前章节尚未抵达此处。");
	}

	const FHorrorCampaignChapterDefinition* Chapter = GetCurrentCampaignChapterDefinition();
	const FHorrorCampaignObjectiveDefinition* Objective = Chapter
		? FHorrorCampaign::FindObjectiveById(*Chapter, ObjectiveId)
		: nullptr;
	if (!Chapter || !Objective)
	{
		return NSLOCTEXT("HorrorGameMode", "CampaignObjectiveLockedUnknown", "已锁定：目标信号尚未稳定。");
	}

	if (CampaignProgress.HasCompletedObjective(ObjectiveId))
	{
		return NSLOCTEXT("HorrorGameMode", "CampaignObjectiveLockedCompleted", "已完成");
	}

	TArray<FText> MissingPrerequisiteTexts;
	for (const FName PrerequisiteObjectiveId : Objective->PrerequisiteObjectiveIds)
	{
		if (PrerequisiteObjectiveId.IsNone() || CampaignProgress.HasCompletedObjective(PrerequisiteObjectiveId))
		{
			continue;
		}

		const FHorrorCampaignObjectiveDefinition* PrerequisiteObjective =
			FHorrorCampaign::FindObjectiveById(*Chapter, PrerequisiteObjectiveId);
		if (PrerequisiteObjective && !PrerequisiteObjective->PromptText.IsEmpty())
		{
			MissingPrerequisiteTexts.Add(PrerequisiteObjective->PromptText);
		}
		else
		{
			MissingPrerequisiteTexts.Add(FText::FromName(PrerequisiteObjectiveId));
		}
	}

	if (!MissingPrerequisiteTexts.IsEmpty())
	{
		return FText::Format(
			NSLOCTEXT("HorrorGameMode", "CampaignObjectiveLockedPrerequisites", "已锁定：先完成{0}"),
			JoinCampaignObjectiveTextList(MissingPrerequisiteTexts));
	}

	const FText CurrentObjectiveText = GetCurrentCampaignObjectivePromptText();
	return CurrentObjectiveText.IsEmpty()
		? NSLOCTEXT("HorrorGameMode", "CampaignObjectiveLockedCompleteCurrent", "已锁定：请先完成当前目标")
		: FText::Format(
			NSLOCTEXT("HorrorGameMode", "CampaignObjectiveLockedCompleteNamedCurrent", "已锁定：先完成「{0}」"),
			CurrentObjectiveText);
}

bool AHorrorGameModeBase::TryCompleteCampaignObjective(FName ChapterId, FName ObjectiveId, AActor* InstigatorActor)
{
	if (!CanCompleteCampaignObjective(ChapterId, ObjectiveId))
	{
		ShowCampaignMessage(BuildCampaignObjectiveLockReasonText(ChapterId, ObjectiveId), FLinearColor(1.0f, 0.72f, 0.2f), 2.0f);
		return false;
	}

	const FHorrorCampaignChapterDefinition* Chapter = GetCurrentCampaignChapterDefinition();
	const FHorrorCampaignObjectiveDefinition* Objective = Chapter
		? FHorrorCampaign::FindObjectiveById(*Chapter, ObjectiveId)
		: nullptr;
	if (!Objective || !CampaignProgress.TryCompleteObjective(ObjectiveId))
	{
		return false;
	}
	RefreshCampaignNavigationFocus();

	ApplyCampaignObjectiveReward(*Objective, InstigatorActor);

	ShowCampaignMessage(
		Objective->CompletionText.IsEmpty() ? NSLOCTEXT("HorrorGameMode", "ObjectiveComplete", "目标已完成。") : Objective->CompletionText,
		FLinearColor(0.48f, 1.0f, 0.62f),
		2.25f);

	if (UWorld* World = GetWorld())
	{
		if (UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>())
		{
			const FHorrorCampaignObjectiveDefinition* NextObjectiveAfterCompletion = CampaignProgress.GetNextObjective();
			FHorrorObjectiveMessageMetadata Metadata;
			Metadata.TrailerBeatId = Objective->ObjectiveId;
			Metadata.DebugLabel = Objective->CompletionText.IsEmpty()
				? NSLOCTEXT("HorrorGameMode", "CampaignObjectiveToastComplete", "目标完成")
				: FText::Format(
					NSLOCTEXT("HorrorGameMode", "CampaignObjectiveToastCompleteNamed", "目标完成：{0}"),
					Objective->CompletionText);
			Metadata.FeedbackSeverity = EHorrorObjectiveFeedbackSeverity::Success;
			Metadata.bRetryable = false;
			Metadata.DisplaySeconds = CampaignProgress.IsChapterComplete() ? 6.0f : 4.75f;
			if (CampaignProgress.IsChapterComplete())
			{
				Metadata.ObjectiveHint = Chapter && Chapter->bIsFinalChapter
					? NSLOCTEXT("HorrorGameMode", "CampaignObjectiveFinalHint", "终章已完成，前往结局出口。")
					: NSLOCTEXT("HorrorGameMode", "CampaignObjectiveChapterExitHint", "章节目标已完成，出口已开启。");
			}
			else if (NextObjectiveAfterCompletion)
			{
				Metadata.ObjectiveHint = NextObjectiveAfterCompletion->PromptText.IsEmpty()
					? NSLOCTEXT("HorrorGameMode", "CampaignObjectiveNextHintFallback", "新的目标已解锁。")
					: FText::Format(
						NSLOCTEXT("HorrorGameMode", "CampaignObjectiveNextHint", "下一目标：{0}"),
						NextObjectiveAfterCompletion->PromptText);
			}
			else
			{
				Metadata.ObjectiveHint = NSLOCTEXT("HorrorGameMode", "CampaignObjectiveContinueHint", "继续调查周围区域，寻找下一条线索。");
			}
			if (Objective->Reward.HasAnyReward())
			{
				const FText RewardHint = Objective->Reward.RewardText.IsEmpty()
					? NSLOCTEXT("HorrorGameMode", "CampaignObjectiveRewardFallback", "奖励：任务奖励已发放")
					: FText::Format(
						NSLOCTEXT("HorrorGameMode", "CampaignObjectiveRewardNamed", "奖励：{0}"),
						Objective->Reward.RewardText);
				Metadata.ObjectiveHint = Metadata.ObjectiveHint.IsEmpty()
					? RewardHint
					: FText::Format(
						NSLOCTEXT("HorrorGameMode", "CampaignObjectiveHintWithReward", "{0}\n{1}"),
						Metadata.ObjectiveHint,
						RewardHint);
			}
			EventBus->RegisterObjectiveMetadata(
				HorrorCampaignAudioEvents::ObjectiveCompleted(),
				Objective->ObjectiveId,
				Metadata);
			EventBus->Publish(
				HorrorCampaignAudioEvents::ObjectiveCompleted(),
				Objective->ObjectiveId,
				HorrorCampaignAudioEvents::ObjectiveCompleted(),
				InstigatorActor ? InstigatorActor : this);
			EventBus->UnregisterObjectiveMetadata(
				HorrorCampaignAudioEvents::ObjectiveCompleted(),
				Objective->ObjectiveId);
		}
	}

	if (RuntimeCampaignBoss && Chapter && Chapter->bRequiresBoss && Objective->ObjectiveType == EHorrorCampaignObjectiveType::RestorePower)
	{
		RuntimeCampaignBoss->SetBossAwake(true);
	}

	if (RuntimeCampaignBoss && Objective->ObjectiveType == EHorrorCampaignObjectiveType::BossWeakPoint)
	{
		RuntimeCampaignBoss->RegisterWeakPointResolved();
		if (UWorld* World = GetWorld())
		{
			if (UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>())
			{
				EventBus->Publish(
					HorrorCampaignAudioEvents::BossWeakPoint(),
					Objective->ObjectiveId,
					HorrorCampaignAudioEvents::BossWeakPoint(),
					InstigatorActor ? InstigatorActor : RuntimeCampaignBoss.Get());
			}
		}
	}
	else if (RuntimeCampaignBoss && CampaignProgress.IsBossDefeated())
	{
		RuntimeCampaignBoss->SetBossDefeated(true);
	}

	RefreshCampaignObjectiveActors();

	if (CampaignProgress.IsChapterComplete())
	{
		ShowCampaignMessage(
			Chapter && Chapter->bIsFinalChapter
				? NSLOCTEXT("HorrorGameMode", "FinalChapterComplete", "终章完成，黑盒出口已开启。")
				: NSLOCTEXT("HorrorGameMode", "ChapterComplete", "章节完成，出口已开启。"),
			FLinearColor(0.45f, 0.9f, 1.0f),
			4.0f);
		if (UWorld* World = GetWorld())
		{
			if (UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>())
			{
				EventBus->Publish(
					HorrorCampaignAudioEvents::ChapterCompleted(),
					Chapter ? Chapter->ChapterId : ChapterId,
					HorrorCampaignAudioEvents::ChapterCompleted(),
					InstigatorActor ? InstigatorActor : this);
			}
		}
		QueueCampaignAutoTravelIfNeeded();
	}
	else
	{
		ShowCurrentCampaignObjectiveHint();
	}

	(void)InstigatorActor;
	return true;
}

bool AHorrorGameModeBase::IsCurrentCampaignChapterComplete() const
{
	if (!CampaignProgress.HasActiveChapter())
	{
		return false;
	}

	return CampaignProgress.IsChapterComplete();
}

const FHorrorCampaignChapterDefinition* AHorrorGameModeBase::GetCurrentCampaignChapterDefinition() const
{
	return FHorrorCampaign::FindChapterById(CampaignProgress.GetActiveChapterId());
}

bool AHorrorGameModeBase::ShouldExposeCampaignObjectivesToHUD() const
{
	return true;
}

bool AHorrorGameModeBase::IsDay1FoundFootageFlowActive() const
{
	if (IsDay1Complete())
	{
		return false;
	}

	if (CampaignProgress.HasActiveChapter())
	{
		return false;
	}

	const FString CurrentMapPackageName = GetCurrentNormalizedMapPackageName();
	if (CurrentMapPackageName.IsEmpty())
	{
		return false;
	}

	const bool bIsDay1Map = CurrentMapPackageName.Contains(TEXT("/DeepWaterStation/"))
		|| CurrentMapPackageName.Contains(TEXT("DemoMap_VerticalSlice_Day1"));
	if (!bIsDay1Map)
	{
		return false;
	}

	return true;
}

bool AHorrorGameModeBase::TryGetCurrentCampaignObjectiveWorldLocation(FVector& OutLocation) const
{
	OutLocation = FVector::ZeroVector;

	if (!ShouldExposeCampaignObjectivesToHUD())
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* CurrentObjective = CampaignProgress.GetNextObjective();
	if (!CurrentObjective)
	{
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* NavigationObjective = ResolveCampaignNavigationObjective();
	if (!NavigationObjective)
	{
		return false;
	}

	if (const AHorrorCampaignObjectiveActor* ObjectiveActor = FindRuntimeCampaignObjectiveActor(CampaignProgress.GetActiveChapterId(), NavigationObjective->ObjectiveId))
	{
		const FHorrorCampaignObjectiveRuntimeState RuntimeState = ObjectiveActor->BuildObjectiveRuntimeState();
		OutLocation = RuntimeState.bTimedObjectiveActive && RuntimeState.bUsesEscapeDestination
			? RuntimeState.EscapeDestinationWorldLocation
			: ObjectiveActor->GetActorLocation();
		return true;
	}

	OutLocation = ResolveCampaignObjectiveTransform(*NavigationObjective).GetLocation();
	return true;
}

bool AHorrorGameModeBase::BuildCurrentCampaignObjectiveNavigationState(const APawn* ViewerPawn, FHorrorObjectiveNavigationState& OutState) const
{
	OutState = FHorrorObjectiveNavigationState();
	if (!ViewerPawn)
	{
		UE_LOG(LogHorrorProject, Warning, TEXT("BuildCurrentCampaignObjectiveNavigationState: ViewerPawn is null"));
		return false;
	}

	if (!ShouldExposeCampaignObjectivesToHUD())
	{
		UE_LOG(LogHorrorProject, Warning, TEXT("BuildCurrentCampaignObjectiveNavigationState: ShouldExposeCampaignObjectivesToHUD returned false"));
		return false;
	}

	const FHorrorCampaignObjectiveDefinition* NavigationObjective = ResolveCampaignNavigationObjective();
	if (!NavigationObjective)
	{
		UE_LOG(LogHorrorProject, Warning, TEXT("BuildCurrentCampaignObjectiveNavigationState: No navigation objective found. ActiveChapter=%s, NextObjective=%s"),
			CampaignProgress.GetActiveChapterId().IsNone() ? TEXT("None") : *CampaignProgress.GetActiveChapterId().ToString(),
			TEXT("null"));
		return false;
	}
	const bool bManualNavigationFocus = !CampaignNavigationFocusObjectiveId.IsNone()
		&& CampaignNavigationFocusObjectiveId == NavigationObjective->ObjectiveId
		&& IsCampaignNavigationFocusObjectiveValid(CampaignNavigationFocusObjectiveId);
	const AHorrorCampaignObjectiveActor* CurrentObjectiveActor =
		FindRuntimeCampaignObjectiveActor(CampaignProgress.GetActiveChapterId(), NavigationObjective->ObjectiveId);

	FHorrorCampaignObjectiveRuntimeState RuntimeState;
	if (CurrentObjectiveActor)
	{
		RuntimeState = CurrentObjectiveActor->BuildObjectiveRuntimeState();
	}

	FVector ObjectiveLocation = FVector::ZeroVector;
	bool bUsingAuthoredFallbackLocation = false;
	if (RuntimeState.bTimedObjectiveActive && RuntimeState.bUsesEscapeDestination)
	{
		ObjectiveLocation = RuntimeState.EscapeDestinationWorldLocation;
		UE_LOG(LogHorrorProject, Log, TEXT("BuildCurrentCampaignObjectiveNavigationState: Using escape destination location: %s"), *ObjectiveLocation.ToString());
	}
	else if (RuntimeState.bRecoverRelicAwaitingDelivery)
	{
		ObjectiveLocation = RuntimeState.RelicDeliveryWorldLocation;
		UE_LOG(LogHorrorProject, Log, TEXT("BuildCurrentCampaignObjectiveNavigationState: Using relic delivery location: %s"), *ObjectiveLocation.ToString());
	}
	else if (CurrentObjectiveActor)
	{
		ObjectiveLocation = CurrentObjectiveActor->GetActorLocation();
		UE_LOG(LogHorrorProject, Log, TEXT("BuildCurrentCampaignObjectiveNavigationState: Using objective actor location: %s"), *ObjectiveLocation.ToString());
	}
	else if (!TryGetCurrentCampaignObjectiveWorldLocation(ObjectiveLocation))
	{
		UE_LOG(LogHorrorProject, Warning, TEXT("BuildCurrentCampaignObjectiveNavigationState: Failed to get objective world location"));
		return false;
	}
	else
	{
		bUsingAuthoredFallbackLocation = CurrentObjectiveActor == nullptr;
		UE_LOG(LogHorrorProject, Log, TEXT("BuildCurrentCampaignObjectiveNavigationState: Using authored fallback location: %s"), *ObjectiveLocation.ToString());
	}

	const FVector ToObjective = ObjectiveLocation - ViewerPawn->GetActorLocation();
	const FVector HorizontalToObjective(ToObjective.X, ToObjective.Y, 0.0f);
	const float DistanceMeters = FMath::Max(0.0f, HorizontalToObjective.Size() / 100.0f);
	const float ArrivalRadiusCm = RuntimeState.bTimedObjectiveActive && RuntimeState.bUsesEscapeDestination
		? FMath::Max(125.0f, RuntimeState.EscapeCompletionRadius)
		: RuntimeState.bRecoverRelicAwaitingDelivery
		? FMath::Max(125.0f, RuntimeState.RelicDeliveryCompletionRadius)
		: 125.0f;
	const bool bArrived = HorizontalToObjective.SizeSquared() <= FMath::Square(ArrivalRadiusCm);
	FText ParallelObjectiveLabel;
	float ParallelObjectiveDistanceMeters = 0.0f;
	int32 OtherAvailableMainlineObjectiveCount = 0;
	for (const FHorrorCampaignObjectiveDefinition* AvailableObjective : CampaignProgress.GetAvailableObjectives())
	{
		if (!AvailableObjective
			|| AvailableObjective->ObjectiveId == NavigationObjective->ObjectiveId
			|| AvailableObjective->bOptional
			|| !AvailableObjective->bRequiredForChapterCompletion)
		{
			continue;
		}

		++OtherAvailableMainlineObjectiveCount;
		FVector AvailableLocation = ResolveCampaignObjectiveTransform(*AvailableObjective).GetLocation();
		if (const AHorrorCampaignObjectiveActor* AvailableActor = FindRuntimeCampaignObjectiveActor(CampaignProgress.GetActiveChapterId(), AvailableObjective->ObjectiveId))
		{
			AvailableLocation = AvailableActor->GetActorLocation();
		}

		const float CandidateDistanceMeters = FVector::Dist2D(ViewerPawn->GetActorLocation(), AvailableLocation) / 100.0f;
		if (ParallelObjectiveLabel.IsEmpty() || CandidateDistanceMeters < ParallelObjectiveDistanceMeters)
		{
			ParallelObjectiveLabel = AvailableObjective->PromptText.IsEmpty()
				? FText::FromName(AvailableObjective->ObjectiveId)
				: AvailableObjective->PromptText;
			ParallelObjectiveDistanceMeters = FMath::Max(0.0f, CandidateDistanceMeters);
		}
	}

	const AController* ViewerController = ViewerPawn->GetController();
	const FRotator ControlRotation = ViewerController ? ViewerController->GetControlRotation() : ViewerPawn->GetActorRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Direction = HorizontalToObjective.GetSafeNormal();
	const float SignedAngleDegrees = HorizontalToObjective.IsNearlyZero()
		? 0.0f
		: FMath::RadiansToDegrees(FMath::Atan2(
			FVector::CrossProduct(Forward, Direction).Z,
			FVector::DotProduct(Forward, Direction)));
	const FText DirectionText = ResolveCampaignNavigationDirectionText(*ViewerPawn, ToObjective, bArrived);
	const FText Label = !RuntimeState.Title.IsEmpty()
		? RuntimeState.Title
		: NavigationObjective && !NavigationObjective->PromptText.IsEmpty()
		? NavigationObjective->PromptText
		: GetCurrentCampaignObjectiveActionText();

	OutState.bVisible = true;
	OutState.Label = Label.IsEmpty()
		? NSLOCTEXT("HorrorGameMode", "CampaignNavigationObjectiveFallback", "当前目标")
		: Label;
	OutState.DirectionText = DirectionText;
	OutState.WorldLocation = ObjectiveLocation;
	OutState.DistanceMeters = DistanceMeters;
	OutState.AngleDegrees = SignedAngleDegrees;
	OutState.bArrived = bArrived;

	UE_LOG(LogHorrorProject, Log, TEXT("BuildCurrentCampaignObjectiveNavigationState: SUCCESS - Label=%s, Direction=%s, Distance=%.1fm, Angle=%.1f°, Arrived=%d"),
		*OutState.Label.ToString(),
		*OutState.DirectionText.ToString(),
		OutState.DistanceMeters,
		OutState.AngleDegrees,
		OutState.bArrived ? 1 : 0);
	OutState.bReachable = bArrived;
	if (!bArrived)
	{
		const UWorld* CachedWorld = GetWorld();
		const double CurrentWorldTime = CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0;
		constexpr float NavReachCacheInvalidationDistSq = 150.0f * 150.0f;
		constexpr double NavReachCacheLifetimeSeconds = 1.0;
		const bool bCacheValid = CachedNavReachabilityWorldTime >= 0.0
			&& (CurrentWorldTime - CachedNavReachabilityWorldTime) < NavReachCacheLifetimeSeconds
			&& FVector::DistSquared(ObjectiveLocation, CachedNavReachabilityTarget) < NavReachCacheInvalidationDistSq;
		if (bCacheValid)
		{
			OutState.bReachable = bCachedNavReachable;
		}
		else
		{
			OutState.bReachable = IsCampaignNavigationTargetReachable(*ViewerPawn, ObjectiveLocation, CurrentObjectiveActor);
			CachedNavReachabilityTarget = ObjectiveLocation;
			bCachedNavReachable = OutState.bReachable;
			CachedNavReachabilityWorldTime = CurrentWorldTime;
		}
	}
	OutState.RuntimeStatus = RuntimeState.Status;
	OutState.bRetryable = RuntimeState.bRetryable;
	OutState.FailureCause = RuntimeState.FailureCause;
	OutState.RecoveryAction = RuntimeState.RecoveryAction;
	OutState.EscapeTimeBudgetSeconds = RuntimeState.EscapeTimeBudgetSeconds;
	OutState.EstimatedEscapeArrivalSeconds = RuntimeState.EstimatedEscapeArrivalSeconds;
	OutState.FocusedObjectiveId = NavigationObjective->ObjectiveId;
	if (!RuntimeState.DeviceStatusLabel.IsEmpty())
	{
		OutState.DeviceStatusLabel = RuntimeState.DeviceStatusLabel;
	}
	if (bManualNavigationFocus)
	{
		OutState.DeviceStatusLabel = OutState.DeviceStatusLabel.IsEmpty()
			? NSLOCTEXT("HorrorGameMode", "CampaignNavigationManualFocusDiagnostics", "手动导航：任务图焦点已锁定")
			: FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationManualFocusDiagnosticsWithStatus", "手动导航：任务图焦点已锁定；{0}"),
				OutState.DeviceStatusLabel);
	}
	if (bUsingAuthoredFallbackLocation)
	{
		OutState.DeviceStatusLabel = OutState.DeviceStatusLabel.IsEmpty()
			? NSLOCTEXT("HorrorGameMode", "CampaignNavigationAuthoredFallbackDiagnostics", "目标信标未生成：正在使用预计位置")
			: FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationAuthoredFallbackDiagnosticsWithStatus", "{0}；目标信标未生成：正在使用预计位置"),
				OutState.DeviceStatusLabel);
	}
	if (!RuntimeState.NextActionLabel.IsEmpty())
	{
		OutState.NextActionLabel = RuntimeState.NextActionLabel;
	}
	if (!RuntimeState.FailureRecoveryLabel.IsEmpty())
	{
		OutState.FailureRecoveryLabel = RuntimeState.FailureRecoveryLabel;
	}
	OutState.PerformanceGradeFraction = RuntimeState.ProgressFraction > 0.0f
		? FMath::Clamp(RuntimeState.ProgressFraction, 0.0f, 1.0f)
		: OutState.PerformanceGradeFraction;
	if (RuntimeState.AdvancedInteraction.bVisible
		|| RuntimeState.bAdvancedInteractionActive
		|| RuntimeState.Status == EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable)
	{
		OutState.DeviceStatusLabel = RuntimeState.AdvancedInteraction.DeviceStatusLabel;
		OutState.NextActionLabel = RuntimeState.AdvancedInteraction.NextActionLabel;
		OutState.FailureRecoveryLabel = RuntimeState.AdvancedInteraction.FailureRecoveryLabel;
		OutState.PerformanceGradeFraction = FMath::Clamp(RuntimeState.AdvancedInteraction.PerformanceGradeFraction, 0.0f, 1.0f);
	}

	if (RuntimeState.bTimedObjectiveActive && RuntimeState.bUsesEscapeDestination)
	{
		OutState.DeviceStatusLabel = RuntimeState.EscapeBudgetLabel;
		OutState.NextActionLabel = RuntimeState.EscapeActionLabel;
		OutState.PerformanceGradeFraction = RuntimeState.TimedRemainingSeconds > 0.0f
			? FMath::Clamp(RuntimeState.EscapeTimeBudgetSeconds / FMath::Max(RuntimeState.TimedRemainingSeconds, UE_SMALL_NUMBER), 0.0f, 1.0f)
			: 0.0f;
		OutState.StatusText = bArrived
			? FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationEscapeArrivedStatus", "{0}：已抵达逃离点，坚持 {1} 秒"),
				OutState.Label,
				FText::AsNumber(FMath::CeilToInt(FMath::Max(0.0f, RuntimeState.TimedRemainingSeconds))))
			: (!RuntimeState.EscapeActionLabel.IsEmpty()
			? FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationEscapeBudgetStatus", "{0}：逃离点在{1} {2} 米，剩余 {3} 秒；{4}"),
				OutState.Label,
				DirectionText,
				FText::AsNumber(FMath::Max(0, FMath::RoundToInt(DistanceMeters))),
				FText::AsNumber(FMath::CeilToInt(FMath::Max(0.0f, RuntimeState.TimedRemainingSeconds))),
				RuntimeState.EscapeActionLabel)
			: FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationEscapeStatus", "{0}：逃离点在{1} {2} 米，剩余 {3} 秒"),
				OutState.Label,
				DirectionText,
				FText::AsNumber(FMath::Max(0, FMath::RoundToInt(DistanceMeters))),
				FText::AsNumber(FMath::CeilToInt(FMath::Max(0.0f, RuntimeState.TimedRemainingSeconds)))));
	}
	else if (RuntimeState.bAdvancedInteractionActive)
	{
		FText AdvancedInstruction = RuntimeState.AdvancedInteraction.NextActionLabel.IsEmpty()
			? (RuntimeState.PhaseText.IsEmpty()
			? NSLOCTEXT("HorrorGameMode", "CampaignNavigationAdvancedFallback", "操作窗口已展开")
			: RuntimeState.PhaseText)
			: RuntimeState.AdvancedInteraction.NextActionLabel;
		if (!RuntimeState.AdvancedInteraction.DeviceStatusLabel.IsEmpty())
		{
			AdvancedInstruction = FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationAdvancedDiagnostics", "{0}，{1}"),
				RuntimeState.AdvancedInteraction.DeviceStatusLabel,
				AdvancedInstruction);
		}
		if (!RuntimeState.AdvancedInteraction.FailureRecoveryLabel.IsEmpty()
			&& (RuntimeState.AdvancedInteraction.bPaused || RuntimeState.AdvancedInteraction.bRecentFailure))
		{
			AdvancedInstruction = FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationAdvancedRecovery", "{0}；{1}"),
				AdvancedInstruction,
				RuntimeState.AdvancedInteraction.FailureRecoveryLabel);
		}
		if (RuntimeState.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
		{
			AdvancedInstruction = FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationSpectralScanAdvancedStatus", "{0}，按 A/D 主动扫频，按 S 锁定异常峰"),
				AdvancedInstruction);
		}
		else if (RuntimeState.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
		{
			AdvancedInstruction = FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationSignalTuningAdvancedStatus", "{0}，按 A/D 微调声道，按 S 锁定中心频率"),
				AdvancedInstruction);
		}
		OutState.StatusText = FText::Format(
			NSLOCTEXT("HorrorGameMode", "CampaignNavigationAdvancedStatus", "{0}：{1}"),
			OutState.Label,
			AdvancedInstruction);
	}
	else if (RuntimeState.bRecoverRelicAwaitingDelivery)
	{
		OutState.DeviceStatusLabel = RuntimeState.DeviceStatusLabel;
		OutState.NextActionLabel = RuntimeState.NextActionLabel;
		OutState.PerformanceGradeFraction = FMath::Clamp(RuntimeState.ProgressFraction, 0.0f, 1.0f);
		OutState.StatusText = bArrived
			? FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationRelicDeliveryArrivedStatus", "{0}：已抵达锚点，互动归档遗物"),
				OutState.Label)
			: FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationRelicDeliveryStatus", "{0}：锚点在{1} {2} 米；{3}"),
				OutState.Label,
				DirectionText,
				FText::AsNumber(FMath::Max(0, FMath::RoundToInt(DistanceMeters))),
				OutState.NextActionLabel.IsEmpty()
					? NSLOCTEXT("HorrorGameMode", "CampaignNavigationRelicDeliveryFallbackAction", "送回锚点完成归档")
					: OutState.NextActionLabel);
	}
	else if (RuntimeState.Status == EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable)
	{
		const bool bPursuitFailure = RuntimeState.InteractionMode == EHorrorCampaignInteractionMode::TimedPursuit
			|| RuntimeState.FailureCause == FName(TEXT("Failure.Campaign.PursuitTimeout"))
			|| RuntimeState.RecoveryAction == FName(TEXT("Recovery.Campaign.ReturnToEscapeStart"));
		if (bPursuitFailure)
		{
			OutState.DeviceStatusLabel = NSLOCTEXT("HorrorGameMode", "CampaignNavigationPursuitRetryDiagnostics", "追逐失败：超时或被石像巨人追上");
			OutState.NextActionLabel = NSLOCTEXT("HorrorGameMode", "CampaignNavigationPursuitRetryAction", "回到追逐起点，重新互动后沿亮点路线冲刺");
			OutState.FailureRecoveryLabel = NSLOCTEXT("HorrorGameMode", "CampaignNavigationPursuitRetryRecovery", "系统已撤回安全点，下一次追逐会重新计时");
			OutState.PerformanceGradeFraction = 0.0f;
			OutState.StatusText = FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationPursuitRetryStatus", "{0}：{1}；{2}"),
				OutState.Label,
				OutState.DeviceStatusLabel,
				OutState.NextActionLabel);
		}
		else
		{
			FText RetryInstruction = RuntimeState.AdvancedInteraction.NextActionLabel.IsEmpty()
				? NSLOCTEXT("HorrorGameMode", "CampaignNavigationRetryFallback", "设备过载，可重试互动")
				: RuntimeState.AdvancedInteraction.NextActionLabel;
			if (!RuntimeState.AdvancedInteraction.DeviceStatusLabel.IsEmpty())
			{
				RetryInstruction = FText::Format(
					NSLOCTEXT("HorrorGameMode", "CampaignNavigationRetryDiagnostics", "{0}，{1}"),
					RuntimeState.AdvancedInteraction.DeviceStatusLabel,
					RetryInstruction);
			}
			if (!RuntimeState.AdvancedInteraction.FailureRecoveryLabel.IsEmpty())
			{
				RetryInstruction = FText::Format(
					NSLOCTEXT("HorrorGameMode", "CampaignNavigationRetryRecovery", "{0}；{1}"),
					RetryInstruction,
					RuntimeState.AdvancedInteraction.FailureRecoveryLabel);
			}
			OutState.StatusText = FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationRetryStatus", "{0}：{1}"),
				OutState.Label,
				RetryInstruction);
		}
	}
	else if (RuntimeState.bVisible && !RuntimeState.bCanInteract && !RuntimeState.bCompleted)
	{
		OutState.StatusText = RuntimeState.BlockedReason.IsEmpty()
			? FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationBlockedStatus", "{0}：当前目标暂未解锁"),
				OutState.Label)
			: RuntimeState.BlockedReason;
	}
	else
	{
		FText StandardStatus = bArrived
			? FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationArrivedStatus", "{0}：已到达目标附近"),
				OutState.Label)
			: FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationStatus", "{0}：{1} {2} 米"),
				OutState.Label,
				DirectionText,
				FText::AsNumber(FMath::Max(0, FMath::RoundToInt(DistanceMeters))));
		if (!OutState.NextActionLabel.IsEmpty())
		{
			StandardStatus = FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationStandardActionStatus", "{0}；{1}"),
				StandardStatus,
				OutState.NextActionLabel);
		}
		OutState.StatusText = StandardStatus;
	}

	if (!OutState.bReachable && !bArrived)
	{
		if (OtherAvailableMainlineObjectiveCount > 0 && !ParallelObjectiveLabel.IsEmpty())
		{
			OutState.NextActionLabel = FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationParallelMainlineAction", "当前路线受阻；沿墙体边缘寻找入口，也可先切到附近并行主线「{0}」（约 {1} 米）"),
				ParallelObjectiveLabel,
				FText::AsNumber(FMath::Max(0, FMath::RoundToInt(ParallelObjectiveDistanceMeters))));
			OutState.FailureRecoveryLabel = NSLOCTEXT("HorrorGameMode", "CampaignNavigationParallelMainlineRecovery", "完成任一可见主线目标后，任务链会自动刷新导航。");
			OutState.StatusText = FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationBlockedRouteParallelMainlineStatus", "{0}：路线受阻，请先绕路寻找入口；另有 {1} 个并行主线目标可推进"),
				OutState.Label,
				FText::AsNumber(OtherAvailableMainlineObjectiveCount));
		}
		else
		{
			const FText BlockedRouteAction = NSLOCTEXT("HorrorGameMode", "CampaignNavigationBlockedRouteAction", "沿墙体边缘寻找入口，必要时返回上一个安全点重进目标区");
			const FText BlockedRouteRecovery = NSLOCTEXT("HorrorGameMode", "CampaignNavigationBlockedRouteRecovery", "如果导航持续受阻，回到上一个任务点触发目标刷新。");
			OutState.NextActionLabel = OutState.NextActionLabel.IsEmpty()
				? BlockedRouteAction
				: FText::Format(
					NSLOCTEXT("HorrorGameMode", "CampaignNavigationBlockedRouteActionWithObjectiveCue", "{0}；原目标提示：{1}"),
					BlockedRouteAction,
					OutState.NextActionLabel);
			OutState.FailureRecoveryLabel = OutState.FailureRecoveryLabel.IsEmpty()
				? BlockedRouteRecovery
				: FText::Format(
					NSLOCTEXT("HorrorGameMode", "CampaignNavigationBlockedRouteRecoveryWithObjectiveCue", "{0}；目标恢复提示：{1}"),
					BlockedRouteRecovery,
					OutState.FailureRecoveryLabel);
			OutState.StatusText = FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationBlockedRouteStatus", "{0}：路线受阻，请沿场景绕路或寻找新的入口"),
				OutState.Label);
		}
	}

	if (OtherAvailableMainlineObjectiveCount > 0)
	{
		const FText CycleFocusHint = NSLOCTEXT("HorrorGameMode", "CampaignNavigationCycleFocusHint", "按 [ / ] 切换任务图导航焦点");
		if (OutState.NextActionLabel.IsEmpty())
		{
			OutState.NextActionLabel = CycleFocusHint;
		}
		else if (!OutState.NextActionLabel.ToString().Contains(TEXT("["))
			&& !OutState.NextActionLabel.ToString().Contains(TEXT("]")))
		{
			OutState.NextActionLabel = FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationNextActionWithCycleFocusHint", "{0}；{1}"),
				OutState.NextActionLabel,
				CycleFocusHint);
		}
	}

	if (bManualNavigationFocus && !OutState.DeviceStatusLabel.ToString().Contains(TEXT("手动导航")))
	{
		OutState.DeviceStatusLabel = OutState.DeviceStatusLabel.IsEmpty()
			? NSLOCTEXT("HorrorGameMode", "CampaignNavigationManualFocusFinalDiagnostics", "手动导航：任务图焦点已锁定")
			: FText::Format(
				NSLOCTEXT("HorrorGameMode", "CampaignNavigationManualFocusFinalDiagnosticsWithStatus", "手动导航：任务图焦点已锁定；{0}"),
				OutState.DeviceStatusLabel);
	}

	return true;
}

bool AHorrorGameModeBase::ShouldUseCampaignFailureRecovery(FName FailureCause) const
{
	if (!CampaignProgress.HasActiveChapter())
	{
		return false;
	}

	const FHorrorCampaignChapterDefinition* Chapter = GetCurrentCampaignChapterDefinition();
	if (!Chapter)
	{
		return false;
	}

	return Chapter->bRequiresBoss
		|| FailureCause == HorrorPlayerFailureDefaults::BossAttackFailureCause
		|| FailureCause.ToString().StartsWith(TEXT("Failure.Boss."))
		|| FailureCause.ToString().StartsWith(TEXT("Failure.Campaign."));
}

FTransform AHorrorGameModeBase::ResolveCampaignFailureRecoveryTransform() const
{
	const FTransform AnchorTransform = ResolveCampaignRuntimeAnchorTransform();
	const FVector AnchorLocation = AnchorTransform.GetLocation();
	auto BuildRecoveryTransformNearObjective = [AnchorLocation](const FVector& ObjectiveLocation)
	{
		FVector AwayFromObjective = AnchorLocation - ObjectiveLocation;
		AwayFromObjective.Z = 0.0f;
		if (AwayFromObjective.IsNearlyZero())
		{
			AwayFromObjective = FVector(-1.0f, 0.0f, 0.0f);
		}
		AwayFromObjective.Normalize();
		const FVector SideStep(-AwayFromObjective.Y, AwayFromObjective.X, 0.0f);
		const FVector RecoveryLocation = ObjectiveLocation
			+ AwayFromObjective * CampaignFailureRecoveryBackstepCm
			+ SideStep * CampaignFailureRecoverySideStepCm
			+ FVector(0.0f, 0.0f, CampaignFailureRecoveryLiftCm);
		const FRotator RecoveryRotation = (ObjectiveLocation - RecoveryLocation).Rotation();
		return FTransform(FRotator(0.0f, RecoveryRotation.Yaw, 0.0f), RecoveryLocation, FVector::OneVector);
	};

	FVector ObjectiveLocation = FVector::ZeroVector;
	if (TryGetCurrentCampaignObjectiveWorldLocation(ObjectiveLocation))
	{
		return BuildRecoveryTransformNearObjective(ObjectiveLocation);
	}

	const FHorrorCampaignObjectiveDefinition* CurrentObjective = CampaignProgress.GetNextObjective();
	if (CurrentObjective)
	{
		const FTransform ObjectiveTransform = ResolveCampaignObjectiveTransform(*CurrentObjective);
		return BuildRecoveryTransformNearObjective(ObjectiveTransform.GetLocation());
	}

	return AnchorTransform;
}

bool AHorrorGameModeBase::TryRecoverFromCampaignFailure(FName FailureCause)
{
	UWorld* World = GetWorld();
	AHorrorPlayerCharacter* PlayerCharacter = ResolveLeadPlayerCharacter();
	if (!World || !PlayerCharacter)
	{
		return false;
	}

	FTransform RecoveryTransform = ResolveCampaignFailureRecoveryTransform();
	FVector RecoveryLocation = RecoveryTransform.GetLocation();
	const FRotator RecoveryRotation = RecoveryTransform.Rotator();
	const FVector CurrentPlayerLocation = PlayerCharacter->GetActorLocation();
	const FVector RuntimeAnchorLocation = ResolveCampaignRuntimeAnchorTransform().GetLocation();
	const FVector CandidateLocations[] = {
		RecoveryLocation,
		CurrentPlayerLocation,
		CurrentPlayerLocation + FVector(0.0f, 0.0f, CampaignFailureRecoveryLiftCm),
		RuntimeAnchorLocation,
		RuntimeAnchorLocation + FVector(0.0f, 0.0f, CampaignFailureRecoveryLiftCm)
	};
	if (!TryResolveSafeCampaignFailureRecoveryLocation(*World, *PlayerCharacter, RecoveryRotation, CandidateLocations, RecoveryLocation))
	{
		UE_LOG(
			LogHorrorProject,
			Error,
			TEXT("Campaign failure recovery could not find a grounded clear teleport spot near %s for cause %s; recovery was aborted."),
			*RecoveryTransform.GetLocation().ToCompactString(),
			*FailureCause.ToString());
		return false;
	}

	RecoveryTransform.SetLocation(RecoveryLocation);
	PlayerCharacter->SetActorTransform(RecoveryTransform, false, nullptr, ETeleportType::TeleportPhysics);
	if (APlayerController* PlayerController = Cast<APlayerController>(PlayerCharacter->GetController()))
	{
		PlayerController->SetControlRotation(RecoveryRotation);
	}

	RefreshCampaignObjectiveActors();
	return true;
}

FText AHorrorGameModeBase::GetCurrentCampaignObjectivePromptText() const
{
	if (!ShouldExposeCampaignObjectivesToHUD())
	{
		return FText::GetEmpty();
	}

	const FHorrorCampaignObjectiveDefinition* CurrentObjective = CampaignProgress.GetNextObjective();
	return CurrentObjective ? CurrentObjective->PromptText : FText::GetEmpty();
}

FText AHorrorGameModeBase::GetCurrentCampaignObjectiveBeatText() const
{
	if (!ShouldExposeCampaignObjectivesToHUD())
	{
		return FText::GetEmpty();
	}

	const FHorrorCampaignObjectiveDefinition* CurrentObjective = CampaignProgress.GetNextObjective();
	if (!CurrentObjective)
	{
		return FText::GetEmpty();
	}

	const FName ActiveChapterId = CampaignProgress.GetActiveChapterId();
	const auto MatchesCurrentObjective = [ActiveChapterId, CurrentObjective](const AHorrorCampaignObjectiveActor* ObjectiveActor)
	{
		return ObjectiveActor
			&& !ObjectiveActor->IsActorBeingDestroyed()
			&& ObjectiveActor->GetChapterId() == ActiveChapterId
			&& ObjectiveActor->GetObjectiveId() == CurrentObjective->ObjectiveId;
	};

	for (const AHorrorCampaignObjectiveActor* ObjectiveActor : RuntimeCampaignObjectiveActorViews)
	{
		if (MatchesCurrentObjective(ObjectiveActor))
		{
			const FText BeatLabel = ObjectiveActor->GetCurrentObjectiveBeatLabel();
			const FText BeatDetail = ObjectiveActor->GetCurrentObjectiveBeatDetail();
			return BeatDetail.IsEmpty()
				? BeatLabel
				: FText::Format(
					NSLOCTEXT("HorrorGameMode", "CampaignBeatFormat", "{0}：{1}"),
					BeatLabel,
					BeatDetail);
		}
	}

	if (CurrentObjective->ObjectiveBeats.IsEmpty())
	{
		return FText::GetEmpty();
	}

	const FHorrorCampaignObjectiveBeat& Beat = CurrentObjective->ObjectiveBeats[0];
	return Beat.Detail.IsEmpty()
		? Beat.Label
		: FText::Format(
			NSLOCTEXT("HorrorGameMode", "CampaignDefinitionBeatFormat", "{0}：{1}"),
			Beat.Label,
			Beat.Detail);
}

FText AHorrorGameModeBase::GetCurrentCampaignObjectiveActionText() const
{
	if (!ShouldExposeCampaignObjectivesToHUD())
	{
		return FText::GetEmpty();
	}

	const FHorrorCampaignObjectiveDefinition* CurrentObjective = CampaignProgress.GetNextObjective();
	if (!CurrentObjective)
	{
		return FText::GetEmpty();
	}

	switch (CurrentObjective->InteractionMode)
	{
	case EHorrorCampaignInteractionMode::CircuitWiring:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionCircuitWiring", "接线");
	case EHorrorCampaignInteractionMode::GearCalibration:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionGearCalibration", "齿轮校准");
	case EHorrorCampaignInteractionMode::SpectralScan:
		return CurrentObjective->ObjectiveType == EHorrorCampaignObjectiveType::BossWeakPoint
			? NSLOCTEXT("HorrorGameMode", "CampaignActionBossSpectralPressure", "频谱压制")
			: NSLOCTEXT("HorrorGameMode", "CampaignActionSpectralScan", "频谱扫描");
	case EHorrorCampaignInteractionMode::SignalTuning:
		return CurrentObjective->ObjectiveType == EHorrorCampaignObjectiveType::PlantBeacon
			? NSLOCTEXT("HorrorGameMode", "CampaignActionBeaconTuning", "锚定调谐")
			: NSLOCTEXT("HorrorGameMode", "CampaignActionSignalTuning", "信号调谐");
	case EHorrorCampaignInteractionMode::TimedPursuit:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionTimedPursuit", "巨人追逐");
	case EHorrorCampaignInteractionMode::MultiStep:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionMultiStep", "多段互动");
	case EHorrorCampaignInteractionMode::Instant:
	default:
		break;
	}

	switch (CurrentObjective->ObjectiveType)
	{
	case EHorrorCampaignObjectiveType::AcquireSignal:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionAcquireSignal", "获取信号");
	case EHorrorCampaignObjectiveType::ScanAnomaly:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionScanAnomaly", "扫描");
	case EHorrorCampaignObjectiveType::RecoverRelic:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionRecoverRelic", "回收");
	case EHorrorCampaignObjectiveType::RestorePower:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionRestorePower", "供电");
	case EHorrorCampaignObjectiveType::PlantBeacon:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionPlantBeacon", "锚定调谐");
	case EHorrorCampaignObjectiveType::SurviveAmbush:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionSurviveAmbush", "巨人追逐");
	case EHorrorCampaignObjectiveType::DisableSeal:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionDisableSeal", "解除封印");
	case EHorrorCampaignObjectiveType::BossWeakPoint:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionBossWeakPoint", "频谱压制");
	case EHorrorCampaignObjectiveType::FinalTerminal:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionFinalTerminal", "终端");
	default:
		return NSLOCTEXT("HorrorGameMode", "CampaignActionObjective", "目标");
	}
}

int32 AHorrorGameModeBase::ApplyCampaignHorrorAtmosphere()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return 0;
	}

	int32 DimmedLightCount = 0;
	const FHorrorCampaignAtmosphereTuning AtmosphereTuning = ResolveCampaignAtmosphereTuningForMap(World->GetPackage()->GetName());
	const float LightScale = FMath::Clamp(AtmosphereTuning.LightScale, 0.0f, 1.0f);
	for (TActorIterator<ALight> It(World); It; ++It)
	{
		ALight* LightActor = *It;
		if (!LightActor
			|| LightActor->Tags.Contains(CampaignAtmosphereLightTag)
			|| LightActor->Tags.Contains(CampaignVisibilityRescueLightTag))
		{
			continue;
		}

		ULightComponent* LightComponent = LightActor->GetLightComponent();
		if (!LightComponent)
		{
			continue;
		}

		LightComponent->SetIntensity(LightComponent->Intensity * LightScale);
		LightActor->Tags.Add(CampaignAtmosphereLightTag);
		++DimmedLightCount;
	}

	APostProcessVolume* RuntimeVolume = nullptr;
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		APostProcessVolume* ExistingVolume = *It;
		if (ExistingVolume && ExistingVolume->Tags.Contains(CampaignAtmosphereVolumeTag))
		{
			RuntimeVolume = ExistingVolume;
			break;
		}
	}

	if (!RuntimeVolume)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		RuntimeVolume = World->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FTransform::Identity, SpawnParameters);
		if (RuntimeVolume)
		{
			RuntimeVolume->Tags.Add(CampaignAtmosphereVolumeTag);
		}
	}

	if (RuntimeVolume)
	{
		RuntimeVolume->bUnbound = true;
		RuntimeVolume->BlendWeight = 1.0f;
		RuntimeVolume->Settings.bOverride_AutoExposureBias = true;
		RuntimeVolume->Settings.AutoExposureBias = AtmosphereTuning.AutoExposureBias;
		RuntimeVolume->Settings.bOverride_VignetteIntensity = true;
		RuntimeVolume->Settings.VignetteIntensity = FMath::Clamp(AtmosphereTuning.VignetteIntensity, 0.0f, 1.0f);
	}

	DimmedLightCount += SanitizeImportedMapVisualObstructions(World->GetPackage()->GetName());

	return DimmedLightCount;
}

bool AHorrorGameModeBase::StartCampaignAmbushThreat(FName SourceId, AActor* ThreatAnchor)
{
	const FTransform AnchorTransform = ThreatAnchor
		? FTransform(FRotator(0.0f, ThreatAnchor->GetActorRotation().Yaw, 0.0f), ThreatAnchor->GetActorLocation(), FVector::OneVector)
		: ResolveCampaignRuntimeAnchorTransform();
	return StartCampaignAmbushThreatFromTransform(SourceId, AnchorTransform);
}

bool AHorrorGameModeBase::StartCampaignAmbushThreatFromTransform(FName SourceId, const FTransform& ThreatAnchorTransform)
{
	if (!bSpawnCampaignAmbushThreats)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	if (RuntimeCampaignAmbushThreat && RuntimeCampaignAmbushThreat->IsActorBeingDestroyed())
	{
		RuntimeCampaignAmbushThreat = nullptr;
		ActiveCampaignAmbushSourceId = NAME_None;
	}

	if (!RuntimeCampaignAmbushThreat)
	{
		const TSubclassOf<AHorrorCampaignBossActor> SpawnClass = RuntimeCampaignBossClass
			? RuntimeCampaignBossClass
			: TSubclassOf<AHorrorCampaignBossActor>(AHorrorCampaignBossActor::StaticClass());

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		RuntimeCampaignAmbushThreat = World->SpawnActor<AHorrorCampaignBossActor>(
			SpawnClass,
			ResolveCampaignAmbushThreatTransformFromAnchor(ThreatAnchorTransform),
			SpawnParameters);
	}
	else
	{
		RuntimeCampaignAmbushThreat->SetActorTransform(ResolveCampaignAmbushThreatTransformFromAnchor(ThreatAnchorTransform));
	}

	if (!RuntimeCampaignAmbushThreat)
	{
		return false;
	}

	const FHorrorCampaignAmbushThreatTuning ThreatTuning = ResolveCampaignAmbushThreatTuningForMap(World->GetPackage()->GetName());
	ActiveCampaignAmbushSourceId = SourceId;
	RuntimeCampaignAmbushThreat->ConfigureBoss(
		SourceId.IsNone() ? FName(TEXT("Campaign.AmbushThreat")) : SourceId,
		NSLOCTEXT("HorrorGameMode", "AmbushStoneGiant", "石像巨人"),
		1);
	RuntimeCampaignAmbushThreat->ConfigureChasePressureWithFearRate(
		ThreatTuning.MoveSpeed,
		ThreatTuning.EngageRadius,
		ThreatTuning.AttackRadius,
		ThreatTuning.FearPressureRadius,
		ThreatTuning.FearPressurePerSecond,
		ThreatTuning.ActorScale);
	RuntimeCampaignAmbushThreat->SetBossAwake(true);
	ShowCampaignMessage(NSLOCTEXT("HorrorGameMode", "AmbushWarning", "石像巨人被惊醒了，沿导航跑向亮起的逃离点。"), FLinearColor(1.0f, 0.28f, 0.18f), 2.5f);
	return true;
}

void AHorrorGameModeBase::StopCampaignAmbushThreat(FName SourceId)
{
	if (!RuntimeCampaignAmbushThreat)
	{
		ActiveCampaignAmbushSourceId = NAME_None;
		return;
	}

	if (!SourceId.IsNone() && !ActiveCampaignAmbushSourceId.IsNone() && SourceId != ActiveCampaignAmbushSourceId)
	{
		return;
	}

	if (!RuntimeCampaignAmbushThreat->IsActorBeingDestroyed())
	{
		RuntimeCampaignAmbushThreat->SetBossAwake(false);
		RuntimeCampaignAmbushThreat->Destroy();
	}

	RuntimeCampaignAmbushThreat = nullptr;
	ActiveCampaignAmbushSourceId = NAME_None;
}

bool AHorrorGameModeBase::AbortActiveCampaignPursuitForRecovery(FName FailureCause, AActor* InstigatorActor)
{
	auto TryAbortObjectiveActor = [FailureCause, InstigatorActor](AHorrorCampaignObjectiveActor* ObjectiveActor)
	{
		if (!ObjectiveActor || ObjectiveActor->IsActorBeingDestroyed() || !ObjectiveActor->IsTimedObjectiveActive())
		{
			return false;
		}

		return ObjectiveActor->AbortTimedObjectiveForRecovery(
			InstigatorActor,
			FailureCause,
			NSLOCTEXT("HorrorGameMode", "CampaignPursuitCaughtFailureTitle", "追逐失败：石像巨人追上了你"),
			NSLOCTEXT("HorrorGameMode", "CampaignPursuitCaughtRetryHint", "你被拉回追逐起点。重新互动后沿导航跑向亮起的逃离点。"));
	};

	for (AHorrorCampaignObjectiveActor* ObjectiveActor : RuntimeCampaignObjectiveActorViews)
	{
		if (TryAbortObjectiveActor(ObjectiveActor))
		{
			return true;
		}
	}

	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AHorrorCampaignObjectiveActor> It(World); It; ++It)
		{
			if (TryAbortObjectiveActor(*It))
			{
				return true;
			}
		}
	}

	return false;
}

bool AHorrorGameModeBase::ClearImportedMapCameraFade(APlayerController* PlayerController)
{
	APlayerCameraManager* CameraManager = PlayerController ? PlayerController->PlayerCameraManager : nullptr;
	if (!CameraManager)
	{
		return false;
	}

	CameraManager->StopCameraFade();
	CameraManager->FadeAmount = 0.0f;
	CameraManager->FadeAlpha = FVector2D::ZeroVector;
	CameraManager->FadeTime = 0.0f;
	CameraManager->FadeTimeRemaining = 0.0f;
	CameraManager->FadeColor = FLinearColor::Transparent;
	CameraManager->bEnableFading = false;
	CameraManager->bFadeAudio = false;
	return true;
}

FHorrorCampaignAtmosphereTuning AHorrorGameModeBase::ResolveCampaignAtmosphereTuningForMapForTests(const FString& MapPackageName) const
{
	return ResolveCampaignAtmosphereTuningForMap(MapPackageName);
}

FHorrorCampaignAmbushThreatTuning AHorrorGameModeBase::ResolveCampaignAmbushThreatTuningForMapForTests(const FString& MapPackageName) const
{
	return ResolveCampaignAmbushThreatTuningForMap(MapPackageName);
}

int32 AHorrorGameModeBase::SanitizeImportedMapVisualObstructionsForTests(const FString& MapPackageName)
{
	return SanitizeImportedMapVisualObstructions(MapPackageName);
}

void AHorrorGameModeBase::ResetCampaignProgressForChapterForTests(FName ChapterId)
{
	if (const FHorrorCampaignChapterDefinition* Chapter = FHorrorCampaign::FindChapterById(ChapterId))
	{
		CampaignProgress.ResetForChapter(*Chapter);
		CampaignNavigationFocusObjectiveId = NAME_None;
	}
}

bool AHorrorGameModeBase::SaveDay1Checkpoint(FName CheckpointId)
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UHorrorSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UHorrorSaveSubsystem>())
		{
			return SaveSubsystem->SaveCheckpoint(this, CheckpointId);
		}
	}

	return false;
}

bool AHorrorGameModeBase::LoadDay1Checkpoint()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UHorrorSaveSubsystem* SaveSubsystem = GameInstance->GetSubsystem<UHorrorSaveSubsystem>())
		{
			return SaveSubsystem->LoadCheckpoint(this);
		}
	}

	return false;
}

bool AHorrorGameModeBase::RequestPlayerDeath(FName DeathCause)
{
	return RequestPlayerFailure(
		DeathCause.IsNone() ? HorrorPlayerFailureDefaults::UnknownDeathCause : DeathCause,
		HorrorPlayerFailureDefaults::DeathFeedback);
}

bool AHorrorGameModeBase::RequestPlayerFailure(FName FailureCause, FText FeedbackText)
{
	const FName ResolvedFailureCause = FailureCause.IsNone()
		? HorrorPlayerFailureDefaults::UnknownFailureCause
		: FailureCause;
	const FText ResolvedFeedbackText = FeedbackText.IsEmpty()
		? HorrorPlayerFailureDefaults::FailureFeedback
		: FeedbackText;

	LastPlayerFailureCause = ResolvedFailureCause;
	++PlayerFailureCount;
	bLastPlayerFailureRecoveredFromCheckpoint = false;

	UWorld* World = GetWorld();
	if (World)
	{
		UGameplayStatics::SetGamePaused(World, false);
	}

	if (AHorrorPlayerController* PlayerController = Cast<AHorrorPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		PlayerController->ShowPlayerMessage(ResolvedFeedbackText, FLinearColor(0.9f, 0.05f, 0.04f), 2.0f);
	}

	UE_LOG(
		LogHorrorProject,
		Warning,
		TEXT("Player failure requested: %s. Attempting contextual recovery."),
		*ResolvedFailureCause.ToString());

	if (World)
	{
		if (UHorrorAudioSubsystem* AudioSubsystem = World->GetSubsystem<UHorrorAudioSubsystem>())
		{
			AudioSubsystem->HandleDay1EventName(TEXT("Day1.Failure"), ResolvedFailureCause);
		}
	}

	bLastPlayerFailureRecoveredFromCheckpoint = ShouldUseCampaignFailureRecovery(ResolvedFailureCause)
		? TryRecoverFromCampaignFailure(ResolvedFailureCause)
		: LoadDay1Checkpoint();
	if (World)
	{
		if (UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>())
		{
			EventBus->Publish(
				HorrorDay1Tags::PlayerFailureEvent(),
				ResolvedFailureCause,
				bLastPlayerFailureRecoveredFromCheckpoint
					? HorrorDay1Tags::CheckpointRestoredState()
					: HorrorDay1Tags::CheckpointMissingState(),
				this);
		}
	}

	if (!bLastPlayerFailureRecoveredFromCheckpoint)
	{
		if (AHorrorPlayerController* PlayerController = Cast<AHorrorPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
		{
			PlayerController->ResetDay1ModalInputState();
			PlayerController->ShowPlayerMessage(HorrorPlayerFailureDefaults::MissingCheckpointFeedback, FLinearColor(1.0f, 0.78f, 0.15f), 2.0f);
		}

		UE_LOG(
			LogHorrorProject,
			Warning,
			TEXT("Player failure recovery could not find a valid Day1 checkpoint for cause: %s."),
			*ResolvedFailureCause.ToString());
	}

	return bLastPlayerFailureRecoveredFromCheckpoint;
}

bool AHorrorGameModeBase::RecordFoundFootageEvent(FGameplayTag EventTag, FName SourceId)
{
	TArray<FHorrorFoundFootageRecordedEvent> RecordedEvents;
	TArray<FHorrorFoundFootageStateChange> StateChanges;
	const bool bRecorded = FoundFootageContract.RecordEvent(EventTag, SourceId, &RecordedEvents, &StateChanges);
	if (bRecorded)
	{
		PublishFoundFootageEvents(GetWorld(), FoundFootageContract, RecordedEvents, this);
		for (const FHorrorFoundFootageStateChange& StateChange : StateChanges)
		{
			if (StateChange.bCompleted)
			{
				HandleObjectiveStateChange(StateChange.StateTag);
			}
		}
	}
	return bRecorded;
}

AHorrorPlayerCharacter* AHorrorGameModeBase::ResolveLeadPlayerCharacter() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	if (AHorrorPlayerCharacter* PlayerCharacter = Cast<AHorrorPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0)))
	{
		return PlayerCharacter;
	}

	for (TActorIterator<AHorrorPlayerCharacter> PlayerIt(World); PlayerIt; ++PlayerIt)
	{
		return *PlayerIt;
	}

	return nullptr;
}

ADeepWaterStationRouteKit* AHorrorGameModeBase::EnsureRouteKit()
{
	if (RuntimeRouteKit)
	{
		return RuntimeRouteKit.Get();
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<ADeepWaterStationRouteKit> It(World); It; ++It)
	{
		RuntimeRouteKit = *It;
		EnsureRouteKitObjectives(*RuntimeRouteKit);
		return RuntimeRouteKit.Get();
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	RuntimeRouteKit = World->SpawnActor<ADeepWaterStationRouteKit>(
		ADeepWaterStationRouteKit::StaticClass(),
		ResolveRuntimeRouteKitSpawnTransform(),
		SpawnParameters);
	if (RuntimeRouteKit)
	{
		EnsureRouteKitObjectives(*RuntimeRouteKit);
	}
	return RuntimeRouteKit.Get();
}

FTransform AHorrorGameModeBase::ResolveRuntimeRouteKitSpawnTransform() const
{
	if (!bAnchorRuntimeRouteKitToPlayerStart)
	{
		return RuntimeRouteKitTransform;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return RuntimeRouteKitTransform;
	}

	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		const FRotator PlayerStartRotation = It->GetActorRotation();
		return FTransform(
			FRotator(0.0f, PlayerStartRotation.Yaw, 0.0f),
			It->GetActorLocation(),
			RuntimeRouteKitTransform.GetScale3D());
	}

	return RuntimeRouteKitTransform;
}

bool AHorrorGameModeBase::IsLeadPlayerRecording() const
{
	const AHorrorPlayerCharacter* PlayerCharacter = ResolveLeadPlayerCharacter();
	const UQuantumCameraComponent* QuantumCamera = PlayerCharacter ? PlayerCharacter->GetQuantumCameraComponent() : nullptr;
	return QuantumCamera && QuantumCamera->IsCameraMode(EQuantumCameraMode::Recording);
}

AHorrorEncounterDirector* AHorrorGameModeBase::EnsureEncounterDirector()
{
	if (RuntimeEncounterDirector)
	{
		return RuntimeEncounterDirector.Get();
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AHorrorEncounterDirector> It(World); It; ++It)
	{
		RuntimeEncounterDirector = *It;
		return RuntimeEncounterDirector.Get();
	}

	const TSubclassOf<AHorrorEncounterDirector> SpawnClass = RuntimeEncounterDirectorClass
		? RuntimeEncounterDirectorClass
		: TSubclassOf<AHorrorEncounterDirector>(AHorrorEncounterDirector::StaticClass());

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	RuntimeEncounterDirector = World->SpawnActor<AHorrorEncounterDirector>(SpawnClass, RuntimeEncounterDirectorTransform, SpawnParameters);
	if (RuntimeEncounterDirector)
	{
		RuntimeEncounterDirector->RevealRadius = 0.0f;
	}
	return RuntimeEncounterDirector.Get();
}

AHorrorMapChainExit* AHorrorGameModeBase::EnsureMapChainExit()
{
	if (RuntimeMapChainExit)
	{
		return RuntimeMapChainExit.Get();
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FString CurrentMapPackageName = FHorrorMapChain::NormalizeMapPackageName(World->GetPackage()->GetName());
	if (!FHorrorMapChain::IsMapInChain(CurrentMapPackageName))
	{
		return nullptr;
	}

	for (TActorIterator<AHorrorMapChainExit> It(World); It; ++It)
	{
		RuntimeMapChainExit = *It;

		FString NextMapPackageName;
		const bool bHasNextMap = FHorrorMapChain::TryGetNextMapPackage(CurrentMapPackageName, NextMapPackageName);
		RuntimeMapChainExit->ConfigureForMapChain(CurrentMapPackageName, NextMapPackageName, !bHasNextMap);
		return RuntimeMapChainExit.Get();
	}

	const TSubclassOf<AHorrorMapChainExit> SpawnClass = RuntimeMapChainExitClass
		? RuntimeMapChainExitClass
		: TSubclassOf<AHorrorMapChainExit>(AHorrorMapChainExit::StaticClass());

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	RuntimeMapChainExit = World->SpawnActor<AHorrorMapChainExit>(
		SpawnClass,
		ResolveMapChainExitSpawnTransform(),
		SpawnParameters);

	if (RuntimeMapChainExit)
	{
		FString NextMapPackageName;
		const bool bHasNextMap = FHorrorMapChain::TryGetNextMapPackage(CurrentMapPackageName, NextMapPackageName);
		RuntimeMapChainExit->ConfigureForMapChain(CurrentMapPackageName, NextMapPackageName, !bHasNextMap);
	}

	return RuntimeMapChainExit.Get();
}

FTransform AHorrorGameModeBase::ResolveMapChainExitSpawnTransform() const
{
	const UWorld* World = GetWorld();
	APlayerStart* FirstPlacedPlayerStart = nullptr;
	APlayerStart* PreferredPlayerStart = nullptr;

	if (World)
	{
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			APlayerStart* PlayerStart = *It;
			if (!PlayerStart)
			{
				continue;
			}

			if (PlayerStart->IsA<APlayerStartPIE>())
			{
				FirstPlacedPlayerStart = PlayerStart;
				break;
			}

			if (!FirstPlacedPlayerStart)
			{
				FirstPlacedPlayerStart = PlayerStart;
			}

			if (!PreferredPlayerStart && !PreferredPlayerStartTag.IsNone() && PlayerStart->PlayerStartTag == PreferredPlayerStartTag)
			{
				PreferredPlayerStart = PlayerStart;
			}
		}
	}

	const AActor* AnchorActor = PreferredPlayerStart ? PreferredPlayerStart : FirstPlacedPlayerStart;
	const FRotator AnchorRotation = AnchorActor ? AnchorActor->GetActorRotation() : FRotator::ZeroRotator;
	const FVector AnchorLocation = AnchorActor ? AnchorActor->GetActorLocation() : FVector::ZeroVector;
	const FRotationMatrix AnchorRotationMatrix(FRotator(0.0f, AnchorRotation.Yaw, 0.0f));
	const FVector Offset =
		AnchorRotationMatrix.GetUnitAxis(EAxis::X) * RuntimeMapChainExitOffset.X
		+ AnchorRotationMatrix.GetUnitAxis(EAxis::Y) * RuntimeMapChainExitOffset.Y
		+ FVector(0.0f, 0.0f, RuntimeMapChainExitOffset.Z);

	return FTransform(
		FRotator(0.0f, AnchorRotation.Yaw + 180.0f, 0.0f),
		AnchorLocation + Offset,
		FVector::OneVector);
}

bool AHorrorGameModeBase::EnsureCampaignRuntime()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const FString CurrentMapPackageName = GetCurrentNormalizedMapPackageName();
	const FString CampaignRuntimeMapName = CurrentMapPackageName.IsEmpty()
		? World->GetPackage()->GetName()
		: CurrentMapPackageName;
	const FHorrorCampaignChapterDefinition* Chapter = FHorrorCampaign::FindChapterForMap(CampaignRuntimeMapName);
	if (!Chapter)
	{
		const TArray<FHorrorCampaignChapterDefinition>& AllChapters = FHorrorCampaign::GetChapters();
		if (AllChapters.IsEmpty())
		{
			return false;
		}
		Chapter = &AllChapters[0];
	}

	bool bResetChapterProgress = false;
	if (CampaignProgress.GetActiveChapterId() != Chapter->ChapterId)
	{
		ClearCampaignRuntimeActors();
		CampaignProgress.ResetForChapter(*Chapter);
		CampaignNavigationFocusObjectiveId = NAME_None;
		bCampaignAutoTravelQueued = false;
		bResetChapterProgress = true;
	}

	if (RuntimeCampaignObjectiveActors.IsEmpty() || RuntimeCampaignObjectiveActorViews.IsEmpty())
	{
		ClearCampaignRuntimeActors();
		SpawnCampaignObjectives(*Chapter);
	}

	if (Chapter->bRequiresBoss && !RuntimeCampaignBoss)
	{
		SpawnCampaignBossIfNeeded(*Chapter);
	}

	if (bResetChapterProgress)
	{
			UE_LOG(LogHorrorProject, Log, TEXT("Campaign runtime initialized chapter %s on map %s with %d objectives%s."),
				*Chapter->ChapterId.ToString(),
				*CampaignRuntimeMapName,
				Chapter->Objectives.Num(),
				Chapter->bRequiresBoss ? TEXT(" and a boss encounter") : TEXT(""));
		if (ShouldExposeCampaignObjectivesToHUD())
		{
			ShowCampaignChapterIntro(*Chapter);
			ShowCurrentCampaignObjectiveHint();
		}
	}

	return true;
}

FString AHorrorGameModeBase::GetCurrentNormalizedMapPackageName() const
{
	if (!CachedMapPackageName.IsEmpty())
	{
		return CachedMapPackageName;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return FString();
	}

	return FHorrorMapChain::NormalizeMapPackageName(World->GetPackage()->GetName());
}

bool AHorrorGameModeBase::ShouldAutoSpawnLegacyRouteKitInCurrentMap() const
{
	const FString CurrentMapPackageName = GetCurrentNormalizedMapPackageName();
	if (CurrentMapPackageName.IsEmpty())
	{
		return true;
	}

	return !FHorrorMapChain::IsMapInChain(CurrentMapPackageName);
}

FTransform AHorrorGameModeBase::ResolveCampaignRuntimeAnchorTransform() const
{
	const UWorld* World = GetWorld();
	APlayerStart* FirstPlacedPlayerStart = nullptr;
	APlayerStart* PreferredPlayerStart = nullptr;

	if (World)
	{
		for (TActorIterator<APlayerStart> It(World); It; ++It)
		{
			APlayerStart* PlayerStart = *It;
			if (!PlayerStart)
			{
				continue;
			}

			if (PlayerStart->IsA<APlayerStartPIE>())
			{
				FirstPlacedPlayerStart = PlayerStart;
				break;
			}

			if (!FirstPlacedPlayerStart)
			{
				FirstPlacedPlayerStart = PlayerStart;
			}

			if (!PreferredPlayerStart && !PreferredPlayerStartTag.IsNone() && PlayerStart->PlayerStartTag == PreferredPlayerStartTag)
			{
				PreferredPlayerStart = PlayerStart;
			}
		}
	}

	const AActor* AnchorActor = PreferredPlayerStart ? PreferredPlayerStart : FirstPlacedPlayerStart;
	return AnchorActor
		? FTransform(FRotator(0.0f, AnchorActor->GetActorRotation().Yaw, 0.0f), AnchorActor->GetActorLocation(), FVector::OneVector)
		: FTransform::Identity;
}

FTransform AHorrorGameModeBase::ResolveCampaignObjectiveTransform(const FHorrorCampaignObjectiveDefinition& Objective) const
{
	return ResolveSafeCampaignObjectiveTransform(Objective);
}

FTransform AHorrorGameModeBase::ResolveSafeCampaignObjectiveTransform(const FHorrorCampaignObjectiveDefinition& Objective) const
{
	const FTransform AnchorTransform = ResolveCampaignRuntimeAnchorTransform();
	const FRotator AnchorYawRotation(0.0f, AnchorTransform.Rotator().Yaw, 0.0f);
	const FRotationMatrix AnchorRotationMatrix(AnchorYawRotation);
	const FVector RelativeLocation = BuildCampaignRelativeOffsetWorldSpace(Objective.RelativeLocation, AnchorRotationMatrix);
	FVector ResolvedLocation = AnchorTransform.GetLocation() + RelativeLocation;
	if (UWorld* World = GetWorld())
	{
		FVector ClearLocation = ResolvedLocation;
		if (TryFindClearCampaignObjectiveLocation(*World, ResolvedLocation, ClearLocation))
		{
			ResolvedLocation = ClearLocation;
		}
	}

	return FTransform(
		FRotator(0.0f, AnchorYawRotation.Yaw + 180.0f, 0.0f),
		ResolvedLocation,
		FVector::OneVector);
}

const AHorrorCampaignObjectiveActor* AHorrorGameModeBase::FindRuntimeCampaignObjectiveActor(FName ChapterId, FName ObjectiveId) const
{
	if (ChapterId.IsNone() || ObjectiveId.IsNone())
	{
		return nullptr;
	}

	const auto MatchesObjective = [ChapterId, ObjectiveId](const AHorrorCampaignObjectiveActor* ObjectiveActor)
	{
		return ObjectiveActor
			&& !ObjectiveActor->IsActorBeingDestroyed()
			&& ObjectiveActor->GetChapterId() == ChapterId
			&& ObjectiveActor->GetObjectiveId() == ObjectiveId;
	};

	for (const AHorrorCampaignObjectiveActor* ObjectiveActor : RuntimeCampaignObjectiveActorViews)
	{
		if (MatchesObjective(ObjectiveActor))
		{
			return ObjectiveActor;
		}
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AHorrorCampaignObjectiveActor> It(World); It; ++It)
	{
		if (MatchesObjective(*It))
		{
			return *It;
		}
	}

	return nullptr;
}

const AHorrorCampaignObjectiveActor* AHorrorGameModeBase::FindCurrentCampaignObjectiveActor() const
{
	const FHorrorCampaignObjectiveDefinition* CurrentObjective = CampaignProgress.GetNextObjective();
	if (!CurrentObjective)
	{
		return nullptr;
	}

	return FindRuntimeCampaignObjectiveActor(CampaignProgress.GetActiveChapterId(), CurrentObjective->ObjectiveId);
}

FTransform AHorrorGameModeBase::ResolveCampaignBossTransform() const
{
	const FTransform AnchorTransform = ResolveCampaignRuntimeAnchorTransform();
	const FRotator AnchorYawRotation(0.0f, AnchorTransform.Rotator().Yaw, 0.0f);
	const FRotationMatrix AnchorRotationMatrix(AnchorYawRotation);
	const FVector RelativeLocation =
		AnchorRotationMatrix.GetUnitAxis(EAxis::X) * RuntimeCampaignBossOffset.X
		+ AnchorRotationMatrix.GetUnitAxis(EAxis::Y) * RuntimeCampaignBossOffset.Y
		+ FVector(0.0f, 0.0f, RuntimeCampaignBossOffset.Z);

	return FTransform(
		FRotator(0.0f, AnchorYawRotation.Yaw + 180.0f, 0.0f),
		AnchorTransform.GetLocation() + RelativeLocation,
		FVector::OneVector);
}

FTransform AHorrorGameModeBase::ResolveCampaignAmbushThreatTransform(const AActor* ThreatAnchor) const
{
	const FTransform AnchorTransform = ThreatAnchor
		? FTransform(FRotator(0.0f, ThreatAnchor->GetActorRotation().Yaw, 0.0f), ThreatAnchor->GetActorLocation(), FVector::OneVector)
		: ResolveCampaignRuntimeAnchorTransform();
	return ResolveCampaignAmbushThreatTransformFromAnchor(AnchorTransform);
}

FTransform AHorrorGameModeBase::ResolveCampaignAmbushThreatTransformFromAnchor(const FTransform& AnchorTransform) const
{
	const FRotator AnchorYawRotation(0.0f, AnchorTransform.Rotator().Yaw, 0.0f);
	const FRotationMatrix AnchorRotationMatrix(AnchorYawRotation);
	const FVector AnchorForward = AnchorRotationMatrix.GetUnitAxis(EAxis::X);
	const FVector AnchorRight = AnchorRotationMatrix.GetUnitAxis(EAxis::Y);
	const float AuthoredBackstep = FMath::Abs(RuntimeCampaignAmbushThreatOffset.X);
	const float SafeBackstep = FMath::Clamp(
		AuthoredBackstep,
		CampaignAmbushThreatMinimumBackstepCm,
		CampaignAmbushThreatMaximumBackstepCm);
	const float AuthoredSideStep = RuntimeCampaignAmbushThreatOffset.Y;
	const float SideStepSign = AuthoredSideStep < 0.0f ? -1.0f : 1.0f;
	const float SafeSideStep = SideStepSign * FMath::Max(FMath::Abs(AuthoredSideStep), CampaignAmbushThreatMinimumSideStepCm);
	const FVector RelativeLocation =
		-AnchorForward * SafeBackstep
		+ AnchorRight * SafeSideStep
		+ FVector(0.0f, 0.0f, RuntimeCampaignAmbushThreatOffset.Z);
	FVector SpawnLocation = AnchorTransform.GetLocation() + RelativeLocation;
	if (UWorld* World = GetWorld())
	{
		FVector ClearLocation = SpawnLocation;
		if (TryFindClearCampaignObjectiveLocation(*World, SpawnLocation, ClearLocation))
		{
			SpawnLocation = ClearLocation;
		}
	}

	FVector ToAnchor = AnchorTransform.GetLocation() - SpawnLocation;
	ToAnchor.Z = 0.0f;
	const FRotator SpawnYaw = ToAnchor.IsNearlyZero()
		? FRotator(0.0f, AnchorYawRotation.Yaw, 0.0f)
		: FRotator(0.0f, ToAnchor.Rotation().Yaw, 0.0f);

	return FTransform(
		SpawnYaw,
		SpawnLocation,
		FVector::OneVector);
}

FHorrorCampaignAtmosphereTuning AHorrorGameModeBase::ResolveCampaignAtmosphereTuningForMap(const FString& MapPackageName) const
{
	FHorrorCampaignAtmosphereTuning Tuning;
	Tuning.LightScale = FMath::Clamp(CampaignHorrorAtmosphereLightScale, 0.0f, 1.0f);
	Tuning.AutoExposureBias = CampaignHorrorAtmosphereAutoExposureBias;
	Tuning.VignetteIntensity = FMath::Clamp(CampaignHorrorAtmosphereVignetteIntensity, 0.0f, 1.0f);

	const FString NormalizedMapPackageName = FHorrorMapChain::NormalizeMapPackageName(MapPackageName);
	if (NormalizedMapPackageName.Contains(TEXT("/Scrapopolis/")) || NormalizedMapPackageName.Contains(TEXT("Level_Scrapopolis_Demo")))
	{
		Tuning.LightScale = FMath::Clamp(ScrapopolisAtmosphereLightScale, 0.0f, 1.0f);
		Tuning.AutoExposureBias = ScrapopolisAtmosphereAutoExposureBias;
		Tuning.VignetteIntensity = FMath::Clamp(ScrapopolisAtmosphereVignetteIntensity, 0.0f, 1.0f);
	}

	return Tuning;
}

FHorrorCampaignAmbushThreatTuning AHorrorGameModeBase::ResolveCampaignAmbushThreatTuningForMap(const FString& MapPackageName) const
{
	FHorrorCampaignAmbushThreatTuning Tuning;

	const FString NormalizedMapPackageName = FHorrorMapChain::NormalizeMapPackageName(MapPackageName);
	if (NormalizedMapPackageName.Contains(TEXT("/DeepWaterStation/")) || NormalizedMapPackageName.Contains(TEXT("DemoMap_VerticalSlice_Day1")))
	{
		Tuning.ActorScale = 0.86f;
		Tuning.MoveSpeed = 95.0f;
		Tuning.EngageRadius = 3600.0f;
		Tuning.AttackRadius = 70.0f;
		Tuning.FearPressureRadius = 850.0f;
		Tuning.FearPressurePerSecond = 0.8f;
	}
	else if (NormalizedMapPackageName.Contains(TEXT("/Bodycam_VHS_Effect/")) || NormalizedMapPackageName.Contains(TEXT("LVL_Showcase_01")))
	{
		Tuning.ActorScale = 0.84f;
		Tuning.MoveSpeed = 96.0f;
		Tuning.EngageRadius = 3800.0f;
		Tuning.AttackRadius = 75.0f;
		Tuning.FearPressureRadius = 850.0f;
		Tuning.FearPressurePerSecond = 0.85f;
	}
	else if (NormalizedMapPackageName.Contains(TEXT("/ForestOfSpikes/")) || NormalizedMapPackageName.Contains(TEXT("Level_ForestOfSpikes_Demo_Night")))
	{
		Tuning.ActorScale = 1.12f;
		Tuning.MoveSpeed = 125.0f;
		Tuning.EngageRadius = 4600.0f;
		Tuning.AttackRadius = 95.0f;
		Tuning.FearPressureRadius = 1100.0f;
		Tuning.FearPressurePerSecond = 1.0f;
	}
	else if (NormalizedMapPackageName.Contains(TEXT("/Scrapopolis/")) || NormalizedMapPackageName.Contains(TEXT("Level_Scrapopolis_Demo")))
	{
		Tuning.ActorScale = 0.9f;
		Tuning.MoveSpeed = 125.0f;
		Tuning.EngageRadius = 4000.0f;
		Tuning.AttackRadius = 95.0f;
		Tuning.FearPressureRadius = 1100.0f;
		Tuning.FearPressurePerSecond = 1.0f;
	}
	else if (NormalizedMapPackageName.Contains(TEXT("/Fantastic_Dungeon_Pack/")) || NormalizedMapPackageName.Contains(TEXT("map_dungeon_")))
	{
		Tuning.ActorScale = 0.95f;
		Tuning.MoveSpeed = 95.0f;
		Tuning.EngageRadius = 3400.0f;
		Tuning.AttackRadius = 78.0f;
		Tuning.FearPressureRadius = 900.0f;
		Tuning.FearPressurePerSecond = 0.85f;
	}

	return Tuning;
}

bool AHorrorGameModeBase::ShouldSanitizeImportedMapVisualObstructions(const FString& MapPackageName) const
{
	const FString NormalizedMapPackageName = FHorrorMapChain::NormalizeMapPackageName(MapPackageName);
	return NormalizedMapPackageName.Contains(TEXT("/Scrapopolis/"))
		|| NormalizedMapPackageName.Contains(TEXT("Level_Scrapopolis_Demo"));
}

int32 AHorrorGameModeBase::SanitizeImportedMapVisualObstructions(const FString& MapPackageName)
{
	if (!ShouldSanitizeImportedMapVisualObstructions(MapPackageName))
	{
		return 0;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return 0;
	}

	int32 SanitizedActorCount = 0;
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		APostProcessVolume* PostProcessVolume = *It;
		if (!PostProcessVolume || PostProcessVolume->Tags.Contains(CampaignAtmosphereVolumeTag))
		{
			continue;
		}

		bool bChanged = false;
		if (PostProcessVolume->bEnabled)
		{
			PostProcessVolume->bEnabled = false;
			bChanged = true;
		}
		if (!FMath::IsNearlyZero(PostProcessVolume->BlendWeight))
		{
			PostProcessVolume->BlendWeight = 0.0f;
			bChanged = true;
		}
		if (PostProcessVolume->bUnbound)
		{
			PostProcessVolume->bUnbound = false;
			bChanged = true;
		}

		FPostProcessSettings& Settings = PostProcessVolume->Settings;
		if (Settings.bOverride_AutoExposureBias || Settings.bOverride_AutoExposureMinBrightness || Settings.bOverride_AutoExposureMaxBrightness)
		{
			Settings.bOverride_AutoExposureBias = false;
			Settings.bOverride_AutoExposureMinBrightness = false;
			Settings.bOverride_AutoExposureMaxBrightness = false;
			Settings.AutoExposureBias = 0.0f;
			bChanged = true;
		}
		if (Settings.bOverride_ColorGain || Settings.bOverride_ColorGamma || Settings.bOverride_ColorSaturation || Settings.bOverride_SceneColorTint)
		{
			Settings.bOverride_ColorGain = false;
			Settings.bOverride_ColorGamma = false;
			Settings.bOverride_ColorSaturation = false;
			Settings.bOverride_SceneColorTint = false;
			Settings.ColorGain = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
			Settings.ColorGamma = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
			Settings.ColorSaturation = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
			Settings.SceneColorTint = FLinearColor::White;
			bChanged = true;
		}
		if (Settings.bOverride_BloomIntensity)
		{
			Settings.bOverride_BloomIntensity = false;
			bChanged = true;
		}
		if (Settings.bOverride_VignetteIntensity)
		{
			Settings.bOverride_VignetteIntensity = false;
			bChanged = true;
		}

		if (bChanged)
		{
			++SanitizedActorCount;
			UE_LOG(LogHorrorProject, Log, TEXT("Disabled imported post-process volume %s for Scrapopolis readability."), *GetNameSafe(PostProcessVolume));
		}
	}

	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		AExponentialHeightFog* FogActor = *It;
		UExponentialHeightFogComponent* FogComponent = FogActor ? FogActor->GetComponent() : nullptr;
		if (!FogComponent)
		{
			continue;
		}

		bool bChanged = false;
		if (FogComponent->FogDensity > ScrapopolisReadableFogDensity)
		{
			FogComponent->SetFogDensity(ScrapopolisReadableFogDensity);
			bChanged = true;
		}
		if (FogComponent->SecondFogData.FogDensity > ScrapopolisReadableFogDensity)
		{
			FogComponent->SetSecondFogDensity(ScrapopolisReadableFogDensity);
			bChanged = true;
		}
		if (FogComponent->FogMaxOpacity > ScrapopolisReadableFogMaxOpacity)
		{
			FogComponent->SetFogMaxOpacity(ScrapopolisReadableFogMaxOpacity);
			bChanged = true;
		}
		if (FogComponent->bEnableVolumetricFog)
		{
			FogComponent->SetVolumetricFog(false);
			bChanged = true;
		}
		if (FogComponent->VolumetricFogExtinctionScale > 1.0f)
		{
			FogComponent->SetVolumetricFogExtinctionScale(1.0f);
			bChanged = true;
		}

		if (bChanged)
		{
			++SanitizedActorCount;
			UE_LOG(LogHorrorProject, Log, TEXT("Clamped imported fog actor %s for Scrapopolis readability."), *GetNameSafe(FogActor));
		}
	}

	APointLight* RescueLight = nullptr;
	for (TActorIterator<APointLight> It(World); It; ++It)
	{
		APointLight* PointLight = *It;
		if (PointLight && PointLight->Tags.Contains(CampaignVisibilityRescueLightTag))
		{
			RescueLight = PointLight;
			break;
		}
	}

	bool bRescueLightChanged = false;
	if (!RescueLight)
	{
		const FTransform AnchorTransform = ResolveCampaignRuntimeAnchorTransform();
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		RescueLight = World->SpawnActor<APointLight>(
			APointLight::StaticClass(),
			FTransform(FRotator::ZeroRotator, AnchorTransform.GetLocation() + FVector(0.0f, 0.0f, 260.0f)),
			SpawnParameters);
		if (RescueLight)
		{
			RescueLight->Tags.Add(CampaignVisibilityRescueLightTag);
			RescueLight->Tags.Add(CampaignAtmosphereLightTag);
			bRescueLightChanged = true;
		}
	}

	if (RescueLight)
	{
		if (ULightComponent* LightComponent = RescueLight->GetLightComponent())
		{
			if (LightComponent->Intensity < ScrapopolisRescueLightIntensity)
			{
				LightComponent->SetIntensity(ScrapopolisRescueLightIntensity);
				bRescueLightChanged = true;
			}
			LightComponent->SetLightColor(FLinearColor(0.82f, 0.9f, 1.0f));
		}
		if (ULocalLightComponent* LocalLightComponent = Cast<ULocalLightComponent>(RescueLight->GetLightComponent()))
		{
			if (LocalLightComponent->AttenuationRadius < ScrapopolisRescueLightRadius)
			{
				LocalLightComponent->SetAttenuationRadius(ScrapopolisRescueLightRadius);
				LocalLightComponent->AttenuationRadius = ScrapopolisRescueLightRadius;
				bRescueLightChanged = true;
			}
		}
	}

	if (bRescueLightChanged)
	{
		++SanitizedActorCount;
	}

	return SanitizedActorCount;
}

void AHorrorGameModeBase::ClearCampaignRuntimeActors()
{
	for (AHorrorCampaignObjectiveActor* ObjectiveActor : RuntimeCampaignObjectiveActorViews)
	{
		if (ObjectiveActor && !ObjectiveActor->IsActorBeingDestroyed())
		{
			ObjectiveActor->Destroy();
		}
	}

	RuntimeCampaignObjectiveActorViews.Reset();
	RuntimeCampaignObjectiveActors.Reset();

	if (RuntimeCampaignBoss && !RuntimeCampaignBoss->IsActorBeingDestroyed())
	{
		RuntimeCampaignBoss->Destroy();
	}
	RuntimeCampaignBoss = nullptr;

	if (RuntimeCampaignAmbushThreat && !RuntimeCampaignAmbushThreat->IsActorBeingDestroyed())
	{
		RuntimeCampaignAmbushThreat->Destroy();
	}
	RuntimeCampaignAmbushThreat = nullptr;
	ActiveCampaignAmbushSourceId = NAME_None;
}

void AHorrorGameModeBase::SpawnCampaignObjectives(const FHorrorCampaignChapterDefinition& Chapter)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const TSubclassOf<AHorrorCampaignObjectiveActor> SpawnClass = RuntimeCampaignObjectiveClass
		? RuntimeCampaignObjectiveClass
		: TSubclassOf<AHorrorCampaignObjectiveActor>(AHorrorCampaignObjectiveActor::StaticClass());

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (const FHorrorCampaignObjectiveDefinition& Objective : Chapter.Objectives)
	{
		AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>(
			SpawnClass,
			ResolveCampaignObjectiveTransform(Objective),
			SpawnParameters);
		if (!ObjectiveActor)
		{
			continue;
		}

		ObjectiveActor->ConfigureObjective(Chapter.ChapterId, Objective);
		RuntimeCampaignObjectiveActors.Add(ObjectiveActor);
		RuntimeCampaignObjectiveActorViews.Add(ObjectiveActor);
	}

	RefreshCampaignObjectiveActors();
}

void AHorrorGameModeBase::SpawnCampaignBossIfNeeded(const FHorrorCampaignChapterDefinition& Chapter)
{
	if (!Chapter.bRequiresBoss || RuntimeCampaignBoss)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const TSubclassOf<AHorrorCampaignBossActor> SpawnClass = RuntimeCampaignBossClass
		? RuntimeCampaignBossClass
		: TSubclassOf<AHorrorCampaignBossActor>(AHorrorCampaignBossActor::StaticClass());

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	RuntimeCampaignBoss = World->SpawnActor<AHorrorCampaignBossActor>(
		SpawnClass,
		ResolveCampaignBossTransform(),
		SpawnParameters);
	if (RuntimeCampaignBoss)
	{
		RuntimeCampaignBoss->ConfigureBoss(
			Chapter.ChapterId,
			NSLOCTEXT("HorrorGameMode", "StoneGiant", "石像巨人"),
			FHorrorCampaign::CountObjectivesOfType(Chapter, EHorrorCampaignObjectiveType::BossWeakPoint));
	}
}

void AHorrorGameModeBase::RefreshCampaignObjectiveActors()
{
	for (AHorrorCampaignObjectiveActor* ObjectiveActor : RuntimeCampaignObjectiveActorViews)
	{
		if (ObjectiveActor && !ObjectiveActor->IsActorBeingDestroyed())
		{
			ObjectiveActor->RefreshObjectiveState();
		}
	}
}

void AHorrorGameModeBase::ApplyCampaignObjectiveReward(const FHorrorCampaignObjectiveDefinition& Objective, AActor* InstigatorActor)
{
	if (!Objective.Reward.HasAnyReward())
	{
		return;
	}

	AHorrorPlayerCharacter* PlayerCharacter = Cast<AHorrorPlayerCharacter>(InstigatorActor);
	if (!PlayerCharacter)
	{
		PlayerCharacter = ResolveLeadPlayerCharacter();
	}

	AActor* RewardTarget = PlayerCharacter ? Cast<AActor>(PlayerCharacter) : InstigatorActor;
	if (!RewardTarget)
	{
		return;
	}

	if (!Objective.Reward.EvidenceId.IsNone())
	{
		if (UInventoryComponent* Inventory = RewardTarget->FindComponentByClass<UInventoryComponent>())
		{
			FHorrorEvidenceMetadata Metadata;
			Metadata.EvidenceId = Objective.Reward.EvidenceId;
			Metadata.DisplayName = Objective.Reward.EvidenceDisplayName.IsEmpty()
				? (Objective.PromptText.IsEmpty()
					? NSLOCTEXT("HorrorGameMode", "CampaignRewardEvidenceFallbackName", "任务证据")
					: Objective.PromptText)
				: Objective.Reward.EvidenceDisplayName;
			Metadata.Description = Objective.Reward.EvidenceDescription.IsEmpty()
				? (Objective.CompletionText.IsEmpty()
					? NSLOCTEXT("HorrorGameMode", "CampaignRewardEvidenceFallbackDescription", "这条证据来自已完成的任务。")
					: Objective.CompletionText)
				: Objective.Reward.EvidenceDescription;
			Inventory->RegisterEvidenceMetadata(Metadata);
			Inventory->AddCollectedEvidenceId(Objective.Reward.EvidenceId);
		}
	}

	if (Objective.Reward.FearRelief > 0.0f)
	{
		if (UFearComponent* Fear = RewardTarget->FindComponentByClass<UFearComponent>())
		{
			Fear->RemoveFear(Objective.Reward.FearRelief);
		}
	}

	if (Objective.Reward.BatteryChargePercent > 0.0f)
	{
		if (UQuantumCameraComponent* QuantumCamera = RewardTarget->FindComponentByClass<UQuantumCameraComponent>())
		{
			if (UCameraBatteryComponent* Battery = QuantumCamera->GetBatteryComponent())
			{
				Battery->SetBatteryPercentage(Battery->GetBatteryPercentage() + Objective.Reward.BatteryChargePercent);
			}
		}
		else if (UCameraBatteryComponent* Battery = RewardTarget->FindComponentByClass<UCameraBatteryComponent>())
		{
			Battery->SetBatteryPercentage(Battery->GetBatteryPercentage() + Objective.Reward.BatteryChargePercent);
		}
	}
}

void AHorrorGameModeBase::ShowCampaignMessage(FText Message, FLinearColor Color, float DurationSeconds) const
{
	if (Message.IsEmpty())
	{
		return;
	}

	if (GetWorld())
	{
		if (AHorrorPlayerController* PlayerController = Cast<AHorrorPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
		{
			PlayerController->ShowPlayerMessage(Message, Color, DurationSeconds);
			return;
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, DurationSeconds, Color.ToFColor(true), Message.ToString());
	}
}

void AHorrorGameModeBase::ShowCampaignChapterIntro(const FHorrorCampaignChapterDefinition& Chapter) const
{
	const FText IntroText = Chapter.StoryBrief.IsEmpty()
		? Chapter.Title
		: FText::Format(FText::FromString(TEXT("{0}\n{1}")), Chapter.Title, Chapter.StoryBrief);

	ShowCampaignMessage(IntroText, FLinearColor(0.4f, 0.85f, 1.0f), 5.0f);
}

void AHorrorGameModeBase::ShowCurrentCampaignObjectiveHint() const
{
	const FHorrorCampaignObjectiveDefinition* NextObjective = CampaignProgress.GetNextObjective();
	if (!NextObjective)
	{
		return;
	}

	ShowCampaignMessage(
		FText::Format(
			NSLOCTEXT("HorrorGameMode", "CurrentObjectiveWithBeat", "当前目标：{0}\n{1}"),
			NextObjective->PromptText,
			GetCurrentCampaignObjectiveBeatText()),
		FLinearColor(0.86f, 0.95f, 1.0f),
		3.0f);
}

void AHorrorGameModeBase::QueueCampaignAutoTravelIfNeeded()
{
	if (!bAutoTravelToNextCampaignMapOnChapterComplete || bCampaignAutoTravelQueued)
	{
		return;
	}

	const FHorrorCampaignChapterDefinition* Chapter = GetCurrentCampaignChapterDefinition();
	if (!Chapter || Chapter->bIsFinalChapter)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FString NextMapPackageName;
	if (!FHorrorMapChain::TryGetNextMapPackage(World->GetPackage()->GetName(), NextMapPackageName))
	{
		return;
	}

	bCampaignAutoTravelQueued = true;
	ShowCampaignMessage(
		Chapter->CompletionBridgeText.IsEmpty()
			? NSLOCTEXT("HorrorGameMode", "TravelingToNextMap", "信号稳定，正在前往下一张地图...")
			: Chapter->CompletionBridgeText,
		FLinearColor(0.5f, 0.85f, 1.0f),
		CampaignAutoTravelDelaySeconds);

	World->GetTimerManager().SetTimer(
		CampaignAutoTravelTimerHandle,
		this,
		&AHorrorGameModeBase::ExecuteCampaignAutoTravel,
		FMath::Max(0.05f, CampaignAutoTravelDelaySeconds),
		false);
}

void AHorrorGameModeBase::ExecuteCampaignAutoTravel()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		bCampaignAutoTravelQueued = false;
		return;
	}

	FString NextMapPackageName;
	if (!FHorrorMapChain::TryGetNextMapPackage(World->GetPackage()->GetName(), NextMapPackageName))
	{
		bCampaignAutoTravelQueued = false;
		return;
	}

	const FString TravelOptions = FHorrorMapChain::BuildTravelOptionsForConfiguredGameMode();
	UGameplayStatics::OpenLevel(World, FName(*NextMapPackageName), true, TravelOptions);
}

void AHorrorGameModeBase::SanitizeImportedMapChainRuntime()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FString CurrentMapPackageName = FHorrorMapChain::NormalizeMapPackageName(World->GetPackage()->GetName());
	const int32 CurrentMapIndex = FHorrorMapChain::FindMapIndex(CurrentMapPackageName);
	if (CurrentMapIndex <= 0)
	{
		return;
	}

	StopImportedMapShowcaseSequences();
	RestoreLeadPlayerViewAndInput();

	World->GetTimerManager().SetTimerForNextTick(this, &AHorrorGameModeBase::StopImportedMapShowcaseSequences);
	World->GetTimerManager().SetTimerForNextTick(this, &AHorrorGameModeBase::RestoreLeadPlayerViewAndInput);

	FTimerHandle DelayedSequenceCleanupTimer;
	World->GetTimerManager().SetTimer(DelayedSequenceCleanupTimer, this, &AHorrorGameModeBase::StopImportedMapShowcaseSequences, 0.35f, false);

	FTimerHandle DelayedPlayerViewRestoreTimer;
	World->GetTimerManager().SetTimer(DelayedPlayerViewRestoreTimer, this, &AHorrorGameModeBase::RestoreLeadPlayerViewAndInput, 0.35f, false);
}

void AHorrorGameModeBase::StopImportedMapShowcaseSequences()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ALevelSequenceActor> It(World); It; ++It)
	{
		ALevelSequenceActor* SequenceActor = *It;
		ULevelSequencePlayer* SequencePlayer = SequenceActor ? SequenceActor->GetSequencePlayer() : nullptr;
		if (!SequencePlayer)
		{
			continue;
		}

		if (SequencePlayer->IsPlaying())
		{
			SequencePlayer->Stop();
			UE_LOG(LogHorrorProject, Log, TEXT("Stopped imported map showcase sequence %s so player view stays possessed."),
				*GetNameSafe(SequenceActor));
		}
	}
}

void AHorrorGameModeBase::RestoreLeadPlayerViewAndInput()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameplayStatics::SetGamePaused(World, false);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	if (!PlayerController)
	{
		return;
	}

	PlayerController->SetIgnoreMoveInput(false);
	PlayerController->SetIgnoreLookInput(false);
	PlayerController->SetInputMode(FInputModeGameOnly());
	PlayerController->bShowMouseCursor = false;
	ClearImportedMapCameraFade(PlayerController);

	if (APawn* PossessedPawn = PlayerController->GetPawn())
	{
		PlayerController->SetViewTarget(PossessedPawn);
	}
}

void AHorrorGameModeBase::RegisterDefaultObjectiveMetadata()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	if (!EventBus)
	{
		return;
	}

	const FHorrorGameModeDefaultMilestoneMetadata Defaults[] = {
		{ HorrorFoundFootageTags::BodycamAcquiredEvent(), HorrorObjectiveMilestoneSources::Bodycam, TEXT("Trailer.Beat.BodycamAcquired"), TEXT("随身摄像机已上线，请保持举起。"), TEXT("已取得随身摄像机") },
		{ HorrorFoundFootageTags::FirstNoteCollectedEvent(), HorrorObjectiveMilestoneSources::FirstNote, TEXT("Trailer.Beat.FirstNote"), TEXT("备忘录已记录，寻找第一个异常。"), TEXT("第一份备忘录已记录") },
		{ HorrorFoundFootageTags::FirstAnomalyRecordedEvent(), HorrorObjectiveMilestoneSources::FirstAnomaly, TEXT("Trailer.Beat.FirstAnomaly"), TEXT("异常已捕捉，前往档案终端。"), TEXT("第一异常已记录") },
		{ HorrorFoundFootageTags::ArchiveReviewedEvent(), HorrorObjectiveMilestoneSources::Archive, TEXT("Trailer.Beat.ArchiveReviewed"), TEXT("档案复查完成，出口正在解锁。"), TEXT("档案已复查") },
		{ HorrorFoundFootageTags::ExitUnlockedEvent(), HorrorObjectiveMilestoneSources::Exit, TEXT("Trailer.Beat.ExitUnlocked"), TEXT("出口已解锁，立刻离开。"), TEXT("出口已解锁") },
		{ HorrorDay1Tags::Day1CompletedEvent(), HorrorObjectiveMilestoneSources::Day1Complete, TEXT("Trailer.Beat.Day1Complete"), TEXT("第一天完成。"), TEXT("第一天完成") }
	};

	for (const FHorrorGameModeDefaultMilestoneMetadata& Default : Defaults)
	{
		RegisterHorrorGameModeMilestoneMetadata(*EventBus, Default);
	}
}

void AHorrorGameModeBase::HandleObjectiveStateChange(FGameplayTag StateTag)
{
	if (!StateTag.IsValid())
	{
		return;
	}

	HandleEncounterObjectiveStateChange(StateTag);
	HandleAutosaveObjectiveStateChange(StateTag);
}

void AHorrorGameModeBase::HandleEncounterObjectiveStateChange(FGameplayTag StateTag)
{
	if (AHorrorEncounterDirector* EncounterDirector = EnsureEncounterDirector())
	{
		if (StateTag == HorrorFoundFootageTags::FirstNoteCollectedState())
		{
			EncounterDirector->PrimeEncounter(NAME_None);
		}
		else if (StateTag == HorrorFoundFootageTags::ArchiveReviewedState())
		{
			EncounterDirector->ResolveEncounter();
		}
	}
}

void AHorrorGameModeBase::HandleAutosaveObjectiveStateChange(FGameplayTag StateTag)
{
	const FName CheckpointId = GetHorrorGameModeMilestoneCheckpoint(StateTag);
	if (!CheckpointId.IsNone())
	{
		TryAutosaveOnMilestone(CheckpointId);
	}
}

void AHorrorGameModeBase::RestoreEncounterStateFromFoundFootageCheckpoint()
{
	AHorrorEncounterDirector* EncounterDirector = EnsureEncounterDirector();
	if (!EncounterDirector)
	{
		return;
	}

	EHorrorEncounterPhase RestoredPhase = EHorrorEncounterPhase::Dormant;
	if (HasReviewedArchive() || IsExitUnlocked() || IsDay1Complete())
	{
		RestoredPhase = EHorrorEncounterPhase::Resolved;
	}
	else if (HasCollectedFirstNote())
	{
		RestoredPhase = EHorrorEncounterPhase::Primed;
	}

	EncounterDirector->RestoreForCheckpoint(
		RestoredPhase,
		RestoredPhase == EHorrorEncounterPhase::Dormant ? NAME_None : EncounterDirector->DefaultEncounterId,
		ResolveLeadPlayerCharacter());
}

void AHorrorGameModeBase::TryAutosaveOnMilestone(FName CheckpointId)
{
	if (!bAutosaveOnObjectiveMilestone || CheckpointId.IsNone())
	{
		return;
	}

	const bool bSaved = SaveDay1Checkpoint(CheckpointId);
	if (UWorld* World = GetWorld())
	{
		if (UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>())
		{
			EventBus->Publish(
				bSaved ? HorrorSaveTags::CheckpointSavedEvent() : HorrorSaveTags::CheckpointSaveFailedEvent(),
				CheckpointId,
				bSaved ? HorrorSaveTags::CheckpointSavedState() : HorrorSaveTags::CheckpointSaveFailedState(),
				this);
		}
	}
}
