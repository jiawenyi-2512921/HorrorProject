#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PlayerStatistics.h"
#include "StatisticsSaveGame.generated.h"

/**
 * Stores persistent Horror Statistics Save Game data for save and restore workflows.
 */
UCLASS(BlueprintType)
class HORRORPROJECT_API UHorrorStatisticsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Statistics")
	int32 SaveVersion = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Statistics")
	FDateTime SavedAtUtc = FDateTime::MinValue();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Statistics")
	FPlayerStatistics PlayerStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Statistics")
	TMap<FName, float> CustomStatistics;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, SaveGame, Category = "Statistics")
	float TotalPlayTime = 0.0f;
};
