// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MemoryDiagnostics.generated.h"

USTRUCT(BlueprintType)
struct FMemorySnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly)
	float UsedPhysicalMB;

	UPROPERTY(BlueprintReadOnly)
	float UsedVirtualMB;

	UPROPERTY(BlueprintReadOnly)
	float AvailablePhysicalMB;

	UPROPERTY(BlueprintReadOnly)
	float PeakUsedPhysicalMB;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalAllocations;
};

USTRUCT(BlueprintType)
struct FMemoryStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float CurrentUsedMB = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float PeakUsedMB = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float AverageUsedMB = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float AvailableMB = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float UsagePercent = 0.0f;
};

/**
 * Memory diagnostics and leak detection
 */
UCLASS(BlueprintType)
class HORRORPROJECT_API UMemoryDiagnostics : public UObject
{
	GENERATED_BODY()

public:
	UMemoryDiagnostics();

	UFUNCTION(BlueprintCallable, Category = "Memory")
	void StartMonitoring(float Interval = 5.0f);

	UFUNCTION(BlueprintCallable, Category = "Memory")
	void StopMonitoring();

	UFUNCTION(BlueprintCallable, Category = "Memory")
	FMemoryStats GetCurrentStats() const;

	UFUNCTION(BlueprintCallable, Category = "Memory")
	void GenerateMemoryReport(const FString& FilePath = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Memory")
	void ClearHistory();

	UFUNCTION(BlueprintCallable, Category = "Memory")
	void TakeMemorySnapshot();

	UFUNCTION(BlueprintCallable, Category = "Memory")
	void DetectMemoryLeaks();

	UFUNCTION(BlueprintCallable, Category = "Memory")
	void ForceGarbageCollection();

	UFUNCTION(BlueprintCallable, Category = "Memory")
	void DumpMemoryStats();

	UFUNCTION(BlueprintPure, Category = "Memory")
	bool IsMonitoring() const { return bIsMonitoring; }

	UFUNCTION(BlueprintPure, Category = "Memory")
	TArray<FMemorySnapshot> GetMemoryHistory() const { return MemoryHistory; }

protected:
	void CollectMemoryData();
	void CheckMemoryThresholds(const FMemorySnapshot& Snapshot);
	void AnalyzeMemoryTrend();

	UPROPERTY()
	TArray<FMemorySnapshot> MemoryHistory;

	UPROPERTY()
	bool bIsMonitoring;

	UPROPERTY()
	float MonitoringInterval;

	UPROPERTY()
	float BaselineMemoryMB;

	UPROPERTY()
	float MemoryLeakThresholdMB;

	FTimerHandle MonitoringTimer;
};
