// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "HorrorCampaign.generated.h"

UENUM(BlueprintType)
enum class EHorrorCampaignObjectiveType : uint8
{
	AcquireSignal UMETA(DisplayName="获取信号"),
	ScanAnomaly UMETA(DisplayName="扫描异常"),
	RecoverRelic UMETA(DisplayName="回收遗物"),
	RestorePower UMETA(DisplayName="恢复电力"),
	PlantBeacon UMETA(DisplayName="放置信标"),
	SurviveAmbush UMETA(DisplayName="撑过伏击"),
	DisableSeal UMETA(DisplayName="解除封印"),
	BossWeakPoint UMETA(DisplayName="首领弱点"),
	FinalTerminal UMETA(DisplayName="最终终端")
};

UENUM(BlueprintType)
enum class EHorrorCampaignInteractionMode : uint8
{
	Instant UMETA(DisplayName="即时互动"),
	MultiStep UMETA(DisplayName="多段互动"),
	CircuitWiring UMETA(DisplayName="电路接线"),
	GearCalibration UMETA(DisplayName="齿轮校准"),
	TimedPursuit UMETA(DisplayName="限时追逐")
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorCampaignObjectiveDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	FName ObjectiveId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	EHorrorCampaignObjectiveType ObjectiveType = EHorrorCampaignObjectiveType::AcquireSignal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	EHorrorCampaignInteractionMode InteractionMode = EHorrorCampaignInteractionMode::Instant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	FText PromptText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	FText CompletionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Pursuit")
	FVector EscapeDestinationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Pursuit", meta=(ClampMin="50.0", Units="cm"))
	float EscapeCompletionRadius = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	FSoftObjectPath VisualMeshPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	bool bRequiredForChapterCompletion = true;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorCampaignChapterDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	FName ChapterId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	FString MapPackageName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	FText StoryBrief;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	FText CompletionBridgeText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	bool bRequiresBoss = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	bool bIsFinalChapter = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	TArray<FHorrorCampaignObjectiveDefinition> Objectives;
};

class HORRORPROJECT_API FHorrorCampaign
{
public:
	static const TArray<FHorrorCampaignChapterDefinition>& GetChapters();
	static const FHorrorCampaignChapterDefinition* FindChapterById(FName ChapterId);
	static const FHorrorCampaignChapterDefinition* FindChapterForMap(const FString& MapPackageName);
	static const FHorrorCampaignChapterDefinition* FindBossChapter();
	static int32 FindChapterIndexForMap(const FString& MapPackageName);
	static int32 CountRequiredObjectives(const FHorrorCampaignChapterDefinition& Chapter);
	static int32 CountObjectivesOfType(const FHorrorCampaignChapterDefinition& Chapter, EHorrorCampaignObjectiveType ObjectiveType);
	static const FHorrorCampaignObjectiveDefinition* FindObjectiveById(
		const FHorrorCampaignChapterDefinition& Chapter,
		FName ObjectiveId);
};

class HORRORPROJECT_API FHorrorCampaignProgress
{
public:
	void ResetForChapter(const FHorrorCampaignChapterDefinition& Chapter);
	bool CanCompleteObjective(FName ObjectiveId) const;
	bool TryCompleteObjective(FName ObjectiveId);
	bool IsChapterComplete() const;
	bool IsBossDefeated() const { return bBossDefeated; }
	bool HasActiveChapter() const { return ActiveChapter != nullptr; }
	FName GetActiveChapterId() const;
	int32 GetCompletedObjectiveCount() const { return CompletedObjectiveIds.Num(); }
	int32 GetRequiredObjectiveCount() const;
	const FHorrorCampaignObjectiveDefinition* GetNextObjective() const;
	bool HasCompletedObjective(FName ObjectiveId) const { return CompletedObjectiveIds.Contains(ObjectiveId); }

private:
	int32 CountCompletedObjectivesOfType(EHorrorCampaignObjectiveType ObjectiveType) const;
	bool AreAllRequiredObjectivesComplete() const;

	const FHorrorCampaignChapterDefinition* ActiveChapter = nullptr;
	TSet<FName> CompletedObjectiveIds;
	bool bBossDefeated = false;
};
