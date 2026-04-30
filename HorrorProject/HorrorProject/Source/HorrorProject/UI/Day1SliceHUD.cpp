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
	constexpr float AdvancedPanelWidth = 780.0f;
	constexpr float AdvancedPanelHeight = 520.0f;
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

	bool FeedbackLooksLikeFailure(const FText& FeedbackText)
	{
		const FString Feedback = FeedbackText.ToString();
		return Feedback.Contains(TEXT("红色火花"))
			|| Feedback.Contains(TEXT("卡死"))
			|| Feedback.Contains(TEXT("咔哒"))
			|| Feedback.Contains(TEXT("时机"));
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
}

void ADay1SliceHUD::ClearInteractionPrompt()
{
	InteractionPrompt = FText::GetEmpty();
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
	ObjectiveChecklistItems = Snapshot.ChecklistItems;
	ObjectiveTrackerProgressFraction = FMath::Clamp(Snapshot.ProgressFraction, 0.0f, 1.0f);
	bObjectiveTrackerUrgent = Snapshot.bUrgent;
	bObjectiveTrackerRequiresRecording = Snapshot.bRequiresRecording;
}

void ADay1SliceHUD::SetObjectiveNavigation(const FText& NavigationText)
{
	ObjectiveNavigation = NavigationText;
}

void ADay1SliceHUD::ClearObjectiveNavigation()
{
	ObjectiveNavigation = FText::GetEmpty();
}

void ADay1SliceHUD::ShowObjectiveToast(const FText& TitleText, const FText& HintText, float DisplaySeconds)
{
	ToastTitle = TitleText;
	ToastHint = HintText;
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
	AdvancedInteractionState = State;
	AdvancedInteractionState.ProgressFraction = FMath::Clamp(State.ProgressFraction, 0.0f, 1.0f);
	AdvancedInteractionState.TimingFraction = FMath::Clamp(State.TimingFraction, 0.0f, 1.0f);
	AdvancedInteractionState.PauseRemainingSeconds = FMath::Max(0.0f, State.PauseRemainingSeconds);
	AdvancedInteractionState.StepIndex = FMath::Max(0, State.StepIndex);
	AdvancedInteractionState.RequiredStepCount = FMath::Max(0, State.RequiredStepCount);
}

void ADay1SliceHUD::ClearAdvancedInteractionState()
{
	AdvancedInteractionState = FHorrorAdvancedInteractionHUDState();
}

void ADay1SliceHUD::DrawCurrentObjective(float CanvasWidth)
{
	if (CurrentObjective.IsEmpty())
	{
		return;
	}

	const float PanelX = OuterPadding;
	const float PanelY = OuterPadding;
	const bool bHasTitle = !ObjectiveTrackerTitle.IsEmpty();
	const bool bHasDetail = !ObjectiveTrackerDetail.IsEmpty();
	const bool bHasNavigation = !ObjectiveNavigation.IsEmpty();
	const bool bHasProgress = !ObjectiveTrackerProgressLabel.IsEmpty();
	const int32 VisibleChecklistCount = FMath::Min(ObjectiveChecklistItems.Num(), 6);
	const float PanelHeight =
		92.0f
		+ (bHasDetail ? 28.0f : 0.0f)
		+ (bHasNavigation ? 28.0f : 0.0f)
		+ (bHasProgress ? 28.0f : 0.0f)
		+ (VisibleChecklistCount > 0 ? 18.0f + VisibleChecklistCount * 22.0f : 0.0f);
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
		FCanvasTextItem NavigationItem(
			FVector2D(PanelX + PanelPadding, RowY),
			ObjectiveNavigation,
			Font,
			Accent);
		NavigationItem.Scale = FVector2D(0.94f);
		Canvas->DrawItem(NavigationItem);
		RowY += 28.0f;
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

	if (VisibleChecklistCount > 0)
	{
		for (int32 ItemIndex = 0; ItemIndex < VisibleChecklistCount; ++ItemIndex)
		{
			const FHorrorObjectiveChecklistItem& Item = ObjectiveChecklistItems[ItemIndex];
			const TCHAR* Marker = Item.bComplete ? TEXT("[完成]") : (Item.bActive ? TEXT("[当前]") : TEXT("[待办]"));
			const FLinearColor ItemColor = Item.bActive ? Accent : (Item.bComplete ? TextSecondary() : FLinearColor(0.42f, 0.50f, 0.50f, 0.86f));
			FText ItemText = FText::Format(
				NSLOCTEXT("Day1SliceHUD", "ChecklistItemFormat", "{0} {1}"),
				FText::FromString(Marker),
				Item.Label);
			if (Item.bRequiresRecording && !Item.bComplete)
			{
				ItemText = FText::Format(
					NSLOCTEXT("Day1SliceHUD", "ChecklistRecordingItemFormat", "{0}    需要录像"),
					ItemText);
			}

			FCanvasTextItem ChecklistItem(
				FVector2D(PanelX + PanelPadding, RowY),
				ItemText,
				Font,
				ItemColor);
			ChecklistItem.Scale = FVector2D(Item.bActive ? 0.9f : 0.84f);
			Canvas->DrawItem(ChecklistItem);
			RowY += 22.0f;
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
	const float PanelX = (CanvasWidth - PanelWidth) * 0.5f;
	const float PanelY = (CanvasHeight - PanelHeight) * 0.5f;
	const FLinearColor Accent = AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::CircuitWiring
		? FLinearColor(0.22f, 0.72f, 1.0f, 0.96f)
		: FLinearColor(0.98f, 0.62f, 0.25f, 0.96f);

	FCanvasTileItem DimTile(FVector2D(0.0f, 0.0f), FVector2D(CanvasWidth, CanvasHeight), FLinearColor(0.0f, 0.0f, 0.0f, 0.42f));
	DimTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(DimTile);

	DrawPanel(PanelX, PanelY, PanelWidth, PanelHeight, FLinearColor(0.006f, 0.010f, 0.014f, 0.94f), Accent);
	DrawAdvancedInteractionHeader(PanelX, PanelY, PanelWidth, Font, Accent);

	const float WorldSeconds = GetWorldSeconds();
	if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::CircuitWiring)
	{
		DrawCircuitWiringPanel(PanelX, PanelY, PanelWidth, PanelHeight, Font, WorldSeconds);
	}
	else if (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::GearCalibration)
	{
		DrawGearCalibrationPanel(PanelX, PanelY, PanelWidth, PanelHeight, Font, WorldSeconds);
	}

	DrawAdvancedInteractionProgress(PanelX, PanelY + PanelHeight - 118.0f, PanelWidth, Font, Accent);
}

void ADay1SliceHUD::DrawAdvancedInteractionHeader(float PanelX, float PanelY, float PanelWidth, UFont* Font, const FLinearColor& AccentColor)
{
	const FText Title = AdvancedInteractionState.Title.IsEmpty()
		? NSLOCTEXT("Day1SliceHUD", "AdvancedInteractionTitle", "设备校准")
		: AdvancedInteractionState.Title;
	const FText ModeText = AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::CircuitWiring
		? NSLOCTEXT("Day1SliceHUD", "CircuitWiringMode", "电路接线台")
		: NSLOCTEXT("Day1SliceHUD", "GearCalibrationMode", "机械齿轮盘");

	FCanvasTextItem ModeItem(
		FVector2D(PanelX + PanelPadding, PanelY + 16.0f),
		ModeText,
		Font,
		AccentColor);
	ModeItem.Scale = FVector2D(1.05f);
	Canvas->DrawItem(ModeItem);

	FCanvasTextItem TitleItem(
		FVector2D(PanelX + PanelPadding, PanelY + 48.0f),
		Title,
		Font,
		TextPrimary());
	TitleItem.Scale = FVector2D(1.24f);
	Canvas->DrawItem(TitleItem);

	const int32 StepDisplay = FMath::Clamp(AdvancedInteractionState.StepIndex + 1, 1, FMath::Max(1, AdvancedInteractionState.RequiredStepCount));
	const FText StepText = FText::Format(
		NSLOCTEXT("Day1SliceHUD", "AdvancedInteractionStep", "步骤 {0}/{1}"),
		FText::AsNumber(StepDisplay),
		FText::AsNumber(FMath::Max(1, AdvancedInteractionState.RequiredStepCount)));
	FCanvasTextItem StepItem(
		FVector2D(PanelX + PanelWidth - 150.0f, PanelY + 24.0f),
		StepText,
		Font,
		TextSecondary());
	StepItem.Scale = FVector2D(0.96f);
	Canvas->DrawItem(StepItem);
}

void ADay1SliceHUD::DrawCircuitWiringPanel(float PanelX, float PanelY, float PanelWidth, float PanelHeight, UFont* Font, float WorldSeconds)
{
	const FVector2D Center(PanelX + PanelWidth * 0.5f, PanelY + 260.0f);
	const float DialRadius = FMath::Min(PanelWidth, PanelHeight) * 0.25f;
	const bool bFailure = FeedbackLooksLikeFailure(AdvancedInteractionState.FeedbackText);
	const FLinearColor TargetColor = CircuitColorForInput(AdvancedInteractionState.ExpectedInputId);
	const float Pulse = 0.5f + 0.5f * FMath::Sin(WorldSeconds * 8.0f);

	DrawCircleLines(Center, DialRadius + 26.0f, FLinearColor(0.06f, 0.16f, 0.20f, 0.86f), 72, 2.0f);
	DrawCircleLines(Center, DialRadius, FLinearColor(0.10f, 0.35f, 0.48f, 0.92f), 72, 3.0f);
	DrawCircleLines(Center, DialRadius * 0.48f, FLinearColor(0.08f, 0.18f, 0.22f, 0.90f), 48, 2.0f);

	const float WindowStartAngle = -PI * 0.60f;
	const float WindowEndAngle = PI * 0.40f;
	const int32 WindowSegments = 18;
	FVector2D PreviousPoint = Center + FVector2D(FMath::Cos(WindowStartAngle), FMath::Sin(WindowStartAngle)) * (DialRadius + 14.0f);
	for (int32 SegmentIndex = 1; SegmentIndex <= WindowSegments; ++SegmentIndex)
	{
		const float Alpha = static_cast<float>(SegmentIndex) / static_cast<float>(WindowSegments);
		const float Angle = FMath::Lerp(WindowStartAngle, WindowEndAngle, Alpha);
		const FVector2D Point = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * (DialRadius + 14.0f);
		DrawLine(PreviousPoint, Point, AdvancedInteractionState.bTimingWindowOpen ? FLinearColor(0.28f, 1.0f, 0.82f, 1.0f) : FLinearColor(0.12f, 0.44f, 0.42f, 0.75f), 6.0f);
		PreviousPoint = Point;
	}

	const float CursorAngle = FMath::Lerp(-PI * 0.95f, PI * 1.05f, AdvancedInteractionState.TimingFraction);
	const FVector2D CursorEnd = Center + FVector2D(FMath::Cos(CursorAngle), FMath::Sin(CursorAngle)) * (DialRadius + 20.0f);
	DrawLine(Center, CursorEnd, AdvancedInteractionState.bTimingWindowOpen ? FLinearColor(0.34f, 1.0f, 0.86f, 1.0f) : FLinearColor(0.85f, 0.94f, 1.0f, 0.88f), 4.0f);
	DrawCircleLines(CursorEnd, 10.0f + Pulse * 4.0f, AdvancedInteractionState.bTimingWindowOpen ? FLinearColor(0.34f, 1.0f, 0.86f, 0.96f) : TargetColor, 18, 2.0f);

	const FName Inputs[] = { TEXT("蓝色端子"), TEXT("红色端子"), TEXT("黄色端子") };
	const FString Labels[] = { TEXT("1"), TEXT("2"), TEXT("3") };
	const float Angles[] = { -PI * 0.78f, -PI * 0.20f, PI * 0.28f };
	for (int32 InputIndex = 0; InputIndex < 3; ++InputIndex)
	{
		const FName InputId = Inputs[InputIndex];
		const bool bTarget = InputId == AdvancedInteractionState.ExpectedInputId;
		const FLinearColor WireColor = CircuitColorForInput(InputId);
		const FVector2D Terminal = Center + FVector2D(FMath::Cos(Angles[InputIndex]), FMath::Sin(Angles[InputIndex])) * (DialRadius + 86.0f);
		const FVector2D Inner = Center + FVector2D(FMath::Cos(Angles[InputIndex]), FMath::Sin(Angles[InputIndex])) * (DialRadius * 0.58f);
		DrawLine(Inner, Terminal, WireColor.CopyWithNewOpacity(bTarget ? 0.96f : 0.46f), bTarget ? 4.0f : 2.0f);
		DrawCircleLines(Terminal, bTarget ? 24.0f + Pulse * 6.0f : 19.0f, WireColor, 28, bTarget ? 4.0f : 2.0f);

		FCanvasTextItem KeyItem(
			FVector2D(Terminal.X - 6.0f, Terminal.Y - 10.0f),
			FText::FromString(Labels[InputIndex]),
			Font,
			bTarget ? TextPrimary() : TextSecondary());
		KeyItem.Scale = FVector2D(bTarget ? 1.08f : 0.92f);
		Canvas->DrawItem(KeyItem);

		FCanvasTextItem NameItem(
			FVector2D(Terminal.X - 42.0f, Terminal.Y + 30.0f),
			FText::FromName(InputId),
			Font,
			bTarget ? WireColor : TextSecondary());
		NameItem.Scale = FVector2D(0.82f);
		Canvas->DrawItem(NameItem);
	}

	if (!AdvancedInteractionState.FeedbackText.IsEmpty())
	{
		const FLinearColor FeedbackColor = bFailure ? FLinearColor(1.0f, 0.20f, 0.10f, 0.96f) : FLinearColor(0.28f, 0.82f, 1.0f, 0.96f);
		for (int32 ArcIndex = 0; ArcIndex < 7; ++ArcIndex)
		{
			const float Angle = WorldSeconds * (bFailure ? 9.0f : 5.5f) + ArcIndex * 0.86f;
			const FVector2D Start = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * (DialRadius * 0.68f);
			const FVector2D End = Center + FVector2D(FMath::Cos(Angle + 0.22f), FMath::Sin(Angle + 0.22f)) * (DialRadius + (bFailure ? 62.0f : 34.0f));
			DrawLine(Start, End, FeedbackColor.CopyWithNewOpacity(0.35f + Pulse * 0.55f), bFailure ? 3.0f : 2.0f);
		}
	}

	FCanvasTextItem ControlsItem(
		FVector2D(PanelX + PanelPadding, PanelY + PanelHeight - 52.0f),
		NSLOCTEXT("Day1SliceHUD", "CircuitControls", "数字键 1/2/3 接入对应端子    互动键：接入当前高亮端子"),
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
	const FName GearInputs[] = { TEXT("齿轮1"), TEXT("齿轮2"), TEXT("齿轮3") };
	const int32 TargetIndex = IndexForAdvancedInput(AdvancedInteractionState.ExpectedInputId);
	const bool bFailure = FeedbackLooksLikeFailure(AdvancedInteractionState.FeedbackText) || AdvancedInteractionState.bPaused;
	const float Pulse = 0.5f + 0.5f * FMath::Sin(WorldSeconds * 7.5f);

	const float TrackX = PanelX + 110.0f;
	const float TrackY = PanelY + 128.0f;
	const float TrackWidth = PanelWidth - 220.0f;
	const float TrackHeight = 18.0f;
	FCanvasTileItem TrackBack(FVector2D(TrackX, TrackY), FVector2D(TrackWidth, TrackHeight), FLinearColor(0.05f, 0.06f, 0.055f, 0.92f));
	TrackBack.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TrackBack);
	FCanvasTileItem WindowTile(
		FVector2D(TrackX + TrackWidth * AdvancedTimingWindowStart, TrackY),
		FVector2D(TrackWidth * (AdvancedTimingWindowEnd - AdvancedTimingWindowStart), TrackHeight),
		AdvancedInteractionState.bTimingWindowOpen ? FLinearColor(0.32f, 0.92f, 0.58f, 0.88f) : FLinearColor(0.28f, 0.44f, 0.32f, 0.62f));
	WindowTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(WindowTile);
	const float CursorX = TrackX + TrackWidth * AdvancedInteractionState.TimingFraction;
	DrawLine(FVector2D(CursorX, TrackY - 8.0f), FVector2D(CursorX, TrackY + TrackHeight + 8.0f), FLinearColor(0.98f, 0.90f, 0.48f, 0.96f), 4.0f);

	for (int32 GearIndex = 0; GearIndex < 3; ++GearIndex)
	{
		const bool bStoppedGear = GearIndex == TargetIndex;
		const FLinearColor GearColor = bStoppedGear
			? (bFailure ? FLinearColor(1.0f, 0.22f, 0.12f, 0.96f) : FLinearColor(1.0f, 0.78f, 0.28f, 0.96f))
			: FLinearColor(0.56f, 0.70f, 0.72f, 0.86f);
		const float Rotation = bStoppedGear && !AdvancedInteractionState.bTimingWindowOpen
			? 0.0f
			: WorldSeconds * (GearIndex % 2 == 0 ? 1.7f : -2.1f);

		DrawCircleLines(GearCenters[GearIndex], GearRadii[GearIndex], GearColor, 54, bStoppedGear ? 4.0f : 2.0f);
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
			DrawCircleLines(GearCenters[GearIndex], GearRadii[GearIndex] + 18.0f + Pulse * 5.0f, GearColor, 50, 3.0f);
		}

		FCanvasTextItem KeyItem(
			FVector2D(GearCenters[GearIndex].X - 20.0f, GearCenters[GearIndex].Y + GearRadii[GearIndex] + 35.0f),
			FText::FromString(FString::Printf(TEXT("%d  %s"), GearIndex + 1, *GearInputs[GearIndex].ToString())),
			Font,
			bStoppedGear ? GearColor : TextSecondary());
		KeyItem.Scale = FVector2D(bStoppedGear ? 0.96f : 0.82f);
		Canvas->DrawItem(KeyItem);
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

	FCanvasTextItem ControlsItem(
		FVector2D(PanelX + PanelPadding, PanelY + PanelHeight - 52.0f),
		NSLOCTEXT("Day1SliceHUD", "GearControls", "数字键 1/2/3 拨动对应齿轮    互动键：拨动当前停转齿轮"),
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

	const FLinearColor FeedbackColor = FeedbackLooksLikeFailure(AdvancedInteractionState.FeedbackText)
		? FLinearColor(1.0f, 0.24f, 0.16f, 0.96f)
		: (AdvancedInteractionState.FeedbackText.IsEmpty() ? TextSecondary() : AccentColor);
	const FText Feedback = AdvancedInteractionState.FeedbackText.IsEmpty()
		? (AdvancedInteractionState.Mode == EHorrorCampaignInteractionMode::CircuitWiring
			? NSLOCTEXT("Day1SliceHUD", "CircuitHUDHint", "观察旋转光标，进入发光窗口后接入高亮端子。")
			: NSLOCTEXT("Day1SliceHUD", "GearHUDHint", "观察停转齿轮，进入校准窗口后拨动对应齿轮。"))
		: AdvancedInteractionState.FeedbackText;
	FCanvasTextItem FeedbackItem(
		FVector2D(BarX, PanelY + 4.0f),
		Feedback,
		Font,
		FeedbackColor);
	FeedbackItem.Scale = FVector2D(0.94f);
	Canvas->DrawItem(FeedbackItem);

	const FText ProgressText = FText::Format(
		NSLOCTEXT("Day1SliceHUD", "AdvancedProgress", "同步进度 {0}%"),
		FText::AsNumber(FMath::RoundToInt(AdvancedInteractionState.ProgressFraction * 100.0f)));
	FCanvasTextItem ProgressTextItem(
		FVector2D(BarX, BarY + 22.0f),
		ProgressText,
		Font,
		TextSecondary());
	ProgressTextItem.Scale = FVector2D(0.84f);
	Canvas->DrawItem(ProgressTextItem);
}

void ADay1SliceHUD::DrawAnomalyCaptureStatus(float CanvasWidth, float CanvasHeight)
{
	if (!bAnomalyCaptureStatusVisible || AnomalyCaptureStatus.IsEmpty())
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
	DrawPanel(PanelX, PanelY, ToastPanelWidth, PanelHeight, PanelBackground(), PanelAccent());

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
		TextPrimary());
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
	if (InteractionPrompt.IsEmpty() || bPasswordPromptVisible)
	{
		return;
	}

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font)
	{
		return;
	}

	const float PromptWidth = 420.0f;
	const float PromptHeight = 52.0f;
	const float PanelX = (CanvasWidth - PromptWidth) * 0.5f;
	const float PanelY = CanvasHeight - 132.0f;
	DrawPanel(PanelX, PanelY, PromptWidth, PromptHeight, PanelBackground(), PanelAccent());

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
	if (TransientMessageExpireWorldSeconds <= WorldSeconds || TransientMessage.IsEmpty())
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

	FCanvasTileItem AccentTile(FVector2D(X, Y), FVector2D(4.0f, Height), AccentColor);
	AccentTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(AccentTile);

	FCanvasTileItem TopTile(FVector2D(X, Y), FVector2D(Width, 1.0f), AccentColor.CopyWithNewOpacity(0.38f));
	TopTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TopTile);
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
