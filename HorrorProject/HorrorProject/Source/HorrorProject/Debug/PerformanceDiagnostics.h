// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceDiagnostics.generated.h"

USTRUCT(BlueprintType)
struct FPerformanceSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly)
	float DeltaTime;

	UPROPERTY(BlueprintReadOnly)
	float FPS;

	UPROPERTY(BlueprintReadOnly)
	float GameThreadTime;

	UPROPERTY(BlueprintReadOnly)
	float RenderThreadTime;

	UPROPERTY(BlueprintReadOnly)
	float GPUTime;

	UPROPERTY(BlueprintReadOnly)
	int32 ActorCount;
};

USTRUCT(BlueprintType)
struct FPerformanceStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float AverageFPS = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float MinFPS = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float MaxFPS = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float CurrentFPS = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float CurrentFrameTime = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float AverageGameThreadTime = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float AverageRenderThreadTime = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float AverageGPUTime = 0.0f;
};

/**
 * Performance diagnostics and monitoring
 */
UCLASS(BlueprintType)
class HORRORPROJECT_API UPerformanceDiagnostics : public UObject
{
	GENERATED_BODY()

public:
	UPerformanceDiagnostics();

	UFUNCTION(BlueprintCallable, Category = "Performance")
	void StartMonitoring(float Interval = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Performance")
	void StopMonitoring();

	UFUNCTION(BlueprintCallable, Category = "Performance")
	FPerformanceStats GetCurrentStats() const;

	UFUNCTION(BlueprintCallable, Category = "Performance")
	void GeneratePerformanceReport(const FString& FilePath = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Performance")
	void ClearHistory();

	UFUNCTION(BlueprintCallable, Category = "Performance")
	void CaptureFrameProfile();

	UFUNCTION(BlueprintCallable, Category = "Performance")
	void StartCPUProfiling();

	UFUNCTION(BlueprintCallable, Category = "Performance")
	void StopCPUProfiling();

	UFUNCTION(BlueprintPure, Category = "Performance")
	bool IsMonitoring() const { return bIsMonitoring; }

	UFUNCTION(BlueprintPure, Category = "Performance")
	TArray<FPerformanceSnapshot> GetPerformanceHistory() const { return PerformanceHistory; }

protected:
	void CollectPerformanceData();
	void CheckPerformanceThresholds(const FPerformanceSnapshot& Snapshot);

	UPROPERTY()
	TArray<FPerformanceSnapshot> PerformanceHistory;

	UPROPERTY()
	TArray<float> FrameTimeHistory;

	UPROPERTY()
	TArray<float> FPSHistory;

	UPROPERTY()
	bool bIsMonitoring;

	UPROPERTY()
	float MonitoringInterval;

	FTimerHandle MonitoringTimer;
};
