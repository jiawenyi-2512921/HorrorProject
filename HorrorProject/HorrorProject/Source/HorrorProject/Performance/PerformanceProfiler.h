// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PerformanceProfiler.generated.h"

/**
 * Performance profiling scope - RAII style profiler
 * Zero overhead in Shipping builds
 */
struct HORRORPROJECT_API FPerformanceScope
{
#if !UE_BUILD_SHIPPING
	FPerformanceScope(const FName& InScopeName);
	~FPerformanceScope();

private:
	FName ScopeName;
	double StartTime;
#else
	FORCEINLINE FPerformanceScope(const FName&) {}
	FORCEINLINE ~FPerformanceScope() {}
#endif
};

/**
 * Performance sample data
 */
USTRUCT(BlueprintType)
struct HORRORPROJECT_API FPerformanceSample
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName ScopeName;

	UPROPERTY(BlueprintReadOnly)
	double DurationMs;

	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly)
	int32 FrameNumber;
};

/**
 * Performance statistics for a scope
 */
USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorPerformanceScopeStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName ScopeName;

	UPROPERTY(BlueprintReadOnly)
	int32 SampleCount = 0;

	UPROPERTY(BlueprintReadOnly)
	double MinMs = 0.0;

	UPROPERTY(BlueprintReadOnly)
	double MaxMs = 0.0;

	UPROPERTY(BlueprintReadOnly)
	double AverageMs = 0.0;

	UPROPERTY(BlueprintReadOnly)
	double TotalMs = 0.0;

	void AddSample(double DurationMs);
	void Reset();
};

/**
 * Custom performance profiler for HorrorProject
 * Provides lightweight profiling with CSV export
 */
UCLASS(BlueprintType)
class HORRORPROJECT_API UPerformanceProfiler : public UObject
{
	GENERATED_BODY()

public:
	UPerformanceProfiler();

	// Enable/disable profiling
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void SetProfilingEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Performance")
	bool IsProfilingEnabled() const { return bProfilingEnabled; }

	// Record a performance sample
	void RecordSample(const FName& ScopeName, double DurationMs);

	// Get statistics for a scope
	UFUNCTION(BlueprintCallable, Category = "Performance")
	FHorrorPerformanceScopeStats GetStats(FName ScopeName) const;

	// Get all statistics
	UFUNCTION(BlueprintCallable, Category = "Performance")
	TArray<FHorrorPerformanceScopeStats> GetAllStats() const;

	// Clear all recorded data
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void ClearStats();

	// Export to CSV
	UFUNCTION(BlueprintCallable, Category = "Performance")
	bool ExportToCSV(const FString& FilePath);

	// Get singleton instance
	static UPerformanceProfiler* Get(UWorld* World);

private:
	UPROPERTY()
	bool bProfilingEnabled = false;

	// Statistics per scope
	TMap<FName, FHorrorPerformanceScopeStats> ScopeStats;

	// Recent samples for CSV export
	TArray<FPerformanceSample> RecentSamples;

	// Maximum samples to keep
	int32 MaxSamples = 10000;

	// Mutex for thread safety
	FCriticalSection DataMutex;
};

// Convenience macro for profiling
#if !UE_BUILD_SHIPPING
	#define HORROR_PROFILE_SCOPE(Name) FPerformanceScope __ProfileScope##__LINE__(Name)
	#define HORROR_PROFILE_FUNCTION() FPerformanceScope __ProfileScope##__LINE__(__FUNCTION__)
#else
	#define HORROR_PROFILE_SCOPE(Name)
	#define HORROR_PROFILE_FUNCTION()
#endif
