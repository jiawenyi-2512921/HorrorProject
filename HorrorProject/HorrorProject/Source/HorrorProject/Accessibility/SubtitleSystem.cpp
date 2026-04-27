// Copyright Epic Games, Inc. All Rights Reserved.

#include "SubtitleSystem.h"

void USubtitleSystem::AddSubtitle(const FText& Text, const FString& SpeakerName, float Duration, int32 Priority, bool bIsEnvironmental)
{
    FSubtitleEntry NewEntry;
    NewEntry.Text = Text;
    NewEntry.SpeakerName = SpeakerName;
    NewEntry.Duration = Duration;
    NewEntry.TimeRemaining = Duration;
    NewEntry.Priority = Priority;
    NewEntry.bIsEnvironmental = bIsEnvironmental;

    ActiveSubtitles.Add(NewEntry);
    SortSubtitlesByPriority();
    TrimSubtitleQueue();

    OnSubtitleQueueChanged.Broadcast(ActiveSubtitles);
}

void USubtitleSystem::RemoveSubtitle(int32 Index)
{
    if (ActiveSubtitles.IsValidIndex(Index))
    {
        ActiveSubtitles.RemoveAt(Index);
        OnSubtitleQueueChanged.Broadcast(ActiveSubtitles);
    }
}

void USubtitleSystem::ClearAllSubtitles()
{
    ActiveSubtitles.Empty();
    OnSubtitleQueueChanged.Broadcast(ActiveSubtitles);
}

void USubtitleSystem::UpdateSubtitles(float DeltaTime)
{
    for (FSubtitleEntry& Subtitle : ActiveSubtitles)
    {
        Subtitle.TimeRemaining -= DeltaTime;
    }

    const int32 RemovedCount = ActiveSubtitles.RemoveAll([](const FSubtitleEntry& Subtitle)
    {
        return Subtitle.TimeRemaining <= 0.0f;
    });

    if (RemovedCount > 0)
    {
        OnSubtitleQueueChanged.Broadcast(ActiveSubtitles);
    }
}

void USubtitleSystem::SetMaxSimultaneousSubtitles(int32 Max)
{
    MaxSimultaneousSubtitles = FMath::Max(1, Max);
    TrimSubtitleQueue();
}

FText USubtitleSystem::FormatSubtitleText(const FSubtitleEntry& Entry) const
{
    if (Entry.SpeakerName.IsEmpty())
    {
        return Entry.Text;
    }

    FString FormattedText = FString::Printf(TEXT("[%s]: %s"),
        *Entry.SpeakerName,
        *Entry.Text.ToString());

    return FText::FromString(FormattedText);
}

void USubtitleSystem::SortSubtitlesByPriority()
{
    ActiveSubtitles.Sort([](const FSubtitleEntry& A, const FSubtitleEntry& B)
    {
        return A.Priority > B.Priority;
    });
}

void USubtitleSystem::TrimSubtitleQueue()
{
    if (ActiveSubtitles.Num() > MaxSimultaneousSubtitles)
    {
        ActiveSubtitles.SetNum(MaxSimultaneousSubtitles);
        OnSubtitleQueueChanged.Broadcast(ActiveSubtitles);
    }
}
