// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceTelemetry.generated.h"

USTRUCT(BlueprintType)
struct FPerformanceTelemetrySnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	float FPS = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	float FrameTimeMs = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	float MemoryUsageMB = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	float CPUUsagePercent = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	float GPUUsagePercent = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	FDateTime Timestamp;
};

/**
 * Performance Telemetry - Tracks game performance metrics
 */
UCLASS()
class HORRORPROJECT_API UPerformanceTelemetry : public UObject
{
	GENERATED_BODY()

public:
	void Update(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	float GetAverageFPS() const;

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	float GetMinFPS() const { return MinFPS; }

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	float GetMaxFPS() const { return MaxFPS; }

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	float GetAverageFrameTime() const;

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	float GetMemoryUsageMB() const;

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	FPerformanceTelemetrySnapshot GetCurrentSnapshot() const;

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	TArray<FPerformanceTelemetrySnapshot> GetPerformanceHistory() const { return PerformanceHistory; }

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void ResetStats();

protected:
	UPROPERTY()
	TArray<FPerformanceTelemetrySnapshot> PerformanceHistory;

	UPROPERTY()
	float MinFPS = TNumericLimits<float>::Max();

	UPROPERTY()
	float MaxFPS = 0.0f;

	UPROPERTY()
	float TotalFPS = 0.0f;

	UPROPERTY()
	float TotalFrameTime = 0.0f;

	UPROPERTY()
	int32 SampleCount = 0;

	static constexpr int32 MaxHistorySize = 1000;

private:
	void RecordSnapshot(const FPerformanceTelemetrySnapshot& Snapshot);
	float CalculateMemoryUsage() const;
};
