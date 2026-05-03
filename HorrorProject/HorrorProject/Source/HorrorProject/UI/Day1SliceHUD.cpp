// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/Day1SliceHUD.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"

namespace
{
	constexpr float OuterPadding = 32.0f;
	constexpr float PanelPadding = 16.0f;
	constexpr float ObjectivePanelWidth = 620.0f;
	constexpr float AnomalyCapturePanelWidth = 420.0f;
	constexpr float AnomalyCapturePanelHeight = 74.0f;
	constexpr float ToastPanelWidth = 560.0f;
	constexpr float StatusPanelWidth = 360.0f;
	constexpr float StatusPanelHeight = 140.0f;
	constexpr float AutosavePanelWidth = 270.0f;
	constexpr float AutosavePanelHeight = 46.0f;
	constexpr float PasswordPanelWidth = 560.0f;
	constexpr float PasswordPanelHeight = 178.0f;
	constexpr float PausePanelWidth = 620.0f;
	constexpr float PausePanelHeight = 310.0f;
	constexpr float NotesJournalPanelWidth = 680.0f;
	constexpr float NotesJournalPanelHeight = 460.0f;
	constexpr float AdvancedPanelWidth = 1080.0f;  // 从780增加到1080，增加约38%
	constexpr float AdvancedPanelHeight = 720.0f;  // 从520增加到720，增加约38%
	constexpr float AdvancedTimingWindowStart = 0.36f;
	constexpr float AdvancedTimingWindowEnd = 0.72f;

	FLinearColor PanelBackground()
	{
		return FLinearColor(0.015f, 0.02f, 0.025f, 0.82f);
	}

	FLinearColor PanelAccent()
	{
		return FLinearColor(0.30f, 0.95f, 0.82f, 0.92f);
	}

	FLinearColor TextPrimary()
	{
		return FLinearColor(0.88f, 0.98f, 0.95f, 0.96f);
	}

	FLinearColor TextSecondary()
	{
		return FLinearColor(0.58f, 0.72f, 0.70f, 0.90f);
	}

	FLinearColor WarningAccent()
	{
		return FLinearColor(1.0f, 0.72f, 0.22f, 0.96f);
	}

	FLinearColor CircuitColorForInput(FName InputId)
	{
		if (InputId == FName(TEXT("蓝色端子")))
		{
			return FLinearColor(0.24f, 0.68f, 1.0f, 0.96f);
		}
		if (InputId == FName(TEXT("红色端子")))
		{
			return FLinearColor(1.0f, 0.18f, 0.10f, 0.96f);
		}
		if (InputId == FName(TEXT("黄色端子")))
		{
			return FLinearColor(1.0f, 0.86f, 0.22f, 0.96f);
		}

		return PanelAccent();
	}

	FLinearColor AccentForAdvancedMode(EHorrorCampaignInteractionMode Mode)
	{
		switch (Mode)
		{
		case EHorrorCampaignInteractionMode::CircuitWiring:
			return FLinearColor(0.22f, 0.72f, 1.0f, 0.96f);
		case EHorrorCampaignInteractionMode::GearCalibration:
			return FLinearColor(0.98f, 0.62f, 0.25f, 0.96f);
		case EHorrorCampaignInteractionMode::SpectralScan:
			return FLinearColor(0.48f, 0.88f, 1.0f, 0.96f);
		case EHorrorCampaignInteractionMode::SignalTuning:
			return FLinearColor(0.36f, 1.0f, 0.76f, 0.96f);
		default:
			return PanelAccent();
		}
	}

	const FHorrorAdvancedInteractionInputOption* FindAdvancedInputOption(const FHorrorAdvancedInteractionHUDState& State, FName InputId)
	{
		return State.InputOptions.FindByPredicate(
			[InputId](const FHorrorAdvancedInteractionInputOption& Option)
			{
				return Option.InputId == InputId;
			});
	}

	FLinearColor VisualColorForAdvancedInput(const FHorrorAdvancedInteractionHUDState& State, FName InputId)
	{
		if (const FHorrorAdvancedInteractionInputOption* Option = FindAdvancedInputOption(State, InputId))
		{
			return Option->VisualColor;
		}

		return CircuitColorForInput(InputId);
	}

	int32 IndexForAdvancedInput(FName InputId)
	{
		if (InputId == FName(TEXT("红色端子")) || InputId == FName(TEXT("齿轮2")))
		{
			return 1;
		}
		if (InputId == FName(TEXT("黄色端子")) || InputId == FName(TEXT("齿轮3")))
		{
			return 2;
		}

		return 0;
	}

	int32 IndexForAdvancedInputOption(const FHorrorAdvancedInteractionHUDState& State, FName InputId)
	{
		if (const FHorrorAdvancedInteractionInputOption* Option = FindAdvancedInputOption(State, InputId))
		{
			return Option->VisualSlotIndex;
		}

		return IndexForAdvancedInput(InputId);
	}

	FText BuildAdvancedInteractionOptionSummary(const FHorrorAdvancedInteractionHUDState& State)
	{
		if (State.InputOptions.IsEmpty())
		{
			return FText::GetEmpty();
		}

		FString Summary;
		for (const FHorrorAdvancedInteractionInputOption& Option : State.InputOptions)
		{
			if (Option.InputId.IsNone() || Option.DisplayLabel.IsEmpty() || Option.KeyHint.IsEmpty())
			{
				continue;
			}

			if (!Summary.IsEmpty())
			{
				Summary += TEXT("    ");
			}
			const FString ActionVerb = Option.ActionVerb.IsEmpty()
				? FString()
				: Option.ActionVerb.ToString();
			FString StateSuffix;
			if (Option.bHazardous)
			{
				StateSuffix = TEXT(" / 危险");
			}
			else if (Option.bStalled)
			{
				StateSuffix = TEXT(" / 停转");
			}
			else if (Option.bChainLinked)
			{
				StateSuffix = TEXT(" / 已咬合");
			}
			else if (Option.bRequiresStableSignal)
			{
				StateSuffix = TEXT(" / 需稳定");
			}
			Summary += FString::Printf(
				TEXT("%s：%s%s%s"),
				*Option.KeyHint.ToString(),
				*ActionVerb,
				*Option.DisplayLabel.ToString(),
				*StateSuffix);
		}
		return Summary.IsEmpty() ? FText::GetEmpty() : FText::FromString(Summary);
	}

	bool FeedbackLooksLikeFailure(const FText& FeedbackText)
	{
		const FString Feedback = FeedbackText.ToString();
		return Feedback.Contains(TEXT("红色火花"))
			|| Feedback.Contains(TEXT("卡死"))
			|| Feedback.Contains(TEXT("咔哒"))
			|| Feedback.Contains(TEXT("时机"));
	}

	FLinearColor AdvancedFeedbackColor(EHorrorAdvancedInteractionFeedbackState FeedbackState, const FText& FeedbackText, const FLinearColor& AccentColor)
	{
		switch (FeedbackState)
		{
			case EHorrorAdvancedInteractionFeedbackState::Success:
				return AccentColor;
			case EHorrorAdvancedInteractionFeedbackState::Failure:
			case EHorrorAdvancedInteractionFeedbackState::Overloaded:
				return FLinearColor(1.0f, 0.24f, 0.16f, 0.96f);
			case EHorrorAdvancedInteractionFeedbackState::Paused:
				return WarningAccent();
			case EHorrorAdvancedInteractionFeedbackState::Neutral:
			default:
				return FeedbackText.IsEmpty() ? TextSecondary() : AccentColor;
		}
	}

	bool AdvancedFeedbackIsFailure(EHorrorAdvancedInteractionFeedbackState FeedbackState, const FText& FeedbackText)
	{
		return FeedbackState == EHorrorAdvancedInteractionFeedbackState::Failure
			|| FeedbackState == EHorrorAdvancedInteractionFeedbackState::Overloaded
			|| (FeedbackState == EHorrorAdvancedInteractionFeedbackState::Neutral && FeedbackLooksLikeFailure(FeedbackText));
	}

	FText ChecklistRiskText(EHorrorCampaignObjectiveRiskLevel RiskLevel)
	{
		switch (RiskLevel)
		{
		case EHorrorCampaignObjectiveRiskLevel::Low:
			return NSLOCTEXT("Day1SliceHUD", "ChecklistRiskLow", "低风险");
		case EHorrorCampaignObjectiveRiskLevel::Medium:
			return NSLOCTEXT("Day1SliceHUD", "ChecklistRiskMedium", "警戒");
		case EHorrorCampaignObjectiveRiskLevel::High:
			return NSLOCTEXT("Day1SliceHUD", "ChecklistRiskHigh", "高危");
		case EHorrorCampaignObjectiveRiskLevel::Critical:
			return NSLOCTEXT("Day1SliceHUD", "ChecklistRiskCritical", "致命");
		case EHorrorCampaignObjectiveRiskLevel::None:
		default:
			return FText::GetEmpty();
		}
	}

	FLinearColor ChecklistRiskColor(EHorrorCampaignObjectiveRiskLevel RiskLevel, const FLinearColor& FallbackColor)
	{
		switch (RiskLevel)
		{
		case EHorrorCampaignObjectiveRiskLevel::Critical:
			return FLinearColor(1.0f, 0.08f, 0.06f, 0.96f);
		case EHorrorCampaignObjectiveRiskLevel::High:
			return FLinearColor(1.0f, 0.24f, 0.14f, 0.94f);
		case EHorrorCampaignObjectiveRiskLevel::Medium:
			return WarningAccent();
		case EHorrorCampaignObjectiveRiskLevel::Low:
			return FLinearColor(0.48f, 0.86f, 0.72f, 0.92f);
		case EHorrorCampaignObjectiveRiskLevel::None:
		default:
			return FallbackColor;
		}
	}

	FText ChecklistRouteText(const FHorrorObjectiveChecklistItem& Item)
	{
		if (Item.bOptional)
		{
			return NSLOCTEXT("Day1SliceHUD", "ChecklistOptionalRoute", "可选线索");
		}
		if (Item.bMainline)
		{
			return NSLOCTEXT("Day1SliceHUD", "ChecklistMainlineRoute", "主线");
		}
		return NSLOCTEXT("Day1SliceHUD", "ChecklistSideRoute", "旁支");
	}

	FText ChecklistRemainingTimeText(float RemainingSeconds)
	{
		if (RemainingSeconds <= KINDA_SMALL_NUMBER)
		{
			return FText::GetEmpty();
		}

		const int32 TotalSeconds = FMath::CeilToInt(RemainingSeconds);
		if (TotalSeconds >= 60)
		{
			return FText::Format(
				NSLOCTEXT("Day1SliceHUD", "ChecklistRemainingMinutes", "剩余 {0}分{1}秒"),
				FText::AsNumber(TotalSeconds / 60),
				FText::AsNumber(TotalSeconds % 60));
		}

		return FText::Format(
			NSLOCTEXT("Day1SliceHUD", "ChecklistRemainingSeconds", "剩余 {0}秒"),
			FText::AsNumber(TotalSeconds));
	}

	FText ChecklistDistanceText(float DistanceMeters)
	{
		if (DistanceMeters <= KINDA_SMALL_NUMBER)
		{
			return FText::GetEmpty();
		}

		if (DistanceMeters >= 1000.0f)
		{
			return FText::Format(
				NSLOCTEXT("Day1SliceHUD", "ChecklistDistanceKilometers", "约 {0} 公里"),
				FText::AsNumber(FMath::RoundToFloat(DistanceMeters / 100.0f) / 10.0f));
		}

		return FText::Format(
			NSLOCTEXT("Day1SliceHUD", "ChecklistDistanceMeters", "约 {0} 米"),
			FText::AsNumber(FMath::RoundToInt(DistanceMeters)));
	}

	FText ChecklistInteractionModeText(EHorrorCampaignInteractionMode InteractionMode)
	{
		switch (InteractionMode)
		{
			case EHorrorCampaignInteractionMode::CircuitWiring:
				return NSLOCTEXT("Day1SliceHUD", "ChecklistModeCircuit", "接线");
			case EHorrorCampaignInteractionMode::GearCalibration:
				return NSLOCTEXT("Day1SliceHUD", "ChecklistModeGear", "齿轮校准");
			case EHorrorCampaignInteractionMode::SpectralScan:
				return NSLOCTEXT("Day1SliceHUD", "ChecklistModeSpectral", "频谱扫描");
			case EHorrorCampaignInteractionMode::SignalTuning:
				return NSLOCTEXT("Day1SliceHUD", "ChecklistModeSignal", "信号调谐");
			case EHorrorCampaignInteractionMode::TimedPursuit:
				return NSLOCTEXT("Day1SliceHUD", "ChecklistModePursuit", "巨人追逐");
			case EHorrorCampaignInteractionMode::MultiStep:
				return NSLOCTEXT("Day1SliceHUD", "ChecklistModeMultiStep", "多段互动");
			case EHorrorCampaignInteractionMode::Instant:
			default:
				return NSLOCTEXT("Day1SliceHUD", "ChecklistModeInstant", "调查");
		}
	}

	FText ChecklistRuntimeStatusText(EHorrorCampaignObjectiveRuntimeStatus RuntimeStatus, bool bRetryable)
	{
		if (bRetryable && RuntimeStatus == EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable)
		{
			return NSLOCTEXT("Day1SliceHUD", "ChecklistStatusRetryable", "失败可重试");
		}

		switch (RuntimeStatus)
		{
			case EHorrorCampaignObjectiveRuntimeStatus::AdvancedInteractionActive:
				return NSLOCTEXT("Day1SliceHUD", "ChecklistStatusAdvancedActive", "交互中");
			case EHorrorCampaignObjectiveRuntimeStatus::TimedPursuitActive:
				return NSLOCTEXT("Day1SliceHUD", "ChecklistStatusPursuitActive", "追逐中");
			case EHorrorCampaignObjectiveRuntimeStatus::CarryingRelic:
				return NSLOCTEXT("Day1SliceHUD", "ChecklistStatusCarryingRelic", "搬运中");
			case EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable:
				return NSLOCTEXT("Day1SliceHUD", "ChecklistStatusFailedRetryable", "失败可重试");
			case EHorrorCampaignObjectiveRuntimeStatus::Completed:
				return NSLOCTEXT("Day1SliceHUD", "ChecklistStatusCompleted", "已完成");
			case EHorrorCampaignObjectiveRuntimeStatus::Locked:
				return NSLOCTEXT("Day1SliceHUD", "ChecklistStatusLocked", "锁定");
			case EHorrorCampaignObjectiveRuntimeStatus::Available:
				return NSLOCTEXT("Day1SliceHUD", "ChecklistStatusAvailable", "可执行");
			case EHorrorCampaignObjectiveRuntimeStatus::Hidden:
			default:
				return FText::GetEmpty();
		}
	}

	void AppendChecklistMetaPart(TArray<FString>& Parts, const FText& Text)
	{
		if (!Text.IsEmpty())
		{
			Parts.Add(Text.ToString());
		}
	}

	FText ChecklistMetadataText(const FHorrorObjectiveChecklistItem& Item)
	{
		TArray<FString> Parts;
		AppendChecklistMetaPart(Parts, ChecklistRouteText(Item));
		AppendChecklistMetaPart(Parts, ChecklistRiskText(Item.RiskLevel));
		AppendChecklistMetaPart(Parts, ChecklistRemainingTimeText(Item.RemainingSeconds));
		// 恢复原始逻辑：所有任务都显示距离，避免高度计算错误
		AppendChecklistMetaPart(Parts, ChecklistDistanceText(Item.DistanceMeters));
		if (!Item.DeviceStatusLabel.IsEmpty())
		{
			Parts.Add(Item.DeviceStatusLabel.ToString());
		}
		if (Item.PerformanceGradeFraction > KINDA_SMALL_NUMBER)
		{
			Parts.Add(FText::Format(
				NSLOCTEXT("Day1SliceHUD", "ChecklistPerformanceGrade", "评分 {0}/10"),
				FText::AsNumber(FMath::RoundToInt(FMath::Clamp(Item.PerformanceGradeFraction, 0.0f, 1.0f) * 10.0f))).ToString());
		}
		if (Item.InputPrecisionFraction > KINDA_SMALL_NUMBER)
		{
			Parts.Add(FText::Format(
				NSLOCTEXT("Day1SliceHUD", "ChecklistInputPrecision", "精度 {0}%"),
				FText::AsNumber(FMath::RoundToInt(FMath::Clamp(Item.InputPrecisionFraction, 0.0f, 1.0f) * 100.0f))).ToString());
		}
		if (Item.DeviceLoadFraction > KINDA_SMALL_NUMBER)
		{
			Parts.Add(FText::Format(
				NSLOCTEXT("Day1SliceHUD", "ChecklistDeviceLoad", "负载 {0}%"),
				FText::AsNumber(FMath::RoundToInt(FMath::Clamp(Item.DeviceLoadFraction, 0.0f, 1.0f) * 100.0f))).ToString());
		}
		if (Item.bOpensInteractionPanel)
		{
			Parts.Add(NSLOCTEXT("Day1SliceHUD", "ChecklistOpensPanel", "打开交互面板").ToString());
		}

		return Parts.IsEmpty() ? FText::GetEmpty() : FText::FromString(FString::Join(Parts, TEXT("  |  ")));
	}

	FText ChecklistRewardText(const FHorrorObjectiveChecklistItem& Item)
	{
		if (Item.RewardText.IsEmpty())
		{
			return FText::GetEmpty();
		}

		return FText::Format(
			NSLOCTEXT("Day1SliceHUD", "ChecklistRewardFormat", "完成奖励：{0}"),
			Item.RewardText);
	}

	bool ChecklistHasTelemetryRail(const FHorrorObjectiveChecklistItem& Item)
	{
		return Item.bActive
			&& (Item.RuntimeProgressFraction > KINDA_SMALL_NUMBER
				|| Item.PerformanceGradeFraction > KINDA_SMALL_NUMBER
				|| Item.InputPrecisionFraction > KINDA_SMALL_NUMBER
				|| Item.DeviceLoadFraction > KINDA_SMALL_NUMBER);
	}

	float ChecklistItemHeight(const FHorrorObjectiveChecklistItem& Item)
	{
		const bool bHasDetail = !Item.LockReason.IsEmpty()
			|| !Item.Detail.IsEmpty()
			|| !Item.InteractionLabel.IsEmpty()
			|| !Item.StatusText.IsEmpty()
			|| !Item.NextActionLabel.IsEmpty()
			|| !Item.FailureRecoveryLabel.IsEmpty();
		const bool bHasTacticalLabel = !Item.TacticalLabel.IsEmpty();
		const bool bHasInstrumentLine = !Item.bComplete && !Item.bBlocked && !Item.bOptional;
		const bool bHasMetadata = !ChecklistMetadataText(Item).IsEmpty();
		const bool bHasReward = !ChecklistRewardText(Item).IsEmpty();
		return 32.0f
			+ (bHasDetail ? 18.0f : 0.0f)
			+ (bHasInstrumentLine ? 18.0f : 0.0f)
			+ (bHasTacticalLabel ? 18.0f : 0.0f)
			+ (bHasMetadata ? 18.0f : 0.0f)
			+ (bHasReward ? 18.0f : 0.0f)
			+ (ChecklistHasTelemetryRail(Item) ? 14.0f : 0.0f);
	}

	FLinearColor ChecklistStateColor(const FHorrorObjectiveChecklistItem& Item, const FLinearColor& AccentColor)
	{
		if (Item.bActive)
		{
			return Item.bRetryable ? WarningAccent() : AccentColor;
		}
		if (Item.bBlocked)
		{
			return WarningAccent();
		}
		if (Item.bComplete)
		{
			return FLinearColor(0.28f, 0.72f, 0.58f, 0.88f);
		}
		return FLinearColor(0.26f, 0.34f, 0.34f, 0.86f);
	}

	float AdvancedWindowStart(const FHorrorAdvancedInteractionHUDState& State)
	{
		return FMath::Clamp(State.TimingWindowStartFraction, 0.0f, 1.0f);
	}

	float AdvancedWindowEnd(const FHorrorAdvancedInteractionHUDState& State)
	{
		return FMath::Clamp(FMath::Max(State.TimingWindowEndFraction, State.TimingWindowStartFraction), 0.0f, 1.0f);
	}

	FLinearColor StabilityColor(float StabilityFraction)
	{
		if (StabilityFraction < 0.35f)
		{
			return FLinearColor(1.0f, 0.22f, 0.12f, 0.96f);
		}
		if (StabilityFraction < 0.68f)
		{
			return WarningAccent();
		}

		return FLinearColor(0.30f, 0.95f, 0.64f, 0.94f);
	}

	FLinearColor PerformanceGradeColor(float GradeFraction)
	{
		if (GradeFraction < 0.35f)
		{
			return FLinearColor(1.0f, 0.22f, 0.12f, 0.96f);
		}
		if (GradeFraction < 0.70f)
		{
			return WarningAccent();
		}

		return FLinearColor(0.30f, 0.95f, 0.72f, 0.94f);
	}

	FLinearColor DeviceLoadColor(float LoadFraction)
	{
		if (LoadFraction > 0.72f)
		{
			return FLinearColor(1.0f, 0.14f, 0.10f, 0.96f);
		}
		if (LoadFraction > 0.42f)
		{
			return WarningAccent();
		}

		return FLinearColor(0.34f, 0.86f, 0.76f, 0.92f);
	}

	FLinearColor ObjectiveToastAccent(EHorrorObjectiveFeedbackSeverity Severity)
	{
		switch (Severity)
		{
		case EHorrorObjectiveFeedbackSeverity::Critical:
			return FLinearColor(1.0f, 0.08f, 0.06f, 0.98f);
		case EHorrorObjectiveFeedbackSeverity::Failure:
			return FLinearColor(1.0f, 0.24f, 0.14f, 0.96f);
		case EHorrorObjectiveFeedbackSeverity::Warning:
			return WarningAccent();
		case EHorrorObjectiveFeedbackSeverity::Success:
			return FLinearColor(0.30f, 0.95f, 0.64f, 0.94f);
		case EHorrorObjectiveFeedbackSeverity::Info:
		default:
			return PanelAccent();
		}
	}

	FText InteractionVerbText(EHorrorInteractionVerb Verb)
	{
		switch (Verb)
		{
		case EHorrorInteractionVerb::Open:
			return NSLOCTEXT("Day1SliceHUD", "InteractionVerbOpen", "打开");
		case EHorrorInteractionVerb::Close:
			return NSLOCTEXT("Day1SliceHUD", "InteractionVerbClose", "关闭");
		case EHorrorInteractionVerb::EnterCode:
			return NSLOCTEXT("Day1SliceHUD", "InteractionVerbEnterCode", "门禁");
		case EHorrorInteractionVerb::Record:
			return NSLOCTEXT("Day1SliceHUD", "InteractionVerbRecord", "录像");
		case EHorrorInteractionVerb::Review:
			return NSLOCTEXT("Day1SliceHUD", "InteractionVerbReview", "复查");
		case EHorrorInteractionVerb::Repair:
			return NSLOCTEXT("Day1SliceHUD", "InteractionVerbRepair", "修复");
		case EHorrorInteractionVerb::Survive:
			return NSLOCTEXT("Day1SliceHUD", "InteractionVerbSurvive", "生存");
		case EHorrorInteractionVerb::Interact:
		default:
			return NSLOCTEXT("Day1SliceHUD", "InteractionVerbInteract", "互动");
		}
	}

	FText InteractionInputStyleText(EHorrorInteractionInputStyle InputStyle)
	{
		switch (InputStyle)
		{
		case EHorrorInteractionInputStyle::Hold:
			return NSLOCTEXT("Day1SliceHUD", "InteractionInputHold", "长按");
		case EHorrorInteractionInputStyle::Modal:
			return NSLOCTEXT("Day1SliceHUD", "InteractionInputModal", "窗口");
		case EHorrorInteractionInputStyle::RecordAim:
			return NSLOCTEXT("Day1SliceHUD", "InteractionInputRecordAim", "录像瞄准");
		case EHorrorInteractionInputStyle::Timed:
			return NSLOCTEXT("Day1SliceHUD", "InteractionInputTimed", "限时");
		case EHorrorInteractionInputStyle::Press:
		default:
			return NSLOCTEXT("Day1SliceHUD", "InteractionInputPress", "按下");
		}
	}

	FText InteractionRiskText(EHorrorInteractionRiskLevel RiskLevel)
	{
		switch (RiskLevel)
		{
		case EHorrorInteractionRiskLevel::Low:
			return NSLOCTEXT("Day1SliceHUD", "InteractionRiskLow", "低风险");
		case EHorrorInteractionRiskLevel::Medium:
			return NSLOCTEXT("Day1SliceHUD", "InteractionRiskMedium", "警戒");
		case EHorrorInteractionRiskLevel::High:
			return NSLOCTEXT("Day1SliceHUD", "InteractionRiskHigh", "高危");
		case EHorrorInteractionRiskLevel::None:
		default:
			return FText::GetEmpty();
		}
	}

	FLinearColor InteractionRiskColor(EHorrorInteractionRiskLevel RiskLevel, bool bCanInteract)
	{
		if (!bCanInteract)
		{
			return WarningAccent();
		}

		switch (RiskLevel)
		{
		case EHorrorInteractionRiskLevel::High:
			return FLinearColor(1.0f, 0.24f, 0.14f, 0.96f);
		case EHorrorInteractionRiskLevel::Medium:
			return WarningAccent();
		case EHorrorInteractionRiskLevel::Low:
		case EHorrorInteractionRiskLevel::None:
		default:
			return PanelAccent();
		}
	}
}

void ADay1SliceHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	const float CanvasWidth = Canvas->SizeX;
	const float CanvasHeight = Canvas->SizeY;

	DrawCurrentObjective(CanvasWidth);
	DrawSurvivalStatus(CanvasWidth);
	DrawAnomalyCaptureStatus(CanvasWidth, CanvasHeight);
	DrawAutosaveIndicator(CanvasWidth);
	DrawObjectiveToast(CanvasWidth);
	DrawAdvancedInteractionPanel(CanvasWidth, CanvasHeight);
	DrawInteractionPrompt(CanvasWidth, CanvasHeight);
	DrawPasswordPanel(CanvasWidth, CanvasHeight);
	DrawTransientMessage(CanvasWidth, CanvasHeight);
	DrawPauseMenu(CanvasWidth, CanvasHeight);
	DrawNotesJournal(CanvasWidth, CanvasHeight);
	DrawDay1CompletionOverlay(CanvasWidth, CanvasHeight);
}

void ADay1SliceHUD::SetInteractionPrompt(const FText& PromptText)
{
	InteractionPrompt = PromptText;
	InteractionContext = FHorrorInteractionContext();
	InteractionContext.bVisible = !PromptText.IsEmpty();
	InteractionContext.bCanInteract = !PromptText.IsEmpty();
	InteractionContext.ActionText = PromptText;
	InteractionContext.InputText = NSLOCTEXT("Day1SliceHUD", "FallbackInteractionInput", "互动键");
}

void ADay1SliceHUD::SetInteractionContext(const FHorrorInteractionContext& Context)
{
	InteractionContext = Context;
	InteractionContext.ProgressFraction = FMath::Clamp(Context.ProgressFraction, 0.0f, 1.0f);
	InteractionPrompt = !InteractionContext.BlockedReason.IsEmpty()
		? InteractionContext.BlockedReason
		: InteractionContext.ActionText;
}

void ADay1SliceHUD::ClearInteractionPrompt()
{
	InteractionPrompt = FText::GetEmpty();
	InteractionContext = FHorrorInteractionContext();
}

void ADay1SliceHUD::SetCurrentObjective(const FText& ObjectiveText)
{
	CurrentObjective = ObjectiveText;
}

void ADay1SliceHUD::SetObjectiveTracker(const FHorrorObjectiveTrackerSnapshot& Snapshot)
{
	ObjectiveTrackerTitle = Snapshot.Title;
	CurrentObjective = Snapshot.PrimaryInstruction;
	ObjectiveTrackerDetail = Snapshot.SecondaryInstruction;
	ObjectiveTrackerProgressLabel = Snapshot.ProgressLabel;
	ObjectiveTrackerInteractionLabel = Snapshot.InteractionLabel;
	ObjectiveTrackerMissionContext = Snapshot.MissionContext;
	ObjectiveTrackerFailureStakes = Snapshot.FailureStakes;
	ObjectiveTrackerActiveChapterId = Snapshot.ActiveChapterId;
	ObjectiveTrackerActiveObjectiveId = Snapshot.ActiveObjectiveId;
	ObjectiveChecklistItems = Snapshot.ChecklistItems;
	ObjectiveGraphItems.Reset();
	constexpr int32 MaxVisibleObjectiveGraphItems = 4;
	ObjectiveGraphOverflowCount = FMath::Max(0, Snapshot.ObjectiveGraphItems.Num() - MaxVisibleObjectiveGraphItems);
	ObjectiveGraphOverflowText = ObjectiveGraphOverflowCount > 0
		? FText::Format(
			NSLOCTEXT("Day1SliceHUD", "ObjectiveGraphOverflowText", "还有 {0} 个任务节点，按 [ / ] 切换导航焦点查看"),
			FText::AsNumber(ObjectiveGraphOverflowCount))
		: FText::GetEmpty();
	TArray<FHorrorObjectiveChecklistItem> PrioritizedGraphItems = Snapshot.ObjectiveGraphItems;
	PrioritizedGraphItems.StableSort(
		[](const FHorrorObjectiveChecklistItem& Left, const FHorrorObjectiveChecklistItem& Right)
		{
			auto PriorityForGraphItem = [](const FHorrorObjectiveChecklistItem& Item)
			{
				if (Item.bActive)
				{
					return 0;
				}
				if (Item.bNavigationFocused)
				{
					return 1;
				}
				if (Item.bMainline && !Item.bBlocked && !Item.bComplete)
				{
					return 2;
				}
				if (Item.bBlocked && !Item.LockReason.IsEmpty())
				{
					return 3;
				}
				if (Item.bOptional && !Item.bComplete)
				{
					return 4;
				}
				if (Item.bComplete)
				{
					return 5;
				}
				return 6;
			};

			return PriorityForGraphItem(Left) < PriorityForGraphItem(Right);
		});
	for (const FHorrorObjectiveChecklistItem& GraphItem : PrioritizedGraphItems)
	{
		if (ObjectiveGraphItems.Num() >= MaxVisibleObjectiveGraphItems)
		{
			break;
		}
		ObjectiveGraphItems.Add(GraphItem);
	}
	if (Snapshot.ObjectiveGraphItems.Num() > MaxVisibleObjectiveGraphItems)
	{
		const FHorrorObjectiveChecklistItem* FocusedOverflowItem = nullptr;
		for (int32 GraphItemIndex = Snapshot.ObjectiveGraphItems.Num() - 1; GraphItemIndex >= 0; --GraphItemIndex)
		{
			const FHorrorObjectiveChecklistItem& CandidateItem = Snapshot.ObjectiveGraphItems[GraphItemIndex];
			if (CandidateItem.bNavigationFocused
				&& !ObjectiveGraphItems.ContainsByPredicate(
					[&CandidateItem](const FHorrorObjectiveChecklistItem& VisibleItem)
					{
						return VisibleItem.ObjectiveId == CandidateItem.ObjectiveId;
					}))
			{
				FocusedOverflowItem = &CandidateItem;
				break;
			}
		}
		if (FocusedOverflowItem && ObjectiveGraphItems.Num() == MaxVisibleObjectiveGraphItems)
		{
			ObjectiveGraphItems[MaxVisibleObjectiveGraphItems - 1] = *FocusedOverflowItem;
		}
	}
	ObjectiveTrackerProgressFraction = FMath::Clamp(Snapshot.ProgressFraction, 0.0f, 1.0f);
	bObjectiveTrackerUrgent = Snapshot.bUrgent;
	bObjectiveTrackerRequiresRecording = Snapshot.bRequiresRecording;
	bObjectiveTrackerOpensInteractionPanel = Snapshot.bOpensInteractionPanel;
}

void ADay1SliceHUD::SetObjectiveNavigation(const FText& NavigationText)
{
	ObjectiveNavigation = NavigationText;
	ObjectiveNavigationState = FHorrorObjectiveNavigationState();
	ObjectiveNavigationState.bVisible = !NavigationText.IsEmpty();
	ObjectiveNavigationState.StatusText = NavigationText;
}

void ADay1SliceHUD::SetObjectiveNavigationState(const FHorrorObjectiveNavigationState& NavigationState)
{
	ObjectiveNavigationState = NavigationState;
	ObjectiveNavigationState.DistanceMeters = FMath::Max(0.0f, NavigationState.DistanceMeters);
	ObjectiveNavigationState.AngleDegrees = FMath::Clamp(NavigationState.AngleDegrees, -180.0f, 180.0f);
	ObjectiveNavigationState.PerformanceGradeFraction = FMath::Clamp(NavigationState.PerformanceGradeFraction, 0.0f, 1.0f);
	ObjectiveNavigationState.EstimatedEscapeArrivalSeconds = FMath::Max(0.0f, NavigationState.EstimatedEscapeArrivalSeconds);
	ObjectiveNavigationState.bVisible = NavigationState.bVisible;
	ObjectiveNavigation = !ObjectiveNavigationState.StatusText.IsEmpty()
		? ObjectiveNavigationState.StatusText
		: ObjectiveNavigationState.DirectionText;
}

void ADay1SliceHUD::ClearObjectiveNavigation()
{
	ObjectiveNavigation = FText::GetEmpty();
	ObjectiveNavigationState = FHorrorObjectiveNavigationState();
}

void ADay1SliceHUD::ShowObjectiveToast(
	const FText& TitleText,
	const FText& HintText,
	float DisplaySeconds,
	EHorrorObjectiveFeedbackSeverity Severity)
{
	ToastTitle = TitleText;
	ToastHint = HintText;
	ToastSeverity = Severity;
	ToastExpireWorldSeconds = GetWorldSeconds() + FMath::Max(0.1f, DisplaySeconds);

	if (!HintText.IsEmpty())
	{
		SetCurrentObjective(HintText);
	}
	else if (!TitleText.IsEmpty())
	{
		SetCurrentObjective(TitleText);
	}
}

void ADay1SliceHUD::SetAnomalyCaptureStatus(const FText& StatusText, float ProgressFraction, bool bLocked, bool bRequiresRecording)
{
	AnomalyCaptureStatus = StatusText;
	AnomalyCaptureProgressFraction = FMath::Clamp(ProgressFraction, 0.0f, 1.0f);
	bAnomalyCaptureLocked = bLocked;
	bAnomalyCaptureRequiresRecording = bRequiresRecording;
	bAnomalyCaptureStatusVisible = !AnomalyCaptureStatus.IsEmpty();
}

void ADay1SliceHUD::ClearAnomalyCaptureStatus()
{
	AnomalyCaptureStatus = FText::GetEmpty();
	AnomalyCaptureProgressFraction = 0.0f;
	bAnomalyCaptureLocked = false;
	bAnomalyCaptureRequiresRecording = false;
	bAnomalyCaptureStatusVisible = false;
}

void ADay1SliceHUD::ShowTransientMessage(const FText& MessageText, const FLinearColor& MessageColor, float DisplaySeconds)
{
	TransientMessage = MessageText;
	TransientMessageColor = MessageColor;
	TransientMessageExpireWorldSeconds = GetWorldSeconds() + FMath::Max(0.1f, DisplaySeconds);
}

void ADay1SliceHUD::ShowAutosaveIndicator(const FText& IndicatorText, float DisplaySeconds)
{
	AutosaveIndicatorText = IndicatorText.IsEmpty()
		? NSLOCTEXT("Day1SliceHUD", "CheckpointSaved", "检查点已保存。")
		: IndicatorText;
	AutosaveIndicatorExpireWorldSeconds = GetWorldSeconds() + FMath::Max(0.1f, DisplaySeconds);
}

void ADay1SliceHUD::ShowDay1CompletionOverlay(const FText& TitleText, const FText& HintText)
{
	Day1CompletionTitle = TitleText.IsEmpty()
		? NSLOCTEXT("Day1SliceHUD", "Day1Complete", "第一天完成")
		: TitleText;
	Day1CompletionHint = HintText.IsEmpty()
		? NSLOCTEXT("Day1SliceHUD", "EvidencePreserved", "证据已保存。")
		: HintText;
	bDay1CompletionOverlayVisible = true;
}

void ADay1SliceHUD::ClearDay1CompletionOverlay()
{
	Day1CompletionTitle = FText::GetEmpty();
	Day1CompletionHint = FText::GetEmpty();
	bDay1CompletionOverlayVisible = false;
}

void ADay1SliceHUD::SetSurvivalStatus(bool bBodycamAcquired, bool bBodycamEnabled, bool bRecording, float FearPercent, float SprintPercent, bool bDanger)
{
	bStatusBodycamAcquired = bBodycamAcquired;
	bStatusBodycamEnabled = bBodycamEnabled;
	bStatusRecording = bRecording;
	StatusFearPercent = FMath::Clamp(FearPercent, 0.0f, 1.0f);
	StatusSprintPercent = FMath::Clamp(SprintPercent, 0.0f, 1.0f);
	bStatusDanger = bDanger;
}

void ADay1SliceHUD::SetBodycamBatteryStatus(float BatteryPercent, bool bLowBattery)
{
	BodycamBatteryPercent = FMath::Clamp(BatteryPercent, 0.0f, 100.0f);
	bBodycamBatteryLow = bLowBattery;
	bBodycamBatteryVisible = true;
}

void ADay1SliceHUD::ClearBodycamBatteryStatus()
{
	BodycamBatteryPercent = 100.0f;
	bBodycamBatteryLow = false;
	bBodycamBatteryVisible = false;
}

void ADay1SliceHUD::ShowNotesJournal(const TArray<FHorrorNoteMetadata>& RecordedNotes)
{
	NotesJournalEntries = RecordedNotes;
	NotesJournalEmptyText = NSLOCTEXT("Day1SliceHUD", "NoNotesRecorded", "还没有记录任何笔记。");
	bNotesJournalVisible = true;
}

void ADay1SliceHUD::ClearNotesJournal()
{
	NotesJournalEntries.Reset();
	bNotesJournalVisible = false;
}

void ADay1SliceHUD::ShowPasswordPrompt(const FText& DoorName, const FText& HintText, int32 EnteredDigits, int32 RequiredDigits)
{
	PasswordDoorName = DoorName;
	PasswordHint = HintText;
	PasswordEnteredDigits = FMath::Max(0, EnteredDigits);
	PasswordRequiredDigits = FMath::Max(0, RequiredDigits);
	bPasswordPromptVisible = true;
}

void ADay1SliceHUD::ClearPasswordPrompt()
{
	PasswordDoorName = FText::GetEmpty();
	PasswordHint = FText::GetEmpty();
	PasswordEnteredDigits = 0;
	PasswordRequiredDigits = 0;
	bPasswordPromptVisible = false;
}

void ADay1SliceHUD::ShowPauseMenu(EDay1PauseMenuSelection Selection, float MouseSensitivity, float MasterVolume, float Brightness)
{
	PauseMenuSelection = Selection;
	PauseMouseSensitivity = FMath::Clamp(MouseSensitivity, 0.1f, 10.0f);
	PauseMasterVolume = FMath::Clamp(MasterVolume, 0.0f, 1.0f);
	PauseBrightness = FMath::Clamp(Brightness, 0.2f, 2.0f);
	bPauseMenuVisible = true;
}

void ADay1SliceHUD::ClearPauseMenu()
{
	bPauseMenuVisible = false;
	PauseMenuSelection = EDay1PauseMenuSelection::Resume;
}

void ADay1SliceHUD::SetAdvancedInteractionState(const FHorrorAdvancedInteractionHUDState& State)
{
	FHorrorAdvancedInteractionHUDState NormalizedState = State;
	NormalizeHorrorAdvancedInteractionHUDState(NormalizedState);
	if (State.bVisible && IsHorrorAdvancedInteractionPanelMode(State.Mode) && !NormalizedState.bVisible)
	{
		AdvancedInteractionState = FHorrorAdvancedInteractionHUDState();
		ShowTransientMessage(
			NSLOCTEXT("Day1SliceHUD", "AdvancedInteractionIncompleteRecovery", "交互面板数据不完整，已切回任务提示。"),
			WarningAccent());
		return;
	}

	AdvancedInteractionState = NormalizedState;
	if (AdvancedInteractionState.bVisible)
	{
		ClearInteractionPrompt();
	}
}

void ADay1SliceHUD::ClearAdvancedInteractionState()
{
	AdvancedInteractionState = FHorrorAdvancedInteractionHUDState();
}

float ADay1SliceHUD::ComputeObjectiveTrackerPanelHeight() const
{
	if (CurrentObjective.IsEmpty())
	{
		return 0.0f;
	}

	const bool bHasTitle = !ObjectiveTrackerTitle.IsEmpty();
	const bool bHasDetail = !ObjectiveTrackerDetail.IsEmpty();
	const bool bHasNavigation = !ObjectiveNavigation.IsEmpty();
	const bool bHasNavigationDiagnostics = !BuildObjectiveNavigationDiagnosticText().IsEmpty();
	const bool bHasProgress = !ObjectiveTrackerProgressLabel.IsEmpty();
	const bool bHasMissionBrief = !ObjectiveTrackerInteractionLabel.IsEmpty()
		|| !ObjectiveTrackerMissionContext.IsEmpty()
		|| !ObjectiveTrackerFailureStakes.IsEmpty();
	const int32 VisibleChecklistCount = FMath::Min(ObjectiveChecklistItems.Num(), 6);
	float ChecklistHeight = 0.0f;
	for (int32 ItemIndex = 0; ItemIndex < VisibleChecklistCount; ++ItemIndex)
	{
		ChecklistHeight += ChecklistItemHeight(ObjectiveChecklistItems[ItemIndex]);
	}

	const int32 VisibleGraphCount = FMath::Min(ObjectiveGraphItems.Num(), 4);
	const float ObjectiveGraphHeight = VisibleGraphCount > 0
		? 22.0f + static_cast<float>(VisibleGraphCount) * 36.0f + (ObjectiveGraphOverflowText.IsEmpty() ? 6.0f : 24.0f)
		: 0.0f;

	return
		92.0f
		+ (bHasDetail ? 28.0f : 0.0f)
		+ (bHasNavigation ? 28.0f : 0.0f)
		+ (bHasNavigationDiagnostics ? 34.0f : 0.0f)
		+ (bHasProgress ? 28.0f : 0.0f)
		+ (bHasMissionBrief ? 46.0f : 0.0f)
		+ ObjectiveGraphHeight
		+ (VisibleChecklistCount > 0 ? 18.0f + ChecklistHeight : 0.0f);
}

FText ADay1SliceHUD::BuildObjectiveNavigationDiagnosticText() const
{
	TArray<FString> Parts;
	if (!ObjectiveNavigationState.DeviceStatusLabel.IsEmpty())
	{
		Parts.Add(ObjectiveNavigationState.DeviceStatusLabel.ToString());
	}
	if (!ObjectiveNavigationState.NextActionLabel.IsEmpty())
	{
		Parts.Add(ObjectiveNavigationState.NextActionLabel.ToString());
	}
	if (!ObjectiveNavigationState.FailureRecoveryLabel.IsEmpty()
		&& (ObjectiveNavigationState.bRetryable || ObjectiveNavigationState.RuntimeStatus == EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable))
	{
		Parts.Add(ObjectiveNavigationState.FailureRecoveryLabel.ToString());
	}

	return Parts.IsEmpty()
		? FText::GetEmpty()
		: FText::FromString(FString::Join(Parts, TEXT("  |  ")));
}

FText ADay1SliceHUD::BuildObjectiveNavigationRouteText() const
{
	if (!ObjectiveNavigationState.bVisible && ObjectiveNavigation.IsEmpty())
	{
		return FText::GetEmpty();
	}

	TArray<FString> Parts;
	if (!ObjectiveNavigationState.Label.IsEmpty())
	{
		Parts.Add(ObjectiveNavigationState.Label.ToString());
	}

	// 合并方向和距离显示，不显示角度
	if (!ObjectiveNavigationState.DirectionText.IsEmpty() && ObjectiveNavigationState.DistanceMeters > KINDA_SMALL_NUMBER)
	{
		if (ObjectiveNavigationState.DistanceMeters >= 1000.0f)
		{
			Parts.Add(FText::Format(
				NSLOCTEXT("Day1SliceHUD", "NavigationRouteDirectionDistanceKm", "{0} {1} 公里"),
				ObjectiveNavigationState.DirectionText,
				FText::AsNumber(FMath::RoundToFloat(ObjectiveNavigationState.DistanceMeters / 100.0f) / 10.0f)).ToString());
		}
		else
		{
			Parts.Add(FText::Format(
				NSLOCTEXT("Day1SliceHUD", "NavigationRouteDirectionDistanceM", "{0} {1} 米"),
				ObjectiveNavigationState.DirectionText,
				FText::AsNumber(FMath::RoundToInt(ObjectiveNavigationState.DistanceMeters))).ToString());
		}
	}
	else if (!ObjectiveNavigationState.DirectionText.IsEmpty())
	{
		Parts.Add(ObjectiveNavigationState.DirectionText.ToString());
	}

	if (ObjectiveNavigationState.bArrived)
	{
		Parts.Add(NSLOCTEXT("Day1SliceHUD", "NavigationRouteArrived", "已到达").ToString());
	}
	else if (!ObjectiveNavigationState.bReachable)
	{
		Parts.Add(NSLOCTEXT("Day1SliceHUD", "NavigationRouteBlocked", "不可达").ToString());
	}
	else if (ObjectiveNavigationState.bRetryable || ObjectiveNavigationState.RuntimeStatus == EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable)
	{
		Parts.Add(NSLOCTEXT("Day1SliceHUD", "NavigationRouteRetryable", "可重试").ToString());
	}
	else if (ObjectiveNavigationState.RuntimeStatus == EHorrorCampaignObjectiveRuntimeStatus::TimedPursuitActive)
	{
		Parts.Add(NSLOCTEXT("Day1SliceHUD", "NavigationRoutePursuit", "追逐中").ToString());
	}
	else if (ObjectiveNavigationState.RuntimeStatus == EHorrorCampaignObjectiveRuntimeStatus::AdvancedInteractionActive)
	{
		Parts.Add(NSLOCTEXT("Day1SliceHUD", "NavigationRouteInteraction", "交互中").ToString());
	}
	else if (ObjectiveNavigationState.RuntimeStatus == EHorrorCampaignObjectiveRuntimeStatus::Completed)
	{
		Parts.Add(NSLOCTEXT("Day1SliceHUD", "NavigationRouteComplete", "已完成").ToString());
	}
	else
	{
		Parts.Add(NSLOCTEXT("Day1SliceHUD", "NavigationRouteReachable", "可抵达").ToString());
	}

	if (!ObjectiveNavigationState.NextActionLabel.IsEmpty())
	{
		Parts.Add(ObjectiveNavigationState.NextActionLabel.ToString());
	}

	return Parts.IsEmpty()
		? FText::GetEmpty()
		: FText::FromString(FString::Join(Parts, TEXT("  |  ")));
}

FText ADay1SliceHUD::BuildObjectiveChecklistActionLine(const FHorrorObjectiveChecklistItem& Item) const
{
	FText ItemDetail = Item.bBlocked && !Item.LockReason.IsEmpty()
		? Item.LockReason
		: (!Item.StatusText.IsEmpty() ? Item.StatusText : (!Item.Detail.IsEmpty() ? Item.Detail : Item.InteractionLabel));
	if (Item.bActive && !Item.StatusText.IsEmpty() && !Item.Detail.IsEmpty())
	{
		ItemDetail = FText::Format(
			NSLOCTEXT("Day1SliceHUD", "ChecklistDetailWithStatus", "{0}  |  {1}"),
			Item.Detail,
			Item.StatusText);
	}
	if (!Item.NextActionLabel.IsEmpty())
	{
		ItemDetail = ItemDetail.IsEmpty()
			? Item.NextActionLabel
			: FText::Format(
				NSLOCTEXT("Day1SliceHUD", "ChecklistDetailWithNextAction", "{0}  |  {1}"),
				ItemDetail,
				Item.NextActionLabel);
	}
	if (!Item.FailureRecoveryLabel.IsEmpty() && (Item.bRetryable || Item.RuntimeStatus == EHorrorCampaignObjectiveRuntimeStatus::AdvancedInteractionActive))
	{
		ItemDetail = ItemDetail.IsEmpty()
			? Item.FailureRecoveryLabel
			: FText::Format(
				NSLOCTEXT("Day1SliceHUD", "ChecklistDetailWithRecovery", "{0}  |  {1}"),
				ItemDetail,
				Item.FailureRecoveryLabel);
	}

	return ItemDetail;
}

FText ADay1SliceHUD::BuildObjectiveChecklistInstrumentLine(const FHorrorObjectiveChecklistItem& Item) const
{
	TArray<FString> Parts;
	AppendChecklistMetaPart(Parts, ChecklistInteractionModeText(Item.InteractionMode));
	AppendChecklistMetaPart(Parts, ChecklistRuntimeStatusText(Item.RuntimeStatus, Item.bRetryable));

	if (Item.RuntimeProgressFraction > KINDA_SMALL_NUMBER)
	{
		Parts.Add(FText::Format(
			NSLOCTEXT("Day1SliceHUD", "ChecklistInstrumentProgress", "进度 {0}%"),
			FText::AsNumber(FMath::RoundToInt(FMath::Clamp(Item.RuntimeProgressFraction, 0.0f, 1.0f) * 100.0f))).ToString());
	}
	if (Item.PerformanceGradeFraction > KINDA_SMALL_NUMBER)
	{
		Parts.Add(FText::Format(
			NSLOCTEXT("Day1SliceHUD", "ChecklistInstrumentGrade", "评分 {0}/10"),
			FText::AsNumber(FMath::RoundToInt(FMath::Clamp(Item.PerformanceGradeFraction, 0.0f, 1.0f) * 10.0f))).ToString());
	}
	if (Item.InputPrecisionFraction > KINDA_SMALL_NUMBER)
	{
		Parts.Add(FText::Format(
			NSLOCTEXT("Day1SliceHUD", "ChecklistInstrumentPrecision", "精度 {0}%"),
			FText::AsNumber(FMath::RoundToInt(FMath::Clamp(Item.InputPrecisionFraction, 0.0f, 1.0f) * 100.0f))).ToString());
	}
	if (Item.DeviceLoadFraction > KINDA_SMALL_NUMBER)
	{
		Parts.Add(FText::Format(
			NSLOCTEXT("Day1SliceHUD", "ChecklistInstrumentLoad", "负载 {0}%"),
			FText::AsNumber(FMath::RoundToInt(FMath::Clamp(Item.DeviceLoadFraction, 0.0f, 1.0f) * 100.0f))).ToString());
	}
	// 移除这里的距离显示，避免与元数据行重复
	if (Item.RemainingSeconds > KINDA_SMALL_NUMBER)
	{
		Parts.Add(ChecklistRemainingTimeText(Item.RemainingSeconds).ToString());
	}

	return Parts.IsEmpty()
		? FText::GetEmpty()
		: FText::FromString(FString::Join(Parts, TEXT("  |  ")));
}

float ADay1SliceHUD::ComputeObjectiveChecklistTelemetryFraction(const FHorrorObjectiveChecklistItem& Item) const
{
	if (!ChecklistHasTelemetryRail(Item))
	{
		return 0.0f;
	}

	return FMath::Clamp(
		FMath::Max(
			FMath::Max(Item.RuntimeProgressFraction, Item.PerformanceGradeFraction),
			FMath::Max(Item.InputPrecisionFraction, Item.DeviceLoadFraction)),
		0.0f,
		1.0f);
}

void ADay1SliceHUD::DrawCurrentObjective(float CanvasWidth)
{
	if (CurrentObjective.IsEmpty())
	{
		return;
	}

	const float PanelX = OuterPadding;
	const float PanelY = OuterPadding;
	const float PanelHeight = ComputeObjectiveTrackerPanelHeight();
	const bool bHasTitle = !ObjectiveTrackerTitle.IsEmpty();
	const bool bHasDetail = !ObjectiveTrackerDetail.IsEmpty();
	const FText NavigationRouteText = BuildObjectiveNavigationRouteText();
	const bool bHasNavigation = !NavigationRouteText.IsEmpty();
	const FText NavigationDiagnosticText = BuildObjectiveNavigationDiagnosticText();
	const bool bHasNavigationDiagnostics = !NavigationDiagnosticText.IsEmpty();
	const bool bHasProgress = !ObjectiveTrackerProgressLabel.IsEmpty();
	const bool bHasMissionBrief = !ObjectiveTrackerInteractionLabel.IsEmpty()
		|| !ObjectiveTrackerMissionContext.IsEmpty()
		|| !ObjectiveTrackerFailureStakes.IsEmpty();
	const int32 VisibleChecklistCount = FMath::Min(ObjectiveChecklistItems.Num(), 6);
	const FLinearColor Accent = bObjectiveTrackerUrgent ? WarningAccent() : PanelAccent();
	DrawPanel(PanelX, PanelY, ObjectivePanelWidth, PanelHeight, PanelBackground(), Accent);

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font)
	{
		return;
	}

	Canvas->SetDrawColor(FColor::White);

	FCanvasTextItem LabelItem(
		FVector2D(PanelX + PanelPadding, PanelY + 12.0f),
		bHasTitle ? ObjectiveTrackerTitle : NSLOCTEXT("Day1SliceHUD", "ObjectiveLabel", "目标"),
		Font,
		Accent);
	LabelItem.Scale = FVector2D(1.0f);
	Canvas->DrawItem(LabelItem);

	FCanvasTextItem ObjectiveItem(
		FVector2D(PanelX + PanelPadding, PanelY + 39.0f),
		CurrentObjective,
		Font,
		TextPrimary());
	ObjectiveItem.Scale = FVector2D(1.08f);
	Canvas->DrawItem(ObjectiveItem);

	float RowY = PanelY + 70.0f;
	if (bHasDetail)
	{
		FCanvasTextItem DetailItem(
			FVector2D(PanelX + PanelPadding, RowY),
			ObjectiveTrackerDetail,
			Font,
			TextSecondary());
		DetailItem.Scale = FVector2D(0.92f);
		Canvas->DrawItem(DetailItem);
		RowY += 28.0f;
	}

	if (bHasNavigation)
	{
		const float RouteCardX = PanelX + PanelPadding - 4.0f;
		const float RouteCardY = RowY - 3.0f;
		const float RouteCardWidth = ObjectivePanelWidth - PanelPadding * 2.0f + 8.0f;
		FCanvasTileItem RouteCardBack(
			FVector2D(RouteCardX, RouteCardY),
			FVector2D(RouteCardWidth, 24.0f),
			FLinearColor(0.03f, 0.07f, 0.075f, 0.72f));
		RouteCardBack.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(RouteCardBack);

		const float NormalizedHeading = FMath::Clamp((ObjectiveNavigationState.AngleDegrees + 90.0f) / 180.0f, 0.0f, 1.0f);
		const float HeadingRailWidth = 92.0f;
		const float HeadingRailX = RouteCardX + RouteCardWidth - HeadingRailWidth - 8.0f;
		const float HeadingRailY = RouteCardY + 16.0f;
		FCanvasTileItem HeadingRail(
			FVector2D(HeadingRailX, HeadingRailY),
			FVector2D(HeadingRailWidth, 3.0f),
			FLinearColor(0.10f, 0.16f, 0.16f, 0.92f));
		HeadingRail.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(HeadingRail);
		FCanvasTileItem HeadingTick(
			FVector2D(HeadingRailX + HeadingRailWidth * NormalizedHeading - 2.0f, HeadingRailY - 3.0f),
			FVector2D(4.0f, 9.0f),
			Accent.CopyWithNewOpacity(ObjectiveNavigationState.bReachable ? 0.95f : 0.62f));
		HeadingTick.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(HeadingTick);

		FCanvasTextItem NavigationItem(
			FVector2D(PanelX + PanelPadding, RowY),
			NavigationRouteText,
			Font,
			ObjectiveNavigationState.bReachable ? Accent : WarningAccent());
		NavigationItem.Scale = FVector2D(0.86f);
		Canvas->DrawItem(NavigationItem);
		RowY += 28.0f;
	}

	if (bHasNavigationDiagnostics)
	{
		const float DiagnosticBarX = PanelX + PanelPadding;
		const float DiagnosticBarY = RowY + 18.0f;
		const float DiagnosticBarWidth = ObjectivePanelWidth - PanelPadding * 2.0f;
		const float DiagnosticBarHeight = 5.0f;
		FCanvasTextItem NavigationDiagnosticItem(
			FVector2D(DiagnosticBarX, RowY),
			NavigationDiagnosticText,
			Font,
			ObjectiveNavigationState.EscapeTimeBudgetSeconds < 2.0f ? WarningAccent() : TextSecondary());
		NavigationDiagnosticItem.Scale = FVector2D(0.78f);
		Canvas->DrawItem(NavigationDiagnosticItem);

		FCanvasTileItem DiagnosticBack(
			FVector2D(DiagnosticBarX, DiagnosticBarY),
			FVector2D(DiagnosticBarWidth, DiagnosticBarHeight),
			FLinearColor(0.08f, 0.11f, 0.12f, 0.86f));
		DiagnosticBack.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(DiagnosticBack);

		FCanvasTileItem DiagnosticFill(
			FVector2D(DiagnosticBarX, DiagnosticBarY),
			FVector2D(DiagnosticBarWidth * FMath::Clamp(ObjectiveNavigationState.PerformanceGradeFraction, 0.0f, 1.0f), DiagnosticBarHeight),
			ObjectiveNavigationState.EscapeTimeBudgetSeconds < 2.0f ? WarningAccent() : Accent.CopyWithNewOpacity(0.88f));
		DiagnosticFill.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(DiagnosticFill);
		RowY += 34.0f;
	}

	if (bHasProgress)
	{
		const float BarX = PanelX + PanelPadding;
		const float BarY = RowY + 4.0f;
		const float BarWidth = ObjectivePanelWidth - PanelPadding * 2.0f;
		const float BarHeight = 8.0f;
		FCanvasTileItem ProgressBack(FVector2D(BarX, BarY), FVector2D(BarWidth, BarHeight), FLinearColor(0.08f, 0.11f, 0.12f, 0.92f));
		ProgressBack.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(ProgressBack);

		FCanvasTileItem ProgressFill(
			FVector2D(BarX, BarY),
			FVector2D(BarWidth * ObjectiveTrackerProgressFraction, BarHeight),
			Accent);
		ProgressFill.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(ProgressFill);

		const FText ProgressText = bObjectiveTrackerRequiresRecording
			? FText::Format(NSLOCTEXT("Day1SliceHUD", "RecordingProgressFormat", "{0}    需要录像"), ObjectiveTrackerProgressLabel)
			: ObjectiveTrackerProgressLabel;
		FCanvasTextItem ProgressItem(
			FVector2D(BarX, RowY + 14.0f),
			ProgressText,
			Font,
			TextSecondary());
		ProgressItem.Scale = FVector2D(0.84f);
		Canvas->DrawItem(ProgressItem);
		RowY += 36.0f;
	}

	if (bHasMissionBrief)
	{
		const FText BriefHeader = ObjectiveTrackerInteractionLabel.IsEmpty()
			? NSLOCTEXT("Day1SliceHUD", "MissionBriefHeader", "任务简报")
			: ObjectiveTrackerInteractionLabel;
		FCanvasTextItem BriefHeaderItem(
			FVector2D(PanelX + PanelPadding, RowY),
			BriefHeader,
			Font,
			Accent);
		BriefHeaderItem.Scale = FVector2D(0.86f);
		Canvas->DrawItem(BriefHeaderItem);

		const FText BriefText = !ObjectiveTrackerMissionContext.IsEmpty()
			? ObjectiveTrackerMissionContext
			: ObjectiveTrackerFailureStakes;
		FCanvasTextItem BriefItem(
			FVector2D(PanelX + PanelPadding, RowY + 22.0f),
			BriefText,
			Font,
			TextSecondary());
		BriefItem.Scale = FVector2D(0.82f);
		Canvas->DrawItem(BriefItem);
		RowY += 46.0f;
	}

	if (!ObjectiveGraphItems.IsEmpty())
	{
		FCanvasTextItem GraphHeaderItem(
			FVector2D(PanelX + PanelPadding, RowY),
			NSLOCTEXT("Day1SliceHUD", "ObjectiveGraphHeader", "章节任务图谱"),
			Font,
			Accent.CopyWithNewOpacity(0.9f));
		GraphHeaderItem.Scale = FVector2D(0.78f);
		Canvas->DrawItem(GraphHeaderItem);
		RowY += 22.0f;

		const int32 VisibleGraphCount = FMath::Min(ObjectiveGraphItems.Num(), 4);
		for (int32 GraphIndex = 0; GraphIndex < VisibleGraphCount; ++GraphIndex)
		{
			const FHorrorObjectiveChecklistItem& GraphItem = ObjectiveGraphItems[GraphIndex];
			const FLinearColor GraphColor = ChecklistStateColor(GraphItem, Accent);
			const FVector2D NodeCenter(PanelX + PanelPadding + 8.0f, RowY + 8.0f);
			const bool bEmphasizeGraphNode = GraphItem.bActive || GraphItem.bNavigationFocused;
			DrawCircleLines(NodeCenter, bEmphasizeGraphNode ? 6.5f : 5.0f, GraphColor, 20, bEmphasizeGraphNode ? 2.4f : 1.8f);
			if (GraphItem.bNavigationFocused)
			{
				DrawCircleLines(NodeCenter, 9.0f, Accent.CopyWithNewOpacity(0.55f), 24, 1.2f);
			}
			if (GraphItem.bComplete)
			{
				DrawLine(NodeCenter + FVector2D(-3.0f, 0.0f), NodeCenter + FVector2D(-1.0f, 3.0f), GraphColor, 1.8f);
				DrawLine(NodeCenter + FVector2D(-1.0f, 3.0f), NodeCenter + FVector2D(4.0f, -4.0f), GraphColor, 1.8f);
			}
			else if (GraphItem.bBlocked)
			{
				DrawLine(NodeCenter + FVector2D(-3.0f, -3.0f), NodeCenter + FVector2D(3.0f, 3.0f), GraphColor, 1.8f);
				DrawLine(NodeCenter + FVector2D(3.0f, -3.0f), NodeCenter + FVector2D(-3.0f, 3.0f), GraphColor, 1.8f);
			}

			FText GraphLabel = GraphItem.Label;
			if (GraphItem.bOptional)
			{
				GraphLabel = FText::Format(
					NSLOCTEXT("Day1SliceHUD", "ObjectiveGraphOptionalFormat", "{0}    可选"),
					GraphLabel);
			}
			if (GraphItem.bNavigationFocused)
			{
				GraphLabel = FText::Format(
					NSLOCTEXT("Day1SliceHUD", "ObjectiveGraphNavigationFocusFormat", "{0}    导航"),
					GraphLabel);
			}
			FCanvasTextItem GraphLabelItem(
				FVector2D(PanelX + PanelPadding + 24.0f, RowY),
				GraphLabel,
				Font,
				GraphColor);
			GraphLabelItem.Scale = FVector2D(bEmphasizeGraphNode ? 0.72f : 0.68f);
			Canvas->DrawItem(GraphLabelItem);

			const FText GraphDetail = GraphItem.bBlocked && !GraphItem.LockReason.IsEmpty()
				? GraphItem.LockReason
				: (!GraphItem.StatusText.IsEmpty() ? GraphItem.StatusText : GraphItem.InteractionLabel);
			if (!GraphDetail.IsEmpty())
			{
				FCanvasTextItem GraphDetailItem(
					FVector2D(PanelX + PanelPadding + 24.0f, RowY + 16.0f),
					GraphDetail,
					Font,
					TextSecondary());
				GraphDetailItem.Scale = FVector2D(0.62f);
				Canvas->DrawItem(GraphDetailItem);
			}
			RowY += 36.0f;
		}
		if (!ObjectiveGraphOverflowText.IsEmpty())
		{
			FCanvasTextItem OverflowItem(
				FVector2D(PanelX + PanelPadding + 24.0f, RowY),
				ObjectiveGraphOverflowText,
				Font,
				TextSecondary());
			OverflowItem.Scale = FVector2D(0.62f);
			Canvas->DrawItem(OverflowItem);
			RowY += 18.0f;
		}
		RowY += 6.0f;
	}

	if (VisibleChecklistCount > 0)
	{
		for (int32 ItemIndex = 0; ItemIndex < VisibleChecklistCount; ++ItemIndex)
		{
			const FHorrorObjectiveChecklistItem& Item = ObjectiveChecklistItems[ItemIndex];
			const FLinearColor ItemColor = ChecklistStateColor(Item, Accent);
			FText ItemText = Item.Label;
			if (Item.bRequiresRecording && !Item.bComplete)
			{
				ItemText = FText::Format(
					NSLOCTEXT("Day1SliceHUD", "ChecklistRecordingItemFormat", "{0}    需要录像"),
					ItemText);
			}
			if (Item.bRetryable)
			{
				ItemText = FText::Format(
					NSLOCTEXT("Day1SliceHUD", "ChecklistRetryItemFormat", "{0}    可重试"),
					ItemText);
			}

			const FVector2D NodeCenter(PanelX + PanelPadding + 8.0f, RowY + 8.0f);
			DrawCircleLines(NodeCenter, Item.bActive ? 8.0f : 6.0f, ItemColor, 24, Item.bActive ? 3.0f : 2.0f);
			if (Item.bComplete)
			{
				DrawLine(NodeCenter + FVector2D(-4.0f, 0.0f), NodeCenter + FVector2D(-1.0f, 4.0f), ItemColor, 2.0f);
				DrawLine(NodeCenter + FVector2D(-1.0f, 4.0f), NodeCenter + FVector2D(5.0f, -5.0f), ItemColor, 2.0f);
			}
			else if (Item.bBlocked)
			{
				DrawLine(NodeCenter + FVector2D(-4.0f, -4.0f), NodeCenter + FVector2D(4.0f, 4.0f), ItemColor, 2.0f);
				DrawLine(NodeCenter + FVector2D(4.0f, -4.0f), NodeCenter + FVector2D(-4.0f, 4.0f), ItemColor, 2.0f);
			}

			FCanvasTextItem ChecklistItem(
				FVector2D(PanelX + PanelPadding + 24.0f, RowY),
				ItemText,
				Font,
				ItemColor);
			ChecklistItem.Scale = FVector2D(Item.bActive ? 0.9f : 0.84f);
			Canvas->DrawItem(ChecklistItem);

			const FText ItemDetail = BuildObjectiveChecklistActionLine(Item);
			float NextContentY = RowY + 19.0f;
			if (!ItemDetail.IsEmpty())
			{
				FCanvasTextItem DetailItem(
					FVector2D(PanelX + PanelPadding + 24.0f, NextContentY),
					ItemDetail,
					Font,
					TextSecondary());
				DetailItem.Scale = FVector2D(0.72f);
				Canvas->DrawItem(DetailItem);
				NextContentY += 18.0f;
			}

			const FText InstrumentLine = BuildObjectiveChecklistInstrumentLine(Item);
			if (!InstrumentLine.IsEmpty() && !Item.bComplete && !Item.bBlocked && !Item.bOptional)
			{
				FCanvasTextItem InstrumentItem(
					FVector2D(PanelX + PanelPadding + 24.0f, NextContentY),
					InstrumentLine,
					Font,
					ItemColor.CopyWithNewOpacity(0.82f));
				InstrumentItem.Scale = FVector2D(0.66f);
				Canvas->DrawItem(InstrumentItem);
				NextContentY += 18.0f;
			}

			if (!Item.TacticalLabel.IsEmpty())
			{
				FCanvasTextItem TacticalItem(
					FVector2D(PanelX + PanelPadding + 24.0f, NextContentY),
					Item.TacticalLabel,
					Font,
					ItemColor.CopyWithNewOpacity(0.88f));
				TacticalItem.Scale = FVector2D(0.66f);
				Canvas->DrawItem(TacticalItem);
				NextContentY += 18.0f;
			}

			const FText MetadataText = ChecklistMetadataText(Item);
			if (!MetadataText.IsEmpty())
			{
				const FLinearColor MetadataColor = ChecklistRiskColor(Item.RiskLevel, TextSecondary()).CopyWithNewOpacity(0.92f);
				FCanvasTextItem MetadataItem(
					FVector2D(PanelX + PanelPadding + 24.0f, NextContentY),
					MetadataText,
					Font,
					MetadataColor);
				MetadataItem.Scale = FVector2D(0.68f);
				Canvas->DrawItem(MetadataItem);
				NextContentY += 18.0f;
			}

			const FText RewardText = ChecklistRewardText(Item);
			if (!RewardText.IsEmpty())
			{
				FCanvasTextItem RewardItem(
					FVector2D(PanelX + PanelPadding + 24.0f, NextContentY),
					RewardText,
					Font,
					PanelAccent().CopyWithNewOpacity(0.86f));
				RewardItem.Scale = FVector2D(0.68f);
				Canvas->DrawItem(RewardItem);
				NextContentY += 18.0f;
			}

			const float TelemetryFraction = ComputeObjectiveChecklistTelemetryFraction(Item);
			if (ChecklistHasTelemetryRail(Item) && TelemetryFraction > KINDA_SMALL_NUMBER)
			{
				const float MiniBarX = PanelX + PanelPadding + 24.0f;
				const float MiniBarY = NextContentY + 4.0f;
				const float MiniBarWidth = ObjectivePanelWidth - PanelPadding * 2.0f - 24.0f;
				FCanvasTileItem MiniBack(FVector2D(MiniBarX, MiniBarY), FVector2D(MiniBarWidth, 5.0f), FLinearColor(0.06f, 0.08f, 0.08f, 0.88f));
				MiniBack.BlendMode = SE_BLEND_Translucent;
				Canvas->DrawItem(MiniBack);
				FCanvasTileItem MiniFill(
					FVector2D(MiniBarX, MiniBarY),
					FVector2D(MiniBarWidth * TelemetryFraction, 5.0f),
					ItemColor.CopyWithNewOpacity(0.92f));
				MiniFill.BlendMode = SE_BLEND_Translucent;
				Canvas->DrawItem(MiniFill);
				if (Item.InputPrecisionFraction > KINDA_SMALL_NUMBER)
				{
					FCanvasTileItem PrecisionTick(
						FVector2D(MiniBarX + MiniBarWidth * FMath::Clamp(Item.InputPrecisionFraction, 0.0f, 1.0f) - 1.0f, MiniBarY - 2.0f),
						FVector2D(2.0f, 9.0f),
						PanelAccent().CopyWithNewOpacity(0.96f));
					PrecisionTick.BlendMode = SE_BLEND_Translucent;
					Canvas->DrawItem(PrecisionTick);
				}
				if (Item.DeviceLoadFraction > KINDA_SMALL_NUMBER)
				{
					FCanvasTileItem LoadTick(
						FVector2D(MiniBarX + MiniBarWidth * FMath::Clamp(Item.DeviceLoadFraction, 0.0f, 1.0f) - 1.0f, MiniBarY - 2.0f),
						FVector2D(2.0f, 9.0f),
						WarningAccent().CopyWithNewOpacity(0.9f));
					LoadTick.BlendMode = SE_BLEND_Translucent;
					Canvas->DrawItem(LoadTick);
				}
			}
			RowY += ChecklistItemHeight(Item);
		}
	}
}

void ADay1SliceHUD::DrawAdvancedInteractionPanel(float CanvasWidth, float CanvasHeight)
{
	if (!AdvancedInteractionState.bVisible)
	{
		return;
	}

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font)
	{
		return;
	}

	const float PanelWidth = FMath::Min(AdvancedPanelWidth, CanvasWidth - 48.0f);
	const float PanelHeight = FMath::Min(AdvancedPanelHeight, CanvasHeight - 80.0f);
	const float WorldSeconds = GetWorldSeconds();
	const float LoadJitter = AdvancedInteractionState.DeviceLoadFraction > 0.58f
		? FMath::Sin(WorldSeconds * 42.0f) * (AdvancedInteractionState.DeviceLoadFraction - 0.58f) * 10.0f
		: 0.0f;
	const float PanelX = (CanvasWidth - PanelWidth) * 0.5f + LoadJitter;
	const float PanelY = (CanvasHeight - PanelHeight) * 0.5f;
	const FLinearColor Accent = AccentForAdvancedMode(AdvancedInteractionState.Mode);

	FCanvasTileItem DimTile(FVector2D(0.0f, 0.0f), FVector2D(CanvasWidth, CanvasHeight), FLinearColor(0.0f, 0.0f, 0.0f, 0.42f));
	DimTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(DimTile);

	DrawPanel(PanelX, PanelY, PanelWidth, PanelHeight, FLinearColor(0.006f, 0.010f, 0.014f, 0.94f), Accent);
	if (AdvancedInteractionState.DeviceLoadFraction > 0.35f)
	{
		const FLinearColor LoadColor = DeviceLoadColor(AdvancedInteractionState.DeviceLoadFraction);
		const float ScanY = PanelY + 42.0f + FMath::Fmod(WorldSeconds * 96.0f, FMath::Max(1.0f, PanelHeight - 84.0f));
		FCanvasTileItem LoadWash(
			FVector2D(PanelX + 4.0f, PanelY + 35.0f),
			FVector2D(PanelWidth - 4.0f, PanelHeight - 36.0f),
			LoadColor.CopyWithNewOpacity(0.04f + AdvancedInteractionState.DeviceLoadFraction * 0.08f));
		LoadWash.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(LoadWash);

		FCanvasTileItem Scanline(
			FVector2D(PanelX + 4.0f, ScanY),
			FVector2D(PanelWidth - 4.0f, 2.0f + AdvancedInteractionState.DeviceLoadFraction * 3.0f),
			LoadColor.CopyWithNewOpacity(0.16f + AdvancedInteractionState.DeviceLoadFraction * 0.20f));
		Scanline.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(Scanline);
	}
	DrawAdvancedInteractionHeader(PanelX, PanelY, PanelWidth, Font, Accent);

	if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::CircuitWiring)
	{
		DrawCircuitWiringPanel(PanelX, PanelY, PanelWidth, PanelHeight, Font, WorldSeconds);
	}
	else if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::GearCalibration)
	{
		DrawGearCalibrationPanel(PanelX, PanelY, PanelWidth, PanelHeight, Font, WorldSeconds);
	}
	else if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::SpectralScan)
	{
		DrawSpectralScanPanel(PanelX, PanelY, PanelWidth, PanelHeight, Font, WorldSeconds);
	}
	else if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::SignalTuning)
	{
		DrawSignalTuningPanel(PanelX, PanelY, PanelWidth, PanelHeight, Font, WorldSeconds);
	}

	DrawAdvancedInteractionStepTrack(PanelX, PanelY + PanelHeight - 154.0f, PanelWidth, Font, Accent);
	DrawAdvancedInteractionProgress(PanelX, PanelY + PanelHeight - 118.0f, PanelWidth, Font, Accent);
}

void ADay1SliceHUD::DrawAdvancedInteractionHeader(float PanelX, float PanelY, float PanelWidth, UFont* Font, const FLinearColor& AccentColor)
{
	// 简化UI：只显示模式名称，去掉所有干扰文字
	FText ModeText;
	switch (AdvancedInteractionState.Mode)
	{
	case EHorrorCampaignInteractionMode::CircuitWiring:
		ModeText = NSLOCTEXT("Day1SliceHUD", "CircuitWiringMode", "电路接线台");
		break;
	case EHorrorCampaignInteractionMode::GearCalibration:
		ModeText = NSLOCTEXT("Day1SliceHUD", "GearCalibrationMode", "机械齿轮盘");
		break;
	case EHorrorCampaignInteractionMode::SpectralScan:
		ModeText = NSLOCTEXT("Day1SliceHUD", "SpectralScanMode", "频谱扫描器");
		break;
	case EHorrorCampaignInteractionMode::SignalTuning:
		ModeText = NSLOCTEXT("Day1SliceHUD", "SignalTuningMode", "信号调谐器");
		break;
	default:
		ModeText = NSLOCTEXT("Day1SliceHUD", "AdvancedInteractionMode", "交互窗口");
		break;
	}

	FCanvasTextItem ModeItem(
		FVector2D(PanelX + PanelPadding, PanelY + 16.0f),
		ModeText,
		Font,
		AccentColor);
	ModeItem.Scale = FVector2D(1.05f);
	Canvas->DrawItem(ModeItem);

	// 去掉所有其他文字：Title、BeatLabel、DiagnosticLine、TacticalSummary
	// 只保留模式名称，让玩家专注于交互本身
}

void ADay1SliceHUD::DrawAdvancedInteractionStepTrack(float PanelX, float PanelY, float PanelWidth, UFont* Font, const FLinearColor& AccentColor)
{
	const int32 TrackCount = AdvancedInteractionState.StepTrack.Num();
	if (TrackCount <= 0)
	{
		return;
	}

	const float TrackX = PanelX + PanelPadding;
	const float TrackWidth = PanelWidth - PanelPadding * 2.0f;
	const float NodeGap = 8.0f;
	const int32 VisibleTrackCount = FMath::Min(TrackCount, 5);
	const float NodeWidth = (TrackWidth - NodeGap * static_cast<float>(VisibleTrackCount - 1)) / static_cast<float>(VisibleTrackCount);
	for (int32 TrackIndex = 0; TrackIndex < VisibleTrackCount; ++TrackIndex)
	{
		const FHorrorAdvancedInteractionStepTrackItem& TrackItem = AdvancedInteractionState.StepTrack[TrackIndex];
		const float NodeX = TrackX + static_cast<float>(TrackIndex) * (NodeWidth + NodeGap);
		const FLinearColor NodeColor = TrackItem.bComplete
			? FLinearColor(0.30f, 1.0f, 0.70f, 0.94f)
			: (TrackItem.bActive ? AccentColor : TextSecondary());
		const FLinearColor NodeBackColor = TrackItem.bActive
			? AccentColor.CopyWithNewOpacity(0.18f)
			: (TrackItem.bComplete
				? FLinearColor(0.05f, 0.12f, 0.09f, 0.78f)
				: FLinearColor(0.035f, 0.044f, 0.046f, 0.72f));
		FCanvasTileItem NodeBack(
			FVector2D(NodeX, PanelY),
			FVector2D(NodeWidth, 30.0f),
			NodeBackColor);
		NodeBack.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(NodeBack);

		if (TrackItem.bActive)
		{
			FCanvasTileItem ActiveBar(
				FVector2D(NodeX, PanelY + 27.0f),
				FVector2D(NodeWidth * FMath::Clamp(AdvancedInteractionState.TargetAlignmentFraction, 0.0f, 1.0f), 3.0f),
				AccentColor.CopyWithNewOpacity(0.92f));
			ActiveBar.BlendMode = SE_BLEND_Translucent;
			Canvas->DrawItem(ActiveBar);
		}

		const FText StepText = FText::Format(
			NSLOCTEXT("Day1SliceHUD", "AdvancedStepTrackNode", "{0}. {1} {2}"),
			FText::AsNumber(TrackItem.StepIndex + 1),
			TrackItem.KeyHint,
			TrackItem.DisplayLabel);
		FCanvasTextItem StepItem(
			FVector2D(NodeX + 7.0f, PanelY + 7.0f),
			StepText,
			Font,
			TrackItem.bHazardous ? WarningAccent() : NodeColor);
		StepItem.Scale = FVector2D(0.66f);
		Canvas->DrawItem(StepItem);

		if (TrackItem.bComplete)
		{
			DrawLine(FVector2D(NodeX + NodeWidth - 18.0f, PanelY + 10.0f), FVector2D(NodeX + NodeWidth - 13.0f, PanelY + 18.0f), NodeColor, 2.0f);
			DrawLine(FVector2D(NodeX + NodeWidth - 13.0f, PanelY + 18.0f), FVector2D(NodeX + NodeWidth - 5.0f, PanelY + 7.0f), NodeColor, 2.0f);
		}
	}
}

void ADay1SliceHUD::DrawCircuitWiringPanel(float PanelX, float PanelY, float PanelWidth, float PanelHeight, UFont* Font, float WorldSeconds)
{
	const FVector2D Center(PanelX + PanelWidth * 0.5f, PanelY + 260.0f);
	const float DialRadius = FMath::Min(PanelWidth, PanelHeight) * 0.25f;
	const bool bFailure = AdvancedFeedbackIsFailure(AdvancedInteractionState.FeedbackState, AdvancedInteractionState.FeedbackText);
	const FLinearColor TargetColor = VisualColorForAdvancedInput(AdvancedInteractionState, AdvancedInteractionState.ExpectedInputId);
	const float Pulse = 0.5f + 0.5f * FMath::Sin(WorldSeconds * 8.0f);
	const float PrecisionPulse = 0.35f + AdvancedInteractionState.InputPrecisionFraction * 0.85f;
	const float LoadSparkScale = 1.0f + AdvancedInteractionState.DeviceLoadFraction * 1.8f;
	const float RouteFlow = FMath::Clamp(AdvancedInteractionState.RouteFlowFraction, 0.0f, 1.0f);
	const float HazardPressure = FMath::Clamp(AdvancedInteractionState.HazardPressureFraction, 0.0f, 1.0f);
	const float TargetAlignment = FMath::Clamp(AdvancedInteractionState.TargetAlignmentFraction, 0.0f, 1.0f);
	const float BoardX = PanelX + 86.0f;
	const float BoardY = PanelY + 408.0f;
	const float BoardWidth = PanelWidth - 172.0f;
	const float BoardHeight = 22.0f;

	DrawCircleLines(Center, DialRadius + 26.0f, DeviceLoadColor(AdvancedInteractionState.DeviceLoadFraction).CopyWithNewOpacity(0.24f + AdvancedInteractionState.DeviceLoadFraction * 0.34f), 72, 2.0f + AdvancedInteractionState.DeviceLoadFraction * 2.0f);
	DrawCircleLines(Center, DialRadius, FLinearColor(0.10f, 0.35f, 0.48f, 0.92f), 72, 3.0f);
	DrawCircleLines(Center, DialRadius * 0.48f, FLinearColor(0.08f, 0.18f, 0.22f, 0.90f), 48, 2.0f);

	FCanvasTileItem BoardBack(FVector2D(BoardX, BoardY), FVector2D(BoardWidth, BoardHeight), FLinearColor(0.018f, 0.045f, 0.050f, 0.86f));
	BoardBack.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BoardBack);
	FCanvasTileItem BoardFlow(
		FVector2D(BoardX, BoardY),
		FVector2D(BoardWidth * RouteFlow, BoardHeight),
		FLinearColor(0.18f, 0.86f, 0.76f, 0.52f + Pulse * 0.20f));
	BoardFlow.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BoardFlow);
	for (int32 TraceIndex = 0; TraceIndex <= 6; ++TraceIndex)
	{
		const float TraceX = BoardX + BoardWidth * static_cast<float>(TraceIndex) / 6.0f;
		const float TracePulse = FMath::Fmod(WorldSeconds * 0.65f + static_cast<float>(TraceIndex) * 0.17f, 1.0f);
		DrawLine(
			FVector2D(TraceX, BoardY + 2.0f),
			FVector2D(FMath::Min(BoardX + BoardWidth, TraceX + BoardWidth * 0.10f * TracePulse), BoardY + BoardHeight - 2.0f),
			FLinearColor(0.24f, 0.92f, 0.86f, 0.18f + RouteFlow * 0.34f),
			2.0f);
	}

	const float PressureX = PanelX + PanelWidth - 156.0f;
	const float PressureY = PanelY + 174.0f;
	const float PressureHeight = 156.0f;
	FCanvasTileItem PressureBack(FVector2D(PressureX, PressureY), FVector2D(12.0f, PressureHeight), FLinearColor(0.10f, 0.025f, 0.018f, 0.78f));
	PressureBack.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(PressureBack);
	FCanvasTileItem PressureFill(
		FVector2D(PressureX, PressureY + PressureHeight * (1.0f - HazardPressure)),
		FVector2D(12.0f, PressureHeight * HazardPressure),
		FLinearColor(1.0f, 0.20f, 0.08f, 0.70f + Pulse * 0.24f));
	PressureFill.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(PressureFill);
	FCanvasTextItem PressureLabel(
		FVector2D(PressureX - 34.0f, PressureY + PressureHeight + 8.0f),
		NSLOCTEXT("Day1SliceHUD", "CircuitHazardPressureLabel", "漏电压力"),
		Font,
		HazardPressure > 0.72f ? FLinearColor(1.0f, 0.24f, 0.10f, 0.92f) : TextSecondary());
	PressureLabel.Scale = FVector2D(0.68f);
	Canvas->DrawItem(PressureLabel);

	const float LensRadius = 18.0f + TargetAlignment * 13.0f;
	DrawCircleLines(Center, DialRadius * 0.28f + LensRadius, TargetColor.CopyWithNewOpacity(0.24f + TargetAlignment * 0.46f), 36, 2.0f + TargetAlignment * 2.0f);

	const float WindowStartAngle = -PI * 0.60f;
	const float WindowEndAngle = FMath::Lerp(-PI * 0.95f, PI * 1.05f, AdvancedWindowEnd(AdvancedInteractionState));
	const int32 WindowSegments = 18;
	const float WindowActualStartAngle = FMath::Lerp(-PI * 0.95f, PI * 1.05f, AdvancedWindowStart(AdvancedInteractionState));
	FVector2D PreviousPoint = Center + FVector2D(FMath::Cos(WindowActualStartAngle), FMath::Sin(WindowActualStartAngle)) * (DialRadius + 14.0f);
	for (int32 SegmentIndex = 1; SegmentIndex <= WindowSegments; ++SegmentIndex)
	{
		const float Alpha = static_cast<float>(SegmentIndex) / static_cast<float>(WindowSegments);
		const float Angle = FMath::Lerp(WindowActualStartAngle, WindowEndAngle, Alpha);
		const FVector2D Point = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * (DialRadius + 14.0f);
		DrawLine(PreviousPoint, Point, AdvancedInteractionState.bTimingWindowOpen ? FLinearColor(0.28f, 1.0f, 0.82f, 1.0f) : FLinearColor(0.12f, 0.44f, 0.42f, 0.75f), 6.0f);
		PreviousPoint = Point;
	}

	const float CursorAngle = FMath::Lerp(-PI * 0.95f, PI * 1.05f, AdvancedInteractionState.TimingFraction);
	const FVector2D CursorEnd = Center + FVector2D(FMath::Cos(CursorAngle), FMath::Sin(CursorAngle)) * (DialRadius + 20.0f);
	DrawLine(Center, CursorEnd, AdvancedInteractionState.bTimingWindowOpen ? FLinearColor(0.34f, 1.0f, 0.86f, 1.0f) : FLinearColor(0.85f, 0.94f, 1.0f, 0.88f), 4.0f);
	DrawCircleLines(CursorEnd, 10.0f + Pulse * 4.0f * PrecisionPulse, AdvancedInteractionState.bTimingWindowOpen ? FLinearColor(0.34f, 1.0f, 0.86f, 0.96f) : TargetColor, 18, 2.0f + AdvancedInteractionState.InputPrecisionFraction);

	const float Angles[] = { -PI * 0.78f, -PI * 0.20f, PI * 0.28f };
	for (int32 InputIndex = 0; InputIndex < FMath::Min(AdvancedInteractionState.InputOptions.Num(), 3); ++InputIndex)
	{
		const FHorrorAdvancedInteractionInputOption& Option = AdvancedInteractionState.InputOptions[InputIndex];
		const FName InputId = Option.InputId;
		const bool bTarget = InputId == AdvancedInteractionState.ExpectedInputId;
		const bool bLinked = Option.bChainLinked;
		const bool bActiveTerminal = bTarget || bLinked;
		const bool bFocusedSlot = AdvancedInteractionState.ActiveInputSlotIndex == InputIndex;
		const float OptionLoad = FMath::Clamp(Option.LoadFraction, 0.0f, 1.0f);
		const FLinearColor WireColor = VisualColorForAdvancedInput(AdvancedInteractionState, InputId);
		const FVector2D Terminal = Center + FVector2D(FMath::Cos(Angles[InputIndex]), FMath::Sin(Angles[InputIndex])) * (DialRadius + 86.0f);
		const FVector2D Inner = Center + FVector2D(FMath::Cos(Angles[InputIndex]), FMath::Sin(Angles[InputIndex])) * (DialRadius * 0.58f);
		const float WireOpacity = bLinked
			? 0.88f
			: (bTarget ? 0.96f : 0.46f + OptionLoad * 0.24f);
		const float WireThickness = bLinked
			? 4.5f
			: (bTarget ? 4.0f : 2.0f + OptionLoad * 1.5f);
			DrawLine(Inner, Terminal, WireColor.CopyWithNewOpacity(WireOpacity), WireThickness);
		if (bFocusedSlot)
		{
			DrawCircleLines(Terminal, 36.0f + TargetAlignment * 12.0f + Pulse * 5.0f, TargetColor.CopyWithNewOpacity(0.32f + TargetAlignment * 0.38f), 32, 3.0f);
			DrawLine(Center, Terminal, TargetColor.CopyWithNewOpacity(0.16f + TargetAlignment * 0.34f), 8.0f);
		}
		if (bLinked)
		{
			DrawLine(
				Inner + FVector2D(0.0f, 3.0f),
				Terminal + FVector2D(0.0f, 3.0f),
				WireColor.CopyWithNewOpacity(0.26f + Pulse * 0.24f),
				7.0f);
		}
		if (Option.bHazardous)
		{
			const FLinearColor HazardColor(1.0f, 0.18f, 0.08f, 0.96f);
			DrawCircleLines(Terminal, 28.0f + Pulse * 8.0f + OptionLoad * 8.0f + HazardPressure * 10.0f, HazardColor.CopyWithNewOpacity(0.48f + Pulse * 0.32f), 28, 3.0f + HazardPressure * 1.5f);
			const int32 SparkCount = FMath::Clamp(3 + FMath::RoundToInt(HazardPressure * 4.0f), 3, 7);
			for (int32 SparkIndex = 0; SparkIndex < SparkCount; ++SparkIndex)
			{
				const float SparkAngle = WorldSeconds * 8.0f + SparkIndex * 2.1f + InputIndex;
				const FVector2D SparkStart = Terminal + FVector2D(FMath::Cos(SparkAngle), FMath::Sin(SparkAngle)) * 19.0f;
				const FVector2D SparkEnd = Terminal + FVector2D(FMath::Cos(SparkAngle + 0.35f), FMath::Sin(SparkAngle + 0.35f)) * (37.0f + OptionLoad * 16.0f + HazardPressure * 14.0f);
				DrawLine(SparkStart, SparkEnd, HazardColor.CopyWithNewOpacity(0.34f + Pulse * 0.46f), 2.0f + OptionLoad * 2.0f + HazardPressure);
			}
		}
		DrawCircleLines(Terminal, bTarget ? 24.0f + Pulse * 6.0f : (bLinked ? 22.0f : 19.0f), WireColor, 28, bActiveTerminal ? 4.0f : 2.0f);
		if (bLinked)
		{
			DrawCircleLines(Terminal, 11.0f, FLinearColor(0.34f, 1.0f, 0.82f, 0.92f), 20, 2.0f);
			DrawLine(Terminal + FVector2D(-6.0f, 0.0f), Terminal + FVector2D(-1.0f, 6.0f), FLinearColor(0.34f, 1.0f, 0.82f, 0.96f), 2.0f);
			DrawLine(Terminal + FVector2D(-1.0f, 6.0f), Terminal + FVector2D(8.0f, -7.0f), FLinearColor(0.34f, 1.0f, 0.82f, 0.96f), 2.0f);
		}

		FCanvasTextItem KeyItem(
			FVector2D(Terminal.X - 6.0f, Terminal.Y - 10.0f),
			Option.KeyHint,
			Font,
			bActiveTerminal ? TextPrimary() : TextSecondary());
		KeyItem.Scale = FVector2D(bTarget ? 1.08f : (bLinked ? 1.0f : 0.92f));
		Canvas->DrawItem(KeyItem);

		FCanvasTextItem NameItem(
			FVector2D(Terminal.X - 42.0f, Terminal.Y + 30.0f),
			Option.DisplayLabel,
			Font,
			Option.bHazardous ? FLinearColor(1.0f, 0.32f, 0.18f, 0.96f) : (bActiveTerminal ? WireColor : TextSecondary()));
		NameItem.Scale = FVector2D(0.82f);
		Canvas->DrawItem(NameItem);

		if (bLinked)
		{
			FCanvasTextItem LinkedItem(
				FVector2D(Terminal.X - 34.0f, Terminal.Y + 50.0f),
				NSLOCTEXT("Day1SliceHUD", "CircuitTerminalLinked", "已接入"),
				Font,
				FLinearColor(0.34f, 1.0f, 0.82f, 0.88f));
			LinkedItem.Scale = FVector2D(0.70f);
			Canvas->DrawItem(LinkedItem);
		}

		if (OptionLoad > 0.55f && !bLinked)
		{
			FCanvasTextItem LoadItem(
				FVector2D(Terminal.X - 34.0f, Terminal.Y + 50.0f),
				NSLOCTEXT("Day1SliceHUD", "CircuitTerminalLoadHigh", "高负载"),
				Font,
				FLinearColor(1.0f, 0.24f, 0.10f, 0.88f));
			LoadItem.Scale = FVector2D(0.70f);
			Canvas->DrawItem(LoadItem);
		}
	}

	if (!AdvancedInteractionState.FeedbackText.IsEmpty())
	{
		const FLinearColor FeedbackColor = AdvancedFeedbackColor(
			AdvancedInteractionState.FeedbackState,
			AdvancedInteractionState.FeedbackText,
			FLinearColor(0.28f, 0.82f, 1.0f, 0.96f));
		const int32 ArcCount = bFailure
			? FMath::Clamp(5 + AdvancedInteractionState.MistakeCount * 2, 5, 13)
			: FMath::Clamp(4 + AdvancedInteractionState.ComboCount, 4, 10);
		for (int32 ArcIndex = 0; ArcIndex < ArcCount; ++ArcIndex)
		{
			const float Angle = WorldSeconds * (bFailure ? 9.0f : 5.5f) + ArcIndex * 0.86f;
			const FVector2D Start = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * (DialRadius * 0.68f);
			const FVector2D End = Center + FVector2D(FMath::Cos(Angle + 0.22f), FMath::Sin(Angle + 0.22f)) * (DialRadius + (bFailure ? 62.0f : 34.0f) * LoadSparkScale);
			DrawLine(Start, End, FeedbackColor.CopyWithNewOpacity(0.35f + Pulse * 0.55f), (bFailure ? 3.0f : 2.0f) + AdvancedInteractionState.DeviceLoadFraction * 2.0f);
		}
	}

	FCanvasTextItem ControlsItem(
		FVector2D(PanelX + PanelPadding, PanelY + PanelHeight - 52.0f),
		BuildAdvancedInteractionControlHint(),
		Font,
		TextSecondary());
	ControlsItem.Scale = FVector2D(0.90f);
	Canvas->DrawItem(ControlsItem);
}

void ADay1SliceHUD::DrawGearCalibrationPanel(float PanelX, float PanelY, float PanelWidth, float PanelHeight, UFont* Font, float WorldSeconds)
{
	const FVector2D Center(PanelX + PanelWidth * 0.5f, PanelY + 266.0f);
	const FVector2D GearCenters[] = {
		Center + FVector2D(-150.0f, 18.0f),
		Center + FVector2D(0.0f, -34.0f),
		Center + FVector2D(150.0f, 18.0f)
	};
	const float GearRadii[] = { 78.0f, 92.0f, 78.0f };
	const int32 TargetIndex = IndexForAdvancedInputOption(AdvancedInteractionState, AdvancedInteractionState.ExpectedInputId);
	const bool bFailure = AdvancedFeedbackIsFailure(AdvancedInteractionState.FeedbackState, AdvancedInteractionState.FeedbackText) || AdvancedInteractionState.bPaused;
	const float Pulse = 0.5f + 0.5f * FMath::Sin(WorldSeconds * 7.5f);
	const float LoadDrag = AdvancedInteractionState.DeviceLoadFraction * 0.55f;
	const float ChainEngagement = FMath::Clamp(AdvancedInteractionState.RouteFlowFraction, 0.0f, 1.0f);
	const float JamPressure = FMath::Clamp(AdvancedInteractionState.HazardPressureFraction, 0.0f, 1.0f);
	const float ToothAlignment = FMath::Clamp(AdvancedInteractionState.TargetAlignmentFraction, 0.0f, 1.0f);

	const float TrackX = PanelX + 110.0f;
	const float TrackY = PanelY + 128.0f;
	const float TrackWidth = PanelWidth - 220.0f;
	const float TrackHeight = 18.0f;
	FCanvasTileItem TrackBack(FVector2D(TrackX, TrackY), FVector2D(TrackWidth, TrackHeight), FLinearColor(0.05f, 0.06f, 0.055f, 0.92f));
	TrackBack.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TrackBack);
	FCanvasTileItem WindowTile(
		FVector2D(TrackX + TrackWidth * AdvancedWindowStart(AdvancedInteractionState), TrackY),
		FVector2D(TrackWidth * (AdvancedWindowEnd(AdvancedInteractionState) - AdvancedWindowStart(AdvancedInteractionState)), TrackHeight),
		AdvancedInteractionState.bTimingWindowOpen ? FLinearColor(0.32f, 0.92f, 0.58f, 0.88f) : FLinearColor(0.28f, 0.44f, 0.32f, 0.62f));
	WindowTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(WindowTile);
	const float CursorX = TrackX + TrackWidth * AdvancedInteractionState.TimingFraction;
	DrawLine(FVector2D(CursorX, TrackY - 8.0f), FVector2D(CursorX, TrackY + TrackHeight + 8.0f), FLinearColor(0.98f, 0.90f, 0.48f, 0.96f), 4.0f);

	const float ReadoutY = PanelY + 158.0f;
	const float ReadoutWidth = (PanelWidth - PanelPadding * 2.0f - 32.0f) / 3.0f;
	const struct FGearReadout
	{
		FText Label;
		float Value = 0.0f;
		FLinearColor Color = FLinearColor::White;
	} Readouts[] = {
		{
			NSLOCTEXT("Day1SliceHUD", "GearReadoutChain", "链路"),
			ChainEngagement,
			FLinearColor(0.30f, 0.94f, 0.62f, 0.92f)
		},
		{
			NSLOCTEXT("Day1SliceHUD", "GearReadoutJam", "卡滞"),
			JamPressure,
			DeviceLoadColor(JamPressure)
		},
		{
			NSLOCTEXT("Day1SliceHUD", "GearReadoutAlignment", "咬合"),
			ToothAlignment,
			FLinearColor(0.92f, 0.82f, 0.48f, 0.94f)
		}
	};
	for (int32 ReadoutIndex = 0; ReadoutIndex < 3; ++ReadoutIndex)
	{
		const float BoxX = PanelX + PanelPadding + ReadoutIndex * (ReadoutWidth + 16.0f);
		FCanvasTileItem BackItem(
			FVector2D(BoxX, ReadoutY),
			FVector2D(ReadoutWidth, 32.0f),
			FLinearColor(0.035f, 0.040f, 0.038f, 0.78f));
		BackItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(BackItem);
		FCanvasTileItem FillItem(
			FVector2D(BoxX, ReadoutY + 24.0f),
			FVector2D(ReadoutWidth * Readouts[ReadoutIndex].Value, 4.0f),
			Readouts[ReadoutIndex].Color.CopyWithNewOpacity(0.72f + Pulse * 0.18f));
		FillItem.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(FillItem);
		FCanvasTextItem ReadoutText(
			FVector2D(BoxX + 9.0f, ReadoutY + 7.0f),
			FText::Format(
				NSLOCTEXT("Day1SliceHUD", "GearReadoutFormat", "{0} {1}%"),
				Readouts[ReadoutIndex].Label,
				FText::AsNumber(FMath::RoundToInt(Readouts[ReadoutIndex].Value * 100.0f))),
			Font,
			Readouts[ReadoutIndex].Color);
		ReadoutText.Scale = FVector2D(0.72f);
		Canvas->DrawItem(ReadoutText);
	}

	for (int32 GearIndex = 0; GearIndex < 3; ++GearIndex)
	{
		const FHorrorAdvancedInteractionInputOption* GearOption = AdvancedInteractionState.InputOptions.IsValidIndex(GearIndex)
			? &AdvancedInteractionState.InputOptions[GearIndex]
			: nullptr;
		const bool bStoppedGear = GearOption ? GearOption->bStalled : (GearIndex == TargetIndex);
		const bool bAlreadySolvedInSequence = GearOption ? GearOption->bChainLinked : false;
		const float GearLoad = GearOption ? FMath::Clamp(GearOption->LoadFraction, 0.0f, 1.0f) : AdvancedInteractionState.DeviceLoadFraction;
		const float MotionFraction = GearOption ? FMath::Clamp(GearOption->MotionFraction, 0.0f, 1.0f) : (bStoppedGear ? 0.08f : 1.0f);
		const float ResponseWindowFraction = GearOption ? FMath::Clamp(GearOption->ResponseWindowFraction, 0.0f, 1.0f) : 0.0f;
		const FLinearColor GearColor = bStoppedGear
			? (bFailure ? FLinearColor(1.0f, 0.22f, 0.12f, 0.96f) : FLinearColor(1.0f, 0.78f, 0.28f, 0.96f))
			: (bAlreadySolvedInSequence ? FLinearColor(0.32f, 0.90f, 0.64f, 0.72f) : FLinearColor(0.56f, 0.70f, 0.72f, 0.86f));
		const float Rotation = WorldSeconds
			* (GearIndex % 2 == 0 ? 1.7f : -2.1f)
			* FMath::Clamp(MotionFraction * (1.0f - LoadDrag), 0.0f, 1.0f);

		DrawCircleLines(GearCenters[GearIndex], GearRadii[GearIndex], GearColor, 54, bStoppedGear ? 4.0f : 2.0f);
		if (GearLoad > 0.30f || AdvancedInteractionState.DeviceLoadFraction > 0.30f)
		{
			DrawCircleLines(
				GearCenters[GearIndex],
				GearRadii[GearIndex] + 10.0f + GearLoad * 14.0f,
				DeviceLoadColor(FMath::Max(GearLoad, AdvancedInteractionState.DeviceLoadFraction)).CopyWithNewOpacity(0.20f + GearLoad * 0.26f),
				54,
				2.0f);
		}
		DrawCircleLines(GearCenters[GearIndex], GearRadii[GearIndex] * 0.44f, GearColor.CopyWithNewOpacity(0.72f), 36, 2.0f);
		for (int32 ToothIndex = 0; ToothIndex < 16; ++ToothIndex)
		{
			const float Angle = Rotation + ToothIndex * TWO_PI / 16.0f;
			const FVector2D ToothCenter = GearCenters[GearIndex] + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * (GearRadii[GearIndex] + 8.0f);
			DrawRotatedRect(ToothCenter, FVector2D(10.0f, 22.0f), Angle, GearColor.CopyWithNewOpacity(bStoppedGear ? 0.95f : 0.70f));
		}

		const FVector2D SpokeA = GearCenters[GearIndex] + FVector2D(FMath::Cos(Rotation), FMath::Sin(Rotation)) * (GearRadii[GearIndex] * 0.72f);
		const FVector2D SpokeB = GearCenters[GearIndex] - FVector2D(FMath::Cos(Rotation), FMath::Sin(Rotation)) * (GearRadii[GearIndex] * 0.72f);
		const FVector2D SpokeC = GearCenters[GearIndex] + FVector2D(FMath::Cos(Rotation + HALF_PI), FMath::Sin(Rotation + HALF_PI)) * (GearRadii[GearIndex] * 0.72f);
		const FVector2D SpokeD = GearCenters[GearIndex] - FVector2D(FMath::Cos(Rotation + HALF_PI), FMath::Sin(Rotation + HALF_PI)) * (GearRadii[GearIndex] * 0.72f);
		DrawLine(SpokeA, SpokeB, GearColor.CopyWithNewOpacity(0.72f), 3.0f);
		DrawLine(SpokeC, SpokeD, GearColor.CopyWithNewOpacity(0.72f), 3.0f);

		if (bStoppedGear)
		{
			DrawCircleLines(GearCenters[GearIndex], GearRadii[GearIndex] + 18.0f + Pulse * 5.0f + ResponseWindowFraction * 8.0f, GearColor, 50, 3.0f + ResponseWindowFraction);
			if (ResponseWindowFraction > 0.05f)
			{
				const float MeterY = GearCenters[GearIndex].Y - GearRadii[GearIndex] - 28.0f;
				const float MeterWidth = 72.0f;
				FCanvasTileItem ResponseBack(
					FVector2D(GearCenters[GearIndex].X - MeterWidth * 0.5f, MeterY),
					FVector2D(MeterWidth, 5.0f),
					FLinearColor(0.06f, 0.055f, 0.045f, 0.88f));
				ResponseBack.BlendMode = SE_BLEND_Translucent;
				Canvas->DrawItem(ResponseBack);
				FCanvasTileItem ResponseFill(
					FVector2D(GearCenters[GearIndex].X - MeterWidth * 0.5f, MeterY),
					FVector2D(MeterWidth * ResponseWindowFraction, 5.0f),
					GearColor.CopyWithNewOpacity(0.86f));
				ResponseFill.BlendMode = SE_BLEND_Translucent;
				Canvas->DrawItem(ResponseFill);
			}
		}

		if (bAlreadySolvedInSequence)
		{
			DrawCircleLines(GearCenters[GearIndex], GearRadii[GearIndex] + 10.0f, FLinearColor(0.28f, 0.92f, 0.60f, 0.42f), 44, 2.0f);
			DrawLine(
				GearCenters[GearIndex] + FVector2D(-28.0f, 0.0f),
				GearCenters[GearIndex] + FVector2D(-7.0f, 23.0f),
				FLinearColor(0.28f, 1.0f, 0.62f, 0.92f),
				4.0f);
			DrawLine(
				GearCenters[GearIndex] + FVector2D(-7.0f, 23.0f),
				GearCenters[GearIndex] + FVector2D(34.0f, -26.0f),
				FLinearColor(0.28f, 1.0f, 0.62f, 0.92f),
				4.0f);
		}

		FCanvasTextItem KeyItem(
			FVector2D(GearCenters[GearIndex].X - 20.0f, GearCenters[GearIndex].Y + GearRadii[GearIndex] + 35.0f),
			AdvancedInteractionState.InputOptions.IsValidIndex(GearIndex)
				? FText::Format(
					NSLOCTEXT("Day1SliceHUD", "GearInputOptionFormat", "{0}  {1}"),
					AdvancedInteractionState.InputOptions[GearIndex].KeyHint,
					AdvancedInteractionState.InputOptions[GearIndex].DisplayLabel)
				: FText::AsNumber(GearIndex + 1),
			Font,
			bStoppedGear ? GearColor : TextSecondary());
		KeyItem.Scale = FVector2D(bStoppedGear ? 0.96f : 0.82f);
		Canvas->DrawItem(KeyItem);

		FText GearStateText = FText::GetEmpty();
		if (bStoppedGear)
		{
			GearStateText = NSLOCTEXT("Day1SliceHUD", "GearStateStalled", "停转目标");
		}
		else if (bAlreadySolvedInSequence)
		{
			GearStateText = NSLOCTEXT("Day1SliceHUD", "GearStateLinked", "已咬合");
		}
		else if (MotionFraction < 0.45f)
		{
			GearStateText = NSLOCTEXT("Day1SliceHUD", "GearStateDragging", "拖滞");
		}
		if (!GearStateText.IsEmpty())
		{
			FCanvasTextItem StateItem(
				FVector2D(GearCenters[GearIndex].X - 28.0f, GearCenters[GearIndex].Y + GearRadii[GearIndex] + 54.0f),
				GearStateText,
				Font,
				bStoppedGear ? GearColor : (bAlreadySolvedInSequence ? FLinearColor(0.28f, 1.0f, 0.62f, 0.86f) : WarningAccent()));
			StateItem.Scale = FVector2D(0.68f);
			Canvas->DrawItem(StateItem);
		}
	}

	if (AdvancedInteractionState.bPaused)
	{
		const FText PauseText = FText::Format(
			NSLOCTEXT("Day1SliceHUD", "GearPaused", "齿轮卡死：{0} 秒"),
			FText::AsNumber(FMath::CeilToInt(AdvancedInteractionState.PauseRemainingSeconds)));
		FCanvasTextItem PauseItem(
			FVector2D(Center.X - 82.0f, Center.Y - 150.0f),
			PauseText,
			Font,
			FLinearColor(1.0f, 0.26f, 0.14f, 0.96f));
		PauseItem.Scale = FVector2D(1.12f);
		Canvas->DrawItem(PauseItem);
	}

	const FText SequenceText = FText::Format(
		NSLOCTEXT("Day1SliceHUD", "GearSequenceReadout", "动态齿轮链路：驱动齿轮 > 侧向传动 > 锁止齿轮    当前第 {0} 拍"),
		FText::AsNumber(FMath::Clamp(AdvancedInteractionState.StepIndex + 1, 1, FMath::Max(1, AdvancedInteractionState.RequiredStepCount))));
	FCanvasTextItem SequenceItem(
		FVector2D(PanelX + PanelPadding, PanelY + 88.0f),
		SequenceText,
		Font,
		FLinearColor(0.88f, 0.96f, 0.84f, 0.92f));
	SequenceItem.Scale = FVector2D(0.86f);
	Canvas->DrawItem(SequenceItem);

	FCanvasTextItem ControlsItem(
		FVector2D(PanelX + PanelPadding, PanelY + PanelHeight - 52.0f),
		BuildAdvancedInteractionControlHint(),
		Font,
		TextSecondary());
	ControlsItem.Scale = FVector2D(0.90f);
	Canvas->DrawItem(ControlsItem);
}

void ADay1SliceHUD::DrawSpectralScanPanel(float PanelX, float PanelY, float PanelWidth, float PanelHeight, UFont* Font, float WorldSeconds)
{
	// 在面板顶部显示操作说明
	const FText HintText = NSLOCTEXT("Day1SliceHUD", "SpectralScanHint", "操作说明：按 A/S/D 选择频段  |  等待光标进入绿色区域  |  按互动键锁定信号");
	const FLinearColor HintColor = FLinearColor(0.85f, 0.95f, 1.0f, 0.92f);
	FCanvasTextItem HintItem(
		FVector2D(PanelX + PanelPadding, PanelY + 80.0f),
		HintText,
		Font,
		HintColor);
	HintItem.Scale = FVector2D(0.88f);
	Canvas->DrawItem(HintItem);

	const FVector2D Center(PanelX + PanelWidth * 0.5f, PanelY + 265.0f);
	const float ScopeWidth = PanelWidth - 150.0f;
	const float ScopeHeight = 230.0f;
	const float ScopeX = Center.X - ScopeWidth * 0.5f;
	const float ScopeY = Center.Y - ScopeHeight * 0.5f;
	const float Pulse = 0.5f + 0.5f * FMath::Sin(WorldSeconds * 8.0f);
	const int32 TargetIndex = IndexForAdvancedInputOption(AdvancedInteractionState, AdvancedInteractionState.ExpectedInputId);
	const FLinearColor Accent = AccentForAdvancedMode(EHorrorCampaignInteractionMode::SpectralScan);
	const float NoiseAmplitude = 0.16f + AdvancedInteractionState.SpectralNoiseFraction * 0.72f + AdvancedInteractionState.DeviceLoadFraction * 0.32f;
	const float SignalAmplitude = 28.0f + AdvancedInteractionState.InputPrecisionFraction * 26.0f;
	const FLinearColor ConfidenceColor = AdvancedInteractionState.SpectralConfidenceFraction >= 0.62f
		? FLinearColor(0.34f, 1.0f, 0.78f, 0.94f)
		: WarningAccent();

	FCanvasTileItem ScopeBack(FVector2D(ScopeX, ScopeY), FVector2D(ScopeWidth, ScopeHeight), FLinearColor(0.012f, 0.026f, 0.034f, 0.92f));
	ScopeBack.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(ScopeBack);

	for (int32 GridIndex = 1; GridIndex < 6; ++GridIndex)
	{
		const float GridX = ScopeX + ScopeWidth * static_cast<float>(GridIndex) / 6.0f;
		DrawLine(FVector2D(GridX, ScopeY), FVector2D(GridX, ScopeY + ScopeHeight), FLinearColor(0.08f, 0.22f, 0.26f, 0.55f), 1.0f);
	}
	for (int32 GridIndex = 1; GridIndex < 4; ++GridIndex)
	{
		const float GridY = ScopeY + ScopeHeight * static_cast<float>(GridIndex) / 4.0f;
		DrawLine(FVector2D(ScopeX, GridY), FVector2D(ScopeX + ScopeWidth, GridY), FLinearColor(0.08f, 0.22f, 0.26f, 0.55f), 1.0f);
	}

	const float WindowX = ScopeX + ScopeWidth * AdvancedWindowStart(AdvancedInteractionState);
	const float WindowW = ScopeWidth * (AdvancedWindowEnd(AdvancedInteractionState) - AdvancedWindowStart(AdvancedInteractionState));
	FCanvasTileItem WindowTile(
		FVector2D(WindowX, ScopeY),
		FVector2D(WindowW, ScopeHeight),
		AdvancedInteractionState.bTimingWindowOpen ? FLinearColor(0.24f, 0.88f, 0.78f, 0.22f) : FLinearColor(0.18f, 0.38f, 0.42f, 0.18f));
	WindowTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(WindowTile);

	const int32 WaveSegments = 72;
	FVector2D PreviousPoint(ScopeX, Center.Y);
	for (int32 SegmentIndex = 1; SegmentIndex <= WaveSegments; ++SegmentIndex)
	{
		const float Alpha = static_cast<float>(SegmentIndex) / static_cast<float>(WaveSegments);
		const float X = ScopeX + ScopeWidth * Alpha;
		const float Carrier = FMath::Sin(Alpha * UE_TWO_PI * (3.0f + TargetIndex) + WorldSeconds * 3.5f);
		const float Noise = FMath::Sin(Alpha * UE_TWO_PI * 13.0f - WorldSeconds * 7.0f) * NoiseAmplitude;
		const float Y = Center.Y - (Carrier + Noise) * (SignalAmplitude + TargetIndex * 12.0f);
		const FVector2D Point(X, Y);
		DrawLine(PreviousPoint, Point, Accent.CopyWithNewOpacity(0.48f + AdvancedInteractionState.InputPrecisionFraction * 0.32f + Pulse * 0.18f), AdvancedInteractionState.bTimingWindowOpen ? 3.0f : 2.0f);
		PreviousPoint = Point;
	}

	if (AdvancedInteractionState.SpectralNoiseFraction > 0.05f || AdvancedInteractionState.DeviceLoadFraction > 0.35f)
	{
		const int32 NoiseBars = FMath::Clamp(FMath::RoundToInt((AdvancedInteractionState.SpectralNoiseFraction + AdvancedInteractionState.DeviceLoadFraction) * 10.0f), 1, 14);
		for (int32 NoiseIndex = 0; NoiseIndex < NoiseBars; ++NoiseIndex)
		{
			const float Alpha = FMath::Frac(WorldSeconds * 0.31f + NoiseIndex * 0.173f);
			const float X = ScopeX + ScopeWidth * Alpha;
			DrawLine(
				FVector2D(X, ScopeY + 8.0f),
				FVector2D(X + 18.0f, ScopeY + ScopeHeight - 8.0f),
				DeviceLoadColor(AdvancedInteractionState.DeviceLoadFraction).CopyWithNewOpacity(0.08f + AdvancedInteractionState.SpectralNoiseFraction * 0.18f),
				1.0f + AdvancedInteractionState.DeviceLoadFraction * 2.0f);
		}
	}

	const float CursorX = ScopeX + ScopeWidth * AdvancedInteractionState.TimingFraction;
	DrawLine(FVector2D(CursorX, ScopeY - 14.0f), FVector2D(CursorX, ScopeY + ScopeHeight + 14.0f), FLinearColor(0.88f, 1.0f, 0.96f, 0.96f), 3.0f);
	DrawCircleLines(FVector2D(CursorX, Center.Y), 14.0f + Pulse * 5.0f, Accent, 24, 2.0f);

	const float ConfidenceY = ScopeY - 26.0f;
	const float ConfidenceWidth = ScopeWidth * 0.58f;
	FCanvasTileItem ConfidenceBack(
		FVector2D(ScopeX, ConfidenceY),
		FVector2D(ConfidenceWidth, 8.0f),
		FLinearColor(0.035f, 0.052f, 0.052f, 0.88f));
	ConfidenceBack.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(ConfidenceBack);
	FCanvasTileItem ConfidenceFill(
		FVector2D(ScopeX, ConfidenceY),
		FVector2D(ConfidenceWidth * AdvancedInteractionState.SpectralConfidenceFraction, 8.0f),
		ConfidenceColor.CopyWithNewOpacity(0.86f));
	ConfidenceFill.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(ConfidenceFill);
	const FText ConfidenceText = FText::Format(
		NSLOCTEXT("Day1SliceHUD", "SpectralScanConfidenceReadout", "峰值置信 {0}%    噪声 {1}%"),
		FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.SpectralConfidenceFraction * 100.0f)),
		FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.SpectralNoiseFraction * 100.0f)));
	FCanvasTextItem ConfidenceItem(
		FVector2D(ScopeX + ConfidenceWidth + 14.0f, ConfidenceY - 5.0f),
		ConfidenceText,
		Font,
		ConfidenceColor);
	ConfidenceItem.Scale = FVector2D(0.72f);
	Canvas->DrawItem(ConfidenceItem);

	for (int32 OptionIndex = 0; OptionIndex < FMath::Min(AdvancedInteractionState.InputOptions.Num(), 3); ++OptionIndex)
	{
		const FHorrorAdvancedInteractionInputOption& Option = AdvancedInteractionState.InputOptions[OptionIndex];
		const bool bTarget = Option.InputId == AdvancedInteractionState.ExpectedInputId;
		const float BandY = ScopeY + ScopeHeight + 28.0f + OptionIndex * 24.0f;
		const FLinearColor BandColor = Option.VisualColor;
		const float OptionLoad = FMath::Clamp(Option.LoadFraction, 0.0f, 1.0f);
		const float ResponseWindowFraction = FMath::Clamp(Option.ResponseWindowFraction, 0.0f, 1.0f);
		const float MotionFraction = FMath::Clamp(Option.MotionFraction, 0.0f, 1.0f);
		const FLinearColor BandStateColor = Option.bHazardous
			? FLinearColor(1.0f, 0.30f, 0.16f, 0.96f)
			: (bTarget ? ConfidenceColor : BandColor);
		DrawLine(FVector2D(ScopeX + OptionIndex * 110.0f, BandY), FVector2D(ScopeX + OptionIndex * 110.0f + 84.0f, BandY), BandColor.CopyWithNewOpacity(bTarget ? 0.96f : 0.48f), bTarget ? 4.0f : 2.0f);
		const float PeakX = ScopeX + OptionIndex * 110.0f + 42.0f;
		const float PeakHeight = 18.0f + ResponseWindowFraction * 46.0f + MotionFraction * 12.0f;
		DrawLine(
			FVector2D(PeakX, BandY - 4.0f),
			FVector2D(PeakX, BandY - PeakHeight),
			BandStateColor.CopyWithNewOpacity(0.42f + ResponseWindowFraction * 0.48f),
			3.0f + ResponseWindowFraction * 3.0f);
		DrawCircleLines(
			FVector2D(PeakX, BandY - PeakHeight),
			6.0f + Pulse * 3.0f + ResponseWindowFraction * 8.0f,
			BandStateColor.CopyWithNewOpacity(0.46f + ResponseWindowFraction * 0.42f),
			18,
			1.5f + ResponseWindowFraction * 2.0f);
		if (OptionLoad > 0.45f)
		{
			DrawLine(
				FVector2D(ScopeX + OptionIndex * 110.0f, BandY - 7.0f),
				FVector2D(ScopeX + OptionIndex * 110.0f + 84.0f, BandY - 7.0f),
				(Option.bHazardous ? FLinearColor(1.0f, 0.22f, 0.12f, 0.92f) : DeviceLoadColor(OptionLoad)).CopyWithNewOpacity(0.32f + OptionLoad * 0.36f),
				1.0f + OptionLoad * 3.0f);
		}
		const float MeterY = BandY + 2.0f;
		const float MeterWidth = 84.0f;
		FCanvasTileItem ResponseBack(
			FVector2D(ScopeX + OptionIndex * 110.0f, MeterY),
			FVector2D(MeterWidth, 4.0f),
			FLinearColor(0.04f, 0.052f, 0.052f, 0.84f));
		ResponseBack.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(ResponseBack);
		FCanvasTileItem ResponseFill(
			FVector2D(ScopeX + OptionIndex * 110.0f, MeterY),
			FVector2D(MeterWidth * ResponseWindowFraction, 4.0f),
			BandStateColor.CopyWithNewOpacity(0.82f));
		ResponseFill.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(ResponseFill);
		FCanvasTextItem BandItem(
			FVector2D(ScopeX + OptionIndex * 110.0f, BandY + 10.0f),
			FText::Format(NSLOCTEXT("Day1SliceHUD", "SpectralBandOptionFormat", "{0}  {1}"), Option.KeyHint, Option.DisplayLabel),
			Font,
			Option.bHazardous ? FLinearColor(1.0f, 0.30f, 0.16f, 0.96f) : (bTarget ? BandStateColor : TextSecondary()));
		BandItem.Scale = FVector2D(bTarget ? 0.90f : 0.76f);
		Canvas->DrawItem(BandItem);
		FText BandStateText = FText::GetEmpty();
		if (Option.bHazardous)
		{
			BandStateText = NSLOCTEXT("Day1SliceHUD", "SpectralBandNoiseWarning", "噪声峰");
		}
		else if (bTarget && ResponseWindowFraction >= 0.62f)
		{
			BandStateText = NSLOCTEXT("Day1SliceHUD", "SpectralBandLockReady", "锁定峰值");
		}
		else if (bTarget)
		{
			BandStateText = NSLOCTEXT("Day1SliceHUD", "SpectralBandBuilding", "峰值不稳");
		}
		if (!BandStateText.IsEmpty())
		{
			FCanvasTextItem StateItem(
				FVector2D(ScopeX + OptionIndex * 110.0f, BandY + 26.0f),
				BandStateText,
				Font,
				Option.bHazardous ? FLinearColor(1.0f, 0.30f, 0.16f, 0.88f) : BandStateColor);
			StateItem.Scale = FVector2D(0.64f);
			Canvas->DrawItem(StateItem);
		}
	}

	FCanvasTextItem ControlsItem(
		FVector2D(PanelX + PanelPadding, PanelY + PanelHeight - 52.0f),
		BuildAdvancedInteractionControlHint(),
		Font,
		TextSecondary());
	ControlsItem.Scale = FVector2D(0.90f);
	Canvas->DrawItem(ControlsItem);
}

void ADay1SliceHUD::DrawSignalTuningPanel(float PanelX, float PanelY, float PanelWidth, float PanelHeight, UFont* Font, float WorldSeconds)
{
	const FVector2D Center(PanelX + PanelWidth * 0.5f, PanelY + 260.0f);
	const float Radius = 128.0f;
	const float Pulse = 0.5f + 0.5f * FMath::Sin(WorldSeconds * 6.5f);
	const FLinearColor Accent = AccentForAdvancedMode(EHorrorCampaignInteractionMode::SignalTuning);
	const int32 TargetIndex = IndexForAdvancedInputOption(AdvancedInteractionState, AdvancedInteractionState.ExpectedInputId);
	const float PrecisionRadius = Radius * (0.50f + AdvancedInteractionState.InputPrecisionFraction * 0.26f);
	const FLinearColor LoadColor = DeviceLoadColor(AdvancedInteractionState.DeviceLoadFraction);

	DrawCircleLines(Center, Radius + 30.0f, FLinearColor(0.05f, 0.16f, 0.14f, 0.82f), 80, 2.0f);
	DrawCircleLines(Center, Radius, Accent.CopyWithNewOpacity(0.68f), 80, 3.0f);
	DrawCircleLines(Center, PrecisionRadius, FLinearColor(0.12f, 0.32f, 0.28f, 0.82f), 64, 2.0f + AdvancedInteractionState.InputPrecisionFraction);
	if (AdvancedInteractionState.DeviceLoadFraction > 0.24f)
	{
		DrawCircleLines(Center, Radius + 46.0f + Pulse * 12.0f, LoadColor.CopyWithNewOpacity(0.16f + AdvancedInteractionState.DeviceLoadFraction * 0.28f), 80, 2.0f + AdvancedInteractionState.DeviceLoadFraction * 3.0f);
	}

	const float WindowStartAngle = FMath::Lerp(-PI, PI, AdvancedWindowStart(AdvancedInteractionState));
	const float WindowEndAngle = FMath::Lerp(-PI, PI, AdvancedWindowEnd(AdvancedInteractionState));
	FVector2D PreviousWindow = Center + FVector2D(FMath::Cos(WindowStartAngle), FMath::Sin(WindowStartAngle)) * (Radius + 16.0f);
	for (int32 SegmentIndex = 1; SegmentIndex <= 24; ++SegmentIndex)
	{
		const float Alpha = static_cast<float>(SegmentIndex) / 24.0f;
		const float Angle = FMath::Lerp(WindowStartAngle, WindowEndAngle, Alpha);
		const FVector2D Point = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * (Radius + 16.0f);
		DrawLine(PreviousWindow, Point, AdvancedInteractionState.bTimingWindowOpen ? Accent : Accent.CopyWithNewOpacity(0.36f), 7.0f);
		PreviousWindow = Point;
	}

	const float NeedleAngle = FMath::Lerp(-PI, PI, AdvancedInteractionState.TimingFraction);
	const FVector2D NeedleEnd = Center + FVector2D(FMath::Cos(NeedleAngle), FMath::Sin(NeedleAngle)) * (Radius + 8.0f);
	DrawLine(Center, NeedleEnd, FLinearColor(0.98f, 1.0f, 0.84f, 0.96f), 4.0f);
	DrawCircleLines(NeedleEnd, 12.0f + Pulse * 4.0f, Accent, 20, 2.0f);

	const float BalanceLineX = Center.X - Radius + Radius * 2.0f * AdvancedInteractionState.SignalBalanceFraction;
	const float TargetLineX = Center.X - Radius + Radius * 2.0f * AdvancedInteractionState.SignalTargetBalanceFraction;
	const float BalanceError = FMath::Abs(AdvancedInteractionState.SignalBalanceFraction - AdvancedInteractionState.SignalTargetBalanceFraction);
	const bool bSignalCentered = BalanceError <= 0.055f;
	const FLinearColor BalanceStateColor = bSignalCentered
		? FLinearColor(0.40f, 1.0f, 0.78f, 0.94f)
		: DeviceLoadColor(FMath::Max(AdvancedInteractionState.DeviceLoadFraction, BalanceError * 2.4f));
	DrawLine(FVector2D(TargetLineX, Center.Y - Radius * 0.82f), FVector2D(TargetLineX, Center.Y + Radius * 0.82f), FLinearColor(1.0f, 0.86f, 0.32f, 0.72f), 4.0f);
	DrawLine(
		FVector2D(BalanceLineX, Center.Y - Radius * 0.64f),
		FVector2D(BalanceLineX, Center.Y + Radius * 0.64f),
		AdvancedInteractionState.InputPrecisionFraction > 0.72f ? FLinearColor(0.78f, 1.0f, 0.94f, 0.92f) : BalanceStateColor,
		5.0f + AdvancedInteractionState.DeviceLoadFraction * 2.0f);
	DrawLine(
		FVector2D(BalanceLineX, Center.Y - Radius * 0.72f),
		FVector2D(TargetLineX, Center.Y - Radius * 0.72f),
		BalanceStateColor.CopyWithNewOpacity(0.38f + BalanceError * 1.2f),
		2.0f + BalanceError * 10.0f);

	const float MixBarX = Center.X - Radius;
	const float MixBarY = Center.Y + Radius + 34.0f;
	const float MixBarWidth = Radius * 2.0f;
	const float MixBarHeight = 10.0f;
	FCanvasTileItem MixBack(
		FVector2D(MixBarX, MixBarY),
		FVector2D(MixBarWidth, MixBarHeight),
		FLinearColor(0.05f, 0.065f, 0.068f, 0.88f));
	MixBack.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(MixBack);
	FCanvasTileItem MixFill(
		FVector2D(MixBarX, MixBarY),
		FVector2D(MixBarWidth * AdvancedInteractionState.SignalBalanceFraction, MixBarHeight),
		BalanceStateColor.CopyWithNewOpacity(0.82f));
	MixFill.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(MixFill);
	DrawLine(
		FVector2D(MixBarX + MixBarWidth * AdvancedInteractionState.SignalTargetBalanceFraction, MixBarY - 5.0f),
		FVector2D(MixBarX + MixBarWidth * AdvancedInteractionState.SignalTargetBalanceFraction, MixBarY + MixBarHeight + 5.0f),
		FLinearColor(1.0f, 0.86f, 0.32f, 0.84f),
		3.0f);
	const FText MixText = FText::Format(
		NSLOCTEXT("Day1SliceHUD", "SignalTuningBalanceReadout", "声像偏移 {0}%    {1}"),
		FText::AsNumber(FMath::RoundToInt((AdvancedInteractionState.SignalBalanceFraction - AdvancedInteractionState.SignalTargetBalanceFraction) * 100.0f)),
		bSignalCentered
			? NSLOCTEXT("Day1SliceHUD", "SignalTuningCentered", "可锁定")
			: NSLOCTEXT("Day1SliceHUD", "SignalTuningOffCenter", "未居中"));
	FCanvasTextItem MixItem(
		FVector2D(MixBarX, MixBarY + 14.0f),
		MixText,
		Font,
		bSignalCentered ? FLinearColor(0.40f, 1.0f, 0.78f, 0.90f) : WarningAccent());
	MixItem.Scale = FVector2D(0.76f);
	Canvas->DrawItem(MixItem);

	for (int32 OptionIndex = 0; OptionIndex < FMath::Min(AdvancedInteractionState.InputOptions.Num(), 3); ++OptionIndex)
	{
		const FHorrorAdvancedInteractionInputOption& Option = AdvancedInteractionState.InputOptions[OptionIndex];
		const float Angle = -PI * 0.72f + OptionIndex * PI * 0.72f;
		const FVector2D OptionCenter = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * (Radius + 92.0f);
		const bool bTarget = OptionIndex == TargetIndex;
		const float OptionLoad = FMath::Clamp(Option.LoadFraction, 0.0f, 1.0f);
		const float MotionFraction = FMath::Clamp(Option.MotionFraction, 0.0f, 1.0f);
		const float ResponseWindowFraction = FMath::Clamp(Option.ResponseWindowFraction, 0.0f, 1.0f);
		const bool bCenterLock = Option.InputId == FName(TEXT("中心频率"));
		const bool bStrongResponse = ResponseWindowFraction > 0.55f;
		const FLinearColor OptionStateColor = Option.bHazardous
			? FLinearColor(1.0f, 0.34f, 0.16f, 0.96f)
			: (bStrongResponse ? FLinearColor(0.34f, 1.0f, 0.78f, 0.96f) : Option.VisualColor);
		if (OptionLoad > 0.35f)
		{
			DrawCircleLines(
				OptionCenter,
				31.0f + Pulse * 5.0f + OptionLoad * 12.0f + ResponseWindowFraction * 6.0f,
				(Option.bHazardous ? FLinearColor(1.0f, 0.22f, 0.12f, 0.92f) : LoadColor).CopyWithNewOpacity(0.22f + OptionLoad * 0.36f),
				28,
				2.0f + OptionLoad * 2.0f);
		}
		if (ResponseWindowFraction > 0.04f)
		{
			DrawCircleLines(
				OptionCenter,
				29.0f + Pulse * 4.0f + ResponseWindowFraction * 16.0f,
				OptionStateColor.CopyWithNewOpacity(0.26f + ResponseWindowFraction * 0.50f),
				30,
				1.6f + ResponseWindowFraction * 2.4f);
		}
		DrawCircleLines(
			OptionCenter,
			bTarget ? 29.0f + Pulse * 4.0f + MotionFraction * 5.0f : 23.0f + MotionFraction * 3.0f,
			OptionStateColor.CopyWithNewOpacity(bTarget || bStrongResponse ? 0.96f : 0.52f),
			28,
			bTarget || bStrongResponse ? 3.0f : 2.0f);
		const float MeterWidth = 74.0f;
		const float MeterY = OptionCenter.Y + 20.0f;
		FCanvasTileItem ResponseBack(
			FVector2D(OptionCenter.X - MeterWidth * 0.5f, MeterY),
			FVector2D(MeterWidth, 5.0f),
			FLinearColor(0.04f, 0.052f, 0.052f, 0.84f));
		ResponseBack.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(ResponseBack);
		FCanvasTileItem ResponseFill(
			FVector2D(OptionCenter.X - MeterWidth * 0.5f, MeterY),
			FVector2D(MeterWidth * ResponseWindowFraction, 5.0f),
			OptionStateColor.CopyWithNewOpacity(0.86f));
		ResponseFill.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(ResponseFill);
		if (!bCenterLock && MotionFraction > 0.08f)
		{
			const float ArrowDirection = Option.InputId == FName(TEXT("左声道")) ? -1.0f : 1.0f;
			const FVector2D ArrowStart = OptionCenter + FVector2D(-18.0f * ArrowDirection, -24.0f);
			const FVector2D ArrowEnd = OptionCenter + FVector2D((18.0f + MotionFraction * 10.0f) * ArrowDirection, -24.0f);
			DrawLine(ArrowStart, ArrowEnd, OptionStateColor.CopyWithNewOpacity(0.58f + MotionFraction * 0.34f), 3.0f + MotionFraction * 2.0f);
			DrawLine(ArrowEnd, ArrowEnd + FVector2D(-8.0f * ArrowDirection, -6.0f), OptionStateColor.CopyWithNewOpacity(0.82f), 3.0f);
			DrawLine(ArrowEnd, ArrowEnd + FVector2D(-8.0f * ArrowDirection, 6.0f), OptionStateColor.CopyWithNewOpacity(0.82f), 3.0f);
		}
		FCanvasTextItem OptionItem(
			FVector2D(OptionCenter.X - 38.0f, OptionCenter.Y + 34.0f),
			FText::Format(NSLOCTEXT("Day1SliceHUD", "SignalTuningOptionFormat", "{0}  {1}"), Option.KeyHint, Option.DisplayLabel),
			Font,
			Option.bHazardous ? FLinearColor(1.0f, 0.34f, 0.16f, 0.96f) : ((bTarget || bStrongResponse) ? OptionStateColor : TextSecondary()));
		OptionItem.Scale = FVector2D((bTarget || bStrongResponse) ? 0.86f : 0.72f);
		Canvas->DrawItem(OptionItem);
		FText StateText = FText::GetEmpty();
		if (Option.bHazardous)
		{
			StateText = NSLOCTEXT("Day1SliceHUD", "SignalTuningUnsafeLock", "未居中");
		}
		else if (bCenterLock && bStrongResponse)
		{
			StateText = NSLOCTEXT("Day1SliceHUD", "SignalTuningLockReady", "锁定就绪");
		}
		else if (!bCenterLock && bStrongResponse)
		{
			StateText = NSLOCTEXT("Day1SliceHUD", "SignalTuningCorrectiveNudge", "纠偏方向");
		}
		if (!StateText.IsEmpty())
		{
			FCanvasTextItem StateItem(
				FVector2D(OptionCenter.X - 34.0f, OptionCenter.Y + 50.0f),
				StateText,
				Font,
				Option.bHazardous ? FLinearColor(1.0f, 0.34f, 0.16f, 0.88f) : OptionStateColor);
			StateItem.Scale = FVector2D(0.64f);
			Canvas->DrawItem(StateItem);
		}
	}

	FCanvasTextItem ControlsItem(
		FVector2D(PanelX + PanelPadding, PanelY + PanelHeight - 52.0f),
		BuildAdvancedInteractionControlHint(),
		Font,
		TextSecondary());
	ControlsItem.Scale = FVector2D(0.90f);
	Canvas->DrawItem(ControlsItem);
}

void ADay1SliceHUD::DrawAdvancedInteractionProgress(float PanelX, float PanelY, float PanelWidth, UFont* Font, const FLinearColor& AccentColor)
{
	const float BarX = PanelX + PanelPadding;
	const float BarY = PanelY + 38.0f;
	const float BarWidth = PanelWidth - PanelPadding * 2.0f;
	const float BarHeight = 14.0f;
	FCanvasTileItem ProgressBack(FVector2D(BarX, BarY), FVector2D(BarWidth, BarHeight), FLinearColor(0.05f, 0.06f, 0.065f, 0.94f));
	ProgressBack.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(ProgressBack);

	FCanvasTileItem ProgressFill(
		FVector2D(BarX, BarY),
		FVector2D(BarWidth * AdvancedInteractionState.ProgressFraction, BarHeight),
		AccentColor);
	ProgressFill.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(ProgressFill);

	const FLinearColor FeedbackColor = AdvancedFeedbackColor(AdvancedInteractionState.FeedbackState, AdvancedInteractionState.FeedbackText, AccentColor);
	const FText Feedback = AdvancedInteractionState.FeedbackText.IsEmpty()
		? (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::CircuitWiring
			? NSLOCTEXT("Day1SliceHUD", "CircuitHUDHint", "观察旋转光标，进入发光窗口后接入高亮端子。")
			: (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::GearCalibration
				? NSLOCTEXT("Day1SliceHUD", "GearHUDHint", "观察停转齿轮，进入校准窗口后拨动对应齿轮。")
				: (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::SpectralScan
					? NSLOCTEXT("Day1SliceHUD", "SpectralScanHUDHint", "观察频谱波形，进入锁定带后选择对应波段。")
					: NSLOCTEXT("Day1SliceHUD", "SignalTuningHUDHint", "观察调谐指针，进入同步带后选择对应通道。"))))
		: AdvancedInteractionState.FeedbackText;
	FCanvasTextItem FeedbackItem(
		FVector2D(BarX, PanelY + 4.0f),
		Feedback,
		Font,
		FeedbackColor);
	FeedbackItem.Scale = FVector2D(0.94f);
	Canvas->DrawItem(FeedbackItem);

	const float StabilityBarY = BarY + 24.0f;
	FCanvasTileItem StabilityBack(
		FVector2D(BarX, StabilityBarY),
		FVector2D(BarWidth, 8.0f),
		FLinearColor(0.07f, 0.055f, 0.055f, 0.92f));
	StabilityBack.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(StabilityBack);
	FCanvasTileItem StabilityFill(
		FVector2D(BarX, StabilityBarY),
		FVector2D(BarWidth * FMath::Clamp(AdvancedInteractionState.StabilityFraction, 0.0f, 1.0f), 8.0f),
		StabilityColor(AdvancedInteractionState.StabilityFraction));
	StabilityFill.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(StabilityFill);

	const FText ProgressText = FText::Format(
		NSLOCTEXT("Day1SliceHUD", "AdvancedProgressCompact", "同步 {0}%    精度 {1}%    负载 {2}%    稳定 {3}%    评分 {4}%    连击 {5}    失误 {6}"),
		FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.ProgressFraction * 100.0f)),
		FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.InputPrecisionFraction * 100.0f)),
		FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.DeviceLoadFraction * 100.0f)),
		FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.StabilityFraction * 100.0f)),
		FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.PerformanceGradeFraction * 100.0f)),
		FText::AsNumber(AdvancedInteractionState.ComboCount),
		FText::AsNumber(AdvancedInteractionState.MistakeCount));
	FCanvasTextItem ProgressTextItem(
		FVector2D(BarX, StabilityBarY + 12.0f),
		ProgressText,
		Font,
		PerformanceGradeColor(AdvancedInteractionState.PerformanceGradeFraction));
	ProgressTextItem.Scale = FVector2D(0.84f);
	Canvas->DrawItem(ProgressTextItem);

	const float PrecisionBarY = StabilityBarY + 25.0f;
	const float MiniBarWidth = BarWidth * 0.48f;
	FCanvasTileItem PrecisionBack(
		FVector2D(BarX, PrecisionBarY),
		FVector2D(MiniBarWidth, 5.0f),
		FLinearColor(0.045f, 0.070f, 0.074f, 0.92f));
	PrecisionBack.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(PrecisionBack);
	FCanvasTileItem PrecisionFill(
		FVector2D(BarX, PrecisionBarY),
		FVector2D(MiniBarWidth * AdvancedInteractionState.InputPrecisionFraction, 5.0f),
		PerformanceGradeColor(AdvancedInteractionState.InputPrecisionFraction));
	PrecisionFill.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(PrecisionFill);

	const float LoadBarX = BarX + BarWidth - MiniBarWidth;
	FCanvasTileItem LoadBack(
		FVector2D(LoadBarX, PrecisionBarY),
		FVector2D(MiniBarWidth, 5.0f),
		FLinearColor(0.075f, 0.055f, 0.055f, 0.92f));
	LoadBack.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(LoadBack);
	FCanvasTileItem LoadFill(
		FVector2D(LoadBarX, PrecisionBarY),
		FVector2D(MiniBarWidth * AdvancedInteractionState.DeviceLoadFraction, 5.0f),
		DeviceLoadColor(AdvancedInteractionState.DeviceLoadFraction));
	LoadFill.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(LoadFill);

	if (!AdvancedInteractionState.PhaseText.IsEmpty() || !AdvancedInteractionState.ExpectedInputLabel.IsEmpty())
	{
		const FText PhaseText = AdvancedInteractionState.ExpectedInputLabel.IsEmpty()
			? AdvancedInteractionState.PhaseText
			: FText::Format(
				NSLOCTEXT("Day1SliceHUD", "AdvancedPhaseWithTarget", "{0}    目标：{1}"),
				AdvancedInteractionState.PhaseText,
				AdvancedInteractionState.ExpectedInputLabel);
		FCanvasTextItem PhaseItem(
			FVector2D(BarX, StabilityBarY + 38.0f),
			PhaseText,
			Font,
			AdvancedFeedbackColor(AdvancedInteractionState.FeedbackState, AdvancedInteractionState.FeedbackText, AccentColor));
		PhaseItem.Scale = FVector2D(0.84f);
		Canvas->DrawItem(PhaseItem);
	}

	if (!AdvancedInteractionState.NextActionLabel.IsEmpty())
	{
		FCanvasTextItem NextActionItem(
			FVector2D(BarX, StabilityBarY + 60.0f),
			AdvancedInteractionState.NextActionLabel,
			Font,
			TextPrimary());
		NextActionItem.Scale = FVector2D(0.82f);
		Canvas->DrawItem(NextActionItem);
	}

	if (!AdvancedInteractionState.FailureRecoveryLabel.IsEmpty())
	{
		FCanvasTextItem RecoveryItem(
			FVector2D(BarX + BarWidth * 0.50f, StabilityBarY + 60.0f),
			AdvancedInteractionState.FailureRecoveryLabel,
			Font,
			WarningAccent());
		RecoveryItem.Scale = FVector2D(0.78f);
		Canvas->DrawItem(RecoveryItem);
	}
}

FText ADay1SliceHUD::BuildAdvancedInteractionControlHint() const
{
	const FText OptionSummary = BuildAdvancedInteractionOptionSummary(AdvancedInteractionState);
	if (!OptionSummary.IsEmpty())
	{
		if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::CircuitWiring)
		{
			return FText::Format(
				NSLOCTEXT("Day1SliceHUD", "CircuitControlsWithOptions", "{0}    A/S/D：接入端子，避开红色高负载    互动键：显示选择提示"),
				OptionSummary);
		}

		if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::GearCalibration)
		{
			return FText::Format(
				NSLOCTEXT("Day1SliceHUD", "GearControlsWithOptions", "{0}    A/S/D：拨动齿轮    互动键：显示选择提示"),
				OptionSummary);
		}

		if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::SpectralScan)
		{
			return FText::Format(
				NSLOCTEXT("Day1SliceHUD", "SpectralScanControlsWithOptions", "{0}    A/D：主动扫频滤噪    S：锁定异常峰    互动键：显示选择提示"),
				OptionSummary);
		}

		if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::SignalTuning)
		{
			return FText::Format(
				NSLOCTEXT("Day1SliceHUD", "SignalTuningControlsWithOptions", "{0}    A/S/D：仪器热键    A/D：连续微调左右声道    S：锁定中心频率    互动键：显示选择提示"),
				OptionSummary);
		}

		return FText::Format(
			NSLOCTEXT("Day1SliceHUD", "AdvancedControlsWithOptions", "{0}    A/S/D：选择目标    互动键：显示选择提示"),
			OptionSummary);
	}

	if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::CircuitWiring)
	{
		return NSLOCTEXT("Day1SliceHUD", "CircuitControls", "A/S/D：接入端子，避开红色高负载    互动键：显示选择提示");
	}

	if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::GearCalibration)
	{
		return NSLOCTEXT("Day1SliceHUD", "GearControls", "A/S/D：拨动对应齿轮    互动键：显示选择提示");
	}

	if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::SpectralScan)
	{
		return NSLOCTEXT("Day1SliceHUD", "SpectralScanControls", "A/D：主动扫频滤噪    S：锁定异常峰    互动键：显示选择提示");
	}

	if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::SignalTuning)
	{
		return NSLOCTEXT("Day1SliceHUD", "SignalTuningControls", "A/S/D：仪器热键    A/D：连续微调左右声道    S：锁定中心频率    互动键：显示选择提示");
	}

	return NSLOCTEXT("Day1SliceHUD", "AdvancedControls", "A/S/D：选择目标    互动键：显示选择提示");
}

FText ADay1SliceHUD::BuildAdvancedInteractionTacticalSummary() const
{
	if (!AdvancedInteractionState.bVisible)
	{
		return FText::GetEmpty();
	}

	FText ModeText;
	switch (AdvancedInteractionState.Mode)
	{
	case EHorrorCampaignInteractionMode::CircuitWiring:
		ModeText = NSLOCTEXT("Day1SliceHUD", "AdvancedSummaryCircuit", "电路接线");
		break;
	case EHorrorCampaignInteractionMode::GearCalibration:
		ModeText = NSLOCTEXT("Day1SliceHUD", "AdvancedSummaryGear", "齿轮校准");
		break;
	case EHorrorCampaignInteractionMode::SpectralScan:
		ModeText = NSLOCTEXT("Day1SliceHUD", "AdvancedSummaryScan", "频谱扫描");
		break;
	case EHorrorCampaignInteractionMode::SignalTuning:
		ModeText = NSLOCTEXT("Day1SliceHUD", "AdvancedSummarySignal", "信号调谐");
		break;
	default:
		ModeText = NSLOCTEXT("Day1SliceHUD", "AdvancedSummaryInteraction", "交互任务");
		break;
	}

	const FText TargetText = AdvancedInteractionState.ExpectedInputLabel.IsEmpty()
		? FText::FromName(AdvancedInteractionState.ExpectedInputId)
		: AdvancedInteractionState.ExpectedInputLabel;
	const int32 TimingStartPercent = FMath::RoundToInt(AdvancedWindowStart(AdvancedInteractionState) * 100.0f);
	const int32 TimingEndPercent = FMath::RoundToInt(AdvancedWindowEnd(AdvancedInteractionState) * 100.0f);
	if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::SpectralScan)
	{
		const bool bLockConfident = AdvancedInteractionState.SpectralConfidenceFraction >= 0.62f
			&& AdvancedInteractionState.SpectralNoiseFraction <= 0.45f;
		const FText LockText = bLockConfident
			? NSLOCTEXT("Day1SliceHUD", "SpectralScanSummaryLockReady", "可锁定")
			: NSLOCTEXT("Day1SliceHUD", "SpectralScanSummaryLockWeak", "不稳定");
		return FText::Format(
			NSLOCTEXT(
				"Day1SliceHUD",
				"AdvancedTacticalSummarySpectralScan",
				"异常频谱仪  |  波段 {0}  |  置信 {1}%  |  噪点 {2}%  |  锁定 {3}  |  评分 {4}%"),
			TargetText.IsEmpty() ? NSLOCTEXT("Day1SliceHUD", "AdvancedSummaryTargetFallbackSpectral", "待识别") : TargetText,
			FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.SpectralConfidenceFraction * 100.0f)),
			FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.SpectralNoiseFraction * 100.0f)),
			LockText,
			FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.PerformanceGradeFraction * 100.0f)));
	}
	if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::SignalTuning)
	{
		const float BalanceDelta = AdvancedInteractionState.SignalBalanceFraction - AdvancedInteractionState.SignalTargetBalanceFraction;
		const FText BalanceDirection = FMath::Abs(BalanceDelta) <= 0.02f
			? NSLOCTEXT("Day1SliceHUD", "SignalTuningSummaryBalanceCentered", "居中")
			: (BalanceDelta < 0.0f
				? NSLOCTEXT("Day1SliceHUD", "SignalTuningSummaryBalanceLeft", "左偏")
				: NSLOCTEXT("Day1SliceHUD", "SignalTuningSummaryBalanceRight", "右偏"));
		const bool bLockCentered = FMath::Abs(BalanceDelta) <= 0.08f;
		float DistortionFraction = AdvancedInteractionState.DeviceLoadFraction;
		if (const FHorrorAdvancedInteractionInputOption* CenterLockOption = FindAdvancedInputOption(AdvancedInteractionState, AdvancedInteractionState.ExpectedInputId))
		{
			DistortionFraction = FMath::Max(DistortionFraction, CenterLockOption->LoadFraction);
		}
		const FText LockText = bLockCentered
			? NSLOCTEXT("Day1SliceHUD", "SignalTuningSummaryLockReady", "可锁定")
			: NSLOCTEXT("Day1SliceHUD", "SignalTuningSummaryLockBlocked", "未居中");

		return FText::Format(
			NSLOCTEXT(
				"Day1SliceHUD",
				"AdvancedTacticalSummarySignalTuning",
				"信号调谐仪  |  声像 {0}{1}%  |  目标 {2}  |  锁定 {3}  |  失真 {4}%  |  评分 {5}%"),
			BalanceDirection,
			FText::AsNumber(FMath::RoundToInt(FMath::Abs(BalanceDelta) * 100.0f)),
			TargetText.IsEmpty() ? NSLOCTEXT("Day1SliceHUD", "AdvancedSummaryTargetFallbackSignal", "中心频率") : TargetText,
			LockText,
			FText::AsNumber(FMath::RoundToInt(DistortionFraction * 100.0f)),
			FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.PerformanceGradeFraction * 100.0f)));
	}
	if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::CircuitWiring)
	{
		return FText::Format(
			NSLOCTEXT(
				"Day1SliceHUD",
				"AdvancedTacticalSummaryCircuit",
				"{0}  |  目标 {1}  |  窗口 {2}-{3}%  |  稳定 {4}%  |  负载 {5}%  |  评分 {6}%  |  线路 {7}%  |  漏电 {8}%"),
			ModeText,
			TargetText.IsEmpty() ? NSLOCTEXT("Day1SliceHUD", "AdvancedSummaryTargetFallbackCircuit", "待识别") : TargetText,
			FText::AsNumber(TimingStartPercent),
			FText::AsNumber(TimingEndPercent),
			FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.StabilityFraction * 100.0f)),
			FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.DeviceLoadFraction * 100.0f)),
			FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.PerformanceGradeFraction * 100.0f)),
			FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.RouteFlowFraction * 100.0f)),
			FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.HazardPressureFraction * 100.0f)));
	}
	if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::GearCalibration)
	{
		return FText::Format(
			NSLOCTEXT(
				"Day1SliceHUD",
				"AdvancedTacticalSummaryGear",
				"{0}  |  目标 {1}  |  窗口 {2}-{3}%  |  稳定 {4}%  |  负载 {5}%  |  评分 {6}%  |  链路 {7}%  |  卡滞 {8}%  |  咬合 {9}%"),
			ModeText,
			TargetText.IsEmpty() ? NSLOCTEXT("Day1SliceHUD", "AdvancedSummaryTargetFallbackGear", "待识别") : TargetText,
			FText::AsNumber(TimingStartPercent),
			FText::AsNumber(TimingEndPercent),
			FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.StabilityFraction * 100.0f)),
			FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.DeviceLoadFraction * 100.0f)),
			FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.PerformanceGradeFraction * 100.0f)),
			FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.RouteFlowFraction * 100.0f)),
			FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.HazardPressureFraction * 100.0f)),
			FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.TargetAlignmentFraction * 100.0f)));
	}
	return FText::Format(
		NSLOCTEXT(
			"Day1SliceHUD",
			"AdvancedTacticalSummary",
			"{0}  |  目标 {1}  |  窗口 {2}-{3}%  |  稳定 {4}%  |  负载 {5}%  |  评分 {6}%"),
		ModeText,
		TargetText.IsEmpty() ? NSLOCTEXT("Day1SliceHUD", "AdvancedSummaryTargetFallback", "待识别") : TargetText,
		FText::AsNumber(TimingStartPercent),
		FText::AsNumber(TimingEndPercent),
		FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.StabilityFraction * 100.0f)),
		FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.DeviceLoadFraction * 100.0f)),
		FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.PerformanceGradeFraction * 100.0f)));
}

FText ADay1SliceHUD::BuildAdvancedInteractionOutcomeLine() const
{
	if (!AdvancedInteractionState.bVisible)
	{
		return FText::GetEmpty();
	}

	const FHorrorAdvancedInteractionOutcome& Outcome = AdvancedInteractionState.LastOutcome;
	FText StateText;
	switch (Outcome.Kind)
	{
	case EHorrorAdvancedInteractionOutcomeKind::TimingFailure:
		StateText = NSLOCTEXT("Day1SliceHUD", "AdvancedOutcomeTimingFailure", "结果：时机偏离");
		break;
	case EHorrorAdvancedInteractionOutcomeKind::WrongInput:
		StateText = NSLOCTEXT("Day1SliceHUD", "AdvancedOutcomeWrongInput", "结果：输入错误");
		break;
	case EHorrorAdvancedInteractionOutcomeKind::Hazard:
		StateText = NSLOCTEXT("Day1SliceHUD", "AdvancedOutcomeHazard", "结果：危险输入");
		break;
	case EHorrorAdvancedInteractionOutcomeKind::SignalBalanceFailure:
		StateText = NSLOCTEXT("Day1SliceHUD", "AdvancedOutcomeSignalBalanceFailure", "结果：信号失衡");
		break;
	case EHorrorAdvancedInteractionOutcomeKind::SpectralConfidenceFailure:
		StateText = NSLOCTEXT("Day1SliceHUD", "AdvancedOutcomeSpectralConfidenceFailure", "结果：置信不足");
		break;
	case EHorrorAdvancedInteractionOutcomeKind::SpectralFilterFailure:
		StateText = NSLOCTEXT("Day1SliceHUD", "AdvancedOutcomeSpectralFilterFailure", "结果：滤波失败");
		break;
	case EHorrorAdvancedInteractionOutcomeKind::Overloaded:
		StateText = NSLOCTEXT("Day1SliceHUD", "AdvancedOutcomeOverloaded", "结果：设备过载");
		break;
	case EHorrorAdvancedInteractionOutcomeKind::Success:
		StateText = NSLOCTEXT("Day1SliceHUD", "AdvancedOutcomeSuccess", "结果：校准成功");
		break;
	case EHorrorAdvancedInteractionOutcomeKind::Completed:
		StateText = NSLOCTEXT("Day1SliceHUD", "AdvancedOutcomeCompleted", "结果：目标完成");
		break;
	case EHorrorAdvancedInteractionOutcomeKind::Adjusted:
		StateText = NSLOCTEXT("Day1SliceHUD", "AdvancedOutcomeAdjusted", "结果：参数已调节");
		break;
	case EHorrorAdvancedInteractionOutcomeKind::Prompted:
		StateText = NSLOCTEXT("Day1SliceHUD", "AdvancedOutcomePrompted", "结果：等待选择");
		break;
	case EHorrorAdvancedInteractionOutcomeKind::Paused:
		StateText = NSLOCTEXT("Day1SliceHUD", "AdvancedOutcomePaused", "结果：暂停冷却");
		break;
	case EHorrorAdvancedInteractionOutcomeKind::Cancelled:
		StateText = NSLOCTEXT("Day1SliceHUD", "AdvancedOutcomeCancelled", "结果：已取消");
		break;
	case EHorrorAdvancedInteractionOutcomeKind::Ignored:
	default:
		return FText::GetEmpty();
	}

	TArray<FString> Parts;
	Parts.Add(StateText.ToString());
	if (!Outcome.InputId.IsNone())
	{
		Parts.Add(FText::Format(
			NSLOCTEXT("Day1SliceHUD", "AdvancedOutcomeInputFormat", "输入 {0}"),
			FText::FromName(Outcome.InputId)).ToString());
	}
	if (!Outcome.FaultId.IsNone())
	{
		Parts.Add(NSLOCTEXT("Day1SliceHUD", "AdvancedOutcomeFaultDetected", "故障已定位").ToString());
	}
	if (Outcome.bRetryable && !AdvancedInteractionState.FailureRecoveryLabel.IsEmpty())
	{
		Parts.Add(AdvancedInteractionState.FailureRecoveryLabel.ToString());
	}
	else if (Outcome.bAdvancesProgress)
	{
		Parts.Add(NSLOCTEXT("Day1SliceHUD", "AdvancedOutcomeProgressAdvanced", "进度已推进").ToString());
	}

	return FText::FromString(FString::Join(Parts, TEXT("  |  ")));
}

EHorrorObjectiveFeedbackSeverity ADay1SliceHUD::ResolveAdvancedInteractionOutcomeSeverity() const
{
	switch (AdvancedInteractionState.LastOutcome.Kind)
	{
	case EHorrorAdvancedInteractionOutcomeKind::Hazard:
	case EHorrorAdvancedInteractionOutcomeKind::Overloaded:
		return EHorrorObjectiveFeedbackSeverity::Critical;
	case EHorrorAdvancedInteractionOutcomeKind::TimingFailure:
	case EHorrorAdvancedInteractionOutcomeKind::WrongInput:
	case EHorrorAdvancedInteractionOutcomeKind::SignalBalanceFailure:
	case EHorrorAdvancedInteractionOutcomeKind::SpectralConfidenceFailure:
	case EHorrorAdvancedInteractionOutcomeKind::SpectralFilterFailure:
		return EHorrorObjectiveFeedbackSeverity::Failure;
	case EHorrorAdvancedInteractionOutcomeKind::Paused:
	case EHorrorAdvancedInteractionOutcomeKind::Cancelled:
		return EHorrorObjectiveFeedbackSeverity::Warning;
	case EHorrorAdvancedInteractionOutcomeKind::Success:
	case EHorrorAdvancedInteractionOutcomeKind::Completed:
		return EHorrorObjectiveFeedbackSeverity::Success;
	case EHorrorAdvancedInteractionOutcomeKind::Adjusted:
	case EHorrorAdvancedInteractionOutcomeKind::Prompted:
	case EHorrorAdvancedInteractionOutcomeKind::Ignored:
	default:
		return EHorrorObjectiveFeedbackSeverity::Info;
	}
}

void ADay1SliceHUD::DrawAnomalyCaptureStatus(float CanvasWidth, float CanvasHeight)
{
	// 当高级交互面板或暂停菜单显示时，隐藏异常捕获状态避免重叠
	if (!bAnomalyCaptureStatusVisible
		|| AnomalyCaptureStatus.IsEmpty()
		|| AdvancedInteractionState.bVisible
		|| bPauseMenuVisible)
	{
		return;
	}

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font)
	{
		return;
	}

	const float PanelX = (CanvasWidth - AnomalyCapturePanelWidth) * 0.5f;
	const float PanelY = CanvasHeight * 0.58f;
	const FLinearColor Accent = bAnomalyCaptureLocked
		? FLinearColor(0.32f, 0.95f, 0.78f, 0.95f)
		: (bAnomalyCaptureRequiresRecording ? WarningAccent() : PanelAccent());
	DrawPanel(PanelX, PanelY, AnomalyCapturePanelWidth, AnomalyCapturePanelHeight, PanelBackground(), Accent);

	FCanvasTextItem StatusItem(
		FVector2D(PanelX + PanelPadding, PanelY + 14.0f),
		AnomalyCaptureStatus,
		Font,
		Accent);
	StatusItem.Scale = FVector2D(1.02f);
	Canvas->DrawItem(StatusItem);

	const float BarX = PanelX + PanelPadding;
	const float BarY = PanelY + 48.0f;
	const float BarWidth = AnomalyCapturePanelWidth - PanelPadding * 2.0f;
	const float BarHeight = 7.0f;
	FCanvasTileItem ProgressBack(FVector2D(BarX, BarY), FVector2D(BarWidth, BarHeight), FLinearColor(0.08f, 0.11f, 0.12f, 0.92f));
	ProgressBack.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(ProgressBack);

	FCanvasTileItem ProgressFill(
		FVector2D(BarX, BarY),
		FVector2D(BarWidth * AnomalyCaptureProgressFraction, BarHeight),
		Accent);
	ProgressFill.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(ProgressFill);
}

void ADay1SliceHUD::DrawObjectiveToast(float CanvasWidth)
{
	const float WorldSeconds = GetWorldSeconds();
	if (ToastExpireWorldSeconds <= WorldSeconds || (ToastTitle.IsEmpty() && ToastHint.IsEmpty()))
	{
		return;
	}

	const float PanelX = (CanvasWidth - ToastPanelWidth) * 0.5f;
	const float PanelY = 92.0f;
	const float PanelHeight = ToastHint.IsEmpty() ? 76.0f : 112.0f;
	const FLinearColor Accent = ObjectiveToastAccent(ToastSeverity);
	DrawPanel(PanelX, PanelY, ToastPanelWidth, PanelHeight, PanelBackground(), Accent);

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font)
	{
		return;
	}

	const FText Title = ToastTitle.IsEmpty() ? NSLOCTEXT("Day1SliceHUD", "ObjectiveUpdated", "目标已更新") : ToastTitle;
	FCanvasTextItem TitleItem(
		FVector2D(PanelX + PanelPadding, PanelY + 16.0f),
		Title,
		Font,
		ToastSeverity == EHorrorObjectiveFeedbackSeverity::Info ? TextPrimary() : Accent);
	TitleItem.Scale = FVector2D(1.18f);
	Canvas->DrawItem(TitleItem);

	if (!ToastHint.IsEmpty())
	{
		FCanvasTextItem HintItem(
			FVector2D(PanelX + PanelPadding, PanelY + 58.0f),
			ToastHint,
			Font,
			TextSecondary());
		HintItem.Scale = FVector2D(1.0f);
		Canvas->DrawItem(HintItem);
	}
}

void ADay1SliceHUD::DrawInteractionPrompt(float CanvasWidth, float CanvasHeight)
{
	// 当密码面板、高级交互面板或暂停菜单显示时，隐藏交互提示避免干扰
	if ((InteractionPrompt.IsEmpty() && !InteractionContext.bVisible)
		|| bPasswordPromptVisible
		|| AdvancedInteractionState.bVisible
		|| bPauseMenuVisible)
	{
		return;
	}

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font)
	{
		return;
	}

	const bool bStructured = InteractionContext.bVisible && InteractionContext.HasAnyText();
	const FLinearColor Accent = bStructured
		? InteractionRiskColor(InteractionContext.RiskLevel, InteractionContext.bCanInteract)
		: PanelAccent();
	const float PromptWidth = bStructured ? 560.0f : 420.0f;
	const float PromptHeight = bStructured ? 98.0f : 52.0f;
	const float PanelX = (CanvasWidth - PromptWidth) * 0.5f;
	const float PanelY = CanvasHeight - 132.0f;
	DrawPanel(PanelX, PanelY, PromptWidth, PromptHeight, PanelBackground(), Accent);

	if (bStructured)
	{
		const FText HeaderText = InteractionContext.InputText.IsEmpty()
			? FText::Format(
				NSLOCTEXT("Day1SliceHUD", "InteractionHeaderFallback", "{0} / {1}"),
				InteractionVerbText(InteractionContext.Verb),
				InteractionInputStyleText(InteractionContext.InputStyle))
			: FText::Format(
				NSLOCTEXT("Day1SliceHUD", "InteractionHeader", "{0} / {1}    {2}"),
				InteractionVerbText(InteractionContext.Verb),
				InteractionInputStyleText(InteractionContext.InputStyle),
				InteractionContext.InputText);
		FCanvasTextItem HeaderItem(
			FVector2D(PanelX + PanelPadding, PanelY + 12.0f),
			HeaderText,
			Font,
			Accent);
		HeaderItem.Scale = FVector2D(0.88f);
		Canvas->DrawItem(HeaderItem);

		const FText MainText = !InteractionContext.bCanInteract && !InteractionContext.BlockedReason.IsEmpty()
			? InteractionContext.BlockedReason
			: (!InteractionContext.ActionText.IsEmpty() ? InteractionContext.ActionText : InteractionPrompt);
		FCanvasTextItem MainItem(
			FVector2D(PanelX + PanelPadding, PanelY + 40.0f),
			MainText,
			Font,
			TextPrimary());
		MainItem.Scale = FVector2D(1.05f);
		Canvas->DrawItem(MainItem);

		const FText RiskText = InteractionRiskText(InteractionContext.RiskLevel);
		FText DetailText = InteractionContext.DetailText;
		if (DetailText.IsEmpty())
		{
			if (InteractionContext.bRequiresRecording)
			{
				DetailText = NSLOCTEXT("Day1SliceHUD", "InteractionRequiresRecording", "需要录像证据。");
			}
			else if (InteractionContext.bOpensPanel)
			{
				DetailText = NSLOCTEXT("Day1SliceHUD", "InteractionOpensPanel", "将打开交互窗口。");
			}
		}

		if (!RiskText.IsEmpty() || !DetailText.IsEmpty())
		{
			const FText FooterText = RiskText.IsEmpty()
				? DetailText
				: (DetailText.IsEmpty()
					? RiskText
					: FText::Format(NSLOCTEXT("Day1SliceHUD", "InteractionFooter", "{0}    {1}"), RiskText, DetailText));
			FCanvasTextItem FooterItem(
				FVector2D(PanelX + PanelPadding, PanelY + 70.0f),
				FooterText,
				Font,
				TextSecondary());
			FooterItem.Scale = FVector2D(0.84f);
			Canvas->DrawItem(FooterItem);
		}
		return;
	}

	FCanvasTextItem PromptItem(
		FVector2D(PanelX + PanelPadding, PanelY + 17.0f),
		InteractionPrompt,
		Font,
		TextPrimary());
	PromptItem.Scale = FVector2D(1.05f);
	Canvas->DrawItem(PromptItem);
}

void ADay1SliceHUD::DrawTransientMessage(float CanvasWidth, float CanvasHeight)
{
	const float WorldSeconds = GetWorldSeconds();
	// 当有其他重要UI显示时，隐藏瞬态消息避免干扰
	if (TransientMessageExpireWorldSeconds <= WorldSeconds
		|| TransientMessage.IsEmpty()
		|| bPasswordPromptVisible
		|| AdvancedInteractionState.bVisible
		|| bPauseMenuVisible)
	{
		return;
	}

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font)
	{
		return;
	}

	const float PanelWidth = 500.0f;
	const float PanelHeight = 54.0f;
	const float PanelX = (CanvasWidth - PanelWidth) * 0.5f;
	const float PanelY = CanvasHeight - 204.0f;
	DrawPanel(PanelX, PanelY, PanelWidth, PanelHeight, PanelBackground(), TransientMessageColor);

	FCanvasTextItem MessageItem(
		FVector2D(PanelX + PanelPadding, PanelY + 17.0f),
		TransientMessage,
		Font,
		TransientMessageColor);
	MessageItem.Scale = FVector2D(1.05f);
	Canvas->DrawItem(MessageItem);
}

void ADay1SliceHUD::DrawAutosaveIndicator(float CanvasWidth)
{
	const float WorldSeconds = GetWorldSeconds();
	if (AutosaveIndicatorExpireWorldSeconds <= WorldSeconds || AutosaveIndicatorText.IsEmpty())
	{
		return;
	}

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font)
	{
		return;
	}

	const float PanelX = CanvasWidth - AutosavePanelWidth - OuterPadding;
	const float PanelY = OuterPadding + StatusPanelHeight + 12.0f;
	const FLinearColor SaveAccent(0.42f, 0.82f, 1.0f, 0.92f);
	DrawPanel(PanelX, PanelY, AutosavePanelWidth, AutosavePanelHeight, PanelBackground(), SaveAccent);

	FCanvasTextItem SaveItem(
		FVector2D(PanelX + PanelPadding, PanelY + 15.0f),
		AutosaveIndicatorText,
		Font,
		SaveAccent);
	SaveItem.Scale = FVector2D(0.96f);
	Canvas->DrawItem(SaveItem);
}

void ADay1SliceHUD::DrawSurvivalStatus(float CanvasWidth)
{
	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font)
	{
		return;
	}

	const float PanelX = CanvasWidth - StatusPanelWidth - OuterPadding;
	const float PanelY = OuterPadding;
	const FLinearColor Accent = bStatusDanger ? FLinearColor(1.0f, 0.18f, 0.12f, 0.92f) : PanelAccent();
	DrawPanel(PanelX, PanelY, StatusPanelWidth, StatusPanelHeight, PanelBackground(), Accent);

	const FString CameraStatus = !bStatusBodycamAcquired
		? TEXT("摄像机丢失")
		: (bStatusRecording ? TEXT("录像中") : (bStatusBodycamEnabled ? TEXT("摄像机在线") : TEXT("摄像机离线")));
	const FString DangerStatus = bStatusDanger ? TEXT("危险") : TEXT("安全");
	FCanvasTextItem CameraItem(
		FVector2D(PanelX + PanelPadding, PanelY + 14.0f),
		FText::FromString(FString::Printf(TEXT("%s    %s"), *CameraStatus, *DangerStatus)),
		Font,
		Accent);
	CameraItem.Scale = FVector2D(1.08f);
	Canvas->DrawItem(CameraItem);

	FCanvasTextItem FearItem(
		FVector2D(PanelX + PanelPadding, PanelY + 50.0f),
		FText::FromString(FString::Printf(TEXT("恐惧     %3d%%"), FMath::RoundToInt(StatusFearPercent * 100.0f))),
		Font,
		TextSecondary());
	FearItem.Scale = FVector2D(0.96f);
	Canvas->DrawItem(FearItem);

	FCanvasTextItem SprintItem(
		FVector2D(PanelX + PanelPadding, PanelY + 78.0f),
		FText::FromString(FString::Printf(TEXT("体力     %3d%%"), FMath::RoundToInt(StatusSprintPercent * 100.0f))),
		Font,
		TextSecondary());
	SprintItem.Scale = FVector2D(0.96f);
	Canvas->DrawItem(SprintItem);

	if (bBodycamBatteryVisible)
	{
		const FLinearColor BatteryColor = bBodycamBatteryLow ? FLinearColor(1.0f, 0.78f, 0.15f, 0.95f) : TextSecondary();
		const FString LowBatterySuffix = bBodycamBatteryLow ? TEXT("  低") : TEXT("");
		FCanvasTextItem BatteryItem(
			FVector2D(PanelX + PanelPadding, PanelY + 106.0f),
			FText::FromString(FString::Printf(TEXT("电量     %3d%%%s"), FMath::RoundToInt(BodycamBatteryPercent), *LowBatterySuffix)),
			Font,
			BatteryColor);
		BatteryItem.Scale = FVector2D(0.96f);
		Canvas->DrawItem(BatteryItem);
	}
}

void ADay1SliceHUD::DrawPasswordPanel(float CanvasWidth, float CanvasHeight)
{
	if (!bPasswordPromptVisible)
	{
		return;
	}

	const float PanelX = (CanvasWidth - PasswordPanelWidth) * 0.5f;
	const float PanelY = CanvasHeight - PasswordPanelHeight - 86.0f;
	DrawPanel(PanelX, PanelY, PasswordPanelWidth, PasswordPanelHeight, PanelBackground(), PanelAccent());

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font)
	{
		return;
	}

	const FText DoorLabel = PasswordDoorName.IsEmpty()
		? NSLOCTEXT("Day1SliceHUD", "LockedDoor", "锁定门")
		: PasswordDoorName;

	FCanvasTextItem TitleItem(
		FVector2D(PanelX + PanelPadding, PanelY + 18.0f),
		DoorLabel,
		Font,
		TextPrimary());
	TitleItem.Scale = FVector2D(1.18f);
	Canvas->DrawItem(TitleItem);

	FString MaskedDigits;
	for (int32 DigitIndex = 0; DigitIndex < PasswordEnteredDigits; ++DigitIndex)
	{
		MaskedDigits.AppendChar(TEXT('*'));
	}
	const int32 RemainingDigits = PasswordRequiredDigits > 0
		? FMath::Max(0, PasswordRequiredDigits - PasswordEnteredDigits)
		: 0;
	for (int32 DigitIndex = 0; DigitIndex < RemainingDigits; ++DigitIndex)
	{
		MaskedDigits.AppendChar(TEXT('_'));
	}

	if (MaskedDigits.IsEmpty())
	{
		MaskedDigits = TEXT("____");
	}

	FCanvasTextItem CodeItem(
		FVector2D(PanelX + PanelPadding, PanelY + 68.0f),
		FText::FromString(FString::Printf(TEXT("密码  %s"), *MaskedDigits)),
		Font,
		PanelAccent());
	CodeItem.Scale = FVector2D(1.35f);
	Canvas->DrawItem(CodeItem);

	const FText HintText = PasswordHint.IsEmpty()
		? NSLOCTEXT("Day1SliceHUD", "FindPasswordHint", "在站内寻找密码。")
		: PasswordHint;
	FCanvasTextItem HintItem(
		FVector2D(PanelX + PanelPadding, PanelY + 112.0f),
		HintText,
		Font,
		TextSecondary());
	HintItem.Scale = FVector2D(0.96f);
	Canvas->DrawItem(HintItem);

	FCanvasTextItem ControlsItem(
		FVector2D(PanelX + PanelPadding, PanelY + 142.0f),
		NSLOCTEXT("Day1SliceHUD", "PasswordControls", "数字键：输入密码    回车：确认    退格：删除    取消键：取消"),
		Font,
		TextSecondary());
	ControlsItem.Scale = FVector2D(0.86f);
	Canvas->DrawItem(ControlsItem);
}

void ADay1SliceHUD::DrawPauseMenu(float CanvasWidth, float CanvasHeight)
{
	if (!bPauseMenuVisible)
	{
		return;
	}

	const float PanelX = (CanvasWidth - PausePanelWidth) * 0.5f;
	const float PanelY = (CanvasHeight - PausePanelHeight) * 0.5f;
	DrawPanel(PanelX, PanelY, PausePanelWidth, PausePanelHeight, PanelBackground(), PanelAccent());

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font)
	{
		return;
	}

	FCanvasTextItem TitleItem(
		FVector2D(PanelX + PanelPadding, PanelY + 18.0f),
		NSLOCTEXT("Day1SliceHUD", "PauseTitle", "暂停"),
		Font,
		TextPrimary());
	TitleItem.Scale = FVector2D(1.35f);
	Canvas->DrawItem(TitleItem);

	const auto DrawRow = [this, Font, PanelX, PanelY](EDay1PauseMenuSelection Selection, float RowY, const FString& Label, const FString& Value)
	{
		const bool bSelected = PauseMenuSelection == Selection;
		const FLinearColor RowColor = bSelected ? PanelAccent() : TextPrimary();
		const FString Prefix = bSelected ? TEXT("> ") : TEXT("  ");
		const FString Suffix = Value.IsEmpty() ? FString() : FString::Printf(TEXT("    %s"), *Value);

		FCanvasTextItem RowItem(
			FVector2D(PanelX + PanelPadding, RowY),
			FText::FromString(Prefix + Label + Suffix),
			Font,
			RowColor);
		RowItem.Scale = FVector2D(bSelected ? 1.13f : 1.0f);
		Canvas->DrawItem(RowItem);
	};

	DrawRow(EDay1PauseMenuSelection::Resume, PanelY + 82.0f, TEXT("继续"), FString());
	DrawRow(
		EDay1PauseMenuSelection::MouseSensitivity,
		PanelY + 124.0f,
		TEXT("鼠标灵敏度"),
		FString::Printf(TEXT("%.1f"), PauseMouseSensitivity));
	DrawRow(
		EDay1PauseMenuSelection::MasterVolume,
		PanelY + 166.0f,
		TEXT("主音量"),
		FString::Printf(TEXT("%d%%"), FMath::RoundToInt(PauseMasterVolume * 100.0f)));
	DrawRow(
		EDay1PauseMenuSelection::Brightness,
		PanelY + 208.0f,
		TEXT("亮度"),
		FString::Printf(TEXT("%.2f"), PauseBrightness));

	FCanvasTextItem ControlsItem(
		FVector2D(PanelX + PanelPadding, PanelY + 262.0f),
		NSLOCTEXT("Day1SliceHUD", "PauseControls", "上/下：选择    左/右：调整    确认键/空格/取消键：继续"),
		Font,
		TextSecondary());
	ControlsItem.Scale = FVector2D(0.92f);
	Canvas->DrawItem(ControlsItem);
}

void ADay1SliceHUD::DrawNotesJournal(float CanvasWidth, float CanvasHeight)
{
	if (!bNotesJournalVisible)
	{
		return;
	}

	const float PanelX = (CanvasWidth - NotesJournalPanelWidth) * 0.5f;
	const float PanelY = (CanvasHeight - NotesJournalPanelHeight) * 0.5f;
	DrawPanel(PanelX, PanelY, NotesJournalPanelWidth, NotesJournalPanelHeight, PanelBackground(), PanelAccent());

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font)
	{
		return;
	}

	FCanvasTextItem TitleItem(
		FVector2D(PanelX + PanelPadding, PanelY + 18.0f),
		NSLOCTEXT("Day1SliceHUD", "NotesTitle", "笔记"),
		Font,
		TextPrimary());
	TitleItem.Scale = FVector2D(1.35f);
	Canvas->DrawItem(TitleItem);

	if (NotesJournalEntries.IsEmpty())
	{
		FCanvasTextItem EmptyItem(
			FVector2D(PanelX + PanelPadding, PanelY + 92.0f),
			NotesJournalEmptyText.IsEmpty() ? NSLOCTEXT("Day1SliceHUD", "NoNotesRecorded", "还没有记录任何笔记。") : NotesJournalEmptyText,
			Font,
			TextSecondary());
		EmptyItem.Scale = FVector2D(1.08f);
		Canvas->DrawItem(EmptyItem);
	}
	else
	{
		float RowY = PanelY + 76.0f;
		const int32 MaxVisibleEntries = FMath::Min(NotesJournalEntries.Num(), 5);
		for (int32 EntryIndex = 0; EntryIndex < MaxVisibleEntries; ++EntryIndex)
		{
			const FHorrorNoteMetadata& Entry = NotesJournalEntries[EntryIndex];
			const FText NoteTitle = Entry.Title.IsEmpty()
				? FText::FromName(Entry.NoteId)
				: Entry.Title;
			const FText NoteBody = Entry.Body.IsEmpty()
				? NSLOCTEXT("Day1SliceHUD", "CannotReadNote", "无法读取这份笔记的内容。")
				: Entry.Body;

			FCanvasTextItem NoteTitleItem(
				FVector2D(PanelX + PanelPadding, RowY),
				NoteTitle,
				Font,
				PanelAccent());
			NoteTitleItem.Scale = FVector2D(1.08f);
			Canvas->DrawItem(NoteTitleItem);

			FCanvasTextItem NoteBodyItem(
				FVector2D(PanelX + PanelPadding, RowY + 28.0f),
				NoteBody,
				Font,
				TextSecondary());
			NoteBodyItem.Scale = FVector2D(0.94f);
			Canvas->DrawItem(NoteBodyItem);

			RowY += 72.0f;
		}
	}

	FCanvasTextItem ControlsItem(
		FVector2D(PanelX + PanelPadding, PanelY + NotesJournalPanelHeight - 42.0f),
		NSLOCTEXT("Day1SliceHUD", "NotesControls", "笔记键/物品栏键/取消键：关闭"),
		Font,
		TextSecondary());
	ControlsItem.Scale = FVector2D(0.92f);
	Canvas->DrawItem(ControlsItem);
}

void ADay1SliceHUD::DrawDay1CompletionOverlay(float CanvasWidth, float CanvasHeight)
{
	if (!bDay1CompletionOverlayVisible)
	{
		return;
	}

	FCanvasTileItem BlackoutTile(FVector2D(0.0f, 0.0f), FVector2D(CanvasWidth, CanvasHeight), FLinearColor(0.0f, 0.0f, 0.0f, 0.96f));
	BlackoutTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BlackoutTile);

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font)
	{
		return;
	}

	const float CenterX = CanvasWidth * 0.5f;
	const float CenterY = CanvasHeight * 0.5f;
	const FLinearColor CompletionAccent(0.32f, 0.95f, 0.78f, 1.0f);

	FCanvasTextItem TitleItem(
		FVector2D(CenterX - 132.0f, CenterY - 42.0f),
		Day1CompletionTitle,
		Font,
		CompletionAccent);
	TitleItem.Scale = FVector2D(1.55f);
	Canvas->DrawItem(TitleItem);

	if (!Day1CompletionHint.IsEmpty())
	{
		FCanvasTextItem HintItem(
			FVector2D(CenterX - 172.0f, CenterY + 12.0f),
			Day1CompletionHint,
			Font,
			TextSecondary());
		HintItem.Scale = FVector2D(1.05f);
		Canvas->DrawItem(HintItem);
	}
}

void ADay1SliceHUD::DrawPanel(float X, float Y, float Width, float Height, const FLinearColor& BackgroundColor, const FLinearColor& AccentColor)
{
	if (!Canvas)
	{
		return;
	}

	FCanvasTileItem BackgroundTile(FVector2D(X, Y), FVector2D(Width, Height), BackgroundColor);
	BackgroundTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BackgroundTile);

	FCanvasTileItem HeaderTile(FVector2D(X, Y), FVector2D(Width, 34.0f), AccentColor.CopyWithNewOpacity(0.12f));
	HeaderTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(HeaderTile);

	FCanvasTileItem AccentTile(FVector2D(X, Y), FVector2D(4.0f, Height), AccentColor);
	AccentTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(AccentTile);

	FCanvasTileItem TopTile(FVector2D(X, Y), FVector2D(Width, 1.0f), AccentColor.CopyWithNewOpacity(0.38f));
	TopTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TopTile);

	FCanvasTileItem HeaderLine(FVector2D(X + 4.0f, Y + 34.0f), FVector2D(Width - 4.0f, 1.0f), AccentColor.CopyWithNewOpacity(0.22f));
	HeaderLine.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(HeaderLine);

	FCanvasTileItem BottomLine(FVector2D(X + 4.0f, Y + Height - 1.0f), FVector2D(Width - 4.0f, 1.0f), AccentColor.CopyWithNewOpacity(0.18f));
	BottomLine.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BottomLine);
}

void ADay1SliceHUD::DrawLine(const FVector2D& Start, const FVector2D& End, const FLinearColor& Color, float Thickness)
{
	if (!Canvas)
	{
		return;
	}

	FCanvasLineItem LineItem(Start, End);
	LineItem.SetColor(Color);
	LineItem.LineThickness = Thickness;
	Canvas->DrawItem(LineItem);
}

void ADay1SliceHUD::DrawCircleLines(const FVector2D& Center, float Radius, const FLinearColor& Color, int32 Segments, float Thickness)
{
	if (!Canvas || Radius <= 0.0f)
	{
		return;
	}

	const int32 SafeSegments = FMath::Max(8, Segments);
	FVector2D PreviousPoint = Center + FVector2D(Radius, 0.0f);
	for (int32 SegmentIndex = 1; SegmentIndex <= SafeSegments; ++SegmentIndex)
	{
		const float Angle = TWO_PI * static_cast<float>(SegmentIndex) / static_cast<float>(SafeSegments);
		const FVector2D Point = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius;
		DrawLine(PreviousPoint, Point, Color, Thickness);
		PreviousPoint = Point;
	}
}

void ADay1SliceHUD::DrawRotatedRect(const FVector2D& Center, const FVector2D& Size, float AngleRadians, const FLinearColor& Color)
{
	if (!Canvas)
	{
		return;
	}

	const FVector2D AxisX(FMath::Cos(AngleRadians), FMath::Sin(AngleRadians));
	const FVector2D AxisY(-FMath::Sin(AngleRadians), FMath::Cos(AngleRadians));
	const FVector2D HalfX = AxisX * (Size.X * 0.5f);
	const FVector2D HalfY = AxisY * (Size.Y * 0.5f);
	const FVector2D Points[] = {
		Center - HalfX - HalfY,
		Center + HalfX - HalfY,
		Center + HalfX + HalfY,
		Center - HalfX + HalfY
	};

	DrawLine(Points[0], Points[1], Color, 2.0f);
	DrawLine(Points[1], Points[2], Color, 2.0f);
	DrawLine(Points[2], Points[3], Color, 2.0f);
	DrawLine(Points[3], Points[0], Color, 2.0f);
}

float ADay1SliceHUD::GetWorldSeconds() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.0f;
}
