// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PerformanceBudget.generated.h"

/**
 * Performance budget category
 */
UENUM(BlueprintType)
enum class EPerformanceBudgetCategory : uint8
{
	GameThread,
	RenderThread,
	GPU,
	Memory,
	DrawCalls,
	Triangles,
	Custom
};

/**
 * Performance budget entry
 */
USTRUCT(BlueprintType)
struct HORRORPROJECT_API FPerformanceBudgetEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BudgetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPerformanceBudgetCategory Category = EPerformanceBudgetCategory::Custom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BudgetValue = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float CurrentValue = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float PercentUsed = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	bool bExceeded = false;
};

/**
 * Performance budget manager
 * Tracks performance budgets and warns when exceeded
 */
UCLASS(Config=Game)
class HORRORPROJECT_API UPerformanceBudget : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Set budget for a category
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void SetBudget(FName BudgetName, EPerformanceBudgetCategory Category, float BudgetValue);

	// Update current value for a budget
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void UpdateBudgetValue(FName BudgetName, float CurrentValue);

	// Get budget entry
	UFUNCTION(BlueprintCallable, Category = "Performance")
	FPerformanceBudgetEntry GetBudget(FName BudgetName) const;

	// Get all budgets
	UFUNCTION(BlueprintCallable, Category = "Performance")
	TArray<FPerformanceBudgetEntry> GetAllBudgets() const;

	// Check if any budget is exceeded
	UFUNCTION(BlueprintPure, Category = "Performance")
	bool IsAnyBudgetExceeded() const;

	// Get exceeded budgets
	UFUNCTION(BlueprintCallable, Category = "Performance")
	TArray<FPerformanceBudgetEntry> GetExceededBudgets() const;

	// Enable/disable budget tracking
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void SetBudgetTrackingEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Performance")
	bool IsBudgetTrackingEnabled() const { return bTrackingEnabled; }

	// Reset all budgets
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void ResetBudgets();

	// Load default budgets for target platform
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void LoadDefaultBudgets();

	// Declare delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBudgetExceeded, FName, BudgetName);

	UPROPERTY(BlueprintAssignable, Category = "Performance")
	FOnBudgetExceeded OnBudgetExceeded;

private:
	void UpdateAutomaticBudgets();

	UPROPERTY(Config)
	bool bTrackingEnabled = false;

	// Budget entries
	TMap<FName, FPerformanceBudgetEntry> Budgets;

	// Timer handle
	FTimerHandle UpdateTimerHandle;

	// Track which budgets have triggered warnings
	TSet<FName> WarningTriggered;
};
