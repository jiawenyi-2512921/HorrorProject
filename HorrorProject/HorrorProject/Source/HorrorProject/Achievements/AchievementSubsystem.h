#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AchievementDefinition.h"
#include "AchievementSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAchievementUnlocked, const FAchievementData&, Achievement);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAchievementProgress, FName, AchievementID, float, Progress);

UCLASS()
class HORRORPROJECT_API UAchievementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Achievement Management
	UFUNCTION(BlueprintCallable, Category = "Achievement")
	void UnlockAchievement(FName AchievementID);

	UFUNCTION(BlueprintCallable, Category = "Achievement")
	void UpdateAchievementProgress(FName AchievementID, float Progress);

	UFUNCTION(BlueprintCallable, Category = "Achievement")
	bool IsAchievementUnlocked(FName AchievementID) const;

	UFUNCTION(BlueprintCallable, Category = "Achievement")
	float GetAchievementProgress(FName AchievementID) const;

	UFUNCTION(BlueprintCallable, Category = "Achievement")
	TArray<FAchievementData> GetAllAchievements() const;

	UFUNCTION(BlueprintCallable, Category = "Achievement")
	TArray<FAchievementData> GetUnlockedAchievements() const;

	UFUNCTION(BlueprintCallable, Category = "Achievement")
	TArray<FAchievementData> GetLockedAchievements() const;

	UFUNCTION(BlueprintCallable, Category = "Achievement")
	int32 GetUnlockedCount() const;

	UFUNCTION(BlueprintCallable, Category = "Achievement")
	float GetCompletionPercentage() const;

	// Platform Integration
	UFUNCTION(BlueprintCallable, Category = "Achievement")
	void SyncWithPlatform();

	UFUNCTION(BlueprintCallable, Category = "Achievement")
	bool IsPlatformAchievementSystemAvailable() const;

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Achievement")
	void SaveAchievementData();

	UFUNCTION(BlueprintCallable, Category = "Achievement")
	void LoadAchievementData();

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Achievement")
	FOnAchievementUnlocked OnAchievementUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Achievement")
	FOnAchievementProgress OnAchievementProgress;

private:
	UPROPERTY()
	TMap<FName, FAchievementData> Achievements;

	UPROPERTY()
	TMap<FName, float> AchievementProgressMap;

	UPROPERTY()
	TSet<FName> UnlockedAchievements;

	void InitializeAchievements();
	void UnlockAchievementInternal(FName AchievementID, bool bSyncPlatform = true);
	void SyncSteamAchievement(FName AchievementID);
	void SyncEpicAchievement(FName AchievementID);

	bool bInitialized;
	bool bPlatformAvailable;
};
