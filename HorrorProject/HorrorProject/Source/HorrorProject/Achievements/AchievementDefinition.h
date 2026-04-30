#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AchievementDefinition.generated.h"

UENUM(BlueprintType)
enum class EAchievementCategory : uint8
{
	Exploration UMETA(DisplayName = "探索"),
	Collection UMETA(DisplayName = "收集"),
	Survival UMETA(DisplayName = "生存"),
	Combat UMETA(DisplayName = "战斗"),
	Speedrun UMETA(DisplayName = "速通"),
	Story UMETA(DisplayName = "剧情"),
	Secret UMETA(DisplayName = "秘密"),
	Challenge UMETA(DisplayName = "挑战")
};

USTRUCT(BlueprintType)
struct FHorrorAchievementDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAchievementCategory Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHidden;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsProgressive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxProgress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Points;

	FHorrorAchievementDefinition()
		: ID(NAME_None)
		, Category(EAchievementCategory::Exploration)
		, bIsHidden(false)
		, bIsProgressive(false)
		, MaxProgress(1.0f)
		, Points(10)
	{}
};

USTRUCT(BlueprintType)
struct FAchievementData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName ID;

	UPROPERTY(BlueprintReadOnly)
	FText Name;

	UPROPERTY(BlueprintReadOnly)
	FText Description;

	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(BlueprintReadOnly)
	EAchievementCategory Category;

	UPROPERTY(BlueprintReadOnly)
	bool bIsHidden;

	UPROPERTY(BlueprintReadOnly)
	bool bIsProgressive;

	UPROPERTY(BlueprintReadOnly)
	float MaxProgress;

	UPROPERTY(BlueprintReadOnly)
	int32 Points;

	UPROPERTY(BlueprintReadOnly)
	bool bUnlocked;

	UPROPERTY(BlueprintReadOnly)
	float CurrentProgress;

	UPROPERTY(BlueprintReadOnly)
	FDateTime UnlockTime;

	FAchievementData()
		: ID(NAME_None)
		, Category(EAchievementCategory::Exploration)
		, bIsHidden(false)
		, bIsProgressive(false)
		, MaxProgress(1.0f)
		, Points(10)
		, bUnlocked(false)
		, CurrentProgress(0.0f)
	{}
};

/**
 * Defines Achievement Definition Catalog data used by the Achievements module.
 */
UCLASS()
class HORRORPROJECT_API UAchievementDefinitionCatalog : public UDataAsset
{
	GENERATED_BODY()

public:
	static TArray<FHorrorAchievementDefinition> GetAllAchievementDefinitions();
};
