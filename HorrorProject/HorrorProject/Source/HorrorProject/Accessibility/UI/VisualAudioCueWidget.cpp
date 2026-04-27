// Copyright Epic Games, Inc. All Rights Reserved.

#include "VisualAudioCueWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "../AccessibilitySubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void UVisualAudioCueWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
    if (GameInstance)
    {
        UAccessibilitySubsystem* AccessibilitySubsystem = GameInstance->GetSubsystem<UAccessibilitySubsystem>();
        if (AccessibilitySubsystem)
        {
            AccessibilitySubsystem->OnVisualAudioCue.AddDynamic(this, &UVisualAudioCueWidget::OnVisualAudioCue);
        }
    }
}

void UVisualAudioCueWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    for (FVisualCue& Cue : ActiveCues)
    {
        Cue.TimeRemaining -= InDeltaTime;
    }
    ActiveCues.RemoveAll([](const FVisualCue& Cue)
    {
        return Cue.TimeRemaining <= 0.0f;
    });

    UpdateCuePositions();
}

void UVisualAudioCueWidget::AddVisualCue(const FVector& WorldLocation, const FString& CueType, float Intensity)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
        return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn)
        return;

    // Check distance
    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), WorldLocation);
    if (Distance > MaxCueDistance)
        return;

    FVisualCue NewCue;
    NewCue.ScreenPosition = WorldToScreen(WorldLocation);
    NewCue.CueType = CueType;
    NewCue.Intensity = Intensity;
    NewCue.TimeRemaining = CueDuration;
    NewCue.Color = GetColorForCueType(CueType);

    ActiveCues.Add(NewCue);
    OnCueAdded(NewCue);
}

void UVisualAudioCueWidget::ClearAllCues()
{
    ActiveCues.Empty();
}

void UVisualAudioCueWidget::OnVisualAudioCue(const FVector& Location, const FString& CueType, float Intensity)
{
    AddVisualCue(Location, CueType, Intensity);
}

FVector2D UVisualAudioCueWidget::WorldToScreen(const FVector& WorldLocation) const
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC)
        return FVector2D::ZeroVector;

    FVector2D ScreenPosition;
    PC->ProjectWorldLocationToScreen(WorldLocation, ScreenPosition);

    return ScreenPosition;
}

FLinearColor UVisualAudioCueWidget::GetColorForCueType(const FString& CueType) const
{
    if (CueType.Contains(TEXT("Footstep")))
        return FLinearColor::Yellow;
    else if (CueType.Contains(TEXT("Voice")))
        return FLinearColor::Green;
    else if (CueType.Contains(TEXT("Danger")))
        return FLinearColor::Red;
    else if (CueType.Contains(TEXT("Door")))
        return FLinearColor::Blue;
    else if (CueType.Contains(TEXT("Item")))
        return FLinearColor::White;
    else
        return FLinearColor(0.5f, 0.5f, 0.5f);
}

void UVisualAudioCueWidget::UpdateCuePositions()
{
    // Update screen positions for all active cues
    for (FVisualCue& Cue : ActiveCues)
    {
        // Position would be updated in Blueprint implementation
    }
}
