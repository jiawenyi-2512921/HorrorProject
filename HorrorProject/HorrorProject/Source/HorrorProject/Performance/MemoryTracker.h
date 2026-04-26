// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MemoryTracker.generated.h"

/**
 * Memory usage statistics
 */
USTRUCT(BlueprintType)
struct HORRORPROJECT_API FMemoryStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float UsedPhysicalMB = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float UsedVirtualMB = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float PeakUsedPhysicalMB = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float PeakUsedVirtualMB = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float AvailablePhysicalMB = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float TotalPhysicalMB = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float TextureMemoryMB = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float RenderTargetMemoryMB = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 AllocatedObjects = 0;
};

/**
 * Memory allocation event
 */
USTRUCT(BlueprintType)
struct HORRORPROJECT_API FMemoryAllocationEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly)
	float MemoryDeltaMB = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	FString Context;
};

/**
 * Memory tracker subsystem
 * Monitors memory usage and detects memory leaks
 */
UCLASS()
class HORRORPROJECT_API UMemoryTracker : public UGameInstanceSubsystem
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

	// Get current memory statistics
	UFUNCTION(BlueprintCallable, Category = "Performance")
	FMemoryStats GetMemoryStats() const;

	// Set memory budget warning threshold (MB)
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void SetMemoryBudgetMB(float BudgetMB);

	UFUNCTION(BlueprintPure, Category = "Performance")
	float GetMemoryBudgetMB() const { return MemoryBudgetMB; }

	// Check if memory usage is within budget
	UFUNCTION(BlueprintPure, Category = "Performance")
	bool IsMemoryWithinBudget() const;

	// Get memory usage percentage of budget
	UFUNCTION(BlueprintPure, Category = "Performance")
	float GetMemoryBudgetPercent() const;

	// Mark a memory checkpoint
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void MarkCheckpoint(const FString& CheckpointName);

	// Get memory delta since last checkpoint
	UFUNCTION(BlueprintCallable, Category = "Performance")
	float GetMemoryDeltaSinceCheckpoint() const;

	// Reset statistics
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void ResetStats();

	// Export memory data to CSV
	UFUNCTION(BlueprintCallable, Category = "Performance")
	bool ExportToCSV(const FString& FilePath);

	// Declare delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryBudgetExceeded, float, CurrentUsageMB);

	UPROPERTY(BlueprintAssignable, Category = "Performance")
	FOnMemoryBudgetExceeded OnMemoryBudgetExceeded;

private:
	void UpdateMemoryStats();
	void CheckMemoryBudget();

	UPROPERTY()
	bool bTrackingEnabled = false;

	UPROPERTY()
	float MemoryBudgetMB = 4096.0f; // 4GB default

	// Checkpoint tracking
	float CheckpointMemoryMB = 0.0f;
	FString LastCheckpointName;

	// Memory history
	TArray<FMemoryAllocationEvent> MemoryHistory;
	int32 MaxHistorySize = 1000;

	// Peak tracking
	float PeakPhysicalMB = 0.0f;
	float PeakVirtualMB = 0.0f;

	// Timer handle
	FTimerHandle UpdateTimerHandle;

	bool bBudgetExceededWarningShown = false;
};
