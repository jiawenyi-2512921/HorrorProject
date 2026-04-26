// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayTelemetry.h"

void UGameplayTelemetry::Update(float DeltaTime)
{
	Metrics.TotalPlaytime += DeltaTime;
}

void UGameplayTelemetry::RecordLevelCompleted(const FString& LevelName)
{
	Metrics.LevelsCompleted++;

	int32& Count = LevelCompletions.FindOrAdd(LevelName, 0);
	Count++;

	UE_LOG(LogTemp, Log, TEXT("Level completed: %s (Total: %d)"), *LevelName, Metrics.LevelsCompleted);
}

void UGameplayTelemetry::RecordDeath(const FString& CauseOfDeath)
{
	Metrics.DeathCount++;

	int32& Count = DeathCauses.FindOrAdd(CauseOfDeath, 0);
	Count++;

	UE_LOG(LogTemp, Log, TEXT("Player death: %s (Total deaths: %d)"), *CauseOfDeath, Metrics.DeathCount);
}

void UGameplayTelemetry::RecordAchievement(const FString& AchievementId)
{
	if (!UnlockedAchievements.Contains(AchievementId))
	{
		UnlockedAchievements.Add(AchievementId);
		Metrics.AchievementsUnlocked++;

		UE_LOG(LogTemp, Log, TEXT("Achievement unlocked: %s (Total: %d)"), *AchievementId, Metrics.AchievementsUnlocked);
	}
}

void UGameplayTelemetry::RecordItemCollected(const FString& ItemId)
{
	Metrics.ItemsCollected++;

	int32& Count = ItemCollections.FindOrAdd(ItemId, 0);
	Count++;

	UE_LOG(LogTemp, Verbose, TEXT("Item collected: %s (Total: %d)"), *ItemId, Metrics.ItemsCollected);
}

void UGameplayTelemetry::RecordEnemyDefeated(const FString& EnemyType)
{
	Metrics.EnemiesDefeated++;

	int32& Count = EnemyDefeats.FindOrAdd(EnemyType, 0);
	Count++;

	UE_LOG(LogTemp, Verbose, TEXT("Enemy defeated: %s (Total: %d)"), *EnemyType, Metrics.EnemiesDefeated);
}

void UGameplayTelemetry::RecordDistanceTraveled(float Distance)
{
	Metrics.DistanceTraveled += Distance;
}
