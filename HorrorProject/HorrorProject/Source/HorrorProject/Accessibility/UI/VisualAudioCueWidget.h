// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VisualAudioCueWidget.generated.h"

class UImage;
class UCanvasPanel;

USTRUCT(BlueprintType)
struct FVisualCue
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "AudioCue")
    FVector2D ScreenPosition;

    UPROPERTY(BlueprintReadWrite, Category = "AudioCue")
    FString CueType;

    UPROPERTY(BlueprintReadWrite, Category = "AudioCue")
    float Intensity = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "AudioCue")
    float TimeRemaining = 2.0f;

    UPROPERTY(BlueprintReadWrite, Category = "AudioCue")
    FLinearColor Color = FLinearColor::White;
};

/**
 * Visual Audio Cue Widget
 * Displays visual indicators for audio events (footsteps, sounds, etc.)
 * Essential for hearing-impaired accessibility
 */
UCLASS()
class HORRORPROJECT_API UVisualAudioCueWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "AudioCue")
    void AddVisualCue(const FVector& WorldLocation, const FString& CueType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "AudioCue")
    void ClearAllCues();

    UFUNCTION(BlueprintImplementableEvent, Category = "AudioCue")
    void OnCueAdded(const FVisualCue& Cue);

protected:
    UPROPERTY(meta = (BindWidget))
    UCanvasPanel* CueCanvas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioCue")
    float CueDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioCue")
    float MaxCueDistance = 3000.0f;

private:
    UFUNCTION()
    void OnVisualAudioCue(const FVector& Location, const FString& CueType, float Intensity);

    UPROPERTY()
    TArray<FVisualCue> ActiveCues;

    FVector2D WorldToScreen(const FVector& WorldLocation) const;
    FLinearColor GetColorForCueType(const FString& CueType) const;
    void UpdateCuePositions();
};
