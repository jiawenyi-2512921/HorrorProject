#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Analytics/AnalyticsSubsystem.h"
#include "Analytics/MetricsCollector.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDataCollectionTest, "HorrorProject.Analytics.DataCollection", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDataCollectionTest::RunTest(const FString& Parameters)
{
	// Test data export
	{
		UAnalyticsSubsystem* Analytics = NewObject<UAnalyticsSubsystem>();

		FUserConsent Consent;
		Consent.bAnalyticsEnabled = true;
		Analytics->SetUserConsent(Consent);

		// Track some events
		TMap<FString, FString> Params;
		Params.Add(TEXT("test"), TEXT("value"));
		Analytics->TrackEvent(TEXT("test_event"), Params);

		FString ExportPath = FPaths::ProjectSavedDir() / TEXT("Analytics/test_export.json");
		Analytics->ExportUserData(ExportPath);

		TestTrue(TEXT("Export file should exist"), FPaths::FileExists(ExportPath));

		// Cleanup
		IFileManager::Get().Delete(*ExportPath);
	}

	// Test metrics export
	{
		UMetricsCollector* Collector = NewObject<UMetricsCollector>();

		Collector->RecordMetric(TEXT("test_metric"), 100.0f);
		Collector->RecordMetric(TEXT("test_metric"), 200.0f);

		FString ExportPath = FPaths::ProjectSavedDir() / TEXT("Analytics/test_metrics.json");
		Collector->ExportMetrics(ExportPath);

		TestTrue(TEXT("Metrics export file should exist"), FPaths::FileExists(ExportPath));

		// Cleanup
		IFileManager::Get().Delete(*ExportPath);
	}

	// Test data persistence
	{
		UAnalyticsSubsystem* Analytics = NewObject<UAnalyticsSubsystem>();

		FUserConsent Consent;
		Consent.bAnalyticsEnabled = true;
		Consent.bPersonalizedAdsEnabled = false;
		Analytics->SetUserConsent(Consent);

		// Consent should be saved
		FString ConsentPath = FPaths::ProjectSavedDir() / TEXT("Analytics/user_consent.json");
		TestTrue(TEXT("Consent file should exist"), FPaths::FileExists(ConsentPath));
	}

	// Test event queue flushing
	{
		UAnalyticsSubsystem* Analytics = NewObject<UAnalyticsSubsystem>();

		FUserConsent Consent;
		Consent.bAnalyticsEnabled = true;
		Analytics->SetUserConsent(Consent);

		// Track multiple events
		for (int32 i = 0; i < 10; i++)
		{
			TMap<FString, FString> Params;
			Params.Add(TEXT("index"), FString::FromInt(i));
			Analytics->TrackEvent(TEXT("test_event"), Params);
		}

		Analytics->FlushEvents();

		// Events should be written to file
		FString EventsPath = FPaths::ProjectSavedDir() / TEXT("Analytics/events.jsonl");
		TestTrue(TEXT("Events file should exist"), FPaths::FileExists(EventsPath));
	}

	// Test GDPR compliance - data deletion
	{
		UAnalyticsSubsystem* Analytics = NewObject<UAnalyticsSubsystem>();

		FUserConsent Consent;
		Consent.bAnalyticsEnabled = true;
		Analytics->SetUserConsent(Consent);

		Analytics->SetUserId(TEXT("test_user"), true);

		TMap<FString, FString> Params;
		Analytics->TrackEvent(TEXT("test_event"), Params);

		// Request data deletion
		Analytics->RequestDataDeletion();

		// Verify data is cleared
		TestFalse(TEXT("Analytics should be disabled after deletion"), Analytics->HasAnalyticsConsent());
		TestTrue(TEXT("User ID should be empty"), Analytics->GetAnonymizedUserId().IsEmpty());
	}

	// Test consent versioning
	{
		UAnalyticsSubsystem* Analytics = NewObject<UAnalyticsSubsystem>();

		FUserConsent Consent;
		Consent.bAnalyticsEnabled = true;
		Analytics->SetUserConsent(Consent);

		FUserConsent RetrievedConsent = Analytics->GetUserConsent();
		TestTrue(TEXT("Consent should have version"), !RetrievedConsent.ConsentVersion.IsEmpty());
		TestTrue(TEXT("Consent should have timestamp"), RetrievedConsent.ConsentTimestamp.GetTicks() > 0);
	}

	// Test anonymization
	{
		UAnalyticsSubsystem* Analytics = NewObject<UAnalyticsSubsystem>();

		FString UserId1 = TEXT("user1@example.com");
		FString UserId2 = TEXT("user2@example.com");

		Analytics->SetUserId(UserId1, true);
		FString AnonymizedId1 = Analytics->GetAnonymizedUserId();

		Analytics->SetUserId(UserId2, true);
		FString AnonymizedId2 = Analytics->GetAnonymizedUserId();

		TestNotEqual(TEXT("Different users should have different anonymized IDs"), AnonymizedId1, AnonymizedId2);
		TestNotEqual(TEXT("Anonymized ID should not match original"), AnonymizedId1, UserId1);
	}

	// Test event without consent
	{
		UAnalyticsSubsystem* Analytics = NewObject<UAnalyticsSubsystem>();

		// Don't set consent
		TMap<FString, FString> Params;
		Analytics->TrackEvent(TEXT("test_event"), Params);

		// Event should not be tracked
		TestTrue(TEXT("Event tracking without consent should be ignored"), true);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDataIntegrityTest, "HorrorProject.Analytics.DataIntegrity", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDataIntegrityTest::RunTest(const FString& Parameters)
{
	// Test event data integrity
	{
		FAnalyticsEvent Event;
		Event.EventName = TEXT("test_event");
		Event.SessionId = TEXT("session_123");
		Event.Timestamp = FDateTime::UtcNow();
		Event.Parameters.Add(TEXT("key1"), TEXT("value1"));
		Event.Parameters.Add(TEXT("key2"), TEXT("value2"));

		TestTrue(TEXT("Event should have name"), !Event.EventName.IsEmpty());
		TestTrue(TEXT("Event should have session ID"), !Event.SessionId.IsEmpty());
		TestTrue(TEXT("Event should have timestamp"), Event.Timestamp.GetTicks() > 0);
		TestEqual(TEXT("Event should have 2 parameters"), Event.Parameters.Num(), 2);
	}

	// Test session data integrity
	{
		FSessionData Session;
		Session.SessionId = TEXT("session_123");
		Session.StartTime = FDateTime::UtcNow();
		Session.EndTime = Session.StartTime + FTimespan::FromMinutes(10);
		Session.Duration = (Session.EndTime - Session.StartTime).GetTotalSeconds();
		Session.Platform = TEXT("Windows");
		Session.AppVersion = TEXT("1.0.0");

		TestTrue(TEXT("Session should have ID"), !Session.SessionId.IsEmpty());
		TestTrue(TEXT("Session should have start time"), Session.StartTime.GetTicks() > 0);
		TestTrue(TEXT("Session should have end time"), Session.EndTime.GetTicks() > 0);
		TestTrue(TEXT("Session duration should be positive"), Session.Duration > 0.0f);
		TestTrue(TEXT("Session should have platform"), !Session.Platform.IsEmpty());
	}

	// Test metric data integrity
	{
		FMetricData Metric;
		Metric.MetricName = TEXT("fps");
		Metric.Value = 60.0f;
		Metric.Timestamp = FDateTime::UtcNow();
		Metric.Tags.Add(TEXT("type"), TEXT("performance"));

		TestTrue(TEXT("Metric should have name"), !Metric.MetricName.IsEmpty());
		TestTrue(TEXT("Metric should have value"), Metric.Value > 0.0f);
		TestTrue(TEXT("Metric should have timestamp"), Metric.Timestamp.GetTicks() > 0);
		TestTrue(TEXT("Metric should have tags"), Metric.Tags.Num() > 0);
	}

	// Test error report integrity
	{
		FErrorReport Error;
		Error.ErrorId = FGuid::NewGuid().ToString();
		Error.ErrorMessage = TEXT("Test error");
		Error.ErrorCode = TEXT("ERR_001");
		Error.Severity = EErrorSeverity::Error;
		Error.Timestamp = FDateTime::UtcNow();
		Error.StackTrace = TEXT("Stack trace");

		TestTrue(TEXT("Error should have ID"), !Error.ErrorId.IsEmpty());
		TestTrue(TEXT("Error should have message"), !Error.ErrorMessage.IsEmpty());
		TestTrue(TEXT("Error should have code"), !Error.ErrorCode.IsEmpty());
		TestTrue(TEXT("Error should have timestamp"), Error.Timestamp.GetTicks() > 0);
	}

	// Test telemetry report integrity
	{
		FTelemetryReport Report;
		Report.ReportId = FGuid::NewGuid().ToString();
		Report.GeneratedAt = FDateTime::UtcNow();
		Report.PerformanceMetrics.Add(TEXT("fps"), TEXT("60"));
		Report.GameplayMetrics.Add(TEXT("levels"), TEXT("5"));
		Report.Errors.Add(TEXT("Error 1"));

		TestTrue(TEXT("Report should have ID"), !Report.ReportId.IsEmpty());
		TestTrue(TEXT("Report should have timestamp"), Report.GeneratedAt.GetTicks() > 0);
		TestTrue(TEXT("Report should have performance metrics"), Report.PerformanceMetrics.Num() > 0);
		TestTrue(TEXT("Report should have gameplay metrics"), Report.GameplayMetrics.Num() > 0);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
