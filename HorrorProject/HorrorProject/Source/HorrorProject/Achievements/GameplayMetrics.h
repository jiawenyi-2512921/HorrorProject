#pragma once

#include "CoreMinimal.h"
#include "GameplayMetrics.generated.h"

USTRUCT(BlueprintType)
struct FHorrorGameplayMetrics
{
	GENERATED_BODY()

	// Performance Metrics
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float AverageFPS = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float MinFPS = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float MaxFPS = 0.0f;

	// Gameplay Metrics
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float AverageSanity = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float MinSanity = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 TotalInteractions = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 FailedInteractions = 0;

	// Horror Metrics
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 JumpScareCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float TotalScareIntensity = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float AverageHeartRate = 0.0f; // Simulated

	// Session Metrics
	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	FDateTime SessionStartTime;

	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float SessionDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 SaveCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 LoadCount = 0;

	FHorrorGameplayMetrics()
		: AverageFPS(0.0f)
		, MinFPS(0.0f)
		, MaxFPS(0.0f)
		, AverageSanity(100.0f)
		, MinSanity(100.0f)
		, TotalInteractions(0)
		, FailedInteractions(0)
		, JumpScareCount(0)
		, TotalScareIntensity(0.0f)
		, AverageHeartRate(0.0f)
		, SessionDuration(0.0f)
		, SaveCount(0)
		, LoadCount(0)
	{}
};

/**
 * Exposes Blueprint helper functions for Horror Gameplay Metrics Library workflows.
 */
UCLASS()
class HORRORPROJECT_API UHorrorGameplayMetricsLibrary : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Metrics")
	static FHorrorGameplayMetrics CreateMetrics();

	UFUNCTION(BlueprintCallable, Category = "Metrics")
	static void UpdateFPSMetrics(UPARAM(ref) FHorrorGameplayMetrics& Metrics, float CurrentFPS);

	UFUNCTION(BlueprintCallable, Category = "Metrics")
	static void UpdateSanityMetrics(UPARAM(ref) FHorrorGameplayMetrics& Metrics, float CurrentSanity);

	UFUNCTION(BlueprintCallable, Category = "Metrics")
	static void RecordJumpScare(UPARAM(ref) FHorrorGameplayMetrics& Metrics, float Intensity);

	UFUNCTION(BlueprintCallable, Category = "Metrics")
	static void RecordInteraction(UPARAM(ref) FHorrorGameplayMetrics& Metrics, bool bSuccess);

	UFUNCTION(BlueprintCallable, Category = "Metrics")
	static FString GenerateMetricsReport(const FHorrorGameplayMetrics& Metrics);
};
