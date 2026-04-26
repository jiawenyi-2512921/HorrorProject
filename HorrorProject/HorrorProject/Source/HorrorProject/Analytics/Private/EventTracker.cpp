// Copyright Epic Games, Inc. All Rights Reserved.

#include "EventTracker.h"
#include "AnalyticsSubsystem.h"

void UEventTracker::TrackEvent(const FAnalyticsEvent& Event)
{
	// Increment event count
	int32& Count = EventCounts.FindOrAdd(Event.EventName, 0);
	Count++;

	// Categorize event
	CategorizeEvent(Event);

	// Store recent events
	RecentEvents.Add(Event);
	if (RecentEvents.Num() > MaxRecentEvents)
	{
		RecentEvents.RemoveAt(0, RecentEvents.Num() - MaxRecentEvents);
	}
}

void UEventTracker::ClearAllEvents()
{
	EventCounts.Empty();
	EventCategories.Empty();
	RecentEvents.Empty();
}

int32 UEventTracker::GetEventCount(const FString& EventName) const
{
	const int32* Count = EventCounts.Find(EventName);
	return Count ? *Count : 0;
}

TArray<FString> UEventTracker::GetTopEvents(int32 Count) const
{
	TArray<TPair<FString, int32>> SortedEvents;
	for (const auto& Pair : EventCounts)
	{
		SortedEvents.Add(Pair);
	}

	SortedEvents.Sort([](const TPair<FString, int32>& A, const TPair<FString, int32>& B)
	{
		return A.Value > B.Value;
	});

	TArray<FString> TopEvents;
	for (int32 i = 0; i < FMath::Min(Count, SortedEvents.Num()); i++)
	{
		TopEvents.Add(SortedEvents[i].Key);
	}

	return TopEvents;
}

TMap<FString, int32> UEventTracker::GetEventsByCategory() const
{
	TMap<FString, int32> CategoryCounts;
	for (const auto& Pair : EventCategories)
	{
		CategoryCounts.Add(Pair.Key, Pair.Value.EventCount);
	}
	return CategoryCounts;
}

float UEventTracker::GetAverageEventsPerSession() const
{
	// Calculate from recent events
	TSet<FString> UniqueSessions;
	for (const FAnalyticsEvent& Event : RecentEvents)
	{
		UniqueSessions.Add(Event.SessionId);
	}

	if (UniqueSessions.Num() == 0)
	{
		return 0.0f;
	}

	return static_cast<float>(RecentEvents.Num()) / UniqueSessions.Num();
}

void UEventTracker::CategorizeEvent(const FAnalyticsEvent& Event)
{
	FString Category = GetEventCategory(Event.EventName);

	FEventCategory& EventCategory = EventCategories.FindOrAdd(Category);
	EventCategory.CategoryName = Category;
	EventCategory.EventCount++;
	EventCategory.LastEventTime = Event.Timestamp;
}

FString UEventTracker::GetEventCategory(const FString& EventName) const
{
	// Categorize based on event name patterns
	if (EventName.Contains(TEXT("screen")) || EventName.Contains(TEXT("view")))
	{
		return TEXT("Navigation");
	}
	else if (EventName.Contains(TEXT("error")) || EventName.Contains(TEXT("crash")))
	{
		return TEXT("Errors");
	}
	else if (EventName.Contains(TEXT("purchase")) || EventName.Contains(TEXT("transaction")))
	{
		return TEXT("Monetization");
	}
	else if (EventName.Contains(TEXT("level")) || EventName.Contains(TEXT("game")))
	{
		return TEXT("Gameplay");
	}
	else if (EventName.Contains(TEXT("social")) || EventName.Contains(TEXT("share")))
	{
		return TEXT("Social");
	}
	else if (EventName.Contains(TEXT("settings")) || EventName.Contains(TEXT("config")))
	{
		return TEXT("Configuration");
	}

	return TEXT("Other");
}
