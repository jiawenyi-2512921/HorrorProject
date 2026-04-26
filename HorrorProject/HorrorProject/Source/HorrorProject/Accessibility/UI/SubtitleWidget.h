// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../AccessibilitySubsystem.h"
#include "../SubtitleSystem.h"
#include "SubtitleWidget.generated.h"

class UTextBlock;
class UBorder;
class UVerticalBox;

/**
 * Subtitle Widget
 * Displays subtitles with customizable size, color, and background
 * WCAG 2.1 AA compliant with proper contrast ratios
 */
UCLASS()
class HORRORPROJECT_API USubtitleWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void DisplaySubtitle(const FText& Text, float Duration, const FString& SpeakerName = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void ClearSubtitles();

    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void UpdateSubtitleAppearance(const FAccessibilitySettings& Settings);

protected:
    UPROPERTY(meta = (BindWidget))
    UVerticalBox* SubtitleContainer;

    UPROPERTY(meta = (BindWidget))
    UBorder* SubtitleBackground;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* SubtitleText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* SpeakerNameText;

private:
    UFUNCTION()
    void OnSubtitleDisplayed(const FText& InSubtitleText, float Duration);

    UFUNCTION()
    void OnAccessibilitySettingsChanged(const FAccessibilitySettings& NewSettings);

    UPROPERTY()
    USubtitleSystem* SubtitleSystem;

    UPROPERTY()
    FAccessibilitySettings CurrentSettings;

    float CurrentSubtitleDuration = 0.0f;
    float SubtitleTimeRemaining = 0.0f;

    void ApplySubtitleStyling();
    int32 GetFontSizeForSubtitleSize(ESubtitleSize Size) const;
};
