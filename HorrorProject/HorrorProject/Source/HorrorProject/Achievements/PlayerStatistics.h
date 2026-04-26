#pragma once

#include "CoreMinimal.h"
#include "PlayerStatistics.generated.h"

USTRUCT(BlueprintType)
struct FPlayerStatistics
{
	GENERATED_BODY()

	// Exploration Stats
	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 RoomsExplored = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 SecretsFound = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float DistanceTraveled = 0.0f;

	// Collection Stats
	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 EvidenceCollected = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 DocumentsRead = 0;

	// Survival Stats
	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 GhostEncounters = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 Deaths = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float TimeInDarkness = 0.0f;

	// Play Time
	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float TotalPlayTime = 0.0f;

	FPlayerStatistics()
		: RoomsExplored(0)
		, SecretsFound(0)
		, DistanceTraveled(0.0f)
		, EvidenceCollected(0)
		, DocumentsRead(0)
		, GhostEncounters(0)
		, Deaths(0)
		, TimeInDarkness(0.0f)
		, TotalPlayTime(0.0f)
	{}
};
