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
	constexpr float ObjectivePanelWidth = 520.0f;
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
	DrawAutosaveIndicator(CanvasWidth);
	DrawObjectiveToast(CanvasWidth);
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

void ADay1SliceHUD::ShowTransientMessage(const FText& MessageText, const FLinearColor& MessageColor, float DisplaySeconds)
{
	TransientMessage = MessageText;
	TransientMessageColor = MessageColor;
	TransientMessageExpireWorldSeconds = GetWorldSeconds() + FMath::Max(0.1f, DisplaySeconds);
}

void ADay1SliceHUD::ShowAutosaveIndicator(const FText& IndicatorText, float DisplaySeconds)
{
	AutosaveIndicatorText = IndicatorText.IsEmpty()
		? FText::FromString(TEXT("检查点已保存。"))
		: IndicatorText;
	AutosaveIndicatorExpireWorldSeconds = GetWorldSeconds() + FMath::Max(0.1f, DisplaySeconds);
}

void ADay1SliceHUD::ShowDay1CompletionOverlay(const FText& TitleText, const FText& HintText)
{
	Day1CompletionTitle = TitleText.IsEmpty()
		? FText::FromString(TEXT("第 1 天完成"))
		: TitleText;
	Day1CompletionHint = HintText.IsEmpty()
		? FText::FromString(TEXT("证据已保全。"))
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
	NotesJournalEmptyText = FText::FromString(TEXT("尚未记录笔记。"));
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

void ADay1SliceHUD::DrawCurrentObjective(float CanvasWidth)
{
	if (CurrentObjective.IsEmpty())
	{
		return;
	}

	const float PanelX = OuterPadding;
	const float PanelY = OuterPadding;
	const bool bHasNavigation = !ObjectiveNavigation.IsEmpty();
	const float PanelHeight = bHasNavigation ? 114.0f : 86.0f;
	DrawPanel(PanelX, PanelY, ObjectivePanelWidth, PanelHeight, PanelBackground(), PanelAccent());

	UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
	if (!Font)
	{
		return;
	}

	Canvas->SetDrawColor(FColor::White);

	FCanvasTextItem LabelItem(
		FVector2D(PanelX + PanelPadding, PanelY + 12.0f),
		FText::FromString(TEXT("目标")),
		Font,
		TextSecondary());
	LabelItem.Scale = FVector2D(1.0f);
	Canvas->DrawItem(LabelItem);

	FCanvasTextItem ObjectiveItem(
		FVector2D(PanelX + PanelPadding, PanelY + 39.0f),
		CurrentObjective,
		Font,
		TextPrimary());
	ObjectiveItem.Scale = FVector2D(1.08f);
	Canvas->DrawItem(ObjectiveItem);

	if (bHasNavigation)
	{
		FCanvasTextItem NavigationItem(
			FVector2D(PanelX + PanelPadding, PanelY + 73.0f),
			ObjectiveNavigation,
			Font,
			PanelAccent());
		NavigationItem.Scale = FVector2D(0.94f);
		Canvas->DrawItem(NavigationItem);
	}
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

	const FText Title = ToastTitle.IsEmpty() ? FText::FromString(TEXT("目标已更新")) : ToastTitle;
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
		: (bStatusRecording ? TEXT("录制中") : (bStatusBodycamEnabled ? TEXT("摄像机在线") : TEXT("摄像机关闭")));
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
		const FString LowBatterySuffix = bBodycamBatteryLow ? TEXT("  低电量") : TEXT("");
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
		? FText::FromString(TEXT("已锁定的门"))
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
		? FText::FromString(TEXT("在站内寻找密码。"))
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
		FText::FromString(TEXT("数字键：输入密码    回车键：确认    退格键：删除    Esc键：取消")),
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
		FText::FromString(TEXT("暂停")),
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
		FText::FromString(TEXT("上/下：选择    左/右：调整    回车键/空格键/Esc键：继续")),
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
		FText::FromString(TEXT("笔记")),
		Font,
		TextPrimary());
	TitleItem.Scale = FVector2D(1.35f);
	Canvas->DrawItem(TitleItem);

	if (NotesJournalEntries.IsEmpty())
	{
		FCanvasTextItem EmptyItem(
			FVector2D(PanelX + PanelPadding, PanelY + 92.0f),
			NotesJournalEmptyText.IsEmpty() ? FText::FromString(TEXT("尚未记录笔记。")) : NotesJournalEmptyText,
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
				? FText::FromString(TEXT("无法读取这份笔记的内容。"))
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
		FText::FromString(TEXT("J键/Tab键/Esc键：关闭")),
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

float ADay1SliceHUD::GetWorldSeconds() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.0f;
}
