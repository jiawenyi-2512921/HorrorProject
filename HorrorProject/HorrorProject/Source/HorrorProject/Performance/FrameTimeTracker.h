// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FrameTimeTracker.generated.h"

namespace HorrorFrameTimeTrackerDefaults
{
	inline constexpr float TargetFPS = 60.0f;
	inline constexpr int32 MaxHistorySize = 300;
}

/**
 * Frame time statistics
 */
USTRUCT(BlueprintType)
struct HORRORPROJECT_API FFrameTimeStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float CurrentFPS = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float AverageFPS = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float MinFPS = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float MaxFPS = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float CurrentFrameTimeMs = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float AverageFrameTimeMs = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float GameThreadTimeMs = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float RenderThreadTimeMs = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float GPUTimeMs = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 FramesBelowTarget = 0;

	UPROPERTY(BlueprintReadOnly)
	float PercentBelowTarget = 0.0f;
};

/**
 * Frame time tracker subsystem
 * Tracks frame times and provides performance statistics
 */
UCLASS()
class HORRORPROJECT_API UFrameTimeTracker : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Enable/disable tracking
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void SetTrackingEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Performance")
	bool IsTrackingEnabled() const { return bTrackingEnabled; }

	// Get current frame time statistics
	UFUNCTION(BlueprintCallable, Category = "Performance")
	FFrameTimeStats GetFrameTimeStats() const;

	// Set target FPS for performance budget
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void SetTargetFPS(float InTargetFPS);

	UFUNCTION(BlueprintPure, Category = "Performance")
	float GetTargetFPS() const { return TargetFPS; }

	// Check if current frame is within budget
	UFUNCTION(BlueprintPure, Category = "Performance")
	bool IsFrameWithinBudget() const;

	// Reset statistics
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void ResetStats();

	// Export frame time data to CSV
	UFUNCTION(BlueprintCallable, Category = "Performance")
	bool ExportToCSV(const FString& FilePath);

private:
	void OnFrameEnd();
	void UpdateStats(float DeltaTime);

	UPROPERTY()
	bool bTrackingEnabled = false;

	UPROPERTY()
	float TargetFPS = HorrorFrameTimeTrackerDefaults::TargetFPS;

	// Frame time history
	TArray<float> FrameTimeHistory;
	int32 MaxHistorySize = HorrorFrameTimeTrackerDefaults::MaxHistorySize;

	// Statistics
	float TotalFrameTime = 0.0f;
	int32 FrameCount = 0;
	float MinFrameTime = FLT_MAX;
	float MaxFrameTime = 0.0f;
	int32 FramesBelowTarget = 0;

	// Delegate handle
	FDelegateHandle FrameEndHandle;
};
