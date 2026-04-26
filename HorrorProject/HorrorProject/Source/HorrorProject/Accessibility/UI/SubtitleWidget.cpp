// Copyright Epic Games, Inc. All Rights Reserved.

#include "SubtitleWidget.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "../AccessibilitySubsystem.h"
#include "Kismet/GameplayStatics.h"

void USubtitleWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
    if (GameInstance)
    {
        UAccessibilitySubsystem* AccessibilitySubsystem = GameInstance->GetSubsystem<UAccessibilitySubsystem>();
        if (AccessibilitySubsystem)
        {
            CurrentSettings = AccessibilitySubsystem->GetAccessibilitySettings();
            AccessibilitySubsystem->OnSubtitleDisplayed.AddDynamic(this, &USubtitleWidget::OnSubtitleDisplayed);
            AccessibilitySubsystem->OnAccessibilitySettingsChanged.AddDynamic(this, &USubtitleWidget::OnAccessibilitySettingsChanged);
        }
    }

    ApplySubtitleStyling();
    SetVisibility(ESlateVisibility::Collapsed);
}

void USubtitleWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (SubtitleTimeRemaining > 0.0f)
    {
        SubtitleTimeRemaining -= InDeltaTime;

        if (SubtitleTimeRemaining <= 0.0f)
        {
            ClearSubtitles();
        }
    }
}

void USubtitleWidget::DisplaySubtitle(const FText& Text, float Duration, const FString& SpeakerName)
{
    if (!CurrentSettings.bSubtitlesEnabled)
    {
        return;
    }

    if (SubtitleText)
    {
        SubtitleText->SetText(Text);
    }

    if (SpeakerNameText)
    {
        if (!SpeakerName.IsEmpty())
        {
            SpeakerNameText->SetText(FText::FromString(SpeakerName));
            SpeakerNameText->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            SpeakerNameText->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    SubtitleTimeRemaining = Duration;
    CurrentSubtitleDuration = Duration;
    SetVisibility(ESlateVisibility::Visible);
}

void USubtitleWidget::ClearSubtitles()
{
    SetVisibility(ESlateVisibility::Collapsed);
    SubtitleTimeRemaining = 0.0f;

    if (SubtitleText)
    {
        SubtitleText->SetText(FText::GetEmpty());
    }

    if (SpeakerNameText)
    {
        SpeakerNameText->SetText(FText::GetEmpty());
    }
}

void USubtitleWidget::UpdateSubtitleAppearance(const FAccessibilitySettings& Settings)
{
    CurrentSettings = Settings;
    ApplySubtitleStyling();
}

void USubtitleWidget::OnSubtitleDisplayed(const FText& SubtitleText, float Duration)
{
    DisplaySubtitle(SubtitleText, Duration);
}

void USubtitleWidget::OnAccessibilitySettingsChanged(const FAccessibilitySettings& NewSettings)
{
    UpdateSubtitleAppearance(NewSettings);
}

void USubtitleWidget::ApplySubtitleStyling()
{
    if (SubtitleText)
    {
        int32 FontSize = GetFontSizeForSubtitleSize(CurrentSettings.SubtitleSize);
        FSlateFontInfo FontInfo = SubtitleText->GetFont();
        FontInfo.Size = FontSize;
        SubtitleText->SetFont(FontInfo);
        SubtitleText->SetColorAndOpacity(FSlateColor(CurrentSettings.SubtitleColor));
    }

    if (SpeakerNameText)
    {
        int32 FontSize = GetFontSizeForSubtitleSize(CurrentSettings.SubtitleSize) + 2;
        FSlateFontInfo FontInfo = SpeakerNameText->GetFont();
        FontInfo.Size = FontSize;
        SpeakerNameText->SetFont(FontInfo);
        SpeakerNameText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
    }

    if (SubtitleBackground)
    {
        SubtitleBackground->SetBrushColor(CurrentSettings.SubtitleBackgroundColor);
    }
}

int32 USubtitleWidget::GetFontSizeForSubtitleSize(ESubtitleSize Size) const
{
    switch (Size)
    {
        case ESubtitleSize::Small:      return 16;
        case ESubtitleSize::Medium:     return 20;
        case ESubtitleSize::Large:      return 26;
        case ESubtitleSize::ExtraLarge: return 32;
        default:                        return 20;
    }
}
