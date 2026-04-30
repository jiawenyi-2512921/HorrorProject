// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/HorrorCampaign.h"
#include "HorrorAdvancedInteractionTypes.generated.h"

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorAdvancedInteractionHUDState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bVisible = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	EHorrorCampaignInteractionMode Mode = EHorrorCampaignInteractionMode::Instant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FName ExpectedInputId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText FeedbackText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float ProgressFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float TimingFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float PauseRemainingSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bTimingWindowOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bPaused = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	int32 StepIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	int32 RequiredStepCount = 0;
};
