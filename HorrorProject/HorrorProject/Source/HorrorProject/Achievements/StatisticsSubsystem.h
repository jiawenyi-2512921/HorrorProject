#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerStatistics.h"
#include "StatisticsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatisticUpdated, FName, StatName, float, NewValue);

/**
 * Coordinates Statistics Subsystem services for the Achievements module.
 */
UCLASS()
class HORRORPROJECT_API UStatisticsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Statistics Management
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	void IncrementStatistic(FName StatName, float Amount = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Statistics")
	void SetStatistic(FName StatName, float Value);

	UFUNCTION(BlueprintCallable, Category = "Statistics")
	float GetStatistic(FName StatName) const;

	UFUNCTION(BlueprintCallable, Category = "Statistics")
	FPlayerStatistics GetPlayerStatistics() const;

	UFUNCTION(BlueprintCallable, Category = "Statistics")
	void ResetStatistics();

	UFUNCTION(BlueprintCallable, Category = "Statistics")
	void ResetStatistic(FName StatName);

	// Session Tracking
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	void StartSession();

	UFUNCTION(BlueprintCallable, Category = "Statistics")
	void EndSession();

	UFUNCTION(BlueprintCallable, Category = "Statistics")
	float GetCurrentSessionDuration() const;

	UFUNCTION(BlueprintCallable, Category = "Statistics")
	float GetTotalPlayTime() const;

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	void SaveStatistics();

	UFUNCTION(BlueprintCallable, Category = "Statistics")
	void LoadStatistics();

	// Report Generation
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	FString GenerateStatisticsReport() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Statistics")
	FOnStatisticUpdated OnStatisticUpdated;

private:
	UPROPERTY()
	FPlayerStatistics PlayerStats;

	UPROPERTY()
	TMap<FName, float> CustomStatistics;

	float SessionStartTime;
	float TotalPlayTime;
	bool bSessionActive;

	void InitializeDefaultStatistics();
	void UpdatePlayTime();
};
