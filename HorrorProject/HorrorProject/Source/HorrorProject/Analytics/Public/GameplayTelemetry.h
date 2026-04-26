// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTelemetry.generated.h"

USTRUCT(BlueprintType)
struct FGameplayMetrics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	float TotalPlaytime = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	int32 LevelsCompleted = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	int32 DeathCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	int32 AchievementsUnlocked = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	int32 ItemsCollected = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	int32 EnemiesDefeated = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	float DistanceTraveled = 0.0f;
};

/**
 * Gameplay Telemetry - Tracks gameplay-specific metrics
 */
UCLASS()
class HORRORPROJECT_API UGameplayTelemetry : public UObject
{
	GENERATED_BODY()

public:
	void Update(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void RecordLevelCompleted(const FString& LevelName);

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void RecordDeath(const FString& CauseOfDeath);

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void RecordAchievement(const FString& AchievementId);

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void RecordItemCollected(const FString& ItemId);

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void RecordEnemyDefeated(const FString& EnemyType);

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void RecordDistanceTraveled(float Distance);

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	float GetTotalPlaytime() const { return Metrics.TotalPlaytime; }

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	int32 GetLevelsCompleted() const { return Metrics.LevelsCompleted; }

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	int32 GetDeathCount() const { return Metrics.DeathCount; }

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	int32 GetAchievementsUnlocked() const { return Metrics.AchievementsUnlocked; }

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	FGameplayMetrics GetMetrics() const { return Metrics; }

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	TMap<FString, int32> GetLevelCompletionCounts() const { return LevelCompletions; }

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	TMap<FString, int32> GetDeathCauses() const { return DeathCauses; }

protected:
	UPROPERTY()
	FGameplayMetrics Metrics;

	UPROPERTY()
	TMap<FString, int32> LevelCompletions;

	UPROPERTY()
	TMap<FString, int32> DeathCauses;

	UPROPERTY()
	TSet<FString> UnlockedAchievements;

	UPROPERTY()
	TMap<FString, int32> ItemCollections;

	UPROPERTY()
	TMap<FString, int32> EnemyDefeats;
};
