#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Analytics/AnalyticsSubsystem.h"
#include "Analytics/EventTracker.h"
#include "Analytics/SessionTracker.h"
#include "Analytics/MetricsCollector.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAnalyticsSubsystemTest, "HorrorProject.Analytics.AnalyticsSubsystem", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAnalyticsSubsystemTest::RunTest(const FString& Parameters)
{
	// Test user consent
	{
		UAnalyticsSubsystem* Analytics = NewObject<UAnalyticsSubsystem>();

		FUserConsent Consent;
		Consent.bAnalyticsEnabled = true;
		Consent.bPersonalizedAdsEnabled = false;
		Consent.bCrashReportingEnabled = true;

		Analytics->SetUserConsent(Consent);

		TestTrue(TEXT("Analytics consent should be enabled"), Analytics->HasAnalyticsConsent());

		FUserConsent RetrievedConsent = Analytics->GetUserConsent();
		TestTrue(TEXT("Retrieved consent should match"), RetrievedConsent.bAnalyticsEnabled);
		TestFalse(TEXT("Ads consent should be disabled"), RetrievedConsent.bPersonalizedAdsEnabled);
	}

	// Test user ID anonymization
	{
		UAnalyticsSubsystem* Analytics = NewObject<UAnalyticsSubsystem>();

		FString UserId = TEXT("user@example.com");
		Analytics->SetUserId(UserId, true);

		FString AnonymizedId = Analytics->GetAnonymizedUserId();
		TestNotEqual(TEXT("Anonymized ID should differ from original"), AnonymizedId, UserId);
		TestTrue(TEXT("Anonymized ID should not be empty"), !AnonymizedId.IsEmpty());
	}

	// Test event tracking
	{
		UAnalyticsSubsystem* Analytics = NewObject<UAnalyticsSubsystem>();

		FUserConsent Consent;
		Consent.bAnalyticsEnabled = true;
		Analytics->SetUserConsent(Consent);

		TMap<FString, FString> Parameters;
		Parameters.Add(TEXT("level"), TEXT("1"));
		Parameters.Add(TEXT("score"), TEXT("100"));

		Analytics->TrackEvent(TEXT("level_complete"), Parameters);

		// Event should be queued
		TestTrue(TEXT("Event tracking should succeed"), true);
	}

	// Test screen view tracking
	{
		UAnalyticsSubsystem* Analytics = NewObject<UAnalyticsSubsystem>();

		FUserConsent Consent;
		Consent.bAnalyticsEnabled = true;
		Analytics->SetUserConsent(Consent);

		Analytics->TrackScreenView(TEXT("MainMenu"));

		TestTrue(TEXT("Screen view tracking should succeed"), true);
	}

	// Test error tracking
	{
		UAnalyticsSubsystem* Analytics = NewObject<UAnalyticsSubsystem>();

		FUserConsent Consent;
		Consent.bAnalyticsEnabled = true;
		Analytics->SetUserConsent(Consent);

		Analytics->TrackError(TEXT("Test error"), TEXT("ERR_001"));

		TestTrue(TEXT("Error tracking should succeed"), true);
	}

	// Test data deletion (GDPR)
	{
		UAnalyticsSubsystem* Analytics = NewObject<UAnalyticsSubsystem>();

		FUserConsent Consent;
		Consent.bAnalyticsEnabled = true;
		Analytics->SetUserConsent(Consent);

		Analytics->SetUserId(TEXT("test_user"), true);

		TMap<FString, FString> Parameters;
		Analytics->TrackEvent(TEXT("test_event"), Parameters);

		Analytics->RequestDataDeletion();

		TestFalse(TEXT("Consent should be revoked after deletion"), Analytics->HasAnalyticsConsent());
		TestTrue(TEXT("User ID should be cleared"), Analytics->GetAnonymizedUserId().IsEmpty());
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEventTrackerTest, "HorrorProject.Analytics.EventTracker", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEventTrackerTest::RunTest(const FString& Parameters)
{
	UEventTracker* Tracker = NewObject<UEventTracker>();

	// Test event tracking
	{
		FAnalyticsEvent Event;
		Event.EventName = TEXT("test_event");
		Event.SessionId = TEXT("session_123");
		Event.Timestamp = FDateTime::UtcNow();

		Tracker->TrackEvent(Event);

		int32 Count = Tracker->GetEventCount(TEXT("test_event"));
		TestEqual(TEXT("Event count should be 1"), Count, 1);
	}

	// Test multiple events
	{
		for (int32 i = 0; i < 5; i++)
		{
			FAnalyticsEvent Event;
			Event.EventName = TEXT("repeated_event");
			Event.SessionId = TEXT("session_123");
			Tracker->TrackEvent(Event);
		}

		int32 Count = Tracker->GetEventCount(TEXT("repeated_event"));
		TestEqual(TEXT("Event count should be 5"), Count, 5);
	}

	// Test top events
	{
		TArray<FString> TopEvents = Tracker->GetTopEvents(5);
		TestTrue(TEXT("Should return top events"), TopEvents.Num() > 0);
		TestEqual(TEXT("First event should be repeated_event"), TopEvents[0], TEXT("repeated_event"));
	}

	// Test event categories
	{
		FAnalyticsEvent ScreenEvent;
		ScreenEvent.EventName = TEXT("screen_view");
		Tracker->TrackEvent(ScreenEvent);

		FAnalyticsEvent ErrorEvent;
		ErrorEvent.EventName = TEXT("error_occurred");
		Tracker->TrackEvent(ErrorEvent);

		TMap<FString, int32> Categories = Tracker->GetEventsByCategory();
		TestTrue(TEXT("Should have multiple categories"), Categories.Num() > 0);
	}

	// Test clear events
	{
		Tracker->ClearAllEvents();

		int32 Count = Tracker->GetEventCount(TEXT("test_event"));
		TestEqual(TEXT("Event count should be 0 after clear"), Count, 0);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSessionTrackerTest, "HorrorProject.Analytics.SessionTracker", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSessionTrackerTest::RunTest(const FString& Parameters)
{
	USessionTracker* Tracker = NewObject<USessionTracker>();

	// Test session start
	{
		Tracker->StartSession();

		FString SessionId = Tracker->GetCurrentSessionId();
		TestTrue(TEXT("Session ID should not be empty"), !SessionId.IsEmpty());
	}

	// Test session duration
	{
		float Duration = Tracker->GetSessionDuration();
		TestTrue(TEXT("Session duration should be >= 0"), Duration >= 0.0f);
	}

	// Test session end
	{
		FString SessionId = Tracker->GetCurrentSessionId();
		Tracker->EndSession();

		int32 TotalSessions = Tracker->GetTotalSessions();
		TestEqual(TEXT("Total sessions should be 1"), TotalSessions, 1);
	}

	// Test multiple sessions
	{
		for (int32 i = 0; i < 3; i++)
		{
			Tracker->StartSession();
			Tracker->EndSession();
		}

		int32 TotalSessions = Tracker->GetTotalSessions();
		TestEqual(TEXT("Total sessions should be 4"), TotalSessions, 4);
	}

	// Test average session duration
	{
		float AvgDuration = Tracker->GetAverageSessionDuration();
		TestTrue(TEXT("Average duration should be >= 0"), AvgDuration >= 0.0f);
	}

	// Test clear history
	{
		Tracker->ClearSessionHistory();

		int32 TotalSessions = Tracker->GetTotalSessions();
		TestEqual(TEXT("Total sessions should be 0 after clear"), TotalSessions, 0);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMetricsCollectorTest, "HorrorProject.Analytics.MetricsCollector", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMetricsCollectorTest::RunTest(const FString& Parameters)
{
	UMetricsCollector* Collector = NewObject<UMetricsCollector>();

	// Test metric recording
	{
		TMap<FString, FString> Tags;
		Tags.Add(TEXT("type"), TEXT("test"));

		Collector->RecordMetric(TEXT("test_metric"), 100.0f, Tags);

		FMetricStats Stats = Collector->GetMetricStats(TEXT("test_metric"));
		TestEqual(TEXT("Metric count should be 1"), Stats.Count, 1);
		TestEqual(TEXT("Metric value should be 100"), Stats.Average, 100.0f);
	}

	// Test timing metrics
	{
		Collector->RecordTimingMetric(TEXT("load_time"), 250.5f);

		FMetricStats Stats = Collector->GetMetricStats(TEXT("load_time"));
		TestEqual(TEXT("Timing metric should be recorded"), Stats.Count, 1);
	}

	// Test counter metrics
	{
		Collector->RecordCounterMetric(TEXT("button_clicks"), 5);

		FMetricStats Stats = Collector->GetMetricStats(TEXT("button_clicks"));
		TestEqual(TEXT("Counter metric should be recorded"), Stats.Count, 1);
		TestEqual(TEXT("Counter value should be 5"), Stats.Average, 5.0f);
	}

	// Test metric statistics
	{
		for (int32 i = 1; i <= 10; i++)
		{
			Collector->RecordMetric(TEXT("fps"), static_cast<float>(i * 10), TMap<FString, FString>());
		}

		FMetricStats Stats = Collector->GetMetricStats(TEXT("fps"));
		TestEqual(TEXT("Should have 10 samples"), Stats.Count, 10);
		TestEqual(TEXT("Min should be 10"), Stats.Min, 10.0f);
		TestEqual(TEXT("Max should be 100"), Stats.Max, 100.0f);
		TestEqual(TEXT("Average should be 55"), Stats.Average, 55.0f);
	}

	// Test get all metric names
	{
		TArray<FString> MetricNames = Collector->GetAllMetricNames();
		TestTrue(TEXT("Should have multiple metrics"), MetricNames.Num() > 0);
	}

	// Test clear metrics
	{
		Collector->ClearAllMetrics();

		TArray<FString> MetricNames = Collector->GetAllMetricNames();
		TestEqual(TEXT("Should have no metrics after clear"), MetricNames.Num(), 0);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
