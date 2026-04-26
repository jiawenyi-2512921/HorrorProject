// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SubtitleSystem.generated.h"

USTRUCT(BlueprintType)
struct FSubtitleEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Subtitle")
    FText Text;

    UPROPERTY(BlueprintReadWrite, Category = "Subtitle")
    FString SpeakerName;

    UPROPERTY(BlueprintReadWrite, Category = "Subtitle")
    float Duration = 3.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Subtitle")
    float TimeRemaining = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Subtitle")
    int32 Priority = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Subtitle")
    bool bIsEnvironmental = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSubtitleQueueChanged, const TArray<FSubtitleEntry>&, ActiveSubtitles);

/**
 * Subtitle System
 * Manages subtitle display queue, timing, and formatting
 */
UCLASS(BlueprintType)
class HORRORPROJECT_API USubtitleSystem : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void AddSubtitle(const FText& Text, const FString& SpeakerName, float Duration, int32 Priority = 0, bool bIsEnvironmental = false);

    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void RemoveSubtitle(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void ClearAllSubtitles();

    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void UpdateSubtitles(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Subtitle")
    TArray<FSubtitleEntry> GetActiveSubtitles() const { return ActiveSubtitles; }

    UFUNCTION(BlueprintPure, Category = "Subtitle")
    int32 GetMaxSimultaneousSubtitles() const { return MaxSimultaneousSubtitles; }

    UFUNCTION(BlueprintCallable, Category = "Subtitle")
    void SetMaxSimultaneousSubtitles(int32 Max);

    UFUNCTION(BlueprintPure, Category = "Subtitle")
    FText FormatSubtitleText(const FSubtitleEntry& Entry) const;

    UPROPERTY(BlueprintAssignable, Category = "Subtitle")
    FOnSubtitleQueueChanged OnSubtitleQueueChanged;

private:
    UPROPERTY()
    TArray<FSubtitleEntry> ActiveSubtitles;

    UPROPERTY()
    int32 MaxSimultaneousSubtitles = 3;

    void SortSubtitlesByPriority();
    void TrimSubtitleQueue();
};
