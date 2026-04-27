#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "AchievementSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FAchievementSaveRecord
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Achievement")
	FName AchievementID = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Achievement")
	bool bUnlocked = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Achievement")
	float Progress = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Achievement")
	FDateTime UnlockTime = FDateTime::MinValue();
};

/**
 * Stores persistent Horror Achievement Save Game data for save and restore workflows.
 */
UCLASS(BlueprintType)
class HORRORPROJECT_API UHorrorAchievementSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Achievement")
	int32 SaveVersion = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Achievement")
	FDateTime SavedAtUtc = FDateTime::MinValue();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Achievement")
	TArray<FAchievementSaveRecord> Records;
};
