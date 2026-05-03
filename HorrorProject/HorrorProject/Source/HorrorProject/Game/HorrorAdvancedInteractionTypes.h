// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Game/HorrorCampaign.h"
#include "HorrorAdvancedInteractionTypes.generated.h"

UENUM(BlueprintType)
enum class EHorrorAdvancedInteractionInputRole : uint8
{
	SelectTarget UMETA(DisplayName="选择目标"),
	ConnectCircuit UMETA(DisplayName="接入电路"),
	AvoidHazard UMETA(DisplayName="避开危险"),
	CalibrateGear UMETA(DisplayName="校准齿轮"),
	LockSpectralBand UMETA(DisplayName="锁定波段"),
	TuneSignalLeft UMETA(DisplayName="左声道微调"),
	LockSignalCenter UMETA(DisplayName="锁定中心频率"),
	TuneSignalRight UMETA(DisplayName="右声道微调")
};

UENUM(BlueprintType)
enum class EHorrorAdvancedInteractionCommandType : uint8
{
	SelectOption UMETA(DisplayName="选择部件"),
	AdjustAxis UMETA(DisplayName="轴向调节"),
	Confirm UMETA(DisplayName="确认"),
	Cancel UMETA(DisplayName="取消")
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorAdvancedInteractionInputCommand
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	EHorrorAdvancedInteractionCommandType CommandType = EHorrorAdvancedInteractionCommandType::SelectOption;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FName InputId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction", meta=(ClampMin="-1.0", ClampMax="1.0"))
	float AxisValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction", meta=(ClampMin="0.0", Units="s"))
	float HoldSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	int32 VisualSlotIndex = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorAdvancedInteractionInputOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FName InputId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText DisplayLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText KeyHint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	EHorrorAdvancedInteractionInputRole ActionRole = EHorrorAdvancedInteractionInputRole::SelectTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText ActionVerb;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	int32 VisualSlotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FLinearColor VisualColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LoadFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MotionFraction = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ResponseWindowFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bHazardous = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bStalled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bChainLinked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bRequiresTimingWindow = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bRequiresStableSignal = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bAdvancesObjective = true;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorAdvancedInteractionStepTrackItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FName InputId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText DisplayLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText KeyHint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	int32 StepIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	int32 VisualSlotIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FLinearColor VisualColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bComplete = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bPreview = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bHazardous = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bRequiresTimingWindow = true;
};

UENUM(BlueprintType)
enum class EHorrorAdvancedInteractionFeedbackState : uint8
{
	Neutral UMETA(DisplayName="普通"),
	Success UMETA(DisplayName="成功"),
	Failure UMETA(DisplayName="失败"),
	Paused UMETA(DisplayName="暂停"),
	Overloaded UMETA(DisplayName="过载")
};

UENUM(BlueprintType)
enum class EHorrorAdvancedInteractionOutcomeKind : uint8
{
	Ignored UMETA(DisplayName="未处理"),
	Prompted UMETA(DisplayName="提示"),
	Paused UMETA(DisplayName="暂停"),
	Cancelled UMETA(DisplayName="取消"),
	Adjusted UMETA(DisplayName="调节"),
	TimingFailure UMETA(DisplayName="时机失败"),
	WrongInput UMETA(DisplayName="输入错误"),
	Hazard UMETA(DisplayName="危险输入"),
	SignalBalanceFailure UMETA(DisplayName="信号失衡"),
	SpectralConfidenceFailure UMETA(DisplayName="频谱置信不足"),
	SpectralFilterFailure UMETA(DisplayName="频谱滤波失败"),
	Overloaded UMETA(DisplayName="过载"),
	Success UMETA(DisplayName="成功"),
	Completed UMETA(DisplayName="完成")
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorAdvancedInteractionOutcome
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	EHorrorAdvancedInteractionOutcomeKind Kind = EHorrorAdvancedInteractionOutcomeKind::Ignored;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	EHorrorAdvancedInteractionFeedbackState FeedbackState = EHorrorAdvancedInteractionFeedbackState::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FName InputId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FName FaultId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FName FailureCause = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FName RecoveryAction = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bConsumesInput = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bAdvancesProgress = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bRetryable = false;
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorAdvancedInteractionHUDState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bVisible = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	EHorrorCampaignInteractionMode Mode = EHorrorCampaignInteractionMode::Instant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FName ExpectedInputId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText FeedbackText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	EHorrorAdvancedInteractionFeedbackState FeedbackState = EHorrorAdvancedInteractionFeedbackState::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FHorrorAdvancedInteractionOutcome LastOutcome;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText CurrentBeatLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText CurrentBeatDetail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText PhaseText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText ExpectedInputLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText DeviceStatusLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText RiskLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText RhythmLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText NextActionLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	FText FailureRecoveryLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	TArray<FHorrorAdvancedInteractionInputOption> InputOptions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	TArray<FHorrorAdvancedInteractionStepTrackItem> StepTrack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float ProgressFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float TimingFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float TimingWindowStartFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float TimingWindowEndFraction = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float StabilityFraction = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float SignalBalanceFraction = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float SignalTargetBalanceFraction = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float SpectralConfidenceFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float SpectralNoiseFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float SpectralFilterFocusFraction = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float SpectralTargetFocusFraction = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float PerformanceGradeFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float InputPrecisionFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float DeviceLoadFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float RouteFlowFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float HazardPressureFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float TargetAlignmentFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	float PauseRemainingSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bTimingWindowOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bPaused = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bRecentSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	bool bRecentFailure = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	int32 StepIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	int32 RequiredStepCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	int32 ComboCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	int32 MistakeCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Advanced Interaction")
	int32 ActiveInputSlotIndex = INDEX_NONE;
};

HORRORPROJECT_API bool IsHorrorAdvancedInteractionPanelMode(EHorrorCampaignInteractionMode Mode);
HORRORPROJECT_API void NormalizeHorrorAdvancedInteractionHUDState(FHorrorAdvancedInteractionHUDState& State);

UENUM(BlueprintType)
enum class EHorrorCampaignObjectiveRuntimeStatus : uint8
{
	Hidden UMETA(DisplayName="隐藏"),
	Locked UMETA(DisplayName="锁定"),
	Available UMETA(DisplayName="可互动"),
	AdvancedInteractionActive UMETA(DisplayName="高级交互中"),
	TimedPursuitActive UMETA(DisplayName="追逐中"),
	CarryingRelic UMETA(DisplayName="遗物搬运中"),
	FailedRetryable UMETA(DisplayName="失败可重试"),
	Completed UMETA(DisplayName="已完成")
};

USTRUCT(BlueprintType)
struct HORRORPROJECT_API FHorrorCampaignObjectiveRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	bool bVisible = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	bool bCanInteract = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	bool bAvailableForInteraction = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	bool bAdvancedInteractionActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	bool bTimedObjectiveActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	bool bRecoverRelicAwaitingDelivery = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	bool bUsesEscapeDestination = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	bool bBeatUrgent = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	bool bRequiresRecording = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	bool bRetryable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FName ChapterId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FName ObjectiveId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FName FailureCause = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FName RecoveryAction = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	EHorrorCampaignInteractionMode InteractionMode = EHorrorCampaignInteractionMode::Instant;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	EHorrorCampaignObjectiveRiskLevel RiskLevel = EHorrorCampaignObjectiveRiskLevel::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	EHorrorCampaignObjectiveRuntimeStatus Status = EHorrorCampaignObjectiveRuntimeStatus::Hidden;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FText PromptText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FText PhaseText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FText DeviceStatusLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FText NextActionLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FText FailureRecoveryLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FText CurrentBeatLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FText CurrentBeatDetail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FText BlockedReason;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	int32 CurrentBeatIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	int32 CompletedStepCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	int32 RequiredStepCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ProgressFraction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime", meta=(ClampMin="0.0", Units="s"))
	float TimedRemainingSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime", meta=(ClampMin="0.0", Units="s"))
	float TimedDurationSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FVector EscapeDestinationWorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime", meta=(ClampMin="0.0", Units="cm"))
	float EscapeCompletionRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime", meta=(ClampMin="0.0", Units="m"))
	float EscapeDistanceMeters = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FVector RelicDeliveryWorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime", meta=(ClampMin="0.0", Units="m"))
	float RelicDeliveryDistanceMeters = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime", meta=(ClampMin="0.0", Units="cm"))
	float RelicDeliveryCompletionRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime", meta=(Units="s"))
	float EscapeTimeBudgetSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime", meta=(ClampMin="0.0", Units="s"))
	float EstimatedEscapeArrivalSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FText EscapeBudgetLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FText EscapeActionLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Horror|Campaign|Runtime")
	FHorrorAdvancedInteractionHUDState AdvancedInteraction;
};
