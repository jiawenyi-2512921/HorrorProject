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
	SurviveAmbush UMETA(DisplayName="巨人追逐"),
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
	SpectralScan UMETA(DisplayName="频谱扫描"),
	SignalTuning UMETA(DisplayName="信号调谐"),
	TimedPursuit UMETA(DisplayName="巨人追逐")
};

UENUM(BlueprintType)
enum class EHorrorCampaignObjectiveRiskLevel : uint8
{
	None UMETA(DisplayName="无"),
	Low UMETA(DisplayName="低"),
	Medium UMETA(DisplayName="中"),
	High UMETA(DisplayName="高"),
	Critical UMETA(DisplayName="致命")
};

UENUM(BlueprintType)
enum class EHorrorCampaignObjectiveBeatCompletionRule : uint8
{
	None UMETA(DisplayName="未配置"),
	InteractOnce UMETA(DisplayName="单次互动"),
	HoldInteract UMETA(DisplayName="持续互动"),
	AdvancedWindow UMETA(DisplayName="高级交互窗口"),
	ReachEscapePoint UMETA(DisplayName="抵达逃离点"),
	RecordEvidence UMETA(DisplayName="录像归档"),
	ConfirmResult UMETA(DisplayName="确认结果")
};

UENUM(BlueprintType)
enum class EHorrorCampaignObjectiveBeatFailurePolicy : uint8
{
	None UMETA(DisplayName="无失败"),
	RetryBeat UMETA(DisplayName="重试当前阶段"),
	ResetObjective UMETA(DisplayName="重置目标"),
	CampaignRecovery UMETA(DisplayName="战役失败恢复")
};

UENUM(BlueprintType)
enum class EHorrorCampaignObjectiveBeatNavigationRole : uint8
{
	None UMETA(DisplayName="无导航"),
	ObjectiveActor UMETA(DisplayName="目标位置"),
	EscapeDestination UMETA(DisplayName="逃离点"),
	SearchArea UMETA(DisplayName="搜索区域"),
	ConfirmationPoint UMETA(DisplayName="确认点")
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorCampaignObjectivePresentation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Presentation")
	FText MechanicLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Presentation")
	FText InputHint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Presentation")
	FText MissionContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Presentation")
	FText FailureStakes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Presentation")
	EHorrorCampaignObjectiveRiskLevel RiskLevel = EHorrorCampaignObjectiveRiskLevel::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Presentation")
	bool bUsesFocusedInteraction = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Presentation")
	bool bOpensInteractionPanel = false;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorCampaignObjectiveReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Reward")
	FText RewardText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Reward")
	FName EvidenceId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Reward")
	FText EvidenceDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Reward")
	FText EvidenceDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Reward", meta=(ClampMin="0.0", ClampMax="100.0"))
	float FearRelief = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Reward", meta=(ClampMin="0.0", ClampMax="100.0"))
	float BatteryChargePercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Reward")
	FName AchievementId = NAME_None;

	bool HasAnyReward() const
	{
		return !RewardText.IsEmpty()
			|| !EvidenceId.IsNone()
			|| FearRelief > 0.0f
			|| BatteryChargePercent > 0.0f
			|| !AchievementId.IsNone();
	}
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorCampaignAdvancedInteractionTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction", meta=(ClampMin="1"))
	int32 RequiredStepCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction", meta=(ClampMin="0.01", ClampMax="1.0"))
	float SuccessProgress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction", meta=(ClampMin="0.1"))
	float CueCycleSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction", meta=(UIMin="0.0", UIMax="1.0"))
	float TimingWindowStartFraction = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction", meta=(UIMin="0.0", UIMax="1.0"))
	float TimingWindowEndFraction = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction", meta=(UIMin="0.0", UIMax="1.0"))
	float FailureProgressPenalty = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction", meta=(UIMin="0.0", UIMax="1.0"))
	float FailureStabilityDamage = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction", meta=(UIMin="0.0", UIMax="1.0"))
	float SuccessStabilityRecovery = -1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction", meta=(UIMin="0.0"))
	float FailurePauseSeconds = -1.0f;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorCampaignObjectiveBeat
{
	GENERATED_BODY()

	FHorrorCampaignObjectiveBeat() = default;

	FHorrorCampaignObjectiveBeat(
		const FText& InLabel,
		const FText& InDetail,
		bool bInUrgent = false,
		bool bInRequiresRecording = false)
		: Label(InLabel)
		, Detail(InDetail)
		, bUrgent(bInUrgent)
		, bRequiresRecording(bInRequiresRecording)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Objective Beat")
	FName BeatId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Objective Beat")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Objective Beat")
	FText Detail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Objective Beat")
	EHorrorCampaignObjectiveBeatCompletionRule CompletionRule = EHorrorCampaignObjectiveBeatCompletionRule::InteractOnce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Objective Beat")
	EHorrorCampaignObjectiveBeatFailurePolicy FailurePolicy = EHorrorCampaignObjectiveBeatFailurePolicy::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Objective Beat")
	EHorrorCampaignObjectiveBeatNavigationRole NavigationRole = EHorrorCampaignObjectiveBeatNavigationRole::ObjectiveActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Objective Beat")
	bool bUrgent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Objective Beat")
	bool bRequiresRecording = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Objective Beat")
	bool bOpensInteractionPanel = false;
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
	TArray<FHorrorCampaignObjectiveBeat> ObjectiveBeats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	FHorrorCampaignObjectiveReward Reward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	FHorrorCampaignObjectivePresentation Presentation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	FHorrorCampaignAdvancedInteractionTuning AdvancedInteractionTuning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	TArray<FName> PrerequisiteObjectiveIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Pursuit")
	FVector EscapeDestinationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Relic", meta=(ClampMin="50.0", Units="cm"))
	float RelicDeliveryCompletionRadius = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Pursuit", meta=(ClampMin="0.0", Units="s"))
	float TimedObjectiveDurationSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Pursuit", meta=(ClampMin="50.0", Units="cm"))
	float EscapeCompletionRadius = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	FSoftObjectPath VisualMeshPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	bool bRequiredForChapterCompletion = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign")
	bool bOptional = false;
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

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorCampaignObjectiveSaveState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	FName ChapterId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	FName ObjectiveId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	int32 InteractionProgressCount = 0;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	int32 ObjectiveFailureAttemptCount = 0;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	bool bObjectiveFailedRetryable = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	FName LastObjectiveFailureCause = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	FName LastObjectiveRecoveryAction = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	bool bAdvancedInteractionActive = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	float AdvancedInteractionProgressFraction = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	int32 AdvancedInteractionStepIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	FName ExpectedAdvancedInputId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	float AdvancedInteractionCueElapsedSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	float AdvancedInteractionTimingFraction = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	bool bAdvancedInteractionCueResolved = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	float AdvancedInteractionStabilityFraction = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	float SignalTuningBalanceFraction = 0.5f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	float SignalTuningTargetBalanceFraction = 0.5f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	float SpectralScanConfidenceFraction = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	float SpectralScanNoiseFraction = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	float SpectralScanFilterFocusFraction = 0.5f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	float SpectralScanTargetFocusFraction = 0.5f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	int32 AdvancedInteractionComboCount = 0;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	int32 AdvancedInteractionMistakeCount = 0;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	float AdvancedInteractionPauseRemainingSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	bool bTimedObjectiveActive = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	bool bRecoverRelicAwaitingDelivery = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	float TimedObjectiveRemainingSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	float TimedObjectiveDurationSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	bool bTimedObjectiveUsesEscapeDestination = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	FVector TimedObjectiveEscapeDestinationLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	FVector RecoverRelicDeliveryLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	bool bHasTimedObjectiveOriginTransform = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Save")
	FTransform TimedObjectiveOriginTransform = FTransform::Identity;

	bool HasRuntimeState() const
	{
		return bAdvancedInteractionActive
			|| bTimedObjectiveActive
			|| bRecoverRelicAwaitingDelivery
			|| bObjectiveFailedRetryable
			|| InteractionProgressCount > 0;
	}
};

struct HORRORPROJECT_API FHorrorCampaignSaveState
{
	FName ChapterId = NAME_None;
	TArray<FName> CompletedObjectiveIds;
	bool bBossDefeated = false;
	TArray<FHorrorCampaignObjectiveSaveState> ObjectiveRuntimeStates;
};

UENUM(BlueprintType)
enum class EHorrorCampaignObjectiveGraphStatus : uint8
{
	Locked UMETA(DisplayName="锁定"),
	Available UMETA(DisplayName="可执行"),
	Completed UMETA(DisplayName="已完成")
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorCampaignObjectiveGraphNode
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Graph")
	FName ChapterId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Graph")
	FName ObjectiveId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Graph")
	EHorrorCampaignObjectiveGraphStatus Status = EHorrorCampaignObjectiveGraphStatus::Locked;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Graph")
	TArray<FName> MissingPrerequisiteObjectiveIds;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Graph")
	int32 ChapterOrder = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Graph")
	bool bRequired = true;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Graph")
	bool bOptional = false;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Objective Graph")
	bool bMainline = true;

	bool IsAvailable() const { return Status == EHorrorCampaignObjectiveGraphStatus::Available; }
	bool IsCompleted() const { return Status == EHorrorCampaignObjectiveGraphStatus::Completed; }
	bool IsLocked() const { return Status == EHorrorCampaignObjectiveGraphStatus::Locked; }
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorCampaignValidationIssue
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Validation")
	FName ChapterId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Validation")
	FName ObjectiveId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Validation")
	FName IssueCode = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category="Horror|Campaign|Validation")
	FText Message;
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
	static TArray<FHorrorCampaignValidationIssue> ValidateCampaignChapters();
#if WITH_DEV_AUTOMATION_TESTS
	static TArray<FHorrorCampaignValidationIssue> ValidateChapterDefinitionForTests(const FHorrorCampaignChapterDefinition& Chapter);
#endif
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
	TArray<const FHorrorCampaignObjectiveDefinition*> GetAvailableObjectives() const;
	TArray<FHorrorCampaignObjectiveGraphNode> BuildObjectiveGraph() const;
	bool HasCompletedObjective(FName ObjectiveId) const { return CompletedObjectiveIds.Contains(ObjectiveId); }
	FHorrorCampaignSaveState ExportSaveState() const;
	void ImportSaveState(const FHorrorCampaignSaveState& SaveState);

private:
	int32 CountCompletedObjectivesOfType(EHorrorCampaignObjectiveType ObjectiveType) const;
	bool AreAllRequiredObjectivesComplete() const;
	bool AreObjectivePrerequisitesComplete(const FHorrorCampaignObjectiveDefinition& Objective) const;
	TArray<FName> GetMissingPrerequisites(const FHorrorCampaignObjectiveDefinition& Objective) const;

	const FHorrorCampaignChapterDefinition* ActiveChapter = nullptr;
	TSet<FName> CompletedObjectiveIds;
	bool bBossDefeated = false;
};
