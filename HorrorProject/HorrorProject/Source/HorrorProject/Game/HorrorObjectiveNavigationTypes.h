// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/HorrorAdvancedInteractionTypes.h"
#include "HorrorObjectiveNavigationTypes.generated.h"

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorObjectiveNavigationState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	bool bVisible = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	FText Label;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	FText DirectionText;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	FText StatusText;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	FText DeviceStatusLabel;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	FText NextActionLabel;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	FText FailureRecoveryLabel;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	FName FocusedObjectiveId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	float DistanceMeters = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	float AngleDegrees = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	float PerformanceGradeFraction = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	float EscapeTimeBudgetSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	float EstimatedEscapeArrivalSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	bool bArrived = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	bool bReachable = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	bool bRetryable = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	EHorrorCampaignObjectiveRuntimeStatus RuntimeStatus = EHorrorCampaignObjectiveRuntimeStatus::Hidden;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	FName FailureCause = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Objectives|Navigation")
	FName RecoveryAction = NAME_None;
};
