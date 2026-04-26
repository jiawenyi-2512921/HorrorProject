// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EventTracker.generated.h"

struct FAnalyticsEvent;

USTRUCT(BlueprintType)
struct FEventCategory
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString CategoryName;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	int32 EventCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FDateTime LastEventTime;
};

/**
 * Event Tracker - Tracks and categorizes analytics events
 */
UCLASS()
class HORRORPROJECT_API UEventTracker : public UObject
{
	GENERATED_BODY()

public:
	void TrackEvent(const FAnalyticsEvent& Event);
	void ClearAllEvents();

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	int32 GetEventCount(const FString& EventName) const;

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	TArray<FString> GetTopEvents(int32 Count = 10) const;

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	TMap<FString, int32> GetEventsByCategory() const;

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	float GetAverageEventsPerSession() const;

protected:
	UPROPERTY()
	TMap<FString, int32> EventCounts;

	UPROPERTY()
	TMap<FString, FEventCategory> EventCategories;

	UPROPERTY()
	TArray<FAnalyticsEvent> RecentEvents;

	static constexpr int32 MaxRecentEvents = 1000;

private:
	void CategorizeEvent(const FAnalyticsEvent& Event);
	FString GetEventCategory(const FString& EventName) const;
};
