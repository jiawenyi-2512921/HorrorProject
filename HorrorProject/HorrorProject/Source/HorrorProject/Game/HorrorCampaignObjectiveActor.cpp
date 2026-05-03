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
#include "GameFramework/Pawn.h"
#include "GameplayTagContainer.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Player/HorrorPlayerController.h"

namespace
{
	const FVector ObjectiveBoundsExtent(75.0f, 75.0f, 95.0f);
	const TCHAR* DefaultObjectiveMeshPath = TEXT("/Engine/BasicShapes/Cube.Cube");
	const TCHAR* DefaultObjectiveVFXPath = TEXT("/Game/Fantastic_Dungeon_Pack/effects/PS_FX_particles_dungeon_01_Niagara.PS_FX_particles_dungeon_01_Niagara");
	constexpr float DefaultSurvivalObjectiveDurationSeconds = 14.0f;
	constexpr float MinimumEscapeDestinationOffsetCm = 100.0f;
	constexpr float MaximumEscapeDestinationOffsetCm = 3600.0f;
	constexpr float MinimumEscapeDestinationZOffsetCm = -450.0f;
	constexpr float MaximumEscapeDestinationZOffsetCm = 450.0f;
	constexpr float EscapeDestinationGroundTraceUpCm = 1000.0f;
	constexpr float EscapeDestinationGroundTraceDownCm = 2200.0f;
	constexpr float EscapeDestinationGroundLiftCm = 95.0f;
	constexpr float EscapeDestinationClearanceRingStepCm = 160.0f;
	constexpr int32 EscapeDestinationClearanceRingCount = 8;
	constexpr int32 EscapeDestinationClearanceDirections = 12;
	constexpr float EscapeDestinationObstacleTopRejectDeltaCm = 160.0f;
	constexpr float ConservativePursuitEscapeSpeedCmPerSecond = 450.0f;
	constexpr float EscapeBudgetCriticalSeconds = 2.0f;
	constexpr float EscapeBudgetThinSeconds = 4.0f;
	constexpr float AdvancedInteractionSuccessProgress = 0.34f;
	constexpr float CircuitFailureProgressPenalty = 0.2f;
	constexpr float GearFailurePauseSeconds = 3.0f;
	constexpr float AdvancedInteractionCueCycleSeconds = 1.6f;
	constexpr float AdvancedInteractionTimingWindowStart = 0.36f;
	constexpr float AdvancedInteractionTimingWindowEnd = 0.72f;
	constexpr float CircuitFailureStabilityDamage = 0.18f;
	constexpr float GearFailureStabilityDamage = 0.24f;
	constexpr float AdvancedSuccessStabilityRecovery = 0.06f;
	constexpr float SignalTuningBalanceStep = 0.22f;
	constexpr float SignalTuningLockTolerance = 0.12f;
	constexpr float SignalTuningInitialLeftBalance = 0.28f;
	constexpr float SignalTuningInitialRightBalance = 0.72f;
	constexpr float SpectralScanMinimumConfidenceToLock = 0.40f;  // 从0.62降到0.40，更容易达到
	constexpr float SpectralScanConfidenceGainPerSecond = 1.5f;   // 从0.75提高到1.5，更快积累
	constexpr float SpectralScanNoiseDecayPerSecond = 0.32f;
	constexpr float SpectralScanFilterStep = 0.26f;
	constexpr float SpectralScanFilterLockTolerance = 0.28f;      // 从0.16放宽到0.28，更容易对齐
	constexpr float SpectralScanInitialLowFocus = 0.24f;
	constexpr float SpectralScanInitialHighFocus = 0.76f;

	const FName CircuitInputs[] = {
		TEXT("蓝色端子"),
		TEXT("红色端子"),
		TEXT("黄色端子")
	};

	const FName CircuitSafeRoutingSequence[] = {
		TEXT("蓝色端子"),
		TEXT("黄色端子")
	};

	bool IsCircuitInputSolvedInRoute(FName InputId, int32 StepIndex)
	{
		const int32 SolvedStepCount = FMath::Clamp(StepIndex, 0, UE_ARRAY_COUNT(CircuitSafeRoutingSequence));
		for (int32 SolvedStepIndex = 0; SolvedStepIndex < SolvedStepCount; ++SolvedStepIndex)
		{
			if (CircuitSafeRoutingSequence[SolvedStepIndex] == InputId)
			{
				return true;
			}
		}
		return false;
	}

	int32 IndexOfAdvancedInput(const FName* Inputs, int32 InputCount, FName InputId)
	{
		for (int32 InputIndex = 0; InputIndex < InputCount; ++InputIndex)
		{
			if (Inputs[InputIndex] == InputId)
			{
				return InputIndex;
			}
		}

		return INDEX_NONE;
	}

	const FName GearInputs[] = {
		TEXT("齿轮1"),
		TEXT("齿轮2"),
		TEXT("齿轮3")
	};

	const FName SpectralScanInputs[] = {
		TEXT("低频波段"),
		TEXT("中频波段"),
		TEXT("高频波段")
	};

	const FName SignalTuningInputs[] = {
		TEXT("左声道"),
		TEXT("中心频率"),
		TEXT("右声道")
	};

	const FName SignalTuningLeftInput(TEXT("左声道"));
	const FName SignalTuningConfirmInput(TEXT("中心频率"));
	const FName SignalTuningRightInput(TEXT("右声道"));
	const FName SpectralScanSweepLeftInput(TEXT("低频波段"));
	const FName SpectralScanLockInput(TEXT("中频波段"));
	const FName SpectralScanSweepRightInput(TEXT("高频波段"));
	const FName CircuitHazardInput(TEXT("红色端子"));

	const FName GearCalibrationSequence[] = {
		TEXT("齿轮2"),
		TEXT("齿轮1"),
		TEXT("齿轮3")
	};

	int32 ResolveStableObjectiveVariantProfile(FName ObjectiveId)
	{
		const FString ObjectiveIdText = ObjectiveId.ToString();
		uint32 StableHash = 0;
		for (int32 CharacterIndex = 0; CharacterIndex < ObjectiveIdText.Len(); ++CharacterIndex)
		{
			StableHash ^= static_cast<uint32>(CharacterIndex + 1) * static_cast<uint32>(ObjectiveIdText[CharacterIndex]);
			StableHash = (StableHash << 5) | (StableHash >> 27);
		}
		return static_cast<int32>(StableHash % 2);
	}

	int32 ResolveStableObjectiveBiasProfile(FName ObjectiveId)
	{
		const FString ObjectiveIdText = ObjectiveId.ToString();
		int32 CharacterSum = 0;
		for (const TCHAR Character : ObjectiveIdText)
		{
			CharacterSum += static_cast<int32>(Character);
		}
		return CharacterSum % 2;
	}

	bool IsGearInputSolvedInChain(FName InputId, int32 StepIndex, FName ObjectiveId)
	{
		const int32 SolvedStepCount = FMath::Clamp(StepIndex, 0, UE_ARRAY_COUNT(GearCalibrationSequence));
		for (int32 SolvedStepIndex = 0; SolvedStepIndex < SolvedStepCount; ++SolvedStepIndex)
		{
			FName SolvedInput = GearCalibrationSequence[0];
			if (SolvedStepIndex > 0)
			{
				const int32 SideGearIndex = (ResolveStableObjectiveVariantProfile(ObjectiveId) + SolvedStepIndex - 1) % 2;
				const FName FirstSideGear = SideGearIndex == 0 ? GearCalibrationSequence[1] : GearCalibrationSequence[2];
				const FName SecondSideGear = SideGearIndex == 0 ? GearCalibrationSequence[2] : GearCalibrationSequence[1];
				SolvedInput = (SolvedStepIndex % 2) == 1 ? FirstSideGear : SecondSideGear;
			}

			if (SolvedInput == InputId)
			{
				return true;
			}
		}
		return false;
	}

	const FText AdvancedDeviceKeyHints[] = {
		NSLOCTEXT("HorrorCampaignObjective", "AdvancedInputKeyA", "A"),
		NSLOCTEXT("HorrorCampaignObjective", "AdvancedInputKeyS", "S"),
		NSLOCTEXT("HorrorCampaignObjective", "AdvancedInputKeyD", "D")
	};

	FLinearColor ResolveAdvancedInputVisualColor(EHorrorCampaignInteractionMode InteractionMode, int32 InputIndex)
	{
		if (InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring)
		{
			const FLinearColor CircuitColors[] = {
				FLinearColor(0.24f, 0.68f, 1.0f, 0.96f),
				FLinearColor(1.0f, 0.18f, 0.10f, 0.96f),
				FLinearColor(1.0f, 0.86f, 0.22f, 0.96f)
			};
			return CircuitColors[FMath::Clamp(InputIndex, 0, UE_ARRAY_COUNT(CircuitColors) - 1)];
		}

		if (InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
		{
			const FLinearColor ScanColors[] = {
				FLinearColor(0.36f, 0.78f, 1.0f, 0.96f),
				FLinearColor(0.34f, 1.0f, 0.68f, 0.96f),
				FLinearColor(0.92f, 0.54f, 1.0f, 0.96f)
			};
			return ScanColors[FMath::Clamp(InputIndex, 0, UE_ARRAY_COUNT(ScanColors) - 1)];
		}

		if (InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
		{
			const FLinearColor TuningColors[] = {
				FLinearColor(0.58f, 0.82f, 1.0f, 0.96f),
				FLinearColor(1.0f, 0.88f, 0.34f, 0.96f),
				FLinearColor(0.38f, 1.0f, 0.84f, 0.96f)
			};
			return TuningColors[FMath::Clamp(InputIndex, 0, UE_ARRAY_COUNT(TuningColors) - 1)];
		}

		const FLinearColor GearColors[] = {
			FLinearColor(0.92f, 0.74f, 0.52f, 0.96f),
			FLinearColor(0.78f, 0.86f, 0.92f, 0.96f),
			FLinearColor(0.96f, 0.60f, 0.34f, 0.96f)
		};
		return GearColors[FMath::Clamp(InputIndex, 0, UE_ARRAY_COUNT(GearColors) - 1)];
	}

	FVector SanitizeEscapeDestinationOffset(const FVector& AuthoredOffset)
	{
		FVector SafeOffset = AuthoredOffset;
		if (!FMath::IsFinite(SafeOffset.X) || !FMath::IsFinite(SafeOffset.Y) || !FMath::IsFinite(SafeOffset.Z) || SafeOffset.ContainsNaN())
		{
			return FVector::ZeroVector;
		}

		FVector HorizontalOffset(SafeOffset.X, SafeOffset.Y, 0.0f);
		const float HorizontalDistance = HorizontalOffset.Size();
		if (HorizontalDistance > MaximumEscapeDestinationOffsetCm)
		{
			HorizontalOffset = HorizontalOffset.GetSafeNormal(UE_SMALL_NUMBER, FVector::ForwardVector) * MaximumEscapeDestinationOffsetCm;
			SafeOffset.X = HorizontalOffset.X;
			SafeOffset.Y = HorizontalOffset.Y;
		}
		SafeOffset.Z = FMath::Clamp(SafeOffset.Z, MinimumEscapeDestinationZOffsetCm, MaximumEscapeDestinationZOffsetCm);
		return SafeOffset;
	}

	bool TryProjectEscapeDestinationToGround(UWorld& World, const FVector& AuthoredLocation, FVector& OutLocation)
	{
		const FVector TraceStart = AuthoredLocation + FVector(0.0f, 0.0f, EscapeDestinationGroundTraceUpCm);
		const FVector TraceEnd = AuthoredLocation - FVector(0.0f, 0.0f, EscapeDestinationGroundTraceDownCm);
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HorrorCampaignEscapeDestinationGroundTrace), false);
		FHitResult Hit;
		if (!World.LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams) || !Hit.bBlockingHit)
		{
			return false;
		}

		OutLocation = Hit.ImpactPoint + FVector(0.0f, 0.0f, EscapeDestinationGroundLiftCm);
		return true;
	}

	bool IsEscapeDestinationLocationClear(UWorld& World, const FVector& Location)
	{
		const FCollisionShape EscapeMarkerShape = FCollisionShape::MakeBox(FVector(85.0f, 85.0f, 85.0f));
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HorrorCampaignEscapeDestinationClearance), false);
		return !World.OverlapBlockingTestByChannel(
			Location + FVector(0.0f, 0.0f, 85.0f),
			FQuat::Identity,
			ECC_WorldStatic,
			EscapeMarkerShape,
			QueryParams);
	}

	bool TryProjectClearEscapeDestinationLocation(UWorld& World, const FVector& CandidateLocation, FVector& OutLocation)
	{
		FVector GroundedLocation = CandidateLocation;
		if (TryProjectEscapeDestinationToGround(World, CandidateLocation, GroundedLocation))
		{
			if (GroundedLocation.Z > CandidateLocation.Z + EscapeDestinationObstacleTopRejectDeltaCm)
			{
				return false;
			}

			if (IsEscapeDestinationLocationClear(World, GroundedLocation))
			{
				OutLocation = GroundedLocation;
				return true;
			}
			return false;
		}

		if (IsEscapeDestinationLocationClear(World, CandidateLocation))
		{
			OutLocation = CandidateLocation;
			return true;
		}
		return false;
	}

	bool TryFindClearEscapeDestinationLocation(UWorld& World, const FVector& DesiredLocation, FVector& OutLocation)
	{
		if (TryProjectClearEscapeDestinationLocation(World, DesiredLocation, OutLocation))
		{
			return true;
		}

		for (int32 RingIndex = 1; RingIndex <= EscapeDestinationClearanceRingCount; ++RingIndex)
		{
			const float Radius = EscapeDestinationClearanceRingStepCm * RingIndex;
			for (int32 DirectionIndex = 0; DirectionIndex < EscapeDestinationClearanceDirections; ++DirectionIndex)
			{
				const float AngleRadians = UE_TWO_PI * static_cast<float>(DirectionIndex) / static_cast<float>(EscapeDestinationClearanceDirections);
				const FVector Candidate = DesiredLocation + FVector(FMath::Cos(AngleRadians) * Radius, FMath::Sin(AngleRadians) * Radius, 0.0f);
				if (TryProjectClearEscapeDestinationLocation(World, Candidate, OutLocation))
				{
					return true;
				}
			}
		}

		return false;
	}

	bool HasDirectEscapeRouteSightline(
		UWorld& World,
		const FVector& StartLocation,
		const FVector& DestinationLocation,
		const AActor* IgnoredActor,
		const AActor* IgnoredInstigator)
	{
		const FVector TraceStart = StartLocation + FVector(0.0f, 0.0f, 60.0f);
		const FVector TraceEnd = DestinationLocation + FVector(0.0f, 0.0f, 60.0f);
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(HorrorCampaignEscapeDestinationSightline), false);
		if (IgnoredActor)
		{
			QueryParams.AddIgnoredActor(IgnoredActor);
		}
		if (IgnoredInstigator)
		{
			QueryParams.AddIgnoredActor(IgnoredInstigator);
		}

		FHitResult Hit;
		return !World.LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams)
			|| !Hit.bBlockingHit;
	}

	bool TryFindReachableEscapeDestinationLocation(
		UWorld& World,
		const FVector& DesiredLocation,
		const FVector& RouteStartLocation,
		const AActor* IgnoredActor,
		const AActor* IgnoredInstigator,
		FVector& OutLocation)
	{
		FVector CandidateLocation = DesiredLocation;
		if (TryProjectClearEscapeDestinationLocation(World, DesiredLocation, CandidateLocation)
			&& HasDirectEscapeRouteSightline(World, RouteStartLocation, CandidateLocation, IgnoredActor, IgnoredInstigator))
		{
			OutLocation = CandidateLocation;
			return true;
		}

		for (int32 RingIndex = 1; RingIndex <= EscapeDestinationClearanceRingCount; ++RingIndex)
		{
			const float Radius = EscapeDestinationClearanceRingStepCm * RingIndex;
			for (int32 DirectionIndex = 0; DirectionIndex < EscapeDestinationClearanceDirections; ++DirectionIndex)
			{
				const float AngleRadians = UE_TWO_PI * static_cast<float>(DirectionIndex) / static_cast<float>(EscapeDestinationClearanceDirections);
				const FVector Candidate = DesiredLocation + FVector(FMath::Cos(AngleRadians) * Radius, FMath::Sin(AngleRadians) * Radius, 0.0f);
				if (TryProjectClearEscapeDestinationLocation(World, Candidate, CandidateLocation)
					&& HasDirectEscapeRouteSightline(World, RouteStartLocation, CandidateLocation, IgnoredActor, IgnoredInstigator))
				{
					OutLocation = CandidateLocation;
					return true;
				}
			}
		}

		return false;
	}

	bool HasAuthoredDeliveryOffset(const FVector& AuthoredOffset)
	{
		return SanitizeEscapeDestinationOffset(AuthoredOffset).Size2D() >= MinimumEscapeDestinationOffsetCm;
	}

	FText BuildObjectivePromptWithBeat(
		const FText& BasePrompt,
		const FHorrorCampaignObjectiveBeat* Beat,
		const FText& ProgressText = FText::GetEmpty())
	{
		if (!Beat)
		{
			return ProgressText.IsEmpty()
				? BasePrompt
				: FText::Format(FText::FromString(TEXT("{0} {1}")), BasePrompt, ProgressText);
		}

		const FText BeatLine = Beat->Detail.IsEmpty()
			? Beat->Label
			: FText::Format(FText::FromString(TEXT("{0}：{1}")), Beat->Label, Beat->Detail);
		if (ProgressText.IsEmpty())
		{
			return FText::Format(FText::FromString(TEXT("{0}\n{1}")), BasePrompt, BeatLine);
		}

		return FText::Format(FText::FromString(TEXT("{0} {1}\n{2}")), BasePrompt, ProgressText, BeatLine);
	}

	FGameplayTag AmbushStartedEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.AmbushStarted")), false);
	}

	FGameplayTag ObjectiveFailedEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.ObjectiveFailed")), false);
	}

	FGameplayTag AdvancedCircuitSuccessEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Success")), false);
	}

	FGameplayTag AdvancedCircuitFailureEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Circuit.Failure")), false);
	}

	FGameplayTag AdvancedGearSuccessEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Gear.Success")), false);
	}

	FGameplayTag AdvancedGearFailureEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.Gear.Failure")), false);
	}

	FGameplayTag AdvancedSpectralScanSuccessEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SpectralScan.Success")), false);
	}

	FGameplayTag AdvancedSpectralScanFailureEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SpectralScan.Failure")), false);
	}

	FGameplayTag AdvancedSignalTuningSuccessEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SignalTuning.Success")), false);
	}

	FGameplayTag AdvancedSignalTuningFailureEventTag()
	{
		return FGameplayTag::RequestGameplayTag(FName(TEXT("Event.Campaign.Advanced.SignalTuning.Failure")), false);
	}

	const FName CampaignPursuitTimeoutFailureCause(TEXT("Failure.Campaign.PursuitTimeout"));
	const FName CircuitOverloadFailureCause(TEXT("Failure.Campaign.CircuitOverload"));
	const FName GearOverloadFailureCause(TEXT("Failure.Campaign.GearOverload"));
	const FName SpectralScanOverloadFailureCause(TEXT("Failure.Campaign.SpectralScanOverload"));
	const FName SignalTuningOverloadFailureCause(TEXT("Failure.Campaign.SignalTuningOverload"));
	const FName AdvancedFaultTiming(TEXT("Fault.Advanced.Timing"));
	const FName AdvancedFaultWrongInput(TEXT("Fault.Advanced.WrongInput"));
	const FName AdvancedFaultCircuitHazard(TEXT("Fault.Advanced.CircuitHazard"));
	const FName AdvancedFaultSpectralLowConfidence(TEXT("Fault.Advanced.SpectralLowConfidence"));
	const FName AdvancedFaultSpectralFilter(TEXT("Fault.Advanced.SpectralFilter"));
	const FName AdvancedFaultSpectralWrongControl(TEXT("Fault.Advanced.SpectralWrongControl"));
	const FName AdvancedFaultSignalWrongControl(TEXT("Fault.Advanced.SignalWrongControl"));
	const FName AdvancedFaultSignalBalance(TEXT("Fault.Advanced.SignalBalance"));
	const FName RetryObjectiveRecoveryAction(TEXT("Recovery.Objective.Retry"));
	const FName ReturnToEscapeStartRecoveryAction(TEXT("Recovery.Campaign.ReturnToEscapeStart"));

	FText BuildEscapeBudgetLabel(float EscapeTimeBudgetSeconds)
	{
		if (EscapeTimeBudgetSeconds < EscapeBudgetCriticalSeconds)
		{
			return NSLOCTEXT("HorrorCampaignObjective", "RuntimePursuitBudgetCritical", "逃生窗口：危险，余量不足");
		}
		if (EscapeTimeBudgetSeconds < EscapeBudgetThinSeconds)
		{
			return NSLOCTEXT("HorrorCampaignObjective", "RuntimePursuitBudgetThin", "逃生窗口：紧张，不能绕路");
		}

		return NSLOCTEXT("HorrorCampaignObjective", "RuntimePursuitBudgetStable", "逃生窗口：可达，保持路线");
	}

	FText BuildEscapeActionLabel(float EscapeDistanceMeters, float EstimatedArrivalSeconds, float EscapeTimeBudgetSeconds)
	{
		const FText TacticalCue = EscapeTimeBudgetSeconds < EscapeBudgetCriticalSeconds
			? NSLOCTEXT("HorrorCampaignObjective", "RuntimePursuitEscapeActionCriticalCue", "切内线冲刺，不要回头")
			: EscapeTimeBudgetSeconds < EscapeBudgetThinSeconds
			? NSLOCTEXT("HorrorCampaignObjective", "RuntimePursuitEscapeActionThinCue", "贴近亮点路线，少转弯")
			: NSLOCTEXT("HorrorCampaignObjective", "RuntimePursuitEscapeActionStableCue", "保持亮点路线推进");
		return FText::Format(
			NSLOCTEXT("HorrorCampaignObjective", "RuntimePursuitEscapeAction", "冲刺 {0} 米，预计 {1} 秒到达，余量 {2} 秒；{3}"),
			FText::AsNumber(FMath::Max(0, FMath::RoundToInt(EscapeDistanceMeters))),
			FText::AsNumber(FMath::Max(1, FMath::CeilToInt(EstimatedArrivalSeconds))),
			FText::AsNumber(FMath::FloorToInt(EscapeTimeBudgetSeconds)),
			TacticalCue);
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
			case EHorrorCampaignObjectiveType::ScanAnomaly:
				return EHorrorCampaignInteractionMode::SpectralScan;
			case EHorrorCampaignObjectiveType::AcquireSignal:
				return EHorrorCampaignInteractionMode::SignalTuning;
			case EHorrorCampaignObjectiveType::SurviveAmbush:
				return EHorrorCampaignInteractionMode::TimedPursuit;
			case EHorrorCampaignObjectiveType::PlantBeacon:
				return EHorrorCampaignInteractionMode::SignalTuning;
			case EHorrorCampaignObjectiveType::BossWeakPoint:
				return EHorrorCampaignInteractionMode::SpectralScan;
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
	PrimaryActorTick.bStartWithTickEnabled = false;
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
	if (IsCarryReturnRelicObjective())
	{
		ObjectiveDefinition.InteractionMode = EHorrorCampaignInteractionMode::MultiStep;
		ObjectiveDefinition.Presentation.bOpensInteractionPanel = false;
		if (ObjectiveDefinition.Presentation.MechanicLabel.IsEmpty()
			|| ObjectiveDefinition.Presentation.MechanicLabel.ToString().Contains(TEXT("窗口")))
		{
			ObjectiveDefinition.Presentation.MechanicLabel = NSLOCTEXT("HorrorCampaignObjective", "RelicCarryMechanicLabel", "遗物搬运回锚点");
		}
		if (ObjectiveDefinition.Presentation.InputHint.IsEmpty()
			|| ObjectiveDefinition.Presentation.InputHint.ToString().Contains(TEXT("窗口")))
		{
			ObjectiveDefinition.Presentation.InputHint = NSLOCTEXT("HorrorCampaignObjective", "RelicCarryInputHint", "第一次互动取出遗物，沿导航把它送回发光锚点，再次互动完成归档。");
		}
		if (ObjectiveDefinition.Presentation.FailureStakes.IsEmpty()
			|| ObjectiveDefinition.Presentation.FailureStakes.ToString().Contains(TEXT("窗口")))
		{
			ObjectiveDefinition.Presentation.FailureStakes = NSLOCTEXT("HorrorCampaignObjective", "RelicCarryFailureStakes", "搬运途中后续路线保持锁定，遗物未送回前不会归档。");
		}
		ObjectiveDefinition.Presentation.RiskLevel = FMath::Max(
			ObjectiveDefinition.Presentation.RiskLevel,
			EHorrorCampaignObjectiveRiskLevel::High);
	}
	if (ObjectiveDefinition.ObjectiveBeats.IsEmpty())
	{
		FHorrorCampaignObjectiveBeat FallbackBeat;
		FallbackBeat.Label = ObjectiveDefinition.PromptText.IsEmpty()
			? NSLOCTEXT("HorrorCampaignObjective", "FallbackBeatLabel", "执行目标")
			: ObjectiveDefinition.PromptText;
		FallbackBeat.Detail = NSLOCTEXT("HorrorCampaignObjective", "FallbackBeatDetail", "完成当前任务并等待系统确认。");
		FallbackBeat.CompletionRule = UsesAdvancedInteraction()
			? EHorrorCampaignObjectiveBeatCompletionRule::AdvancedWindow
			: (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::TimedPursuit
				? EHorrorCampaignObjectiveBeatCompletionRule::ReachEscapePoint
				: EHorrorCampaignObjectiveBeatCompletionRule::InteractOnce);
		FallbackBeat.FailurePolicy = UsesAdvancedInteraction()
			? EHorrorCampaignObjectiveBeatFailurePolicy::RetryBeat
			: (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::TimedPursuit
				? EHorrorCampaignObjectiveBeatFailurePolicy::CampaignRecovery
				: EHorrorCampaignObjectiveBeatFailurePolicy::None);
		FallbackBeat.NavigationRole = ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::TimedPursuit
			? EHorrorCampaignObjectiveBeatNavigationRole::EscapeDestination
			: EHorrorCampaignObjectiveBeatNavigationRole::ObjectiveActor;
		FallbackBeat.bOpensInteractionPanel = UsesAdvancedInteraction();
		FallbackBeat.bUrgent = ObjectiveDefinition.ObjectiveType == EHorrorCampaignObjectiveType::SurviveAmbush
			|| ObjectiveDefinition.ObjectiveType == EHorrorCampaignObjectiveType::BossWeakPoint;
		ObjectiveDefinition.ObjectiveBeats.Add(FallbackBeat);
	}
	bCompleted = false;
	RequiredInteractionCount = FMath::Max(1, GetDefaultInteractionCountForObjective(ObjectiveDefinition.ObjectiveType));
	if (!ObjectiveDefinition.ObjectiveBeats.IsEmpty())
	{
		RequiredInteractionCount = FMath::Max(RequiredInteractionCount, ObjectiveDefinition.ObjectiveBeats.Num());
	}
	if (UsesAdvancedInteraction() && ObjectiveDefinition.AdvancedInteractionTuning.RequiredStepCount > 0)
	{
		RequiredInteractionCount = FMath::Max(1, ObjectiveDefinition.AdvancedInteractionTuning.RequiredStepCount);
	}
	if (IsCarryReturnRelicObjective())
	{
		RequiredInteractionCount = FMath::Max(2, RequiredInteractionCount);
	}
	InteractionProgressCount = 0;
	ObjectiveFailureAttemptCount = 0;
	bObjectiveFailedRetryable = false;
	LastObjectiveFailureCause = NAME_None;
	LastObjectiveRecoveryAction = NAME_None;
	ResetAdvancedInteractionState();
	bTimedObjectiveActive = false;
	TimedObjectiveDurationSeconds = ObjectiveDefinition.TimedObjectiveDurationSeconds > 0.0f
		? FMath::Max(0.1f, ObjectiveDefinition.TimedObjectiveDurationSeconds)
		: GetDefaultTimedDurationForObjective(ObjectiveDefinition.ObjectiveType);
	TimedObjectiveRemainingSeconds = TimedObjectiveDurationSeconds;
	bTimedObjectiveUsesEscapeDestination = false;
	TimedObjectiveEscapeDestinationLocation = FVector::ZeroVector;
	bRecoverRelicAwaitingDelivery = false;
	RecoverRelicDeliveryLocation = FVector::ZeroVector;
	bHasTimedObjectiveOriginTransform = false;
	TimedObjectiveOriginTransform = FTransform::Identity;
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
		bObjectiveFailedRetryable = false;
		LastObjectiveFailureCause = NAME_None;
		LastObjectiveRecoveryAction = NAME_None;
		StartTimedObjective(InstigatorActor);
		return true;
	}

	if (UsesAdvancedInteraction())
	{
		if (!bAdvancedInteractionActive)
		{
			bObjectiveFailedRetryable = false;
			LastObjectiveFailureCause = NAME_None;
			LastObjectiveRecoveryAction = NAME_None;
			StartAdvancedInteraction();
			AController* InstigatorController = nullptr;
			if (const APawn* InstigatorPawn = Cast<APawn>(InstigatorActor))
			{
				InstigatorController = InstigatorPawn->GetController();
			}
			if (!InstigatorController && InstigatorActor)
			{
				InstigatorController = InstigatorActor->GetInstigatorController();
			}
			if (AHorrorPlayerController* PlayerController = Cast<AHorrorPlayerController>(InstigatorController))
			{
				PlayerController->NotifyAdvancedInteractionObjectiveOpened(this);
			}
			return true;
		}

		return PromptForExplicitAdvancedInteractionSelection();
	}

	if (IsCarryReturnRelicObjective())
	{
		if (!bRecoverRelicAwaitingDelivery)
		{
			return BeginRecoverRelicDelivery(InstigatorActor);
		}

		return CompleteObjective(InstigatorActor);
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
	RefreshRuntimeTickState();

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
	bAvailableForInteraction = !HorrorGameMode
		|| !IsAuthoredCampaignObjective()
		|| HorrorGameMode->CanCompleteCampaignObjective(ChapterId, ObjectiveDefinition.ObjectiveId);
	bPresentationVisible = bAvailableForInteraction || bCompleted || bTimedObjectiveActive || bAdvancedInteractionActive || bObjectiveFailedRetryable;
	ApplyObjectiveVisuals();
}

void AHorrorCampaignObjectiveActor::RefreshRuntimeTickState()
{
	SetActorTickEnabled(!bCompleted && (bAdvancedInteractionActive || bTimedObjectiveActive));
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

	if (bRecoverRelicAwaitingDelivery)
	{
		return FText::Format(
			NSLOCTEXT("HorrorCampaignObjective", "RelicDeliveryPrompt", "互动键：送回锚点并归档  ({0} 米)"),
			FText::AsNumber(FMath::Max(0, FMath::CeilToInt(ResolveRecoverRelicDeliveryDistanceMeters()))));
	}

	if (!bAvailableForInteraction)
	{
		return BuildObjectiveLockReasonText();
	}

	const FText BasePrompt = ObjectiveDefinition.PromptText.IsEmpty()
		? NSLOCTEXT("HorrorCampaignObjective", "InteractDefault", "互动")
		: ObjectiveDefinition.PromptText;
	const FHorrorCampaignObjectiveBeat* Beat = GetCurrentObjectiveBeat();

	return RequiredInteractionCount > 1
		? BuildObjectivePromptWithBeat(
			FText::Format(NSLOCTEXT("HorrorCampaignObjective", "InteractMultiBase", "互动键：{0}"), BasePrompt),
			Beat,
			FText::Format(
				NSLOCTEXT("HorrorCampaignObjective", "InteractMultiProgress", "({0}/{1})"),
				FText::AsNumber(InteractionProgressCount),
				FText::AsNumber(RequiredInteractionCount)))
		: BuildObjectivePromptWithBeat(
			FText::Format(NSLOCTEXT("HorrorCampaignObjective", "InteractSingleFormat", "互动键：{0}"), BasePrompt),
			Beat);
}

FText AHorrorCampaignObjectiveActor::BuildObjectiveLockReasonText() const
{
	const UWorld* World = GetWorld();
	const AHorrorGameModeBase* HorrorGameMode = World ? World->GetAuthGameMode<AHorrorGameModeBase>() : nullptr;
	return HorrorGameMode && IsAuthoredCampaignObjective()
		? HorrorGameMode->BuildCampaignObjectiveLockReasonText(ChapterId, ObjectiveDefinition.ObjectiveId)
		: NSLOCTEXT("HorrorCampaignObjective", "LockedCompleteCurrent", "已锁定：请先完成当前目标");
}

FText AHorrorCampaignObjectiveActor::BuildMultiStepPhaseText() const
{
	const FHorrorCampaignObjectiveBeat* Beat = GetCurrentObjectiveBeat();
	const int32 StepDisplay = FMath::Clamp(InteractionProgressCount + 1, 1, FMath::Max(1, RequiredInteractionCount));
	const FText StepText = FText::Format(
		NSLOCTEXT("HorrorCampaignObjective", "MultiStepPhaseStep", "阶段 {0}/{1}"),
		FText::AsNumber(StepDisplay),
		FText::AsNumber(FMath::Max(1, RequiredInteractionCount)));
	if (!Beat)
	{
		return StepText;
	}

	if (!Beat->Detail.IsEmpty())
	{
		return FText::Format(
			NSLOCTEXT("HorrorCampaignObjective", "MultiStepPhaseWithDetail", "{0}：{1}，{2}"),
			StepText,
			Beat->Label,
			Beat->Detail);
	}

	return FText::Format(
		NSLOCTEXT("HorrorCampaignObjective", "MultiStepPhaseWithLabel", "{0}：{1}"),
		StepText,
		Beat->Label);
}

FText AHorrorCampaignObjectiveActor::BuildStandardInteractionDeviceStatusLabel() const
{
	if (bCompleted)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "StandardDeviceCompleted", "现场状态：目标已归档");
	}

	if (!bAvailableForInteraction)
	{
		return FText::Format(
			NSLOCTEXT("HorrorCampaignObjective", "StandardDeviceLockedWithReason", "现场状态：{0}"),
			BuildObjectiveLockReasonText());
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::MultiStep)
	{
		if (bRecoverRelicAwaitingDelivery)
		{
			return FText::Format(
				NSLOCTEXT("HorrorCampaignObjective", "RelicDeliveryDeviceStatus", "现场状态：遗物已取出，锚点距离 {0} 米"),
				FText::AsNumber(FMath::Max(0, FMath::CeilToInt(ResolveRecoverRelicDeliveryDistanceMeters()))));
		}

		return FText::Format(
			NSLOCTEXT("HorrorCampaignObjective", "StandardDeviceMultiStep", "现场状态：阶段 {0}/{1} 可执行"),
			FText::AsNumber(FMath::Clamp(InteractionProgressCount + 1, 1, FMath::Max(1, RequiredInteractionCount))),
			FText::AsNumber(FMath::Max(1, RequiredInteractionCount)));
	}

	return NSLOCTEXT("HorrorCampaignObjective", "StandardDeviceReady", "现场状态：目标可互动");
}

FText AHorrorCampaignObjectiveActor::BuildStandardInteractionNextActionLabel() const
{
	if (!bAvailableForInteraction)
	{
		return FText::Format(
			NSLOCTEXT("HorrorCampaignObjective", "StandardNextLockedPrerequisite", "{0}"),
			BuildObjectiveLockReasonText());
	}

	if (bRecoverRelicAwaitingDelivery)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "RelicDeliveryNextAction", "沿导航把遗物送回锚点，在锚点处再次按互动键归档。");
	}

	const FHorrorCampaignObjectiveBeat* Beat = GetCurrentObjectiveBeat();
	const EHorrorCampaignObjectiveBeatCompletionRule Rule = Beat
		? Beat->CompletionRule
		: EHorrorCampaignObjectiveBeatCompletionRule::InteractOnce;

	switch (Rule)
	{
		case EHorrorCampaignObjectiveBeatCompletionRule::HoldInteract:
			return NSLOCTEXT("HorrorCampaignObjective", "StandardNextHoldInteract", "长按互动键，保持到当前阶段完成。");
		case EHorrorCampaignObjectiveBeatCompletionRule::RecordEvidence:
			return NSLOCTEXT("HorrorCampaignObjective", "StandardNextRecordEvidence", "保持录像并对准证据，直到归档完成。");
		case EHorrorCampaignObjectiveBeatCompletionRule::ConfirmResult:
			return NSLOCTEXT("HorrorCampaignObjective", "StandardNextConfirmResult", "确认结果反馈后，推进到下一目标。");
		case EHorrorCampaignObjectiveBeatCompletionRule::ReachEscapePoint:
			return NSLOCTEXT("HorrorCampaignObjective", "StandardNextReachEscapePoint", "沿导航亮点移动到目标点。");
		case EHorrorCampaignObjectiveBeatCompletionRule::AdvancedWindow:
			return NSLOCTEXT("HorrorCampaignObjective", "StandardNextOpenWindow", "按互动键打开专用操作窗口。");
		case EHorrorCampaignObjectiveBeatCompletionRule::InteractOnce:
		case EHorrorCampaignObjectiveBeatCompletionRule::None:
		default:
			return RequiredInteractionCount > 1
				? NSLOCTEXT("HorrorCampaignObjective", "StandardNextMultiInteract", "按互动键完成当前阶段。")
				: NSLOCTEXT("HorrorCampaignObjective", "StandardNextInteractOnce", "靠近目标并按互动键。");
	}
}

FText AHorrorCampaignObjectiveActor::BuildStandardInteractionFailureRecoveryLabel() const
{
	const FHorrorCampaignObjectiveBeat* Beat = GetCurrentObjectiveBeat();
	const EHorrorCampaignObjectiveBeatFailurePolicy Policy = Beat
		? Beat->FailurePolicy
		: EHorrorCampaignObjectiveBeatFailurePolicy::None;

	switch (Policy)
	{
		case EHorrorCampaignObjectiveBeatFailurePolicy::RetryBeat:
			return NSLOCTEXT("HorrorCampaignObjective", "StandardRecoveryRetryBeat", "失败后保留目标，重新完成当前阶段。");
		case EHorrorCampaignObjectiveBeatFailurePolicy::ResetObjective:
			return NSLOCTEXT("HorrorCampaignObjective", "StandardRecoveryResetObjective", "失败后目标进度会重置，需要从第一阶段重来。");
		case EHorrorCampaignObjectiveBeatFailurePolicy::CampaignRecovery:
			return NSLOCTEXT("HorrorCampaignObjective", "StandardRecoveryCampaign", "失败后撤回安全点，再重新接近目标。");
		case EHorrorCampaignObjectiveBeatFailurePolicy::None:
		default:
			return NSLOCTEXT("HorrorCampaignObjective", "StandardRecoveryNoPenalty", "没有失败惩罚，保持调查节奏即可。");
	}
}

void AHorrorCampaignObjectiveActor::ApplyObjectiveVisuals()
{
	if (!bPresentationVisible)
	{
		ApplyHiddenLockedVisuals();
		return;
	}

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
		LabelText->SetVisibility(true);
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
			: bRecoverRelicAwaitingDelivery
				? FText::Format(
					NSLOCTEXT("HorrorCampaignObjective", "RelicDeliveryLabelFormat", "{0}\n送回锚点 {1} 米"),
					BaseLabel,
					FText::AsNumber(FMath::Max(0, FMath::CeilToInt(ResolveRecoverRelicDeliveryDistanceMeters()))))
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
		MarkerLight->SetVisibility(true);
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
		InteractionBounds->SetCollisionEnabled(bAvailableForInteraction || bTimedObjectiveActive || bAdvancedInteractionActive
			? ECollisionEnabled::QueryOnly
			: ECollisionEnabled::NoCollision);
	}
}

void AHorrorCampaignObjectiveActor::ApplyCompletedVisuals()
{
	bPresentationVisible = true;

	if (InteractionBounds)
	{
		InteractionBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (VisualMesh)
	{
		VisualMesh->SetVisibility(true);
	}

	if (LabelText)
	{
		LabelText->SetVisibility(true);
		LabelText->SetText(ObjectiveDefinition.CompletionText.IsEmpty()
			? FText::FromString(TEXT("已完成"))
			: ObjectiveDefinition.CompletionText);
		LabelText->SetTextRenderColor(FColor(120, 255, 160));
	}

	if (MarkerLight)
	{
		MarkerLight->SetVisibility(true);
		MarkerLight->SetLightColor(FLinearColor(FColor(120, 255, 160)));
		MarkerLight->SetIntensity(450.0f);
	}

	if (ObjectiveVFX)
	{
		ObjectiveVFX->Deactivate();
		ObjectiveVFX->SetVisibility(false);
	}
}

void AHorrorCampaignObjectiveActor::ApplyHiddenLockedVisuals()
{
	if (VisualMesh)
	{
		VisualMesh->SetVisibility(false);
	}

	if (LabelText)
	{
		LabelText->SetText(FText::GetEmpty());
		LabelText->SetVisibility(false);
	}

	if (MarkerLight)
	{
		MarkerLight->SetIntensity(0.0f);
		MarkerLight->SetVisibility(false);
	}

	if (ObjectiveVFX)
	{
		ObjectiveVFX->Deactivate();
		ObjectiveVFX->SetVisibility(false);
	}

	if (InteractionBounds)
	{
		InteractionBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
		|| ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration
		|| ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan
		|| ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning;
}

bool AHorrorCampaignObjectiveActor::IsAuthoredCampaignObjective() const
{
	const FHorrorCampaignChapterDefinition* Chapter = FHorrorCampaign::FindChapterById(ChapterId);
	return Chapter && FHorrorCampaign::FindObjectiveById(*Chapter, ObjectiveDefinition.ObjectiveId) != nullptr;
}

int32 AHorrorCampaignObjectiveActor::GetCurrentObjectiveBeatIndex() const
{
	if (ObjectiveDefinition.ObjectiveBeats.IsEmpty())
	{
		return INDEX_NONE;
	}

	if (bCompleted)
	{
		return ObjectiveDefinition.ObjectiveBeats.Num() - 1;
	}

	const float ProgressAsBeat = bAdvancedInteractionActive
		? AdvancedInteractionProgressFraction * static_cast<float>(ObjectiveDefinition.ObjectiveBeats.Num())
		: static_cast<float>(InteractionProgressCount);
	return FMath::Clamp(
		FMath::FloorToInt(ProgressAsBeat),
		0,
		ObjectiveDefinition.ObjectiveBeats.Num() - 1);
}

const FHorrorCampaignObjectiveBeat* AHorrorCampaignObjectiveActor::GetCurrentObjectiveBeat() const
{
	const int32 BeatIndex = GetCurrentObjectiveBeatIndex();
	return ObjectiveDefinition.ObjectiveBeats.IsValidIndex(BeatIndex)
		? &ObjectiveDefinition.ObjectiveBeats[BeatIndex]
		: nullptr;
}

FText AHorrorCampaignObjectiveActor::GetCurrentObjectiveBeatLabel() const
{
	const FHorrorCampaignObjectiveBeat* Beat = GetCurrentObjectiveBeat();
	return Beat ? Beat->Label : FText::GetEmpty();
}

FText AHorrorCampaignObjectiveActor::GetCurrentObjectiveBeatDetail() const
{
	const FHorrorCampaignObjectiveBeat* Beat = GetCurrentObjectiveBeat();
	return Beat ? Beat->Detail : FText::GetEmpty();
}

bool AHorrorCampaignObjectiveActor::IsCurrentObjectiveBeatUrgent() const
{
	const FHorrorCampaignObjectiveBeat* Beat = GetCurrentObjectiveBeat();
	return Beat && Beat->bUrgent;
}

bool AHorrorCampaignObjectiveActor::DoesCurrentObjectiveBeatRequireRecording() const
{
	const FHorrorCampaignObjectiveBeat* Beat = GetCurrentObjectiveBeat();
	return Beat && Beat->bRequiresRecording;
}

bool AHorrorCampaignObjectiveActor::HasConfiguredEscapeDestination() const
{
	return ObjectiveDefinition.ObjectiveType == EHorrorCampaignObjectiveType::SurviveAmbush
		&& SanitizeEscapeDestinationOffset(ObjectiveDefinition.EscapeDestinationOffset).Size2D() >= MinimumEscapeDestinationOffsetCm;
}

bool AHorrorCampaignObjectiveActor::IsCarryReturnRelicObjective() const
{
	return ObjectiveDefinition.ObjectiveType == EHorrorCampaignObjectiveType::RecoverRelic
		&& (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::MultiStep
			|| ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::Instant)
		&& HasConfiguredRelicDeliveryAnchor();
}

bool AHorrorCampaignObjectiveActor::HasConfiguredRelicDeliveryAnchor() const
{
	return ObjectiveDefinition.ObjectiveType == EHorrorCampaignObjectiveType::RecoverRelic
		&& HasAuthoredDeliveryOffset(ObjectiveDefinition.EscapeDestinationOffset);
}

FVector AHorrorCampaignObjectiveActor::ResolveEscapeDestinationWorldLocation(AActor* InstigatorActor) const
{
	const FRotator YawRotation(0.0f, GetActorRotation().Yaw, 0.0f);
	const FRotationMatrix RotationMatrix(YawRotation);
	const FVector SafeEscapeDestinationOffset = SanitizeEscapeDestinationOffset(ObjectiveDefinition.EscapeDestinationOffset);
	const FVector EscapeOffset =
		RotationMatrix.GetUnitAxis(EAxis::X) * SafeEscapeDestinationOffset.X
		+ RotationMatrix.GetUnitAxis(EAxis::Y) * SafeEscapeDestinationOffset.Y
		+ FVector(0.0f, 0.0f, SafeEscapeDestinationOffset.Z);

	FVector DestinationLocation = GetActorLocation() + EscapeOffset;
	if (UWorld* World = GetWorld())
	{
		FVector ClearDestinationLocation = DestinationLocation;
		const FVector RouteStartLocation = InstigatorActor ? InstigatorActor->GetActorLocation() : GetActorLocation();
		if (InstigatorActor
			&& TryFindReachableEscapeDestinationLocation(*World, DestinationLocation, RouteStartLocation, this, InstigatorActor, ClearDestinationLocation))
		{
			DestinationLocation = ClearDestinationLocation;
		}
		else if (TryFindClearEscapeDestinationLocation(*World, DestinationLocation, ClearDestinationLocation))
		{
			DestinationLocation = ClearDestinationLocation;
		}
	}
	return DestinationLocation;
}

FVector AHorrorCampaignObjectiveActor::ResolveRecoverRelicDeliveryWorldLocation(AActor* InstigatorActor) const
{
	if (bRecoverRelicAwaitingDelivery && !RecoverRelicDeliveryLocation.IsNearlyZero())
	{
		return RecoverRelicDeliveryLocation;
	}

	return ResolveEscapeDestinationWorldLocation(InstigatorActor);
}

float AHorrorCampaignObjectiveActor::ResolveRecoverRelicDeliveryDistanceMeters(AActor* InstigatorActor) const
{
	const AActor* DistanceReference = InstigatorActor ? InstigatorActor : this;
	if (!DistanceReference)
	{
		return 0.0f;
	}

	return FMath::Max(
		0.0f,
		FVector::Dist2D(DistanceReference->GetActorLocation(), ResolveRecoverRelicDeliveryWorldLocation(InstigatorActor)) / 100.0f);
}

float AHorrorCampaignObjectiveActor::ResolveRecoverRelicRouteDistanceMeters() const
{
	const FVector RouteOffset = SanitizeEscapeDestinationOffset(ObjectiveDefinition.EscapeDestinationOffset);
	return FMath::Max(0.0f, RouteOffset.Size2D() / 100.0f);
}

bool AHorrorCampaignObjectiveActor::BeginRecoverRelicDelivery(AActor* InstigatorActor)
{
	if (!IsCarryReturnRelicObjective() || bRecoverRelicAwaitingDelivery)
	{
		return false;
	}

	InteractionProgressCount = 1;
	bRecoverRelicAwaitingDelivery = true;
	RecoverRelicDeliveryLocation = ResolveRecoverRelicDeliveryWorldLocation(InstigatorActor);
	SetActorLocation(RecoverRelicDeliveryLocation, false);
	RefreshObjectiveState();
	return true;
}

FTransform AHorrorCampaignObjectiveActor::ResolveAmbushThreatAnchorTransform(AActor* InstigatorActor) const
{
	const FVector AnchorLocation = InstigatorActor ? InstigatorActor->GetActorLocation() : GetActorLocation();
	FVector EscapeDirection = TimedObjectiveEscapeDestinationLocation - AnchorLocation;
	EscapeDirection.Z = 0.0f;
	if (EscapeDirection.IsNearlyZero())
	{
		EscapeDirection = GetActorForwardVector();
		EscapeDirection.Z = 0.0f;
	}
	EscapeDirection = EscapeDirection.GetSafeNormal(UE_SMALL_NUMBER, FVector::ForwardVector);
	return FTransform(
		FRotator(0.0f, EscapeDirection.Rotation().Yaw, 0.0f),
		AnchorLocation,
		FVector::OneVector);
}

float AHorrorCampaignObjectiveActor::GetAdvancedInteractionSuccessProgress() const
{
	return ObjectiveDefinition.AdvancedInteractionTuning.SuccessProgress > 0.0f
		? FMath::Clamp(ObjectiveDefinition.AdvancedInteractionTuning.SuccessProgress, 0.01f, 1.0f)
		: AdvancedInteractionSuccessProgress;
}

float AHorrorCampaignObjectiveActor::GetAdvancedInteractionCueCycleSeconds() const
{
	return ObjectiveDefinition.AdvancedInteractionTuning.CueCycleSeconds > 0.0f
		? FMath::Max(0.1f, ObjectiveDefinition.AdvancedInteractionTuning.CueCycleSeconds)
		: AdvancedInteractionCueCycleSeconds;
}

float AHorrorCampaignObjectiveActor::GetAdvancedInteractionRetryAssistFraction() const
{
	return FMath::Clamp(static_cast<float>(ObjectiveFailureAttemptCount) * 0.35f, 0.0f, 0.70f);
}

float AHorrorCampaignObjectiveActor::GetAdvancedInteractionTimingWindowStart() const
{
	const float ConfiguredStart = ObjectiveDefinition.AdvancedInteractionTuning.TimingWindowStartFraction;
	const float ConfiguredEnd = ObjectiveDefinition.AdvancedInteractionTuning.TimingWindowEndFraction;
	float WindowStart = AdvancedInteractionTimingWindowStart;
	if (ConfiguredStart >= 0.0f && ConfiguredEnd >= 0.0f && ConfiguredStart < ConfiguredEnd)
	{
		WindowStart = FMath::Clamp(ConfiguredStart, 0.0f, 1.0f);
	}

	return FMath::Clamp(WindowStart - GetAdvancedInteractionRetryAssistFraction() * 0.12f, 0.0f, 1.0f);
}

float AHorrorCampaignObjectiveActor::GetAdvancedInteractionTimingWindowEnd() const
{
	const float ConfiguredStart = ObjectiveDefinition.AdvancedInteractionTuning.TimingWindowStartFraction;
	const float ConfiguredEnd = ObjectiveDefinition.AdvancedInteractionTuning.TimingWindowEndFraction;
	float WindowEnd = AdvancedInteractionTimingWindowEnd;
	if (ConfiguredStart >= 0.0f && ConfiguredEnd >= 0.0f && ConfiguredStart < ConfiguredEnd)
	{
		WindowEnd = FMath::Clamp(ConfiguredEnd, 0.0f, 1.0f);
	}

	return FMath::Clamp(WindowEnd + GetAdvancedInteractionRetryAssistFraction() * 0.12f, 0.0f, 1.0f);
}

float AHorrorCampaignObjectiveActor::GetAdvancedInteractionFailureProgressPenalty() const
{
	return ObjectiveDefinition.AdvancedInteractionTuning.FailureProgressPenalty >= 0.0f
		? FMath::Clamp(ObjectiveDefinition.AdvancedInteractionTuning.FailureProgressPenalty, 0.0f, 1.0f)
		: CircuitFailureProgressPenalty;
}

float AHorrorCampaignObjectiveActor::GetAdvancedInteractionFailureStabilityDamage() const
{
	const float RetryAssistMultiplier = 1.0f - GetAdvancedInteractionRetryAssistFraction() * 0.72f;
	if (ObjectiveDefinition.AdvancedInteractionTuning.FailureStabilityDamage >= 0.0f)
	{
		return FMath::Clamp(ObjectiveDefinition.AdvancedInteractionTuning.FailureStabilityDamage * RetryAssistMultiplier, 0.0f, 1.0f);
	}

	const float BaseDamage = ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration
		? GearFailureStabilityDamage
		: CircuitFailureStabilityDamage;
	return FMath::Clamp(BaseDamage * RetryAssistMultiplier, 0.0f, 1.0f);
}

float AHorrorCampaignObjectiveActor::GetAdvancedInteractionSuccessStabilityRecovery() const
{
	return ObjectiveDefinition.AdvancedInteractionTuning.SuccessStabilityRecovery >= 0.0f
		? FMath::Clamp(ObjectiveDefinition.AdvancedInteractionTuning.SuccessStabilityRecovery, 0.0f, 1.0f)
		: AdvancedSuccessStabilityRecovery;
}

float AHorrorCampaignObjectiveActor::GetAdvancedInteractionFailurePauseSeconds() const
{
	return ObjectiveDefinition.AdvancedInteractionTuning.FailurePauseSeconds >= 0.0f
		? FMath::Max(0.0f, ObjectiveDefinition.AdvancedInteractionTuning.FailurePauseSeconds)
		: GearFailurePauseSeconds;
}

bool AHorrorCampaignObjectiveActor::IsSignalTuningMode() const
{
	return ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning;
}

bool AHorrorCampaignObjectiveActor::IsSpectralScanMode() const
{
	return ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan;
}

bool AHorrorCampaignObjectiveActor::IsGearCalibrationMode() const
{
	return ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration;
}

bool AHorrorCampaignObjectiveActor::IsCircuitHazardInput(FName InputId) const
{
	return ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring
		&& InputId == CircuitHazardInput;
}

bool AHorrorCampaignObjectiveActor::IsSignalTuningConfirmInput(FName InputId) const
{
	return IsSignalTuningMode() && InputId == SignalTuningConfirmInput;
}

bool AHorrorCampaignObjectiveActor::IsSignalTuningAdjustInput(FName InputId) const
{
	return IsSignalTuningMode()
		&& (InputId == SignalTuningLeftInput || InputId == SignalTuningRightInput);
}

bool AHorrorCampaignObjectiveActor::IsSpectralScanFilterAligned() const
{
	// 简化玩法：不需要滤波器对齐，只要光标在绿色区域就可以
	return true;
}

float AHorrorCampaignObjectiveActor::ResolveSpectralScanTargetFocus() const
{
	if (!IsSpectralScanMode())
	{
		return 0.5f;
	}

	return ResolveStableObjectiveBiasProfile(ObjectiveDefinition.ObjectiveId) == 0
		? SpectralScanInitialLowFocus
		: SpectralScanInitialHighFocus;
}

float AHorrorCampaignObjectiveActor::ResolveInitialSignalTuningBalance() const
{
	if (!IsSignalTuningMode())
	{
		return 0.5f;
	}

	return ResolveStableObjectiveBiasProfile(ObjectiveDefinition.ObjectiveId) == 0
		? SignalTuningInitialLeftBalance
		: SignalTuningInitialRightBalance;
}

void AHorrorCampaignObjectiveActor::AdjustSpectralScanFilter(FName InputId)
{
	const float Direction = InputId == SpectralScanSweepLeftInput ? -1.0f : 1.0f;
	const float PreviousFocus = SpectralScanFilterFocusFraction;
	const float ProposedFocus = FMath::Clamp(
		SpectralScanFilterFocusFraction + Direction * SpectralScanFilterStep,
		0.0f,
		1.0f);
	SpectralScanFilterFocusFraction = FMath::Clamp(
		(PreviousFocus - SpectralScanTargetFocusFraction) * (ProposedFocus - SpectralScanTargetFocusFraction) <= 0.0f
			? SpectralScanTargetFocusFraction
			: ProposedFocus,
		0.0f,
		1.0f);
	SpectralScanConfidenceFraction = FMath::Clamp(
		SpectralScanConfidenceFraction + 0.18f,
		0.0f,
		1.0f);
	SpectralScanNoiseFraction = FMath::Clamp(
		SpectralScanNoiseFraction - 0.18f,
		0.0f,
		1.0f);
	AdvancedInteractionFeedbackText = InputId == SpectralScanSweepLeftInput
		? NSLOCTEXT("HorrorCampaignObjective", "SpectralScanFilterLeftNudge", "手动扫频：滤波焦点压向低频峰。")
		: NSLOCTEXT("HorrorCampaignObjective", "SpectralScanFilterRightNudge", "手动扫频：滤波焦点压向高频峰。");
	bAdvancedInteractionRecentSuccess = false;
	bAdvancedInteractionRecentFailure = false;
}

void AHorrorCampaignObjectiveActor::AdjustSpectralScanFilterByAxis(float AxisValue, float HoldSeconds)
{
	const float ClampedAxis = FMath::Clamp(AxisValue, -1.0f, 1.0f);
	const float EffectiveHoldSeconds = FMath::Clamp(HoldSeconds, 0.1f, 1.2f);
	if (FMath::IsNearlyZero(ClampedAxis, 0.03f))
	{
		AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SpectralScanAxisIdle", "滤波焦点保持稳定，等待扫频输入。");
		bAdvancedInteractionRecentSuccess = false;
		bAdvancedInteractionRecentFailure = false;
		return;
	}

	const float PreviousFocus = SpectralScanFilterFocusFraction;
	const float ProposedFocus = FMath::Clamp(
		SpectralScanFilterFocusFraction + ClampedAxis * SpectralScanFilterStep * EffectiveHoldSeconds * 1.2f,
		0.0f,
		1.0f);
	SpectralScanFilterFocusFraction = FMath::Clamp(
		(PreviousFocus - SpectralScanTargetFocusFraction) * (ProposedFocus - SpectralScanTargetFocusFraction) <= 0.0f
			? SpectralScanTargetFocusFraction
			: ProposedFocus,
		0.0f,
		1.0f);
	SpectralScanConfidenceFraction = FMath::Clamp(
		SpectralScanConfidenceFraction + FMath::Abs(ClampedAxis) * EffectiveHoldSeconds * 0.26f,
		0.0f,
		1.0f);
	SpectralScanNoiseFraction = FMath::Clamp(
		SpectralScanNoiseFraction - FMath::Abs(ClampedAxis) * EffectiveHoldSeconds * 0.22f,
		0.0f,
		1.0f);
	AdvancedInteractionFeedbackText = ClampedAxis < 0.0f
		? NSLOCTEXT("HorrorCampaignObjective", "SpectralScanAxisLeft", "连续扫频：滤波焦点向低频异常峰移动。")
		: NSLOCTEXT("HorrorCampaignObjective", "SpectralScanAxisRight", "连续扫频：滤波焦点向高频异常峰移动。");
	bAdvancedInteractionRecentSuccess = false;
	bAdvancedInteractionRecentFailure = false;
}

void AHorrorCampaignObjectiveActor::AdjustSignalTuningBalance(FName InputId)
{
	const float Direction = InputId == SignalTuningLeftInput ? -1.0f : 1.0f;
	SignalTuningBalanceFraction = FMath::Clamp(
		SignalTuningBalanceFraction + Direction * SignalTuningBalanceStep,
		0.0f,
		1.0f);
	AdvancedInteractionFeedbackText = InputId == SignalTuningLeftInput
		? NSLOCTEXT("HorrorCampaignObjective", "SignalTuningLeftNudge", "左声道压低，回放声像向左偏移。")
		: NSLOCTEXT("HorrorCampaignObjective", "SignalTuningRightNudge", "右声道抬高，回放声像向右回正。");
	bAdvancedInteractionRecentSuccess = false;
	bAdvancedInteractionRecentFailure = false;
}

void AHorrorCampaignObjectiveActor::AdjustSignalTuningBalanceByAxis(float AxisValue, float HoldSeconds)
{
	const float ClampedAxis = FMath::Clamp(AxisValue, -1.0f, 1.0f);
	const float EffectiveHoldSeconds = FMath::Clamp(HoldSeconds, 0.1f, 1.0f);
	if (FMath::IsNearlyZero(ClampedAxis, 0.03f))
	{
		AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SignalTuningAxisIdle", "声像保持稳定，等待微调输入。");
		bAdvancedInteractionRecentSuccess = false;
		bAdvancedInteractionRecentFailure = false;
		return;
	}

	SignalTuningBalanceFraction = FMath::Clamp(
		SignalTuningBalanceFraction + ClampedAxis * SignalTuningBalanceStep * EffectiveHoldSeconds * 1.15f,
		0.0f,
		1.0f);
	AdvancedInteractionFeedbackText = ClampedAxis < 0.0f
		? NSLOCTEXT("HorrorCampaignObjective", "SignalTuningAxisLeft", "连续微调：左声道压低，声像向左滑动。")
		: NSLOCTEXT("HorrorCampaignObjective", "SignalTuningAxisRight", "连续微调：右声道抬高，声像向右回正。");
	bAdvancedInteractionRecentSuccess = false;
	bAdvancedInteractionRecentFailure = false;
}

bool AHorrorCampaignObjectiveActor::IsSignalTuningBalanceAligned() const
{
	return FMath::Abs(SignalTuningBalanceFraction - SignalTuningTargetBalanceFraction) <= SignalTuningLockTolerance;
}

bool AHorrorCampaignObjectiveActor::IsSpectralScanConfidenceReady() const
{
	// 简化玩法：只要在时序窗口内就可以，不需要置信度
	return true;
}

void AHorrorCampaignObjectiveActor::StartAdvancedInteraction()
{
	if (!UsesAdvancedInteraction() || bCompleted)
	{
		return;
	}

	ResetAdvancedInteractionState();
	bAdvancedInteractionActive = true;
	AdvancedInteractionProgressFraction = 0.0f;
	AdvancedInteractionStepIndex = 0;
	AdvancedInteractionPauseRemainingSeconds = 0.0f;
	AdvanceExpectedAdvancedInput();
	ResetAdvancedInteractionCue();
	switch (ObjectiveDefinition.InteractionMode)
	{
		case EHorrorCampaignInteractionMode::CircuitWiring:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "CircuitStarted", "接线盘已展开，等端子亮到蓝色窗口再接入。");
			break;
		case EHorrorCampaignInteractionMode::GearCalibration:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "GearStarted", "齿轮组已展开，等停转齿轮进入校准窗口再拨动。");
			break;
		case EHorrorCampaignInteractionMode::SpectralScan:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SpectralScanStarted", "频谱扫描窗口已展开，先用 A/D 扫频滤噪，再按 S 锁定异常峰。");
			break;
		case EHorrorCampaignInteractionMode::SignalTuning:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SignalTuningStarted", "信号调谐窗口已展开，等待回放频率进入同步带。");
			break;
		default:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "AdvancedStarted", "交互窗口已展开，等待同步。");
			break;
	}
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
	AdvancedInteractionStabilityFraction = 1.0f;
	SignalTuningTargetBalanceFraction = 0.5f;
	SignalTuningBalanceFraction = ResolveInitialSignalTuningBalance();
	SpectralScanConfidenceFraction = 0.0f;
	SpectralScanNoiseFraction = 0.0f;
	SpectralScanTargetFocusFraction = ResolveSpectralScanTargetFocus();
	SpectralScanFilterFocusFraction = SpectralScanTargetFocusFraction < 0.5f
		? SpectralScanInitialHighFocus
		: SpectralScanInitialLowFocus;
	AdvancedInteractionComboCount = 0;
	AdvancedInteractionMistakeCount = 0;
	bAdvancedInteractionRecentSuccess = false;
	bAdvancedInteractionRecentFailure = false;
	ClearAdvancedInteractionFault();
	ResetLastAdvancedInteractionOutcome();
	AdvancedInteractionPauseRemainingSeconds = 0.0f;
}

void AHorrorCampaignObjectiveActor::ResetAdvancedInteractionCue()
{
	AdvancedInteractionCueElapsedSeconds = 0.0f;
	AdvancedInteractionTimingFraction = 0.0f;
	bAdvancedInteractionCueResolved = false;
}

void AHorrorCampaignObjectiveActor::MarkAdvancedInteractionSuccess()
{
	bAdvancedInteractionRecentSuccess = true;
	bAdvancedInteractionRecentFailure = false;
	ClearAdvancedInteractionFault();
	++AdvancedInteractionComboCount;
	AdvancedInteractionStabilityFraction = FMath::Clamp(
		AdvancedInteractionStabilityFraction + GetAdvancedInteractionSuccessStabilityRecovery(),
		0.0f,
		1.0f);
}

void AHorrorCampaignObjectiveActor::MarkAdvancedInteractionFailure(float StabilityDamage)
{
	bAdvancedInteractionRecentSuccess = false;
	bAdvancedInteractionRecentFailure = true;
	AdvancedInteractionComboCount = 0;
	++AdvancedInteractionMistakeCount;
	AdvancedInteractionStabilityFraction = FMath::Clamp(
		AdvancedInteractionStabilityFraction - FMath::Max(0.0f, StabilityDamage),
		0.0f,
		1.0f);
	if (IsSpectralScanMode())
	{
		SpectralScanConfidenceFraction = FMath::Clamp(
			SpectralScanConfidenceFraction - FMath::Max(0.12f, StabilityDamage * 0.5f),
			0.0f,
			1.0f);
		SpectralScanNoiseFraction = FMath::Clamp(
			SpectralScanNoiseFraction + FMath::Max(0.18f, StabilityDamage),
			0.0f,
			1.0f);
		const float FilterPushDirection = SpectralScanFilterFocusFraction < SpectralScanTargetFocusFraction ? -1.0f : 1.0f;
		SpectralScanFilterFocusFraction = FMath::Clamp(
			SpectralScanFilterFocusFraction + FilterPushDirection * FMath::Max(0.06f, StabilityDamage * 0.18f),
			0.0f,
			1.0f);
	}
}

void AHorrorCampaignObjectiveActor::SetAdvancedInteractionFault(FName FaultId)
{
	LastAdvancedInteractionFaultId = FaultId;
}

void AHorrorCampaignObjectiveActor::ClearAdvancedInteractionFault()
{
	LastAdvancedInteractionFaultId = NAME_None;
}

void AHorrorCampaignObjectiveActor::ResetLastAdvancedInteractionOutcome()
{
	LastAdvancedInteractionOutcome = FHorrorAdvancedInteractionOutcome();
}

void AHorrorCampaignObjectiveActor::RecordAdvancedInteractionOutcome(
	EHorrorAdvancedInteractionOutcomeKind Kind,
	FName InputId,
	EHorrorAdvancedInteractionFeedbackState FeedbackState,
	bool bConsumesInput,
	bool bAdvancesProgress,
	bool bRetryable,
	FName FailureCause,
	FName RecoveryAction,
	FName FaultId)
{
	LastAdvancedInteractionOutcome.Kind = Kind;
	LastAdvancedInteractionOutcome.FeedbackState = FeedbackState;
	LastAdvancedInteractionOutcome.InputId = InputId;
	LastAdvancedInteractionOutcome.FaultId = FaultId.IsNone() ? LastAdvancedInteractionFaultId : FaultId;
	LastAdvancedInteractionOutcome.FailureCause = FailureCause;
	LastAdvancedInteractionOutcome.RecoveryAction = RecoveryAction;
	LastAdvancedInteractionOutcome.bConsumesInput = bConsumesInput;
	LastAdvancedInteractionOutcome.bAdvancesProgress = bAdvancesProgress;
	LastAdvancedInteractionOutcome.bRetryable = bRetryable;
}

bool AHorrorCampaignObjectiveActor::RecordAdvancedInteractionFailureOutcomeAfterFeedback(
	EHorrorAdvancedInteractionOutcomeKind FailureKind,
	FName InputId,
	FName FailureFaultId,
	AActor* InstigatorActor)
{
	if (TryHandleAdvancedInteractionFailureExhausted(InstigatorActor))
	{
		RecordAdvancedInteractionOutcome(
			EHorrorAdvancedInteractionOutcomeKind::Overloaded,
			InputId,
			EHorrorAdvancedInteractionFeedbackState::Overloaded,
			true,
			false,
			true,
			GetAdvancedInteractionFailureCause(),
			RetryObjectiveRecoveryAction,
			FailureFaultId);
		return true;
	}

	RecordAdvancedInteractionOutcome(
		FailureKind,
		InputId,
		EHorrorAdvancedInteractionFeedbackState::Failure,
		true,
		false,
		false,
		NAME_None,
		NAME_None,
		FailureFaultId);
	RefreshObjectiveState();
	return true;
}

FName AHorrorCampaignObjectiveActor::ResolveGearCalibrationInputForStep(int32 StepIndex) const
{
	if (StepIndex <= 0)
	{
		return GearCalibrationSequence[0];
	}

	const int32 SideGearIndex = (ResolveStableObjectiveVariantProfile(ObjectiveDefinition.ObjectiveId) + StepIndex - 1) % 2;
	const FName FirstSideGear = SideGearIndex == 0 ? GearCalibrationSequence[1] : GearCalibrationSequence[2];
	const FName SecondSideGear = SideGearIndex == 0 ? GearCalibrationSequence[2] : GearCalibrationSequence[1];
	return (StepIndex % 2) == 1 ? FirstSideGear : SecondSideGear;
}

void AHorrorCampaignObjectiveActor::AdvanceExpectedAdvancedInput()
{
	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring)
	{
		const int32 InputIndex = AdvancedInteractionStepIndex % UE_ARRAY_COUNT(CircuitSafeRoutingSequence);
		ExpectedAdvancedInputId = CircuitSafeRoutingSequence[InputIndex];
		return;
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration)
	{
		ExpectedAdvancedInputId = ResolveGearCalibrationInputForStep(AdvancedInteractionStepIndex);
		return;
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
	{
		ExpectedAdvancedInputId = SpectralScanLockInput;
		return;
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
	{
		ExpectedAdvancedInputId = SignalTuningConfirmInput;
		return;
	}

	ExpectedAdvancedInputId = NAME_None;
}

TArray<FHorrorAdvancedInteractionInputOption> AHorrorCampaignObjectiveActor::BuildAdvancedInteractionInputOptions() const
{
	TArray<FHorrorAdvancedInteractionInputOption> Options;
	const FName* SourceInputs = nullptr;
	int32 SourceInputCount = 0;
	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring)
	{
		SourceInputs = CircuitInputs;
		SourceInputCount = UE_ARRAY_COUNT(CircuitInputs);
	}
	else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration)
	{
		SourceInputs = GearInputs;
		SourceInputCount = UE_ARRAY_COUNT(GearInputs);
	}
	else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
	{
		SourceInputs = SpectralScanInputs;
		SourceInputCount = UE_ARRAY_COUNT(SpectralScanInputs);
	}
	else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
	{
		SourceInputs = SignalTuningInputs;
		SourceInputCount = UE_ARRAY_COUNT(SignalTuningInputs);
	}

	if (!SourceInputs || SourceInputCount <= 0)
	{
		return Options;
	}

	Options.Reserve(SourceInputCount);
	for (int32 InputIndex = 0; InputIndex < SourceInputCount; ++InputIndex)
	{
		FHorrorAdvancedInteractionInputOption Option;
		Option.InputId = SourceInputs[InputIndex];
		Option.DisplayLabel = FText::FromName(SourceInputs[InputIndex]);
		Option.KeyHint = AdvancedDeviceKeyHints[InputIndex];
		Option.VisualSlotIndex = InputIndex;
		Option.VisualColor = ResolveAdvancedInputVisualColor(ObjectiveDefinition.InteractionMode, InputIndex);
		Option.ActionVerb = NSLOCTEXT("HorrorCampaignObjective", "AdvancedInputDefaultVerb", "选择");
		Option.bRequiresTimingWindow = true;
		Option.bRequiresStableSignal = false;
		Option.bAdvancesObjective = true;
		if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring)
		{
			const bool bCurrentSafeTerminal = Option.InputId == ExpectedAdvancedInputId;
			const bool bSolvedInRoute = IsCircuitInputSolvedInRoute(Option.InputId, AdvancedInteractionStepIndex);
			const float WindowStart = GetAdvancedInteractionTimingWindowStart();
			const float WindowEnd = GetAdvancedInteractionTimingWindowEnd();
			const float WindowCenter = (WindowStart + WindowEnd) * 0.5f;
			const float HalfWindow = FMath::Max((WindowEnd - WindowStart) * 0.5f, UE_SMALL_NUMBER);
			const float DistanceToWindow = AdvancedInteractionTimingFraction < WindowStart
				? WindowStart - AdvancedInteractionTimingFraction
				: (AdvancedInteractionTimingFraction > WindowEnd ? AdvancedInteractionTimingFraction - WindowEnd : 0.0f);
			const float WindowPrecision = IsAdvancedInteractionTimingWindowOpen()
				? 1.0f - FMath::Clamp(FMath::Abs(AdvancedInteractionTimingFraction - WindowCenter) / HalfWindow, 0.0f, 1.0f) * 0.30f
				: FMath::Clamp(1.0f - DistanceToWindow / 0.40f, 0.0f, 0.78f);
			const bool bProtectionPaused = AdvancedInteractionPauseRemainingSeconds > 0.0f;
			Option.bHazardous = Option.InputId == CircuitHazardInput;
			Option.bStalled = bCurrentSafeTerminal && !bSolvedInRoute;
			Option.bChainLinked = bSolvedInRoute;
			Option.ResponseWindowFraction = bSolvedInRoute ? 1.0f : (bCurrentSafeTerminal ? WindowPrecision : 0.10f);
			Option.MotionFraction = bProtectionPaused
				? (bSolvedInRoute ? 0.34f : 0.0f)
				: (bSolvedInRoute ? 1.0f : (bCurrentSafeTerminal ? 0.72f : 0.42f));
			Option.LoadFraction = Option.bHazardous
				? FMath::Clamp(
					0.82f
						+ static_cast<float>(AdvancedInteractionMistakeCount) * 0.05f
						+ (LastAdvancedInteractionFaultId == AdvancedFaultCircuitHazard ? 0.10f : 0.0f),
					0.0f,
					1.0f)
				: FMath::Clamp(
					(bSolvedInRoute ? 0.06f : (bCurrentSafeTerminal ? 0.16f : 0.38f))
						+ (bProtectionPaused ? 0.18f : 0.0f)
						+ static_cast<float>(AdvancedInteractionMistakeCount) * 0.04f,
					0.0f,
					1.0f);
			if (Option.bHazardous)
			{
				Option.ActionRole = EHorrorAdvancedInteractionInputRole::AvoidHazard;
				Option.ActionVerb = NSLOCTEXT("HorrorCampaignObjective", "CircuitInputAvoidVerb", "避开");
				Option.bRequiresTimingWindow = false;
				Option.bAdvancesObjective = false;
			}
			else
			{
				Option.ActionRole = EHorrorAdvancedInteractionInputRole::ConnectCircuit;
				Option.ActionVerb = NSLOCTEXT("HorrorCampaignObjective", "CircuitInputConnectVerb", "接入");
			}
		}
		else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration)
		{
			Option.ActionRole = EHorrorAdvancedInteractionInputRole::CalibrateGear;
			Option.ActionVerb = NSLOCTEXT("HorrorCampaignObjective", "GearInputCalibrateVerb", "拨动");
			const bool bCurrentStoppedGear = Option.InputId == ExpectedAdvancedInputId;
			const bool bSolvedInChain = IsGearInputSolvedInChain(
				Option.InputId,
				AdvancedInteractionStepIndex,
				ObjectiveDefinition.ObjectiveId);
			const float WindowStart = GetAdvancedInteractionTimingWindowStart();
			const float WindowEnd = GetAdvancedInteractionTimingWindowEnd();
			const float WindowCenter = (WindowStart + WindowEnd) * 0.5f;
			const float HalfWindow = FMath::Max((WindowEnd - WindowStart) * 0.5f, UE_SMALL_NUMBER);
			const float DistanceToWindow = AdvancedInteractionTimingFraction < WindowStart
				? WindowStart - AdvancedInteractionTimingFraction
				: (AdvancedInteractionTimingFraction > WindowEnd ? AdvancedInteractionTimingFraction - WindowEnd : 0.0f);
			const float WindowPrecision = IsAdvancedInteractionTimingWindowOpen()
				? 1.0f - FMath::Clamp(FMath::Abs(AdvancedInteractionTimingFraction - WindowCenter) / HalfWindow, 0.0f, 1.0f) * 0.35f
				: FMath::Clamp(1.0f - DistanceToWindow / 0.42f, 0.0f, 0.76f);
			const bool bMechanismPaused = AdvancedInteractionPauseRemainingSeconds > 0.0f;
			Option.bStalled = bCurrentStoppedGear && !bSolvedInChain;
			Option.bChainLinked = bSolvedInChain;
			Option.ResponseWindowFraction = bCurrentStoppedGear ? WindowPrecision : (bSolvedInChain ? 1.0f : 0.12f);
			Option.MotionFraction = bMechanismPaused
				? (bSolvedInChain ? 0.18f : 0.0f)
				: (bCurrentStoppedGear
					? (IsAdvancedInteractionTimingWindowOpen() ? 0.30f : 0.08f)
					: (bSolvedInChain ? 1.0f : 0.76f - FMath::Clamp(static_cast<float>(InputIndex) * 0.06f, 0.0f, 0.18f)));
			Option.LoadFraction = FMath::Clamp(
				(bCurrentStoppedGear ? 0.34f : 0.10f)
					+ (bMechanismPaused ? 0.32f : 0.0f)
					+ (bSolvedInChain ? -0.08f : 0.0f)
					+ FMath::Clamp(static_cast<float>(AdvancedInteractionMistakeCount) * 0.08f, 0.0f, 0.28f),
				0.0f,
				1.0f);
		}
		else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
		{
			Option.ActionRole = EHorrorAdvancedInteractionInputRole::LockSpectralBand;
			Option.ActionVerb = NSLOCTEXT("HorrorCampaignObjective", "SpectralInputLockVerb", "锁定");
			Option.bRequiresStableSignal = true;
			const bool bTargetBand = Option.InputId == ExpectedAdvancedInputId;
			const bool bSweepLeft = Option.InputId == SpectralScanSweepLeftInput;
			const bool bSweepRight = Option.InputId == SpectralScanSweepRightInput;
			const bool bFilterLeftOfTarget = SpectralScanFilterFocusFraction < SpectralScanTargetFocusFraction;
			const bool bCorrectiveSweep = bFilterLeftOfTarget ? bSweepRight : bSweepLeft;
			const float FilterErrorFraction = FMath::Clamp(
				FMath::Abs(SpectralScanFilterFocusFraction - SpectralScanTargetFocusFraction) / SpectralScanFilterLockTolerance,
				0.0f,
				1.0f);
			const float FilterPrecisionFraction = 1.0f - FilterErrorFraction;
			const float ConfidenceFraction = FMath::Clamp(SpectralScanConfidenceFraction, 0.0f, 1.0f);
			const float NoiseFraction = FMath::Clamp(SpectralScanNoiseFraction, 0.0f, 1.0f);
			const float LockReadiness = FMath::Clamp(
				(ConfidenceFraction - SpectralScanMinimumConfidenceToLock) / FMath::Max(1.0f - SpectralScanMinimumConfidenceToLock, UE_SMALL_NUMBER),
				0.0f,
				1.0f);
			if (bSweepLeft || bSweepRight)
			{
				Option.ActionRole = bSweepLeft
					? EHorrorAdvancedInteractionInputRole::TuneSignalLeft
					: EHorrorAdvancedInteractionInputRole::TuneSignalRight;
				Option.ActionVerb = bSweepLeft
					? NSLOCTEXT("HorrorCampaignObjective", "SpectralInputSweepLeftVerb", "左扫")
					: NSLOCTEXT("HorrorCampaignObjective", "SpectralInputSweepRightVerb", "右扫");
				Option.bRequiresTimingWindow = false;
				Option.bRequiresStableSignal = false;
				Option.bAdvancesObjective = false;
			}
			if (Option.InputId == SpectralScanLockInput)
			{
				Option.ActionRole = EHorrorAdvancedInteractionInputRole::LockSpectralBand;
				Option.ActionVerb = NSLOCTEXT("HorrorCampaignObjective", "SpectralInputLockFocusedVerb", "锁定");
				Option.bRequiresTimingWindow = true;
				Option.bRequiresStableSignal = true;
				Option.bAdvancesObjective = true;
			}
			Option.LoadFraction = bTargetBand
				? FMath::Clamp(NoiseFraction * (1.0f - ConfidenceFraction) * 0.36f + FilterErrorFraction * 0.42f, 0.05f, 0.82f)
				: FMath::Clamp(0.48f + NoiseFraction * 0.42f + static_cast<float>(InputIndex) * 0.035f, 0.0f, 1.0f);
			Option.ResponseWindowFraction = bTargetBand
				? FMath::Clamp(ConfidenceFraction * 0.46f + LockReadiness * 0.18f + FilterPrecisionFraction * 0.36f - NoiseFraction * 0.12f, 0.05f, 1.0f)
				: (bCorrectiveSweep
					? FMath::Clamp(0.54f + FilterErrorFraction * 0.34f, 0.0f, 1.0f)
					: FMath::Clamp((1.0f - ConfidenceFraction) * 0.20f + NoiseFraction * 0.30f + FilterPrecisionFraction * 0.10f, 0.08f, 0.58f));
			Option.MotionFraction = bTargetBand
				? FMath::Clamp(0.18f + ConfidenceFraction * 0.36f + LockReadiness * 0.18f + FilterPrecisionFraction * 0.28f - NoiseFraction * 0.12f, 0.08f, 1.0f)
				: (bCorrectiveSweep
					? FMath::Clamp(0.58f + FilterErrorFraction * 0.30f, 0.0f, 1.0f)
					: FMath::Clamp(0.44f - ConfidenceFraction * 0.14f + NoiseFraction * 0.24f, 0.12f, 0.72f));
			Option.bHazardous = !bTargetBand && Option.LoadFraction >= 0.72f;
		}
		else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
		{
			const float BalanceErrorFraction = FMath::Clamp(
				FMath::Abs(SignalTuningBalanceFraction - SignalTuningTargetBalanceFraction) / SignalTuningLockTolerance,
				0.0f,
				1.0f);
			const float BalancePrecisionFraction = 1.0f - BalanceErrorFraction;
			const bool bBalanceLeftOfTarget = SignalTuningBalanceFraction < SignalTuningTargetBalanceFraction;
			const bool bCenterConfirm = Option.InputId == SignalTuningConfirmInput;
			const bool bCorrectiveNudge =
				!bCenterConfirm
				&& (bBalanceLeftOfTarget
					? Option.InputId == SignalTuningRightInput
					: Option.InputId == SignalTuningLeftInput);

			if (Option.InputId == SignalTuningLeftInput)
			{
				Option.ActionRole = EHorrorAdvancedInteractionInputRole::TuneSignalLeft;
				Option.ActionVerb = NSLOCTEXT("HorrorCampaignObjective", "SignalInputLeftVerb", "左调");
				Option.bRequiresTimingWindow = false;
				Option.bAdvancesObjective = false;
			}
			else if (Option.InputId == SignalTuningRightInput)
			{
				Option.ActionRole = EHorrorAdvancedInteractionInputRole::TuneSignalRight;
				Option.ActionVerb = NSLOCTEXT("HorrorCampaignObjective", "SignalInputRightVerb", "右调");
				Option.bRequiresTimingWindow = false;
				Option.bAdvancesObjective = false;
			}
			else
			{
				Option.ActionRole = EHorrorAdvancedInteractionInputRole::LockSignalCenter;
				Option.ActionVerb = NSLOCTEXT("HorrorCampaignObjective", "SignalInputLockVerb", "锁定");
				Option.bRequiresStableSignal = true;
			}
			if (bCenterConfirm)
			{
				Option.LoadFraction = FMath::Clamp(0.16f + BalanceErrorFraction * 0.82f, 0.0f, 1.0f);
				Option.bHazardous = BalanceErrorFraction > 0.50f;
				Option.ResponseWindowFraction = BalancePrecisionFraction;
				Option.MotionFraction = BalancePrecisionFraction > 0.85f ? 1.0f : FMath::Clamp(0.22f + BalancePrecisionFraction * 0.58f, 0.0f, 1.0f);
			}
			else
			{
				Option.LoadFraction = FMath::Clamp(
					(bCorrectiveNudge ? 0.10f : 0.24f) + BalanceErrorFraction * (bCorrectiveNudge ? 0.18f : 0.32f),
					0.0f,
					0.68f);
				Option.ResponseWindowFraction = BalanceErrorFraction <= 0.05f
					? 0.28f
					: (bCorrectiveNudge ? FMath::Clamp(0.58f + BalanceErrorFraction * 0.36f, 0.0f, 1.0f) : FMath::Clamp(0.18f - BalanceErrorFraction * 0.08f, 0.04f, 0.18f));
				Option.MotionFraction = BalanceErrorFraction <= 0.05f
					? 0.36f
					: (bCorrectiveNudge ? FMath::Clamp(0.64f + BalanceErrorFraction * 0.30f, 0.0f, 1.0f) : FMath::Clamp(0.20f - BalanceErrorFraction * 0.06f, 0.08f, 0.20f));
			}
		}
		Options.Add(Option);
	}
	return Options;
}

TArray<FHorrorAdvancedInteractionStepTrackItem> AHorrorCampaignObjectiveActor::BuildAdvancedInteractionStepTrack() const
{
	TArray<FHorrorAdvancedInteractionStepTrackItem> TrackItems;
	if (!UsesAdvancedInteraction())
	{
		return TrackItems;
	}

	const int32 TrackCount = FMath::Max(RequiredInteractionCount, 1);
	TrackItems.Reserve(TrackCount);
	for (int32 StepIndex = 0; StepIndex < TrackCount; ++StepIndex)
	{
		FName StepInputId = ExpectedAdvancedInputId;
		if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring)
		{
			const int32 InputIndex = StepIndex % UE_ARRAY_COUNT(CircuitSafeRoutingSequence);
			StepInputId = CircuitSafeRoutingSequence[InputIndex];
		}
		else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration)
		{
			StepInputId = ResolveGearCalibrationInputForStep(StepIndex);
		}
		else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
		{
			StepInputId = SpectralScanLockInput;
		}
		else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
		{
			StepInputId = SignalTuningConfirmInput;
		}

		FHorrorAdvancedInteractionStepTrackItem TrackItem;
		TrackItem.InputId = StepInputId;
		TrackItem.DisplayLabel = StepInputId.IsNone() ? FText::GetEmpty() : FText::FromName(StepInputId);
		TrackItem.StepIndex = StepIndex;
		TrackItem.VisualSlotIndex = BuildAdvancedInteractionActiveInputSlot();
		if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring)
		{
			TrackItem.VisualSlotIndex = IndexOfAdvancedInput(CircuitInputs, UE_ARRAY_COUNT(CircuitInputs), StepInputId);
		}
		else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration)
		{
			TrackItem.VisualSlotIndex = IndexOfAdvancedInput(GearInputs, UE_ARRAY_COUNT(GearInputs), StepInputId);
		}
		else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
		{
			TrackItem.VisualSlotIndex = IndexOfAdvancedInput(SpectralScanInputs, UE_ARRAY_COUNT(SpectralScanInputs), StepInputId);
		}
		else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
		{
			TrackItem.VisualSlotIndex = IndexOfAdvancedInput(SignalTuningInputs, UE_ARRAY_COUNT(SignalTuningInputs), StepInputId);
		}
		TrackItem.KeyHint = AdvancedDeviceKeyHints[FMath::Clamp(TrackItem.VisualSlotIndex, 0, UE_ARRAY_COUNT(AdvancedDeviceKeyHints) - 1)];
		TrackItem.VisualColor = ResolveAdvancedInputVisualColor(
			ObjectiveDefinition.InteractionMode,
			FMath::Clamp(TrackItem.VisualSlotIndex, 0, UE_ARRAY_COUNT(AdvancedDeviceKeyHints) - 1));
		TrackItem.bComplete = StepIndex < AdvancedInteractionStepIndex;
		TrackItem.bActive = StepIndex == AdvancedInteractionStepIndex && bAdvancedInteractionActive && !bObjectiveFailedRetryable;
		TrackItem.bPreview = StepIndex > AdvancedInteractionStepIndex;
		TrackItem.bHazardous = ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring
			&& StepInputId == CircuitHazardInput;
		TrackItem.bRequiresTimingWindow = ObjectiveDefinition.InteractionMode != EHorrorCampaignInteractionMode::SignalTuning
			|| StepInputId == SignalTuningConfirmInput;
		TrackItems.Add(TrackItem);
	}

	return TrackItems;
}

FText AHorrorCampaignObjectiveActor::BuildAdvancedInteractionPhaseText() const
{
	if (bObjectiveFailedRetryable)
	{
		switch (ObjectiveDefinition.InteractionMode)
		{
			case EHorrorCampaignInteractionMode::CircuitWiring:
				return NSLOCTEXT("HorrorCampaignObjective", "CircuitPhaseRetryableFailure", "电路已断开，等待重新接线");
			case EHorrorCampaignInteractionMode::GearCalibration:
				return NSLOCTEXT("HorrorCampaignObjective", "GearPhaseRetryableFailure", "齿轮机构已锁死，等待重新校准");
			case EHorrorCampaignInteractionMode::SpectralScan:
				return NSLOCTEXT("HorrorCampaignObjective", "SpectralScanPhaseRetryableFailure", "频谱扫描已过载，等待重新扫描");
			case EHorrorCampaignInteractionMode::SignalTuning:
				return NSLOCTEXT("HorrorCampaignObjective", "SignalTuningPhaseRetryableFailure", "黑盒回放已静音，等待重新调谐");
			default:
				return NSLOCTEXT("HorrorCampaignObjective", "AdvancedPhaseRetryableFailure", "设备已保护断开，等待重新互动");
		}
	}

	if (!bAdvancedInteractionActive)
	{
		return FText::GetEmpty();
	}

	if (AdvancedInteractionPauseRemainingSeconds > 0.0f)
	{
		return ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration
			? NSLOCTEXT("HorrorCampaignObjective", "GearPhaseJammed", "齿轮卡死，等待复位")
			: (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan
				? NSLOCTEXT("HorrorCampaignObjective", "SpectralScanPhaseDenoising", "扫描器降噪中，等待置信度恢复")
				: (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning
					? NSLOCTEXT("HorrorCampaignObjective", "SignalTuningPhaseMuted", "回放静音保护中，等待声像稳定")
					: NSLOCTEXT("HorrorCampaignObjective", "AdvancedPhasePaused", "设备保护中，等待复位")));
	}

	if (IsAdvancedInteractionTimingWindowOpen())
	{
		switch (ObjectiveDefinition.InteractionMode)
		{
			case EHorrorCampaignInteractionMode::CircuitWiring:
				return NSLOCTEXT("HorrorCampaignObjective", "CircuitPhaseConnectNow", "蓝色同步窗口已打开");
			case EHorrorCampaignInteractionMode::GearCalibration:
				return NSLOCTEXT("HorrorCampaignObjective", "GearPhaseCalibrateNow", "校准窗口已打开");
			case EHorrorCampaignInteractionMode::SpectralScan:
				return NSLOCTEXT("HorrorCampaignObjective", "SpectralScanPhaseLockNow", "扫描锁定带已打开");
			case EHorrorCampaignInteractionMode::SignalTuning:
				return NSLOCTEXT("HorrorCampaignObjective", "SignalTuningPhaseTuneNow", "调谐同步带已打开");
			default:
				return NSLOCTEXT("HorrorCampaignObjective", "AdvancedPhaseWindowOpen", "同步窗口已打开");
		}
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "CircuitPhaseWaiting", "等待端子同步");
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "GearPhaseWaiting", "等待停转齿轮进入窗口");
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
	{
		return IsSpectralScanFilterAligned()
			? NSLOCTEXT("HorrorCampaignObjective", "SpectralScanPhaseWaitingAligned", "滤波焦点已对准异常峰，等待扫描窗口")
			: NSLOCTEXT("HorrorCampaignObjective", "SpectralScanPhaseWaitingFilter", "滤波焦点未对准，先用 A/D 主动扫频");
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
	{
		return IsSignalTuningBalanceAligned()
			? NSLOCTEXT("HorrorCampaignObjective", "SignalTuningPhaseWaitingAligned", "声像已对齐，等待黑盒频率进入同步窗口")
			: NSLOCTEXT("HorrorCampaignObjective", "SignalTuningPhaseWaitingBalance", "调谐声像未居中，先用左右声道微调回中心");
	}

	return NSLOCTEXT("HorrorCampaignObjective", "AdvancedPhaseWaiting", "等待设备同步");
}

FText AHorrorCampaignObjectiveActor::BuildExpectedAdvancedInputLabel() const
{
	return ExpectedAdvancedInputId.IsNone()
		? FText::GetEmpty()
		: FText::FromName(ExpectedAdvancedInputId);
}

FText AHorrorCampaignObjectiveActor::BuildGearCalibrationChainLabel() const
{
	const int32 ChainStep = FMath::Clamp(AdvancedInteractionStepIndex, 0, UE_ARRAY_COUNT(GearCalibrationSequence) - 1);
	switch (ChainStep)
	{
		case 0:
			return NSLOCTEXT("HorrorCampaignObjective", "GearChainDrive", "驱动齿轮（齿轮2）");
		case 1:
			if (ResolveGearCalibrationInputForStep(ChainStep) == GearCalibrationSequence[1])
			{
				return NSLOCTEXT("HorrorCampaignObjective", "GearChainLeft", "左侧传动齿轮（齿轮1）");
			}
			return NSLOCTEXT("HorrorCampaignObjective", "GearChainRightFirst", "右侧锁止齿轮（齿轮3）");
		case 2:
			if (ResolveGearCalibrationInputForStep(ChainStep) == GearCalibrationSequence[1])
			{
				return NSLOCTEXT("HorrorCampaignObjective", "GearChainLeftSecond", "左侧传动齿轮（齿轮1）");
			}
			return NSLOCTEXT("HorrorCampaignObjective", "GearChainRight", "右侧锁止齿轮（齿轮3）");
		default:
			return NSLOCTEXT("HorrorCampaignObjective", "GearChainFallback", "传动齿轮");
	}
}

FText AHorrorCampaignObjectiveActor::BuildAdvancedInteractionDeviceStatusLabel() const
{
	if (!bAdvancedInteractionActive && !bObjectiveFailedRetryable)
	{
		return FText::GetEmpty();
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultCircuitHazard)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "CircuitDiagnosticShortCircuitFault", "短路警报：红色漏电端子过载");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSpectralLowConfidence)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "SpectralDiagnosticLowConfidenceFault", "置信度不足：假峰值正在反冲");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSpectralFilter)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "SpectralDiagnosticFilterFaultV2", "扫描滤波焦点偏移：异常峰未压住");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSpectralWrongControl)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "SpectralDiagnosticWrongControlFault", "锁定指令错误：需要中心波段确认");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSignalBalance)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "SignalDiagnosticBalanceFault", "声像未居中：回放通道正在撕裂");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSignalWrongControl)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "SignalDiagnosticWrongControlFault", "锁定指令错误：需要中心频率确认");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultTiming)
	{
		return IsGearCalibrationMode()
			? NSLOCTEXT("HorrorCampaignObjective", "GearDiagnosticTimingFault", "连锁中断：校准窗口已错过")
			: NSLOCTEXT("HorrorCampaignObjective", "AdvancedDiagnosticTimingFault", "时机偏离：同步窗口已错过");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultWrongInput)
	{
		return IsGearCalibrationMode()
			? NSLOCTEXT("HorrorCampaignObjective", "GearDiagnosticWrongInputFault", "连锁中断：拨错停转齿轮")
			: NSLOCTEXT("HorrorCampaignObjective", "AdvancedDiagnosticWrongInputFault", "输入不匹配：目标通道未选中");
	}

	if (bObjectiveFailedRetryable || AdvancedInteractionStabilityFraction <= 0.05f)
	{
		switch (ObjectiveDefinition.InteractionMode)
		{
			case EHorrorCampaignInteractionMode::CircuitWiring:
				return NSLOCTEXT("HorrorCampaignObjective", "CircuitDiagnosticOverloaded", "端子过载，电弧失控");
			case EHorrorCampaignInteractionMode::GearCalibration:
				return NSLOCTEXT("HorrorCampaignObjective", "GearDiagnosticOverloaded", "齿轮过载，轴承抱死");
			case EHorrorCampaignInteractionMode::SpectralScan:
				return NSLOCTEXT("HorrorCampaignObjective", "SpectralDiagnosticOverloaded", "扫描过载，假峰值淹没信号");
			case EHorrorCampaignInteractionMode::SignalTuning:
				return NSLOCTEXT("HorrorCampaignObjective", "SignalDiagnosticOverloaded", "声像过载，黑盒回放撕裂");
			default:
				return NSLOCTEXT("HorrorCampaignObjective", "AdvancedDiagnosticOverloaded", "设备过载，等待重启");
		}
	}

	if (AdvancedInteractionPauseRemainingSeconds > 0.0f)
	{
		switch (ObjectiveDefinition.InteractionMode)
		{
			case EHorrorCampaignInteractionMode::CircuitWiring:
				return NSLOCTEXT("HorrorCampaignObjective", "CircuitDiagnosticPaused", "端子保护中，残压释放");
			case EHorrorCampaignInteractionMode::GearCalibration:
				return NSLOCTEXT("HorrorCampaignObjective", "GearDiagnosticPaused", "齿轮卡滞，等待复位");
			case EHorrorCampaignInteractionMode::SpectralScan:
				return NSLOCTEXT("HorrorCampaignObjective", "SpectralDiagnosticPaused", "扫描降噪中，置信度回升");
			case EHorrorCampaignInteractionMode::SignalTuning:
				return NSLOCTEXT("HorrorCampaignObjective", "SignalDiagnosticPaused", "声像保护中，回放静音");
			default:
				return NSLOCTEXT("HorrorCampaignObjective", "AdvancedDiagnosticPaused", "设备保护中");
		}
	}

	if (ObjectiveFailureAttemptCount > 0)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "AdvancedDiagnosticRetryAssist", "恢复协议已启动，同步窗口加宽");
	}

	switch (ObjectiveDefinition.InteractionMode)
	{
		case EHorrorCampaignInteractionMode::CircuitWiring:
			return IsAdvancedInteractionTimingWindowOpen()
				? NSLOCTEXT("HorrorCampaignObjective", "CircuitDiagnosticWindow", "端子同步，电弧稳定")
				: NSLOCTEXT("HorrorCampaignObjective", "CircuitDiagnosticWaiting", "端子预热，等待蓝光峰值");
		case EHorrorCampaignInteractionMode::GearCalibration:
			if (IsAdvancedInteractionTimingWindowOpen())
			{
				return FText::Format(
					NSLOCTEXT("HorrorCampaignObjective", "GearDiagnosticWindowChain", "齿轮连锁同步：{0} 扭矩稳定"),
					BuildGearCalibrationChainLabel());
			}
			return FText::Format(
				NSLOCTEXT("HorrorCampaignObjective", "GearDiagnosticWaitingChain", "齿轮连锁巡检：等待 {0} 停转点"),
				BuildGearCalibrationChainLabel());
		case EHorrorCampaignInteractionMode::SpectralScan:
			if (!IsSpectralScanConfidenceReady())
			{
				return NSLOCTEXT("HorrorCampaignObjective", "SpectralDiagnosticWaiting", "扫描积累中，主动过滤假峰值");
			}
			return IsSpectralScanFilterAligned()
				? NSLOCTEXT("HorrorCampaignObjective", "SpectralDiagnosticReadyV2", "扫描滤波焦点稳定，异常可锁定")
				: NSLOCTEXT("HorrorCampaignObjective", "SpectralDiagnosticFilterOffsetV2", "扫描滤波焦点偏移，继续扫频压峰");
		case EHorrorCampaignInteractionMode::SignalTuning:
			return IsSignalTuningBalanceAligned()
				? NSLOCTEXT("HorrorCampaignObjective", "SignalDiagnosticReady", "声像居中，黑盒频率可锁定")
				: NSLOCTEXT("HorrorCampaignObjective", "SignalDiagnosticWaiting", "声像偏移，先校正中心");
		default:
			return NSLOCTEXT("HorrorCampaignObjective", "AdvancedDiagnosticWaiting", "设备同步中");
	}
}

FText AHorrorCampaignObjectiveActor::BuildAdvancedInteractionRiskLabel() const
{
	if (!bAdvancedInteractionActive && !bObjectiveFailedRetryable)
	{
		return FText::GetEmpty();
	}

	if (bObjectiveFailedRetryable || AdvancedInteractionStabilityFraction <= 0.18f)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "AdvancedRiskOverload", "过载风险：临界");
	}

	if (AdvancedInteractionMistakeCount >= 2 || AdvancedInteractionStabilityFraction < 0.45f)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "AdvancedRiskHigh", "过载风险：高");
	}

	if (AdvancedInteractionMistakeCount > 0 || AdvancedInteractionStabilityFraction < 0.72f)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "AdvancedRiskWatch", "过载风险：警戒");
	}

	return AdvancedInteractionComboCount >= 2
		? NSLOCTEXT("HorrorCampaignObjective", "AdvancedRiskCleanCombo", "过载风险：低，连线稳定")
		: NSLOCTEXT("HorrorCampaignObjective", "AdvancedRiskLow", "过载风险：低");
}

FText AHorrorCampaignObjectiveActor::BuildAdvancedInteractionRhythmLabel() const
{
	if (!bAdvancedInteractionActive && !bObjectiveFailedRetryable)
	{
		return FText::GetEmpty();
	}

	if (bObjectiveFailedRetryable)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "AdvancedRhythmRetryableFailure", "恢复状态：等待重新互动");
	}

	if (ObjectiveFailureAttemptCount > 0 && AdvancedInteractionPauseRemainingSeconds <= 0.0f)
	{
		return IsAdvancedInteractionTimingWindowOpen()
			? NSLOCTEXT("HorrorCampaignObjective", "AdvancedRhythmRetryAssistWindow", "恢复协议：窗口已加宽，稳定输入")
			: NSLOCTEXT("HorrorCampaignObjective", "AdvancedRhythmRetryAssistWaiting", "恢复协议：下一次窗口更宽");
	}

	if (AdvancedInteractionPauseRemainingSeconds > 0.0f)
	{
		return FText::Format(
			NSLOCTEXT("HorrorCampaignObjective", "AdvancedRhythmPaused", "保护倒计时：{0}秒"),
			FText::AsNumber(FMath::CeilToInt(AdvancedInteractionPauseRemainingSeconds)));
	}

	if (IsAdvancedInteractionTimingWindowOpen())
	{
		switch (ObjectiveDefinition.InteractionMode)
		{
			case EHorrorCampaignInteractionMode::CircuitWiring:
				return FText::Format(
					NSLOCTEXT("HorrorCampaignObjective", "CircuitRhythmWindowRoute", "安全线路第 {0} 段窗口开启，接入 {1}"),
					FText::AsNumber(FMath::Clamp(AdvancedInteractionStepIndex + 1, 1, FMath::Max(1, RequiredInteractionCount))),
					BuildExpectedAdvancedInputLabel());
			case EHorrorCampaignInteractionMode::GearCalibration:
				return NSLOCTEXT("HorrorCampaignObjective", "GearRhythmWindow", "校准窗口开启，拨动停转齿轮");
			case EHorrorCampaignInteractionMode::SpectralScan:
				return IsSpectralScanConfidenceReady()
					? NSLOCTEXT("HorrorCampaignObjective", "SpectralRhythmReadyWindow", "置信窗口开启，锁定波段")
					: NSLOCTEXT("HorrorCampaignObjective", "SpectralRhythmWeakWindow", "置信不足，先稳住扫描");
			case EHorrorCampaignInteractionMode::SignalTuning:
				return IsSignalTuningBalanceAligned()
					? NSLOCTEXT("HorrorCampaignObjective", "SignalRhythmReadyWindow", "中心窗口开启，锁定频率")
					: NSLOCTEXT("HorrorCampaignObjective", "SignalRhythmOffsetWindow", "中心偏移，先微调声道");
			default:
				return NSLOCTEXT("HorrorCampaignObjective", "AdvancedRhythmWindow", "同步窗口开启");
		}
	}

	const float WindowStart = GetAdvancedInteractionTimingWindowStart();
	float DistanceToWindow = WindowStart - AdvancedInteractionTimingFraction;
	if (DistanceToWindow < 0.0f)
	{
		DistanceToWindow += 1.0f;
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
	{
		return SpectralScanConfidenceFraction >= SpectralScanMinimumConfidenceToLock
			? (IsSpectralScanFilterAligned()
				? NSLOCTEXT("HorrorCampaignObjective", "SpectralRhythmWaitingReady", "滤波焦点已压中异常峰，等待锁定窗口")
				: NSLOCTEXT("HorrorCampaignObjective", "SpectralRhythmWaitingFilter", "置信已足，继续 A/D 扫频压住异常峰"))
			: NSLOCTEXT("HorrorCampaignObjective", "SpectralRhythmWaitingConfidence", "置信积累中，主动扫频滤噪");
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
	{
		return IsSignalTuningBalanceAligned()
			? NSLOCTEXT("HorrorCampaignObjective", "SignalRhythmWaitingCentered", "声像居中，等待同步窗口")
			: NSLOCTEXT("HorrorCampaignObjective", "SignalRhythmWaitingBalance", "声像未居中，微调到中心");
	}

	if (IsGearCalibrationMode())
	{
		if (AdvancedInteractionComboCount > 0)
		{
			return FText::Format(
				NSLOCTEXT("HorrorCampaignObjective", "GearRhythmChainCombo", "齿轮连锁：第 {0} 拍已咬合，准备下一拍"),
				FText::AsNumber(FMath::Clamp(AdvancedInteractionComboCount, 1, FMath::Max(1, RequiredInteractionCount))));
		}

		return FText::Format(
			NSLOCTEXT("HorrorCampaignObjective", "GearRhythmChainWaiting", "齿轮连锁：先锁 {0}，再接左右传动"),
			BuildGearCalibrationChainLabel());
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring)
	{
		if (AdvancedInteractionComboCount > 0)
		{
			return FText::Format(
				NSLOCTEXT("HorrorCampaignObjective", "CircuitRhythmRouteCombo", "安全线路：第 {0} 段已接入，准备下一段 {1}"),
				FText::AsNumber(FMath::Clamp(AdvancedInteractionComboCount, 1, FMath::Max(1, RequiredInteractionCount))),
				BuildExpectedAdvancedInputLabel());
		}

		return FText::Format(
			NSLOCTEXT("HorrorCampaignObjective", "CircuitRhythmRouteWaiting", "安全线路：等待 {0} 进入蓝光同步窗"),
			BuildExpectedAdvancedInputLabel());
	}

	return DistanceToWindow <= 0.18f
		? NSLOCTEXT("HorrorCampaignObjective", "AdvancedRhythmNear", "窗口将至，准备输入")
		: NSLOCTEXT("HorrorCampaignObjective", "AdvancedRhythmWaiting", "等待下一次窗口");
}

FText AHorrorCampaignObjectiveActor::BuildAdvancedInteractionNextActionLabel() const
{
	if (!bAdvancedInteractionActive && !bObjectiveFailedRetryable)
	{
		return FText::GetEmpty();
	}

	if (bObjectiveFailedRetryable)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "AdvancedNextActionRetry", "重新互动，重启设备窗口");
	}

	if (AdvancedInteractionPauseRemainingSeconds > 0.0f)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "AdvancedNextActionWaitProtection", "等待保护结束，不要连续输入");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultCircuitHazard)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "CircuitNextActionAfterShortCircuit", "避开红色端子，等待下一轮安全高亮端子");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSpectralLowConfidence)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "SpectralNextActionAfterLowConfidence", "先保持扫描，置信度足够后再锁定波段");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSpectralFilter)
	{
		return SpectralScanFilterFocusFraction < SpectralScanTargetFocusFraction
			? NSLOCTEXT("HorrorCampaignObjective", "SpectralNextActionAfterFilterRight", "先按 D 扫频到异常峰，再用 S 锁定")
			: NSLOCTEXT("HorrorCampaignObjective", "SpectralNextActionAfterFilterLeft", "先按 A 扫频到异常峰，再用 S 锁定");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSpectralWrongControl)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "SpectralNextActionAfterWrongControl", "A/D 只负责扫频，锁定必须使用 S");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSignalBalance)
	{
		return SignalTuningBalanceFraction < SignalTuningTargetBalanceFraction
			? NSLOCTEXT("HorrorCampaignObjective", "SignalNextActionAfterBalanceRight", "先按 D 微调右声道，再用 S 锁定中心频率")
			: NSLOCTEXT("HorrorCampaignObjective", "SignalNextActionAfterBalanceLeft", "先按 A 微调左声道，再用 S 锁定中心频率");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSignalWrongControl)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "SignalNextActionAfterWrongControl", "不要用左右声道锁定，声像居中后按 S");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultTiming)
	{
		return IsGearCalibrationMode()
			? NSLOCTEXT("HorrorCampaignObjective", "GearNextActionAfterTimingFault", "连锁中断，等下一次校准窗口亮起再拨动高亮齿轮")
			: NSLOCTEXT("HorrorCampaignObjective", "AdvancedNextActionAfterTimingFault", "等下一次同步窗口亮起，再按高亮键");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultWrongInput)
	{
		return IsGearCalibrationMode()
			? FText::Format(
				NSLOCTEXT("HorrorCampaignObjective", "GearNextActionAfterWrongInput", "连锁中断，下一轮改拨当前高亮齿轮：{0}"),
				BuildExpectedAdvancedInputLabel())
			: FText::Format(
				NSLOCTEXT("HorrorCampaignObjective", "AdvancedNextActionAfterWrongInput", "改按当前高亮目标：{0}"),
				BuildExpectedAdvancedInputLabel());
	}

	switch (ObjectiveDefinition.InteractionMode)
	{
		case EHorrorCampaignInteractionMode::CircuitWiring:
			return IsAdvancedInteractionTimingWindowOpen()
				? FText::Format(
					NSLOCTEXT("HorrorCampaignObjective", "CircuitNextActionWindow", "按高亮键接入：{0}"),
					BuildExpectedAdvancedInputLabel())
				: FText::Format(
					NSLOCTEXT("HorrorCampaignObjective", "CircuitNextActionWaitingTerminal", "盯住光标，靠近亮窗后接入：{0}"),
					BuildExpectedAdvancedInputLabel());
		case EHorrorCampaignInteractionMode::GearCalibration:
			if (IsAdvancedInteractionTimingWindowOpen())
			{
				return FText::Format(
					NSLOCTEXT("HorrorCampaignObjective", "GearNextActionWindowChain", "连锁第 {0} 拍：按高亮键拨动 {1}"),
					FText::AsNumber(FMath::Clamp(AdvancedInteractionStepIndex + 1, 1, FMath::Max(1, RequiredInteractionCount))),
					BuildExpectedAdvancedInputLabel());
			}
			return AdvancedInteractionComboCount > 0
				? FText::Format(
					NSLOCTEXT("HorrorCampaignObjective", "GearNextActionNextBeat", "下一拍准备：{0}，等停转窗口再拨动"),
					BuildGearCalibrationChainLabel())
				: FText::Format(
					NSLOCTEXT("HorrorCampaignObjective", "GearNextActionDriveWaiting", "观察驱动连锁，先等 {0} 进入窗口"),
					BuildGearCalibrationChainLabel());
		case EHorrorCampaignInteractionMode::SpectralScan:
			if (!IsSpectralScanConfidenceReady())
			{
				return NSLOCTEXT("HorrorCampaignObjective", "SpectralNextActionBuildConfidence", "用 A/D 主动扫频滤噪，抬高置信度");
			}
			if (!IsSpectralScanFilterAligned())
			{
				return SpectralScanFilterFocusFraction < SpectralScanTargetFocusFraction
					? NSLOCTEXT("HorrorCampaignObjective", "SpectralNextActionSweepRight", "按 D 扫向高频，把滤波焦点压上异常峰")
					: NSLOCTEXT("HorrorCampaignObjective", "SpectralNextActionSweepLeft", "按 A 扫向低频，把滤波焦点压上异常峰");
			}
			return IsAdvancedInteractionTimingWindowOpen()
				? NSLOCTEXT("HorrorCampaignObjective", "SpectralNextActionWindowFocused", "按 S 锁定异常峰，写入黑盒")
				: NSLOCTEXT("HorrorCampaignObjective", "SpectralNextActionWaiting", "置信已足，等待锁定带");
		case EHorrorCampaignInteractionMode::SignalTuning:
			if (!IsSignalTuningBalanceAligned())
			{
				return SignalTuningBalanceFraction < SignalTuningTargetBalanceFraction
					? NSLOCTEXT("HorrorCampaignObjective", "SignalNextActionRight", "按 D 微调右声道，把声像推回中心")
					: NSLOCTEXT("HorrorCampaignObjective", "SignalNextActionLeft", "按 A 微调左声道，把声像拉回中心");
			}
			return IsAdvancedInteractionTimingWindowOpen()
				? NSLOCTEXT("HorrorCampaignObjective", "SignalNextActionWindow", "按 S 锁定中心频率，稳定黑盒回放")
				: NSLOCTEXT("HorrorCampaignObjective", "SignalNextActionWaiting", "声像已居中，等待同步带");
		default:
			return NSLOCTEXT("HorrorCampaignObjective", "AdvancedNextActionWaiting", "等待同步窗口");
	}
}

FText AHorrorCampaignObjectiveActor::BuildAdvancedInteractionFailureRecoveryLabel() const
{
	if (!bAdvancedInteractionActive && !bObjectiveFailedRetryable)
	{
		return FText::GetEmpty();
	}

	if (!bAdvancedInteractionRecentFailure && AdvancedInteractionPauseRemainingSeconds <= 0.0f && !bObjectiveFailedRetryable)
	{
		return FText::GetEmpty();
	}

	if (bObjectiveFailedRetryable)
	{
		switch (ObjectiveDefinition.InteractionMode)
		{
			case EHorrorCampaignInteractionMode::CircuitWiring:
				return NSLOCTEXT("HorrorCampaignObjective", "CircuitRetryableFailureRecovery", "按互动键重新互动，重启接线盘后只接高亮安全端子");
			case EHorrorCampaignInteractionMode::GearCalibration:
				return NSLOCTEXT("HorrorCampaignObjective", "GearRetryableFailureRecovery", "按互动键重新互动，重试齿轮校准并按连锁节拍输入");
			case EHorrorCampaignInteractionMode::SpectralScan:
				return NSLOCTEXT("HorrorCampaignObjective", "SpectralRetryableFailureRecovery", "按互动键重新互动，重试扫描并等置信度足够后锁定");
			case EHorrorCampaignInteractionMode::SignalTuning:
				return NSLOCTEXT("HorrorCampaignObjective", "SignalRetryableFailureRecovery", "按互动键重新互动，重试调谐；先居中声像，再锁定中心频率");
			default:
				return NSLOCTEXT("HorrorCampaignObjective", "AdvancedRetryableFailureRecovery", "按互动键重新互动，重试当前设备窗口");
		}
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultCircuitHazard)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "CircuitHazardFailureRecovery", "漏电端子会短路，等待下一轮同步后只接安全高亮端子");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSpectralLowConfidence)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "SpectralLowConfidenceFailureRecovery", "不要抢锁假峰值，等置信度抬升后再输入");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSpectralFilter)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "SpectralFilterFailureRecovery", "锁定前先用 A/D 扫频，把滤波焦点压在异常峰上");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSpectralWrongControl)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "SpectralWrongControlFailureRecovery", "低频和高频只负责扫频，确认锁定必须使用中心波段");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSignalBalance)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "SignalBalanceFailureRecovery", "先把声像推回中心，再按 S 锁定回放");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSignalWrongControl)
	{
		return NSLOCTEXT("HorrorCampaignObjective", "SignalWrongControlFailureRecovery", "左右声道只负责微调，锁定必须使用中心频率");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultTiming)
	{
		return IsGearCalibrationMode()
			? NSLOCTEXT("HorrorCampaignObjective", "GearTimingFailureRecoveryChain", "齿轮连锁已断，等驱动齿轮重新入窗后从当前拍重启")
			: NSLOCTEXT("HorrorCampaignObjective", "AdvancedTimingFailureRecovery", "观察光标节奏，等待同步窗口完全亮起再输入");
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultWrongInput)
	{
		return IsGearCalibrationMode()
			? NSLOCTEXT("HorrorCampaignObjective", "GearWrongInputFailureRecoveryChain", "齿轮连锁已断，按高亮齿轮顺序重新接上节拍")
			: NSLOCTEXT("HorrorCampaignObjective", "AdvancedWrongInputFailureRecovery", "目标会轮换，等待窗口亮起后按高亮设备键");
	}

	switch (ObjectiveDefinition.InteractionMode)
	{
		case EHorrorCampaignInteractionMode::CircuitWiring:
			return NSLOCTEXT("HorrorCampaignObjective", "CircuitFailureRecovery", "等待残压消退，下一轮只接入高亮端子");
		case EHorrorCampaignInteractionMode::GearCalibration:
			return NSLOCTEXT("HorrorCampaignObjective", "GearFailureRecovery", "停手等齿轮复位，再按连锁节拍拨动发亮齿轮");
		case EHorrorCampaignInteractionMode::SpectralScan:
			return NSLOCTEXT("HorrorCampaignObjective", "SpectralFailureRecovery", "先主动扫频滤噪，置信度和焦点都稳定后再锁定");
		case EHorrorCampaignInteractionMode::SignalTuning:
			return NSLOCTEXT("HorrorCampaignObjective", "SignalFailureRecovery", "先用左右声道居中，再按中心频率");
		default:
			return NSLOCTEXT("HorrorCampaignObjective", "AdvancedFailureRecovery", "等待设备稳定后重试");
	}
}

float AHorrorCampaignObjectiveActor::BuildAdvancedInteractionPerformanceGrade() const
{
	if (bObjectiveFailedRetryable)
	{
		return 0.0f;
	}

	const float ProgressScore = FMath::Clamp(AdvancedInteractionProgressFraction, 0.0f, 1.0f) * 0.34f;
	const float StabilityScore = FMath::Clamp(AdvancedInteractionStabilityFraction, 0.0f, 1.0f) * 0.38f;
	const float TimingScore = IsAdvancedInteractionTimingWindowOpen() ? 0.08f : 0.03f;
	const float ComboScore = FMath::Clamp(static_cast<float>(AdvancedInteractionComboCount) / 3.0f, 0.0f, 1.0f) * 0.14f;
	const float ModeScore = IsSpectralScanMode()
		? FMath::Clamp(
			(SpectralScanConfidenceFraction - SpectralScanNoiseFraction * 0.35f) * 0.58f
				+ (1.0f - FMath::Clamp(FMath::Abs(SpectralScanFilterFocusFraction - SpectralScanTargetFocusFraction) / SpectralScanFilterLockTolerance, 0.0f, 1.0f)) * 0.42f,
			0.0f,
			1.0f) * 0.06f
		: (IsSignalTuningMode()
			? (1.0f - FMath::Clamp(FMath::Abs(SignalTuningBalanceFraction - SignalTuningTargetBalanceFraction) / SignalTuningLockTolerance, 0.0f, 1.0f)) * 0.06f
			: 0.04f);
	const float MistakePenalty = FMath::Clamp(static_cast<float>(AdvancedInteractionMistakeCount) * 0.12f, 0.0f, 0.42f);
	const float PausePenalty = AdvancedInteractionPauseRemainingSeconds > 0.0f ? 0.12f : 0.0f;
	return FMath::Clamp(ProgressScore + StabilityScore + TimingScore + ComboScore + ModeScore - MistakePenalty - PausePenalty, 0.0f, 1.0f);
}

float AHorrorCampaignObjectiveActor::BuildAdvancedInteractionInputPrecision() const
{
	if (!bAdvancedInteractionActive || bObjectiveFailedRetryable)
	{
		return 0.0f;
	}

	if (AdvancedInteractionPauseRemainingSeconds > 0.0f)
	{
		return FMath::Clamp(AdvancedInteractionStabilityFraction * 0.35f, 0.0f, 1.0f);
	}

	float TimingPrecision = 0.0f;
	const float WindowStart = GetAdvancedInteractionTimingWindowStart();
	const float WindowEnd = GetAdvancedInteractionTimingWindowEnd();
	const float WindowCenter = (WindowStart + WindowEnd) * 0.5f;
	const float HalfWindow = FMath::Max((WindowEnd - WindowStart) * 0.5f, UE_SMALL_NUMBER);
	if (AdvancedInteractionTimingFraction >= WindowStart && AdvancedInteractionTimingFraction <= WindowEnd)
	{
		const float NormalizedDistance = FMath::Abs(AdvancedInteractionTimingFraction - WindowCenter) / HalfWindow;
		TimingPrecision = 1.0f - FMath::Clamp(NormalizedDistance, 0.0f, 1.0f) * 0.35f;
	}
	else
	{
		const float DistanceBefore = FMath::Abs(AdvancedInteractionTimingFraction - WindowStart);
		const float DistanceAfter = FMath::Abs(AdvancedInteractionTimingFraction - WindowEnd);
		const float WrappedDistance = FMath::Min3(DistanceBefore, DistanceAfter, 1.0f - AdvancedInteractionTimingFraction + WindowStart);
		TimingPrecision = FMath::Clamp(1.0f - WrappedDistance / 0.45f, 0.0f, 0.62f);
	}

	if (IsSignalTuningMode())
	{
		const float BalanceError = FMath::Abs(SignalTuningBalanceFraction - SignalTuningTargetBalanceFraction);
		const float BalancePrecision = 1.0f - FMath::Clamp(BalanceError / SignalTuningLockTolerance, 0.0f, 1.0f);
		return FMath::Clamp(FMath::Max(TimingPrecision * 0.45f, BalancePrecision), 0.0f, 1.0f);
	}

	if (IsSpectralScanMode())
	{
		const float ScanPrecision = FMath::Clamp(SpectralScanConfidenceFraction - SpectralScanNoiseFraction * 0.35f, 0.0f, 1.0f);
		const float FilterPrecision = 1.0f - FMath::Clamp(
			FMath::Abs(SpectralScanFilterFocusFraction - SpectralScanTargetFocusFraction) / SpectralScanFilterLockTolerance,
			0.0f,
			1.0f);
		return FMath::Clamp(TimingPrecision * 0.25f + ScanPrecision * 0.45f + FilterPrecision * 0.30f, 0.0f, 1.0f);
	}

	return FMath::Clamp(TimingPrecision, 0.0f, 1.0f);
}

float AHorrorCampaignObjectiveActor::BuildAdvancedInteractionDeviceLoad() const
{
	if (bObjectiveFailedRetryable)
	{
		return 1.0f;
	}

	if (!bAdvancedInteractionActive)
	{
		return 0.0f;
	}

	const float InstabilityLoad = 1.0f - FMath::Clamp(AdvancedInteractionStabilityFraction, 0.0f, 1.0f);
	const float MistakeLoad = FMath::Clamp(static_cast<float>(AdvancedInteractionMistakeCount) * 0.16f, 0.0f, 0.48f);
	const float PauseLoad = AdvancedInteractionPauseRemainingSeconds > 0.0f ? 0.28f : 0.0f;
	const float FeedbackLoad = bAdvancedInteractionRecentFailure
		? 0.22f
		: (bAdvancedInteractionRecentSuccess ? -0.08f : 0.0f);

	float ModeLoad = 0.0f;
	if (IsSignalTuningMode())
	{
		ModeLoad = FMath::Clamp(
			FMath::Abs(SignalTuningBalanceFraction - SignalTuningTargetBalanceFraction) / SignalTuningLockTolerance,
			0.0f,
			1.0f) * 0.18f;
	}
	else if (IsSpectralScanMode())
	{
		const float FilterError = FMath::Clamp(
			FMath::Abs(SpectralScanFilterFocusFraction - SpectralScanTargetFocusFraction) / SpectralScanFilterLockTolerance,
			0.0f,
			1.0f);
		ModeLoad = FMath::Clamp(SpectralScanNoiseFraction * 0.16f + FilterError * 0.18f, 0.0f, 0.34f);
	}
	else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration)
	{
		ModeLoad = IsAdvancedInteractionTimingWindowOpen() ? 0.06f : 0.14f;
	}
	else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring)
	{
		ModeLoad = IsAdvancedInteractionTimingWindowOpen() ? 0.04f : 0.10f;
	}

	return FMath::Clamp(InstabilityLoad + MistakeLoad + PauseLoad + FeedbackLoad + ModeLoad, 0.0f, 1.0f);
}

float AHorrorCampaignObjectiveActor::BuildAdvancedInteractionRouteFlow() const
{
	if (!bAdvancedInteractionActive || bObjectiveFailedRetryable)
	{
		return 0.0f;
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring)
	{
		const int32 SafeRouteCount = UE_ARRAY_COUNT(CircuitSafeRoutingSequence);
		if (SafeRouteCount <= 0)
		{
			return 0.0f;
		}

		return FMath::Clamp(
			static_cast<float>(AdvancedInteractionStepIndex) / static_cast<float>(SafeRouteCount),
			0.0f,
			1.0f);
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration)
	{
		return FMath::Clamp(
			static_cast<float>(AdvancedInteractionStepIndex) / static_cast<float>(FMath::Max(RequiredInteractionCount, 1)),
			0.0f,
			1.0f);
	}

	return FMath::Clamp(AdvancedInteractionProgressFraction, 0.0f, 1.0f);
}

float AHorrorCampaignObjectiveActor::BuildAdvancedInteractionHazardPressure() const
{
	if (bObjectiveFailedRetryable)
	{
		return 1.0f;
	}

	if (!bAdvancedInteractionActive)
	{
		return 0.0f;
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring)
	{
		return FMath::Clamp(
			0.82f
				+ static_cast<float>(AdvancedInteractionMistakeCount) * 0.05f
				+ (LastAdvancedInteractionFaultId == AdvancedFaultCircuitHazard ? 0.10f : 0.0f),
			0.0f,
			1.0f);
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration)
	{
		const int32 StoppedGearIndex = IndexOfAdvancedInput(GearInputs, UE_ARRAY_COUNT(GearInputs), ExpectedAdvancedInputId);
		const float JamLoad = FMath::Clamp(
			0.34f
				+ (AdvancedInteractionPauseRemainingSeconds > 0.0f ? 0.32f : 0.0f)
				+ FMath::Clamp(static_cast<float>(AdvancedInteractionMistakeCount) * 0.08f, 0.0f, 0.28f)
				+ (LastAdvancedInteractionFaultId == AdvancedFaultTiming || LastAdvancedInteractionFaultId == AdvancedFaultWrongInput ? 0.12f : 0.0f),
			0.0f,
			1.0f);
		const float NeighborDrag = StoppedGearIndex == INDEX_NONE
			? 0.10f
			: FMath::Clamp(0.10f + FMath::Abs(StoppedGearIndex - 1) * 0.04f, 0.10f, 0.18f);
		return FMath::Clamp(FMath::Max(JamLoad, NeighborDrag), 0.0f, 1.0f);
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
	{
		const float FilterError = FMath::Clamp(
			FMath::Abs(SpectralScanFilterFocusFraction - SpectralScanTargetFocusFraction) / SpectralScanFilterLockTolerance,
			0.0f,
			1.0f);
		return FMath::Clamp(SpectralScanNoiseFraction * 0.58f + FilterError * 0.42f, 0.0f, 1.0f);
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
	{
		return FMath::Clamp(
			FMath::Abs(SignalTuningBalanceFraction - SignalTuningTargetBalanceFraction) / SignalTuningLockTolerance,
			0.0f,
			1.0f);
	}

	return BuildAdvancedInteractionDeviceLoad();
}

float AHorrorCampaignObjectiveActor::BuildAdvancedInteractionTargetAlignment() const
{
	if (!bAdvancedInteractionActive || bObjectiveFailedRetryable)
	{
		return 0.0f;
	}

	const float WindowStart = GetAdvancedInteractionTimingWindowStart();
	const float WindowEnd = GetAdvancedInteractionTimingWindowEnd();
	const float WindowCenter = (WindowStart + WindowEnd) * 0.5f;
	const float HalfWindow = FMath::Max((WindowEnd - WindowStart) * 0.5f, UE_SMALL_NUMBER);
	const float DistanceToCenter = FMath::Abs(AdvancedInteractionTimingFraction - WindowCenter);
	const float WindowAlignment = FMath::Clamp(1.0f - DistanceToCenter / FMath::Max(HalfWindow, 0.01f), 0.0f, 1.0f);

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
	{
		const float BalanceError = FMath::Abs(SignalTuningBalanceFraction - SignalTuningTargetBalanceFraction);
		const float BalanceAlignment = 1.0f - FMath::Clamp(BalanceError / SignalTuningLockTolerance, 0.0f, 1.0f);
		return FMath::Clamp(FMath::Max(WindowAlignment * 0.45f, BalanceAlignment), 0.0f, 1.0f);
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
	{
		const float FilterAlignment = 1.0f - FMath::Clamp(
			FMath::Abs(SpectralScanFilterFocusFraction - SpectralScanTargetFocusFraction) / SpectralScanFilterLockTolerance,
			0.0f,
			1.0f);
		return FMath::Clamp(WindowAlignment * 0.25f + SpectralScanConfidenceFraction * 0.45f + FilterAlignment * 0.30f, 0.0f, 1.0f);
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration)
	{
		const float ChainStability = FMath::Clamp(AdvancedInteractionStabilityFraction, 0.0f, 1.0f);
		const float JamDrag = 1.0f - BuildAdvancedInteractionHazardPressure() * 0.25f;
		return FMath::Clamp(WindowAlignment * 0.72f + ChainStability * 0.28f, 0.0f, 1.0f) * FMath::Clamp(JamDrag, 0.62f, 1.0f);
	}

	return WindowAlignment;
}

int32 AHorrorCampaignObjectiveActor::BuildAdvancedInteractionActiveInputSlot() const
{
	if (ExpectedAdvancedInputId.IsNone())
	{
		return INDEX_NONE;
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring)
	{
		return IndexOfAdvancedInput(CircuitInputs, UE_ARRAY_COUNT(CircuitInputs), ExpectedAdvancedInputId);
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration)
	{
		return IndexOfAdvancedInput(GearInputs, UE_ARRAY_COUNT(GearInputs), ExpectedAdvancedInputId);
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
	{
		return IndexOfAdvancedInput(SpectralScanInputs, UE_ARRAY_COUNT(SpectralScanInputs), ExpectedAdvancedInputId);
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
	{
		return IndexOfAdvancedInput(SignalTuningInputs, UE_ARRAY_COUNT(SignalTuningInputs), ExpectedAdvancedInputId);
	}

	return INDEX_NONE;
}

EHorrorAdvancedInteractionFeedbackState AHorrorCampaignObjectiveActor::GetAdvancedInteractionFeedbackState() const
{
	if (bObjectiveFailedRetryable)
	{
		return EHorrorAdvancedInteractionFeedbackState::Overloaded;
	}

	if (AdvancedInteractionPauseRemainingSeconds > 0.0f)
	{
		return EHorrorAdvancedInteractionFeedbackState::Paused;
	}

	if (bAdvancedInteractionRecentFailure)
	{
		return EHorrorAdvancedInteractionFeedbackState::Failure;
	}

	if (bAdvancedInteractionRecentSuccess)
	{
		return EHorrorAdvancedInteractionFeedbackState::Success;
	}

	return EHorrorAdvancedInteractionFeedbackState::Neutral;
}

bool AHorrorCampaignObjectiveActor::IsAdvancedInteractionTimingWindowOpen() const
{
	return bAdvancedInteractionActive
		&& !bAdvancedInteractionCueResolved
		&& AdvancedInteractionPauseRemainingSeconds <= 0.0f
		&& AdvancedInteractionTimingFraction >= GetAdvancedInteractionTimingWindowStart()
		&& AdvancedInteractionTimingFraction <= GetAdvancedInteractionTimingWindowEnd();
}

FHorrorAdvancedInteractionHUDState AHorrorCampaignObjectiveActor::BuildAdvancedInteractionHUDState() const
{
	FHorrorAdvancedInteractionHUDState State;
	State.bVisible = (bAdvancedInteractionActive || bObjectiveFailedRetryable) && UsesAdvancedInteraction() && !bCompleted;
	State.Mode = ObjectiveDefinition.InteractionMode;
	State.Title = ObjectiveDefinition.PromptText.IsEmpty()
		? NSLOCTEXT("HorrorCampaignObjective", "AdvancedHUDDefaultTitle", "校准目标")
		: ObjectiveDefinition.PromptText;
	State.ExpectedInputId = ExpectedAdvancedInputId;
	State.FeedbackText = AdvancedInteractionFeedbackText;
	State.FeedbackState = GetAdvancedInteractionFeedbackState();
	State.LastOutcome = LastAdvancedInteractionOutcome;
	State.CurrentBeatLabel = GetCurrentObjectiveBeatLabel();
	State.CurrentBeatDetail = GetCurrentObjectiveBeatDetail();
	State.PhaseText = BuildAdvancedInteractionPhaseText();
	State.ExpectedInputLabel = BuildExpectedAdvancedInputLabel();
	State.DeviceStatusLabel = BuildAdvancedInteractionDeviceStatusLabel();
	State.RiskLabel = BuildAdvancedInteractionRiskLabel();
	State.RhythmLabel = BuildAdvancedInteractionRhythmLabel();
	State.NextActionLabel = BuildAdvancedInteractionNextActionLabel();
	State.FailureRecoveryLabel = BuildAdvancedInteractionFailureRecoveryLabel();
	State.InputOptions = BuildAdvancedInteractionInputOptions();
	State.StepTrack = BuildAdvancedInteractionStepTrack();
	State.ProgressFraction = FMath::Clamp(AdvancedInteractionProgressFraction, 0.0f, 1.0f);
	State.TimingFraction = FMath::Clamp(AdvancedInteractionTimingFraction, 0.0f, 1.0f);
	State.TimingWindowStartFraction = GetAdvancedInteractionTimingWindowStart();
	State.TimingWindowEndFraction = GetAdvancedInteractionTimingWindowEnd();
	State.StabilityFraction = FMath::Clamp(AdvancedInteractionStabilityFraction, 0.0f, 1.0f);
	State.SignalBalanceFraction = FMath::Clamp(SignalTuningBalanceFraction, 0.0f, 1.0f);
	State.SignalTargetBalanceFraction = FMath::Clamp(SignalTuningTargetBalanceFraction, 0.0f, 1.0f);
	State.SpectralConfidenceFraction = FMath::Clamp(SpectralScanConfidenceFraction, 0.0f, 1.0f);
	State.SpectralNoiseFraction = FMath::Clamp(SpectralScanNoiseFraction, 0.0f, 1.0f);
	State.SpectralFilterFocusFraction = FMath::Clamp(SpectralScanFilterFocusFraction, 0.0f, 1.0f);
	State.SpectralTargetFocusFraction = FMath::Clamp(SpectralScanTargetFocusFraction, 0.0f, 1.0f);
	State.PerformanceGradeFraction = BuildAdvancedInteractionPerformanceGrade();
	State.InputPrecisionFraction = BuildAdvancedInteractionInputPrecision();
	State.DeviceLoadFraction = BuildAdvancedInteractionDeviceLoad();
	State.RouteFlowFraction = BuildAdvancedInteractionRouteFlow();
	State.HazardPressureFraction = BuildAdvancedInteractionHazardPressure();
	State.TargetAlignmentFraction = BuildAdvancedInteractionTargetAlignment();
	State.PauseRemainingSeconds = FMath::Max(0.0f, AdvancedInteractionPauseRemainingSeconds);
	State.bTimingWindowOpen = IsAdvancedInteractionTimingWindowOpen();
	State.bPaused = AdvancedInteractionPauseRemainingSeconds > 0.0f;
	State.bRecentSuccess = bAdvancedInteractionRecentSuccess;
	State.bRecentFailure = bAdvancedInteractionRecentFailure;
	State.StepIndex = FMath::Max(0, AdvancedInteractionStepIndex);
	State.RequiredStepCount = FMath::Max(RequiredInteractionCount, 1);
	State.ComboCount = FMath::Max(0, AdvancedInteractionComboCount);
	State.MistakeCount = FMath::Max(0, AdvancedInteractionMistakeCount);
	State.ActiveInputSlotIndex = BuildAdvancedInteractionActiveInputSlot();
	NormalizeHorrorAdvancedInteractionHUDState(State);
	return State;
}

FHorrorCampaignObjectiveRuntimeState AHorrorCampaignObjectiveActor::BuildObjectiveRuntimeState() const
{
	FHorrorCampaignObjectiveRuntimeState State;
	State.bVisible = !ObjectiveDefinition.ObjectiveId.IsNone() && bPresentationVisible;
	State.bCanInteract = bAvailableForInteraction && !bCompleted && !bTimedObjectiveActive;
	State.bAvailableForInteraction = bAvailableForInteraction;
	State.bCompleted = bCompleted;
	State.bAdvancedInteractionActive = bAdvancedInteractionActive;
	State.bTimedObjectiveActive = bTimedObjectiveActive;
	State.bRecoverRelicAwaitingDelivery = bRecoverRelicAwaitingDelivery;
	State.bUsesEscapeDestination = bTimedObjectiveUsesEscapeDestination;
	State.bBeatUrgent = IsCurrentObjectiveBeatUrgent();
	State.bRequiresRecording = DoesCurrentObjectiveBeatRequireRecording();
	State.bRetryable = bObjectiveFailedRetryable;
	State.ChapterId = ChapterId;
	State.ObjectiveId = ObjectiveDefinition.ObjectiveId;
	State.FailureCause = LastObjectiveFailureCause;
	State.RecoveryAction = LastObjectiveRecoveryAction;
	State.InteractionMode = ObjectiveDefinition.InteractionMode;
	State.RiskLevel = ObjectiveDefinition.Presentation.RiskLevel;
	State.Title = ObjectiveDefinition.PromptText.IsEmpty()
		? NSLOCTEXT("HorrorCampaignObjective", "RuntimeStateDefaultTitle", "当前目标")
		: ObjectiveDefinition.PromptText;
	State.PromptText = GetInteractionPromptText();
	State.CurrentBeatIndex = GetCurrentObjectiveBeatIndex();
	State.CurrentBeatLabel = GetCurrentObjectiveBeatLabel();
	State.CurrentBeatDetail = GetCurrentObjectiveBeatDetail();
	State.CompletedStepCount = FMath::Clamp(InteractionProgressCount, 0, FMath::Max(RequiredInteractionCount, 1));
	State.RequiredStepCount = FMath::Max(RequiredInteractionCount, 1);
	State.TimedRemainingSeconds = FMath::Max(0.0f, TimedObjectiveRemainingSeconds);
	State.TimedDurationSeconds = FMath::Max(0.0f, TimedObjectiveDurationSeconds);
	State.EscapeDestinationWorldLocation = TimedObjectiveEscapeDestinationLocation;
	State.EscapeCompletionRadius = GetEscapeCompletionRadius();
	State.RelicDeliveryWorldLocation = ResolveRecoverRelicDeliveryWorldLocation();
	State.RelicDeliveryDistanceMeters = bRecoverRelicAwaitingDelivery
		? ResolveRecoverRelicRouteDistanceMeters()
		: 0.0f;
	State.RelicDeliveryCompletionRadius = GetRecoverRelicDeliveryCompletionRadius();

	if (bAdvancedInteractionActive)
	{
		State.Status = EHorrorCampaignObjectiveRuntimeStatus::AdvancedInteractionActive;
		State.AdvancedInteraction = BuildAdvancedInteractionHUDState();
		State.PhaseText = State.AdvancedInteraction.PhaseText;
		State.DeviceStatusLabel = State.AdvancedInteraction.DeviceStatusLabel;
		State.NextActionLabel = State.AdvancedInteraction.NextActionLabel;
		State.FailureRecoveryLabel = State.AdvancedInteraction.FailureRecoveryLabel;
		State.ProgressFraction = State.AdvancedInteraction.ProgressFraction;
	}
	else if (bTimedObjectiveActive)
	{
		State.Status = EHorrorCampaignObjectiveRuntimeStatus::TimedPursuitActive;
		const float Duration = FMath::Max(TimedObjectiveDurationSeconds, UE_SMALL_NUMBER);
		if (bTimedObjectiveUsesEscapeDestination)
		{
			const AActor* InstigatorActor = TimedObjectiveInstigator.Get();
			const FVector RunnerLocation = InstigatorActor ? InstigatorActor->GetActorLocation() : GetActorLocation();
			const float EscapeDistanceCm = FVector::Dist2D(RunnerLocation, TimedObjectiveEscapeDestinationLocation);
			State.EscapeDistanceMeters = FMath::Max(0.0f, EscapeDistanceCm / 100.0f);
			State.EstimatedEscapeArrivalSeconds = EscapeDistanceCm / ConservativePursuitEscapeSpeedCmPerSecond;
			State.EscapeTimeBudgetSeconds = State.TimedRemainingSeconds - State.EstimatedEscapeArrivalSeconds;
			State.EscapeBudgetLabel = BuildEscapeBudgetLabel(State.EscapeTimeBudgetSeconds);
			State.EscapeActionLabel = BuildEscapeActionLabel(
				State.EscapeDistanceMeters,
				State.EstimatedEscapeArrivalSeconds,
				State.EscapeTimeBudgetSeconds);
		}
		State.PhaseText = bTimedObjectiveUsesEscapeDestination
			? (State.EscapeBudgetLabel.IsEmpty()
				? NSLOCTEXT("HorrorCampaignObjective", "RuntimePursuitEscapePhase", "石像巨人追来了，逃向亮起的逃离点")
				: FText::Format(
					NSLOCTEXT("HorrorCampaignObjective", "RuntimePursuitEscapeBudgetPhase", "石像巨人追来了，逃向亮起的逃离点（{0}）"),
					State.EscapeBudgetLabel))
			: NSLOCTEXT("HorrorCampaignObjective", "RuntimePursuitSurvivePhase", "甩开石像巨人的追赶");
		State.ProgressFraction = FMath::Clamp(1.0f - (TimedObjectiveRemainingSeconds / Duration), 0.0f, 1.0f);
	}
	else if (bRecoverRelicAwaitingDelivery)
	{
		State.Status = EHorrorCampaignObjectiveRuntimeStatus::CarryingRelic;
		State.PhaseText = NSLOCTEXT("HorrorCampaignObjective", "RuntimeRelicDeliveryPhase", "遗物已取出，送回锚点完成归档");
		State.DeviceStatusLabel = BuildStandardInteractionDeviceStatusLabel();
		State.NextActionLabel = BuildStandardInteractionNextActionLabel();
		State.FailureRecoveryLabel = BuildStandardInteractionFailureRecoveryLabel();
		State.CompletedStepCount = 1;
		State.RequiredStepCount = FMath::Max(2, State.RequiredStepCount);
		State.ProgressFraction = 0.5f;
	}
	else if (bCompleted)
	{
		State.Status = EHorrorCampaignObjectiveRuntimeStatus::Completed;
		State.PhaseText = ObjectiveDefinition.CompletionText.IsEmpty()
			? NSLOCTEXT("HorrorCampaignObjective", "RuntimeCompletedPhase", "目标已完成")
			: ObjectiveDefinition.CompletionText;
		State.ProgressFraction = 1.0f;
	}
	else if (bObjectiveFailedRetryable)
	{
		State.Status = EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable;
		State.PhaseText = AdvancedInteractionFeedbackText.IsEmpty()
			? NSLOCTEXT("HorrorCampaignObjective", "RuntimeRetryableFailurePhase", "目标失败，可重新互动")
			: AdvancedInteractionFeedbackText;
		State.ProgressFraction = 0.0f;
		State.AdvancedInteraction = BuildAdvancedInteractionHUDState();
		State.DeviceStatusLabel = State.AdvancedInteraction.DeviceStatusLabel;
		State.NextActionLabel = State.AdvancedInteraction.NextActionLabel;
		State.FailureRecoveryLabel = State.AdvancedInteraction.FailureRecoveryLabel;
	}
	else
	{
		State.Status = bAvailableForInteraction
			? EHorrorCampaignObjectiveRuntimeStatus::Available
			: EHorrorCampaignObjectiveRuntimeStatus::Locked;
		State.PhaseText = RequiredInteractionCount > 1
			? BuildMultiStepPhaseText()
			: (ObjectiveDefinition.Presentation.MechanicLabel.IsEmpty()
			? NSLOCTEXT("HorrorCampaignObjective", "RuntimeReadyPhase", "等待互动")
			: ObjectiveDefinition.Presentation.MechanicLabel);
		State.ProgressFraction = State.RequiredStepCount > 0
			? FMath::Clamp(
				static_cast<float>(State.CompletedStepCount) / static_cast<float>(State.RequiredStepCount),
				0.0f,
				1.0f)
			: 0.0f;
		State.DeviceStatusLabel = BuildStandardInteractionDeviceStatusLabel();
		State.NextActionLabel = BuildStandardInteractionNextActionLabel();
		State.FailureRecoveryLabel = BuildStandardInteractionFailureRecoveryLabel();
	}

	if (!State.bCanInteract && !bCompleted && !bTimedObjectiveActive)
	{
		State.BlockedReason = GetInteractionPromptText();
	}

	return State;
}

bool AHorrorCampaignObjectiveActor::HasPersistentObjectiveRuntimeState() const
{
	return bAdvancedInteractionActive
		|| bTimedObjectiveActive
		|| bRecoverRelicAwaitingDelivery
		|| bObjectiveFailedRetryable
		|| InteractionProgressCount > 0;
}

FHorrorCampaignObjectiveSaveState AHorrorCampaignObjectiveActor::ExportObjectiveSaveState() const
{
	FHorrorCampaignObjectiveSaveState SaveState;
	SaveState.ChapterId = ChapterId;
	SaveState.ObjectiveId = ObjectiveDefinition.ObjectiveId;
	SaveState.InteractionProgressCount = InteractionProgressCount;
	SaveState.ObjectiveFailureAttemptCount = ObjectiveFailureAttemptCount;
	SaveState.bObjectiveFailedRetryable = bObjectiveFailedRetryable;
	SaveState.LastObjectiveFailureCause = LastObjectiveFailureCause;
	SaveState.LastObjectiveRecoveryAction = LastObjectiveRecoveryAction;
	SaveState.bAdvancedInteractionActive = bAdvancedInteractionActive;
	SaveState.AdvancedInteractionProgressFraction = AdvancedInteractionProgressFraction;
	SaveState.AdvancedInteractionStepIndex = AdvancedInteractionStepIndex;
	SaveState.ExpectedAdvancedInputId = ExpectedAdvancedInputId;
	SaveState.AdvancedInteractionCueElapsedSeconds = AdvancedInteractionCueElapsedSeconds;
	SaveState.AdvancedInteractionTimingFraction = AdvancedInteractionTimingFraction;
	SaveState.bAdvancedInteractionCueResolved = bAdvancedInteractionCueResolved;
	SaveState.AdvancedInteractionStabilityFraction = AdvancedInteractionStabilityFraction;
	SaveState.SignalTuningBalanceFraction = SignalTuningBalanceFraction;
	SaveState.SignalTuningTargetBalanceFraction = SignalTuningTargetBalanceFraction;
	SaveState.SpectralScanConfidenceFraction = SpectralScanConfidenceFraction;
	SaveState.SpectralScanNoiseFraction = SpectralScanNoiseFraction;
	SaveState.SpectralScanFilterFocusFraction = SpectralScanFilterFocusFraction;
	SaveState.SpectralScanTargetFocusFraction = SpectralScanTargetFocusFraction;
	SaveState.AdvancedInteractionComboCount = AdvancedInteractionComboCount;
	SaveState.AdvancedInteractionMistakeCount = AdvancedInteractionMistakeCount;
	SaveState.AdvancedInteractionPauseRemainingSeconds = AdvancedInteractionPauseRemainingSeconds;
	SaveState.bTimedObjectiveActive = bTimedObjectiveActive;
	SaveState.bRecoverRelicAwaitingDelivery = bRecoverRelicAwaitingDelivery;
	SaveState.TimedObjectiveRemainingSeconds = TimedObjectiveRemainingSeconds;
	SaveState.TimedObjectiveDurationSeconds = TimedObjectiveDurationSeconds;
	SaveState.bTimedObjectiveUsesEscapeDestination = bTimedObjectiveUsesEscapeDestination;
	SaveState.TimedObjectiveEscapeDestinationLocation = TimedObjectiveEscapeDestinationLocation;
	SaveState.RecoverRelicDeliveryLocation = RecoverRelicDeliveryLocation;
	SaveState.bHasTimedObjectiveOriginTransform = bHasTimedObjectiveOriginTransform;
	SaveState.TimedObjectiveOriginTransform = TimedObjectiveOriginTransform;
	return SaveState;
}

void AHorrorCampaignObjectiveActor::ImportObjectiveSaveState(const FHorrorCampaignObjectiveSaveState& SaveState, AActor* RestoredInstigator)
{
	if (SaveState.ChapterId != ChapterId || SaveState.ObjectiveId != ObjectiveDefinition.ObjectiveId)
	{
		return;
	}

	InteractionProgressCount = FMath::Clamp(SaveState.InteractionProgressCount, 0, FMath::Max(RequiredInteractionCount, 1));
	ObjectiveFailureAttemptCount = FMath::Max(0, SaveState.ObjectiveFailureAttemptCount);
	bObjectiveFailedRetryable = SaveState.bObjectiveFailedRetryable;
	LastObjectiveFailureCause = SaveState.LastObjectiveFailureCause;
	LastObjectiveRecoveryAction = SaveState.LastObjectiveRecoveryAction;
	bAdvancedInteractionActive = SaveState.bAdvancedInteractionActive && UsesAdvancedInteraction() && !bCompleted;
	AdvancedInteractionProgressFraction = FMath::Clamp(SaveState.AdvancedInteractionProgressFraction, 0.0f, 1.0f);
	AdvancedInteractionStepIndex = FMath::Clamp(SaveState.AdvancedInteractionStepIndex, 0, FMath::Max(RequiredInteractionCount, 1));
	ExpectedAdvancedInputId = SaveState.ExpectedAdvancedInputId;
	AdvancedInteractionCueElapsedSeconds = FMath::Max(0.0f, SaveState.AdvancedInteractionCueElapsedSeconds);
	AdvancedInteractionTimingFraction = FMath::Clamp(SaveState.AdvancedInteractionTimingFraction, 0.0f, 1.0f);
	bAdvancedInteractionCueResolved = SaveState.bAdvancedInteractionCueResolved;
	AdvancedInteractionStabilityFraction = FMath::Clamp(SaveState.AdvancedInteractionStabilityFraction, 0.0f, 1.0f);
	SignalTuningBalanceFraction = FMath::Clamp(SaveState.SignalTuningBalanceFraction, 0.0f, 1.0f);
	SignalTuningTargetBalanceFraction = FMath::Clamp(SaveState.SignalTuningTargetBalanceFraction, 0.0f, 1.0f);
	SpectralScanConfidenceFraction = FMath::Clamp(SaveState.SpectralScanConfidenceFraction, 0.0f, 1.0f);
	SpectralScanNoiseFraction = FMath::Clamp(SaveState.SpectralScanNoiseFraction, 0.0f, 1.0f);
	SpectralScanTargetFocusFraction = SaveState.SpectralScanTargetFocusFraction > 0.0f
		? FMath::Clamp(SaveState.SpectralScanTargetFocusFraction, 0.0f, 1.0f)
		: ResolveSpectralScanTargetFocus();
	SpectralScanFilterFocusFraction = SaveState.SpectralScanFilterFocusFraction > 0.0f
		? FMath::Clamp(SaveState.SpectralScanFilterFocusFraction, 0.0f, 1.0f)
		: SpectralScanFilterFocusFraction;
	AdvancedInteractionComboCount = FMath::Max(0, SaveState.AdvancedInteractionComboCount);
	AdvancedInteractionMistakeCount = FMath::Max(0, SaveState.AdvancedInteractionMistakeCount);
	AdvancedInteractionPauseRemainingSeconds = FMath::Max(0.0f, SaveState.AdvancedInteractionPauseRemainingSeconds);
	bAdvancedInteractionRecentSuccess = false;
	bAdvancedInteractionRecentFailure = false;
	if (bAdvancedInteractionActive && ExpectedAdvancedInputId.IsNone())
	{
		AdvanceExpectedAdvancedInput();
	}

	if (SaveState.TimedObjectiveDurationSeconds > 0.0f)
	{
		TimedObjectiveDurationSeconds = FMath::Max(0.1f, SaveState.TimedObjectiveDurationSeconds);
	}
	bTimedObjectiveActive = SaveState.bTimedObjectiveActive && IsTimedSurvivalObjective() && !bCompleted;
	TimedObjectiveRemainingSeconds = FMath::Clamp(
		SaveState.TimedObjectiveRemainingSeconds,
		0.0f,
		FMath::Max(TimedObjectiveDurationSeconds, 0.1f));
	bTimedObjectiveUsesEscapeDestination = bTimedObjectiveActive && SaveState.bTimedObjectiveUsesEscapeDestination;
	TimedObjectiveEscapeDestinationLocation = SaveState.TimedObjectiveEscapeDestinationLocation;
	bRecoverRelicAwaitingDelivery = SaveState.bRecoverRelicAwaitingDelivery && IsCarryReturnRelicObjective() && !bCompleted;
	RecoverRelicDeliveryLocation = bRecoverRelicAwaitingDelivery
		? SaveState.RecoverRelicDeliveryLocation
		: FVector::ZeroVector;
	bHasTimedObjectiveOriginTransform = SaveState.bHasTimedObjectiveOriginTransform;
	TimedObjectiveOriginTransform = SaveState.TimedObjectiveOriginTransform;
	TimedObjectiveInstigator = RestoredInstigator;
	if (bTimedObjectiveActive && bTimedObjectiveUsesEscapeDestination)
	{
		SetActorLocation(TimedObjectiveEscapeDestinationLocation, false);
	}
	else if (bRecoverRelicAwaitingDelivery)
	{
		if (RecoverRelicDeliveryLocation.IsNearlyZero())
		{
			RecoverRelicDeliveryLocation = ResolveRecoverRelicDeliveryWorldLocation(RestoredInstigator);
		}
		SetActorLocation(RecoverRelicDeliveryLocation, false);
	}
	else if (!bTimedObjectiveActive && bHasTimedObjectiveOriginTransform)
	{
		SetActorTransform(TimedObjectiveOriginTransform, false, nullptr, ETeleportType::TeleportPhysics);
	}

	if (bAdvancedInteractionActive)
	{
		switch (ObjectiveDefinition.InteractionMode)
		{
			case EHorrorCampaignInteractionMode::CircuitWiring:
				AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "CircuitRestoredFromSave", "接线盘已恢复，继续完成当前同步。");
				break;
			case EHorrorCampaignInteractionMode::GearCalibration:
				AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "GearRestoredFromSave", "齿轮盘已恢复，继续完成当前校准。");
				break;
			case EHorrorCampaignInteractionMode::SpectralScan:
				AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SpectralScanRestoredFromSave", "频谱扫描已恢复，继续锁定异常波段。");
				break;
			case EHorrorCampaignInteractionMode::SignalTuning:
				AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SignalTuningRestoredFromSave", "信号调谐已恢复，继续对齐回放频率。");
				break;
			default:
				AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "AdvancedRestoredFromSave", "交互窗口已恢复，继续当前同步。");
				break;
		}
	}
	else if (bObjectiveFailedRetryable)
	{
		AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "RetryableFailureRestoredFromSave", "目标失败，可重新互动。");
	}

	RefreshObjectiveState();
}

void AHorrorCampaignObjectiveActor::ApplyAdvancedInteractionTimingFailure()
{
	bAdvancedInteractionCueResolved = true;
	SetAdvancedInteractionFault(AdvancedFaultTiming);
	const float FailureStabilityDamage = GetAdvancedInteractionFailureStabilityDamage();
	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring
		|| ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan
		|| ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
	{
		MarkAdvancedInteractionFailure(FailureStabilityDamage);
		AdvancedInteractionProgressFraction = FMath::Clamp(
			AdvancedInteractionProgressFraction - GetAdvancedInteractionFailureProgressPenalty(),
			0.0f,
			0.99f);
		InteractionProgressCount = FMath::FloorToInt(AdvancedInteractionProgressFraction * RequiredInteractionCount);
		if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
		{
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SpectralScanTimingFailure", "时机偏离：黑盒噪点反冲，扫描进度回退。");
		}
		else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
		{
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SignalTuningTimingFailure", "调谐偏离：回放撕裂，进度回退。");
		}
		else
		{
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "CircuitTimingFailure", "时机过早：红色火花炸开，进度回退。");
		}
		ResetAdvancedInteractionCue();
	}
	else
	{
		MarkAdvancedInteractionFailure(FailureStabilityDamage);
		AdvancedInteractionPauseRemainingSeconds = GetAdvancedInteractionFailurePauseSeconds();
		AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "GearTimingFailure", "时机不对：齿轮连锁中断，机构咬死并暂停三秒。");
	}
}

void AHorrorCampaignObjectiveActor::ApplyAdvancedInteractionWrongInputFailure()
{
	bAdvancedInteractionCueResolved = true;
	SetAdvancedInteractionFault(AdvancedFaultWrongInput);
	const float FailureStabilityDamage = GetAdvancedInteractionFailureStabilityDamage();
	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring
		|| ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan
		|| ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
	{
		MarkAdvancedInteractionFailure(FailureStabilityDamage);
		AdvancedInteractionProgressFraction = FMath::Clamp(
			AdvancedInteractionProgressFraction - GetAdvancedInteractionFailureProgressPenalty(),
			0.0f,
			0.99f);
		InteractionProgressCount = FMath::FloorToInt(AdvancedInteractionProgressFraction * RequiredInteractionCount);
		if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
		{
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SpectralScanFailure", "噪点反冲：锁错波段，扫描进度回退。");
		}
		else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
		{
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SignalTuningFailure", "频率错位：回放失真，调谐进度回退。");
		}
		else
		{
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "CircuitFailure", "红色火花：接线错误，进度回退。");
		}
		ResetAdvancedInteractionCue();
	}
	else
	{
		MarkAdvancedInteractionFailure(FailureStabilityDamage);
		AdvancedInteractionPauseRemainingSeconds = GetAdvancedInteractionFailurePauseSeconds();
		AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "GearFailure", "咔哒：齿轮连锁中断，校准暂停三秒。");
	}
}

FText AHorrorCampaignObjectiveActor::BuildAdvancedInteractionPausedText(bool bExplicitSelection) const
{
	switch (ObjectiveDefinition.InteractionMode)
	{
		case EHorrorCampaignInteractionMode::CircuitWiring:
			return bExplicitSelection
				? NSLOCTEXT("HorrorCampaignObjective", "CircuitSelectionPaused", "线路保护中，等待火花散去后再按 A/S/D 选择端子。")
				: NSLOCTEXT("HorrorCampaignObjective", "CircuitPaused", "线路保护中，等待火花散去。");
		case EHorrorCampaignInteractionMode::GearCalibration:
			return bExplicitSelection
				? NSLOCTEXT("HorrorCampaignObjective", "GearSelectionPaused", "机构卡死，等待齿轮重新咬合后再按 A/S/D 选择齿轮。")
				: NSLOCTEXT("HorrorCampaignObjective", "GearPaused", "机构卡死，等待齿轮重新咬合。");
		case EHorrorCampaignInteractionMode::SpectralScan:
			return bExplicitSelection
				? NSLOCTEXT("HorrorCampaignObjective", "SpectralScanSelectionPaused", "扫描器正在降噪，等待频谱稳定后先用 A/D 扫频，再按 S 锁定。")
				: NSLOCTEXT("HorrorCampaignObjective", "SpectralScanPaused", "扫描器正在降噪，等待频谱稳定。");
		case EHorrorCampaignInteractionMode::SignalTuning:
			return bExplicitSelection
				? NSLOCTEXT("HorrorCampaignObjective", "SignalTuningSelectionPaused", "黑盒回放正在静音保护，等待频率稳定后再按 A/S/D 调谐。")
				: NSLOCTEXT("HorrorCampaignObjective", "SignalTuningPaused", "黑盒回放正在静音保护，等待频率稳定。");
		default:
			return bExplicitSelection
				? NSLOCTEXT("HorrorCampaignObjective", "AdvancedSelectionPaused", "交互窗口正在恢复，稍后再按 A/S/D 选择目标。")
				: NSLOCTEXT("HorrorCampaignObjective", "AdvancedPaused", "交互窗口正在恢复。");
	}
}

FName AHorrorCampaignObjectiveActor::GetAdvancedInteractionFailureCause() const
{
	switch (ObjectiveDefinition.InteractionMode)
	{
		case EHorrorCampaignInteractionMode::GearCalibration:
			return GearOverloadFailureCause;
		case EHorrorCampaignInteractionMode::SpectralScan:
			return SpectralScanOverloadFailureCause;
		case EHorrorCampaignInteractionMode::SignalTuning:
			return SignalTuningOverloadFailureCause;
		case EHorrorCampaignInteractionMode::CircuitWiring:
		default:
			return CircuitOverloadFailureCause;
	}
}

bool AHorrorCampaignObjectiveActor::SubmitAdvancedInteractionInput(FName InputId, AActor* InstigatorActor)
{
	FHorrorAdvancedInteractionInputCommand Command;
	Command.CommandType = EHorrorAdvancedInteractionCommandType::SelectOption;
	Command.InputId = InputId;
	return SubmitAdvancedInteractionCommand(Command, InstigatorActor);
}

bool AHorrorCampaignObjectiveActor::SubmitAdvancedInteractionCommand(const FHorrorAdvancedInteractionInputCommand& Command, AActor* InstigatorActor)
{
	ResetLastAdvancedInteractionOutcome();
	if (!bAdvancedInteractionActive || bCompleted || !UsesAdvancedInteraction())
	{
		return false;
	}

	if (AdvancedInteractionPauseRemainingSeconds > 0.0f)
	{
		AdvancedInteractionFeedbackText = BuildAdvancedInteractionPausedText(false);
		RecordAdvancedInteractionOutcome(
			EHorrorAdvancedInteractionOutcomeKind::Paused,
			Command.InputId,
			EHorrorAdvancedInteractionFeedbackState::Paused,
			false,
			false);
		RefreshObjectiveState();
		return false;
	}

	if (Command.CommandType == EHorrorAdvancedInteractionCommandType::Cancel)
	{
		AdvancedInteractionFeedbackText = BuildAdvancedInteractionPausedText(true);
		RecordAdvancedInteractionOutcome(
			EHorrorAdvancedInteractionOutcomeKind::Cancelled,
			Command.InputId,
			EHorrorAdvancedInteractionFeedbackState::Neutral,
			true,
			false);
		RefreshObjectiveState();
		return true;
	}

	if (IsSignalTuningMode() && Command.CommandType == EHorrorAdvancedInteractionCommandType::AdjustAxis)
	{
		AdjustSignalTuningBalanceByAxis(Command.AxisValue, Command.HoldSeconds);
		RecordAdvancedInteractionOutcome(
			EHorrorAdvancedInteractionOutcomeKind::Adjusted,
			Command.AxisValue < 0.0f ? SignalTuningLeftInput : (Command.AxisValue > 0.0f ? SignalTuningRightInput : NAME_None),
			EHorrorAdvancedInteractionFeedbackState::Neutral,
			true,
			false);
		RefreshObjectiveState();
		return true;
	}

	if (IsSpectralScanMode() && Command.CommandType == EHorrorAdvancedInteractionCommandType::AdjustAxis)
	{
		AdjustSpectralScanFilterByAxis(Command.AxisValue, Command.HoldSeconds);
		RecordAdvancedInteractionOutcome(
			EHorrorAdvancedInteractionOutcomeKind::Adjusted,
			Command.AxisValue < 0.0f ? SpectralScanSweepLeftInput : (Command.AxisValue > 0.0f ? SpectralScanSweepRightInput : NAME_None),
			EHorrorAdvancedInteractionFeedbackState::Neutral,
			true,
			false);
		RefreshObjectiveState();
		return true;
	}

	if (Command.CommandType == EHorrorAdvancedInteractionCommandType::AdjustAxis)
	{
		RecordAdvancedInteractionOutcome(
			EHorrorAdvancedInteractionOutcomeKind::Ignored,
			Command.InputId,
			EHorrorAdvancedInteractionFeedbackState::Neutral,
			false,
			false);
		return false;
	}

	FName InputId = Command.InputId;
	if (Command.CommandType == EHorrorAdvancedInteractionCommandType::Confirm && InputId.IsNone())
	{
		InputId = ExpectedAdvancedInputId;
	}

	if (IsSpectralScanMode()
		&& (InputId == SpectralScanSweepLeftInput || InputId == SpectralScanSweepRightInput)
		&& InputId != ExpectedAdvancedInputId)
	{
		AdjustSpectralScanFilter(InputId);
		RecordAdvancedInteractionOutcome(
			EHorrorAdvancedInteractionOutcomeKind::Adjusted,
			InputId,
			EHorrorAdvancedInteractionFeedbackState::Neutral,
			true,
			false);
		RefreshObjectiveState();
		return true;
	}

	if (IsSignalTuningAdjustInput(InputId))
	{
		AdjustSignalTuningBalance(InputId);
		RecordAdvancedInteractionOutcome(
			EHorrorAdvancedInteractionOutcomeKind::Adjusted,
			InputId,
			EHorrorAdvancedInteractionFeedbackState::Neutral,
			true,
			false);
		RefreshObjectiveState();
		return true;
	}

	if (!IsAdvancedInteractionTimingWindowOpen())
	{
		ApplyAdvancedInteractionTimingFailure();
		PublishAdvancedInteractionFeedbackEvent(false, InstigatorActor);
		const FName FailureFaultId = LastAdvancedInteractionFaultId;
		return RecordAdvancedInteractionFailureOutcomeAfterFeedback(
			EHorrorAdvancedInteractionOutcomeKind::TimingFailure,
			InputId,
			FailureFaultId,
			InstigatorActor);
	}

	if (IsSpectralScanMode() && InputId != ExpectedAdvancedInputId)
	{
		ApplyAdvancedInteractionWrongInputFailure();
		SetAdvancedInteractionFault(AdvancedFaultSpectralWrongControl);
		AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SpectralScanNonConfirmFailure", "扫频指令错误：请用中心波段确认锁定。");
		PublishAdvancedInteractionFeedbackEvent(false, InstigatorActor);
		const FName FailureFaultId = LastAdvancedInteractionFaultId;
		return RecordAdvancedInteractionFailureOutcomeAfterFeedback(
			EHorrorAdvancedInteractionOutcomeKind::WrongInput,
			InputId,
			FailureFaultId,
			InstigatorActor);
	}

	if (IsSignalTuningMode() && !IsSignalTuningConfirmInput(InputId))
	{
		ApplyAdvancedInteractionWrongInputFailure();
		SetAdvancedInteractionFault(AdvancedFaultSignalWrongControl);
		AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SignalTuningNonConfirmFailure", "调谐指令错误：请用中心频率确认锁定。");
		PublishAdvancedInteractionFeedbackEvent(false, InstigatorActor);
		const FName FailureFaultId = LastAdvancedInteractionFaultId;
		return RecordAdvancedInteractionFailureOutcomeAfterFeedback(
			EHorrorAdvancedInteractionOutcomeKind::WrongInput,
			InputId,
			FailureFaultId,
			InstigatorActor);
	}

	if (IsSignalTuningMode() && !IsSignalTuningBalanceAligned())
	{
		ApplyAdvancedInteractionWrongInputFailure();
		SetAdvancedInteractionFault(AdvancedFaultSignalBalance);
		AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SignalTuningBalanceFailure", "声像没有对齐：黑盒回放撕裂，调谐进度回退。");
		PublishAdvancedInteractionFeedbackEvent(false, InstigatorActor);
		const FName FailureFaultId = LastAdvancedInteractionFaultId;
		return RecordAdvancedInteractionFailureOutcomeAfterFeedback(
			EHorrorAdvancedInteractionOutcomeKind::SignalBalanceFailure,
			InputId,
			FailureFaultId,
			InstigatorActor);
	}

	if (IsCircuitHazardInput(InputId) && InputId != ExpectedAdvancedInputId)
	{
		ApplyAdvancedInteractionWrongInputFailure();
		SetAdvancedInteractionFault(AdvancedFaultCircuitHazard);
		AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "CircuitHazardFailure", "短路负载：漏电端子正在放电，切换到安全高亮端子。");
		PublishAdvancedInteractionFeedbackEvent(false, InstigatorActor);
		const FName FailureFaultId = LastAdvancedInteractionFaultId;
		return RecordAdvancedInteractionFailureOutcomeAfterFeedback(
			EHorrorAdvancedInteractionOutcomeKind::Hazard,
			InputId,
			FailureFaultId,
			InstigatorActor);
	}

	if (IsSpectralScanMode() && !IsSpectralScanConfidenceReady())
	{
		ApplyAdvancedInteractionWrongInputFailure();
		SetAdvancedInteractionFault(AdvancedFaultSpectralLowConfidence);
		AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SpectralScanLowConfidenceFailure", "置信度不足：假峰值反冲，扫描稳定度下降。");
		PublishAdvancedInteractionFeedbackEvent(false, InstigatorActor);
		const FName FailureFaultId = LastAdvancedInteractionFaultId;
		return RecordAdvancedInteractionFailureOutcomeAfterFeedback(
			EHorrorAdvancedInteractionOutcomeKind::SpectralConfidenceFailure,
			InputId,
			FailureFaultId,
			InstigatorActor);
	}

	if (IsSpectralScanMode() && !IsSpectralScanFilterAligned())
	{
		ApplyAdvancedInteractionWrongInputFailure();
		SetAdvancedInteractionFault(AdvancedFaultSpectralFilter);
		AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SpectralScanFilterFailure", "滤波焦点没有压住异常峰：先主动扫频，再锁定。");
		PublishAdvancedInteractionFeedbackEvent(false, InstigatorActor);
		const FName FailureFaultId = LastAdvancedInteractionFaultId;
		return RecordAdvancedInteractionFailureOutcomeAfterFeedback(
			EHorrorAdvancedInteractionOutcomeKind::SpectralFilterFailure,
			InputId,
			FailureFaultId,
			InstigatorActor);
	}

	const bool bCorrectInput = InputId == ExpectedAdvancedInputId;
	if (!bCorrectInput)
	{
		ApplyAdvancedInteractionWrongInputFailure();
		PublishAdvancedInteractionFeedbackEvent(false, InstigatorActor);
		const FName FailureFaultId = LastAdvancedInteractionFaultId;
		return RecordAdvancedInteractionFailureOutcomeAfterFeedback(
			EHorrorAdvancedInteractionOutcomeKind::WrongInput,
			InputId,
			FailureFaultId,
			InstigatorActor);
	}

	AdvancedInteractionProgressFraction = FMath::Clamp(
		AdvancedInteractionProgressFraction + GetAdvancedInteractionSuccessProgress(),
		0.0f,
		1.0f);
	MarkAdvancedInteractionSuccess();
	InteractionProgressCount = FMath::Clamp(
		FMath::CeilToInt(AdvancedInteractionProgressFraction * RequiredInteractionCount),
		0,
		RequiredInteractionCount);

	if (AdvancedInteractionProgressFraction >= 1.0f)
	{
		const bool bCompletedObjective = CompleteAdvancedInteraction(InstigatorActor);
		RecordAdvancedInteractionOutcome(
			bCompletedObjective ? EHorrorAdvancedInteractionOutcomeKind::Completed : EHorrorAdvancedInteractionOutcomeKind::Success,
			InputId,
			EHorrorAdvancedInteractionFeedbackState::Success,
			true,
			true);
		return bCompletedObjective;
	}

	switch (ObjectiveDefinition.InteractionMode)
	{
		case EHorrorCampaignInteractionMode::CircuitWiring:
			AdvancedInteractionFeedbackText = FText::Format(
				NSLOCTEXT("HorrorCampaignObjective", "CircuitSuccessTerminal", "蓝色电弧：{0} 接入成功，安全线路延伸。"),
				FText::FromName(InputId));
			break;
		case EHorrorCampaignInteractionMode::GearCalibration:
			AdvancedInteractionFeedbackText = FText::Format(
				NSLOCTEXT("HorrorCampaignObjective", "GearSuccess", "齿轮连锁第 {0} 拍咬合，机械节律恢复。"),
				FText::AsNumber(FMath::Clamp(AdvancedInteractionStepIndex + 1, 1, FMath::Max(1, RequiredInteractionCount))));
			break;
		case EHorrorCampaignInteractionMode::SpectralScan:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SpectralScanSuccess", "波段锁定：异常频谱正在写入黑盒。");
			break;
		case EHorrorCampaignInteractionMode::SignalTuning:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SignalTuningSuccess", "频率对齐：黑盒回放正在变清晰。");
			break;
		default:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "AdvancedSuccess", "同步成功，进度推进。");
			break;
	}
	PublishAdvancedInteractionFeedbackEvent(true, InstigatorActor);
	RecordAdvancedInteractionOutcome(
		EHorrorAdvancedInteractionOutcomeKind::Success,
		InputId,
		EHorrorAdvancedInteractionFeedbackState::Success,
		true,
		true);
	++AdvancedInteractionStepIndex;
	AdvanceExpectedAdvancedInput();
	ResetAdvancedInteractionCue();
	RefreshObjectiveState();
	return true;
}

bool AHorrorCampaignObjectiveActor::PromptForExplicitAdvancedInteractionSelection()
{
	ResetLastAdvancedInteractionOutcome();
	if (!bAdvancedInteractionActive || bCompleted || !UsesAdvancedInteraction())
	{
		return false;
	}

	if (AdvancedInteractionPauseRemainingSeconds > 0.0f)
	{
		AdvancedInteractionFeedbackText = BuildAdvancedInteractionPausedText(true);
		RecordAdvancedInteractionOutcome(
			EHorrorAdvancedInteractionOutcomeKind::Paused,
			ExpectedAdvancedInputId,
			EHorrorAdvancedInteractionFeedbackState::Paused,
			true,
			false);
		RefreshObjectiveState();
		return true;
	}

	ResetAdvancedInteractionCue();
	switch (ObjectiveDefinition.InteractionMode)
	{
		case EHorrorCampaignInteractionMode::CircuitWiring:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "CircuitNeedsExplicitSelection", "按 A/S/D 接入对应颜色端子。");
			break;
		case EHorrorCampaignInteractionMode::GearCalibration:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "GearNeedsExplicitSelection", "按 A/S/D 拨动对应停转齿轮。");
			break;
		case EHorrorCampaignInteractionMode::SpectralScan:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SpectralScanNeedsExplicitSelection", "按 A/D 主动扫频压住异常峰，焦点稳定后按 S 锁定。");
			break;
		case EHorrorCampaignInteractionMode::SignalTuning:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SignalTuningNeedsExplicitSelection", "按 A/D 微调左右声道，声像居中后按 S 锁定中心频率。");
			break;
		default:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "AdvancedNeedsExplicitSelection", "按 A/S/D 完成当前同步。");
			break;
	}
	RecordAdvancedInteractionOutcome(
		EHorrorAdvancedInteractionOutcomeKind::Prompted,
		ExpectedAdvancedInputId,
		EHorrorAdvancedInteractionFeedbackState::Neutral,
		true,
		false);
	RefreshObjectiveState();
	return true;
}

bool AHorrorCampaignObjectiveActor::TryHandleAdvancedInteractionFailureExhausted(AActor* InstigatorActor)
{
	if (AdvancedInteractionStabilityFraction > 0.0f)
	{
		return false;
	}

	FText FailureTitle;
	FText RetryHint;
	switch (ObjectiveDefinition.InteractionMode)
	{
		case EHorrorCampaignInteractionMode::GearCalibration:
			FailureTitle = NSLOCTEXT("HorrorCampaignObjective", "GearObjectiveOverloadedTitle", "校准失败：齿轮机构过载");
			RetryHint = NSLOCTEXT("HorrorCampaignObjective", "GearObjectiveRetryHint", "机构已自动断开，重新互动可重试齿轮校准。");
			break;
		case EHorrorCampaignInteractionMode::SpectralScan:
			FailureTitle = NSLOCTEXT("HorrorCampaignObjective", "SpectralScanObjectiveOverloadedTitle", "扫描失败：频谱噪点过载");
			RetryHint = NSLOCTEXT("HorrorCampaignObjective", "SpectralScanObjectiveRetryHint", "扫描器已自动降噪，重新互动可重试频谱扫描。");
			break;
		case EHorrorCampaignInteractionMode::SignalTuning:
			FailureTitle = NSLOCTEXT("HorrorCampaignObjective", "SignalTuningObjectiveOverloadedTitle", "调谐失败：黑盒回放过载");
			RetryHint = NSLOCTEXT("HorrorCampaignObjective", "SignalTuningObjectiveRetryHint", "黑盒已自动静音，重新互动可重试信号调谐。");
			break;
		case EHorrorCampaignInteractionMode::CircuitWiring:
		default:
			FailureTitle = NSLOCTEXT("HorrorCampaignObjective", "CircuitObjectiveOverloadedTitle", "接线失败：电路过载");
			RetryHint = NSLOCTEXT("HorrorCampaignObjective", "CircuitObjectiveRetryHint", "线路保护器已断开，重新互动可重试接线。");
			break;
	}

	++ObjectiveFailureAttemptCount;
	const FName FailureCause = GetAdvancedInteractionFailureCause();
	bObjectiveFailedRetryable = true;
	LastObjectiveFailureCause = FailureCause;
	LastObjectiveRecoveryAction = RetryObjectiveRecoveryAction;
	PublishObjectiveFailedEvent(InstigatorActor, FailureTitle, RetryHint, FailureCause, RetryObjectiveRecoveryAction);

	ResetAdvancedInteractionState();
	InteractionProgressCount = 0;
	AdvancedInteractionFeedbackText = FailureTitle;
	RefreshObjectiveState();
	return true;
}

bool AHorrorCampaignObjectiveActor::CompleteAdvancedInteraction(AActor* InstigatorActor)
{
	bAdvancedInteractionActive = false;
	AdvancedInteractionProgressFraction = 1.0f;
	InteractionProgressCount = RequiredInteractionCount;
	switch (ObjectiveDefinition.InteractionMode)
	{
		case EHorrorCampaignInteractionMode::CircuitWiring:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "CircuitComplete", "电路闭合，系统恢复。");
			break;
		case EHorrorCampaignInteractionMode::GearCalibration:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "GearComplete", "齿轮校准完成，封锁解除。");
			break;
		case EHorrorCampaignInteractionMode::SpectralScan:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SpectralScanComplete", "频谱锁定完成，异常证据已归档。");
			break;
		case EHorrorCampaignInteractionMode::SignalTuning:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SignalTuningComplete", "信号调谐完成，黑盒回放已稳定。");
			break;
		default:
			AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "AdvancedComplete", "同步完成。");
			break;
	}
	MarkAdvancedInteractionSuccess();
	PublishAdvancedInteractionFeedbackEvent(true, InstigatorActor);
	ExpectedAdvancedInputId = NAME_None;
	AdvancedInteractionPauseRemainingSeconds = 0.0f;
	return CompleteObjective(InstigatorActor);
}

EHorrorAdvancedInteractionOutcomeKind AHorrorCampaignObjectiveActor::ResolveAdvancedInteractionEventOutcomeKind(bool bSuccess) const
{
	if (bSuccess)
	{
		return AdvancedInteractionProgressFraction >= 1.0f
			? EHorrorAdvancedInteractionOutcomeKind::Completed
			: EHorrorAdvancedInteractionOutcomeKind::Success;
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultCircuitHazard)
	{
		return EHorrorAdvancedInteractionOutcomeKind::Hazard;
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSignalBalance)
	{
		return EHorrorAdvancedInteractionOutcomeKind::SignalBalanceFailure;
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSpectralLowConfidence)
	{
		return EHorrorAdvancedInteractionOutcomeKind::SpectralConfidenceFailure;
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultSpectralFilter)
	{
		return EHorrorAdvancedInteractionOutcomeKind::SpectralFilterFailure;
	}

	if (LastAdvancedInteractionFaultId == AdvancedFaultTiming)
	{
		return EHorrorAdvancedInteractionOutcomeKind::TimingFailure;
	}

	return EHorrorAdvancedInteractionOutcomeKind::WrongInput;
}

void AHorrorCampaignObjectiveActor::PublishAdvancedInteractionFeedbackEvent(bool bSuccess, AActor* InstigatorActor) const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
	if (!EventBus)
	{
		return;
	}

	FGameplayTag EventTag;
	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::CircuitWiring)
	{
		EventTag = bSuccess ? AdvancedCircuitSuccessEventTag() : AdvancedCircuitFailureEventTag();
	}
	else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration)
	{
		EventTag = bSuccess ? AdvancedGearSuccessEventTag() : AdvancedGearFailureEventTag();
	}
	else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
	{
		EventTag = bSuccess ? AdvancedSpectralScanSuccessEventTag() : AdvancedSpectralScanFailureEventTag();
	}
	else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
	{
		EventTag = bSuccess ? AdvancedSignalTuningSuccessEventTag() : AdvancedSignalTuningFailureEventTag();
	}

	if (!EventTag.IsValid())
	{
		return;
	}

	const FName SourceId = ObjectiveDefinition.ObjectiveId.IsNone()
		? (ChapterId.IsNone() ? GetFName() : ChapterId)
		: ObjectiveDefinition.ObjectiveId;
	FHorrorObjectiveMessageMetadata Metadata;
	Metadata.FeedbackSeverity = bSuccess ? EHorrorObjectiveFeedbackSeverity::Success : EHorrorObjectiveFeedbackSeverity::Failure;
	Metadata.AdvancedOutcomeKind = ResolveAdvancedInteractionEventOutcomeKind(bSuccess);
	Metadata.AdvancedFaultId = bSuccess ? NAME_None : LastAdvancedInteractionFaultId;
	Metadata.DisplaySeconds = bSuccess ? 3.0f : 4.5f;
	EventBus->RegisterObjectiveMetadata(EventTag, SourceId, Metadata);
	EventBus->Publish(EventTag, SourceId, EventTag, InstigatorActor ? InstigatorActor : const_cast<AHorrorCampaignObjectiveActor*>(this));
}

void AHorrorCampaignObjectiveActor::PublishObjectiveFailedEvent(
	AActor* InstigatorActor,
	const FText& FailureTitle,
	const FText& RetryHint,
	FName FailureCause,
	FName RecoveryAction) const
{
	const UWorld* World = GetWorld();
	UHorrorEventBusSubsystem* EventBus = World ? World->GetSubsystem<UHorrorEventBusSubsystem>() : nullptr;
	const FGameplayTag EventTag = ObjectiveFailedEventTag();
	if (!EventBus || !EventTag.IsValid())
	{
		return;
	}

	const FName SourceId = ObjectiveDefinition.ObjectiveId.IsNone()
		? (ChapterId.IsNone() ? GetFName() : ChapterId)
		: ObjectiveDefinition.ObjectiveId;

	FHorrorObjectiveMessageMetadata Metadata;
	Metadata.TrailerBeatId = SourceId;
	Metadata.DebugLabel = FailureTitle.IsEmpty()
		? NSLOCTEXT("HorrorCampaignObjective", "ObjectiveFailedFallbackTitle", "目标失败")
		: FailureTitle;
	Metadata.ObjectiveHint = RetryHint.IsEmpty()
		? NSLOCTEXT("HorrorCampaignObjective", "ObjectiveFailedFallbackHint", "重新互动可重试当前目标。")
		: RetryHint;
	Metadata.FeedbackSeverity = EHorrorObjectiveFeedbackSeverity::Failure;
	Metadata.bRetryable = true;
	Metadata.DisplaySeconds = 6.5f;
	Metadata.FailureCause = FailureCause;
	Metadata.RecoveryAction = RecoveryAction;
	Metadata.AttemptIndex = FMath::Max(1, ObjectiveFailureAttemptCount);

	EventBus->RegisterObjectiveMetadata(EventTag, SourceId, Metadata);
	EventBus->Publish(EventTag, SourceId, EventTag, InstigatorActor ? InstigatorActor : const_cast<AHorrorCampaignObjectiveActor*>(this));
	EventBus->UnregisterObjectiveMetadata(EventTag, SourceId);
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
				? NSLOCTEXT("HorrorCampaignObjective", "CircuitHint", "光标进入蓝色窗口时按 A/S/D 选择端子。")
				: AdvancedInteractionFeedbackText);
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
	{
		return FText::Format(
			NSLOCTEXT("HorrorCampaignObjective", "SpectralScanPrompt", "{0}\n频谱扫描：焦点 {1}%  置信 {2}%  进度 {3}%\n{4}"),
			BasePrompt,
			FText::AsNumber(FMath::RoundToInt(SpectralScanFilterFocusFraction * 100.0f)),
			FText::AsNumber(FMath::RoundToInt(SpectralScanConfidenceFraction * 100.0f)),
			FText::AsNumber(ProgressPercent),
			AdvancedInteractionFeedbackText.IsEmpty()
				? NSLOCTEXT("HorrorCampaignObjective", "SpectralScanHint", "先用 A/D 主动扫频滤噪，焦点对准后在锁定带内按 S。")
				: AdvancedInteractionFeedbackText);
	}

	if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
	{
		return FText::Format(
			NSLOCTEXT("HorrorCampaignObjective", "SignalTuningPrompt", "{0}\n信号调谐：声像 {1}%  同步 {2}%  进度 {3}%\n{4}"),
			BasePrompt,
			FText::AsNumber(FMath::RoundToInt(SignalTuningBalanceFraction * 100.0f)),
			FText::AsNumber(TimingPercent),
			FText::AsNumber(ProgressPercent),
			AdvancedInteractionFeedbackText.IsEmpty()
				? NSLOCTEXT("HorrorCampaignObjective", "SignalTuningHint", "先用 A/D 微调左右声道，声像居中后在同步带内按 S 锁定。")
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
			? NSLOCTEXT("HorrorCampaignObjective", "GearHint", "停转齿轮进入校准窗口时按 A/S/D 拨动齿轮。")
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
	bObjectiveFailedRetryable = false;
	LastObjectiveFailureCause = NAME_None;
	LastObjectiveRecoveryAction = NAME_None;
	TimedObjectiveRemainingSeconds = TimedObjectiveDurationSeconds;
	TimedObjectiveOriginTransform = GetActorTransform();
	bHasTimedObjectiveOriginTransform = true;
	bTimedObjectiveUsesEscapeDestination = HasConfiguredEscapeDestination();
	TimedObjectiveEscapeDestinationLocation = bTimedObjectiveUsesEscapeDestination
		? ResolveEscapeDestinationWorldLocation(InstigatorActor)
		: GetActorLocation();
	bTimedObjectiveActive = true;
	InteractionProgressCount = 0;
	PublishAmbushStartedEvent(InstigatorActor);
	if (UWorld* World = GetWorld())
	{
		if (AHorrorGameModeBase* HorrorGameMode = World->GetAuthGameMode<AHorrorGameModeBase>())
		{
			const FTransform AmbushAnchorTransform = ResolveAmbushThreatAnchorTransform(InstigatorActor);
			HorrorGameMode->StartCampaignAmbushThreatFromTransform(
				ObjectiveDefinition.ObjectiveId,
				AmbushAnchorTransform);
		}
	}
	if (bTimedObjectiveUsesEscapeDestination)
	{
		SetActorLocation(TimedObjectiveEscapeDestinationLocation, false);
	}
	RefreshObjectiveState();
}

void AHorrorCampaignObjectiveActor::FailTimedObjective(AActor* InstigatorActor)
{
	AbortTimedObjectiveForRecovery(
		InstigatorActor,
		CampaignPursuitTimeoutFailureCause,
		NSLOCTEXT("HorrorCampaignObjective", "PursuitTimeoutFailureTitle", "追逐失败：未能抵达逃离点"),
		NSLOCTEXT("HorrorCampaignObjective", "PursuitTimeoutRetryHint", "重新互动可再次惊醒石像巨人，沿导航跑向亮起的逃离点。"));
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
			if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::GearCalibration)
			{
				AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "GearPauseCleared", "齿轮重新松动，继续校准。");
			}
			else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SpectralScan)
			{
				AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SpectralScanPauseCleared", "噪点压低，继续扫描。");
			}
			else if (ObjectiveDefinition.InteractionMode == EHorrorCampaignInteractionMode::SignalTuning)
			{
				AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "SignalTuningPauseCleared", "回放噪声稳定，继续调谐。");
			}
			else
			{
				AdvancedInteractionFeedbackText = NSLOCTEXT("HorrorCampaignObjective", "AdvancedPauseCleared", "系统复位，继续操作。");
			}
		}
		RefreshObjectiveState();
		return;
	}

	if (!bAdvancedInteractionCueResolved)
	{
		AdvancedInteractionCueElapsedSeconds += DeltaTime;
		AdvancedInteractionTimingFraction = FMath::Clamp(
			AdvancedInteractionCueElapsedSeconds / GetAdvancedInteractionCueCycleSeconds(),
			0.0f,
			1.0f);
		if (IsSpectralScanMode())
		{
			SpectralScanConfidenceFraction = FMath::Clamp(
				SpectralScanConfidenceFraction + DeltaTime * SpectralScanConfidenceGainPerSecond,
				0.0f,
				1.0f);
			SpectralScanNoiseFraction = FMath::Clamp(
				1.0f - SpectralScanConfidenceFraction + FMath::Abs(AdvancedInteractionTimingFraction - 0.5f) * 0.24f,
				0.0f,
				1.0f);
		}

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

	if (bTimedObjectiveUsesEscapeDestination && HasTimedObjectiveReachedEscapeDestination())
	{
		bTimedObjectiveActive = false;
		const bool bCompletedObjective = CompleteObjective(TimedObjectiveInstigator.Get());
		if (UWorld* World = GetWorld())
		{
			if (AHorrorGameModeBase* HorrorGameMode = World->GetAuthGameMode<AHorrorGameModeBase>())
			{
				HorrorGameMode->StopCampaignAmbushThreat(ObjectiveDefinition.ObjectiveId);
			}
		}
		if (bCompletedObjective)
		{
			return;
		}

		TimedObjectiveRemainingSeconds = TimedObjectiveDurationSeconds;
		bTimedObjectiveUsesEscapeDestination = false;
		TimedObjectiveEscapeDestinationLocation = FVector::ZeroVector;
		if (bHasTimedObjectiveOriginTransform)
		{
			SetActorTransform(TimedObjectiveOriginTransform, false, nullptr, ETeleportType::TeleportPhysics);
		}
		bHasTimedObjectiveOriginTransform = false;
		TimedObjectiveOriginTransform = FTransform::Identity;
		TimedObjectiveInstigator.Reset();
		RefreshObjectiveState();
		return;
	}

	TimedObjectiveRemainingSeconds = FMath::Max(0.0f, TimedObjectiveRemainingSeconds - DeltaTime);
	if (TimedObjectiveRemainingSeconds > 0.0f)
	{
		ApplyObjectiveVisuals();
		return;
	}

	if (!HasTimedObjectiveReachedEscapeDestination())
	{
		FailTimedObjective(TimedObjectiveInstigator.Get());
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
		if (bHasTimedObjectiveOriginTransform)
		{
			SetActorTransform(TimedObjectiveOriginTransform, false, nullptr, ETeleportType::TeleportPhysics);
		}
		bHasTimedObjectiveOriginTransform = false;
		TimedObjectiveOriginTransform = FTransform::Identity;
		TimedObjectiveInstigator.Reset();
		RefreshObjectiveState();
	}
}

bool AHorrorCampaignObjectiveActor::AbortTimedObjectiveForRecovery(AActor* InstigatorActor, FName FailureCause, FText FailureTitle, FText RetryHint)
{
	if (!bTimedObjectiveActive && !bTimedObjectiveUsesEscapeDestination)
	{
		return false;
	}

	++ObjectiveFailureAttemptCount;
	const FName ResolvedFailureCause = FailureCause.IsNone()
		? CampaignPursuitTimeoutFailureCause
		: FailureCause;
	bTimedObjectiveActive = false;
	bObjectiveFailedRetryable = true;
	LastObjectiveFailureCause = ResolvedFailureCause;
	LastObjectiveRecoveryAction = ReturnToEscapeStartRecoveryAction;
	TimedObjectiveRemainingSeconds = TimedObjectiveDurationSeconds;
	bTimedObjectiveUsesEscapeDestination = false;
	TimedObjectiveEscapeDestinationLocation = FVector::ZeroVector;
	TimedObjectiveInstigator.Reset();
	InteractionProgressCount = 0;
	if (bHasTimedObjectiveOriginTransform)
	{
		SetActorTransform(TimedObjectiveOriginTransform, false, nullptr, ETeleportType::TeleportPhysics);
	}
	bHasTimedObjectiveOriginTransform = false;
	TimedObjectiveOriginTransform = FTransform::Identity;

	if (UWorld* World = GetWorld())
	{
		if (AHorrorGameModeBase* HorrorGameMode = World->GetAuthGameMode<AHorrorGameModeBase>())
		{
			HorrorGameMode->StopCampaignAmbushThreat(ObjectiveDefinition.ObjectiveId);
			HorrorGameMode->RequestPlayerFailure(
				ResolvedFailureCause,
				NSLOCTEXT("HorrorCampaignObjective", "PursuitRecoveryFailureFeedback", "追逐失败，正在拉回最近的安全点..."));
		}
	}

	PublishObjectiveFailedEvent(
		InstigatorActor,
		FailureTitle.IsEmpty()
			? NSLOCTEXT("HorrorCampaignObjective", "PursuitRecoveryFailureTitle", "追逐失败：巨人追上了你")
			: FailureTitle,
		RetryHint.IsEmpty()
			? NSLOCTEXT("HorrorCampaignObjective", "PursuitRecoveryRetryHint", "重新互动可再次开始追逐，沿导航跑向亮起的逃离点。")
			: RetryHint,
		ResolvedFailureCause,
		ReturnToEscapeStartRecoveryAction);
	RefreshObjectiveState();
	return true;
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
	bObjectiveFailedRetryable = false;
	LastObjectiveFailureCause = NAME_None;
	LastObjectiveRecoveryAction = NAME_None;
	TimedObjectiveRemainingSeconds = 0.0f;
	bTimedObjectiveUsesEscapeDestination = false;
	bRecoverRelicAwaitingDelivery = false;
	RecoverRelicDeliveryLocation = FVector::ZeroVector;
	bHasTimedObjectiveOriginTransform = false;
	TimedObjectiveOriginTransform = FTransform::Identity;
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
