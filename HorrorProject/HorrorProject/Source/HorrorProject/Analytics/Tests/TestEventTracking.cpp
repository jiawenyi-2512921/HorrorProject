#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Analytics/TelemetrySubsystem.h"
#include "Analytics/PerformanceTelemetry.h"
#include "Analytics/GameplayTelemetry.h"
#include "Analytics/ErrorTelemetry.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPerformanceTelemetryTest, "HorrorProject.Analytics.PerformanceTelemetry", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPerformanceTelemetryTest::RunTest(const FString& Parameters)
{
	UPerformanceTelemetry* Telemetry = NewObject<UPerformanceTelemetry>();

	// Test performance tracking
	{
		float DeltaTime = 0.016f; // ~60 FPS
		Telemetry->Update(DeltaTime);

		float AvgFPS = Telemetry->GetAverageFPS();
		TestTrue(TEXT("Average FPS should be > 0"), AvgFPS > 0.0f);
	}

	// Test multiple updates
	{
		for (int32 i = 0; i < 100; i++)
		{
			float DeltaTime = 0.016f + (FMath::FRand() * 0.004f); // 60 FPS +/- variance
			Telemetry->Update(DeltaTime);
		}

		float MinFPS = Telemetry->GetMinFPS();
		float MaxFPS = Telemetry->GetMaxFPS();
		float AvgFPS = Telemetry->GetAverageFPS();

		TestTrue(TEXT("Min FPS should be <= Avg FPS"), MinFPS <= AvgFPS);
		TestTrue(TEXT("Max FPS should be >= Avg FPS"), MaxFPS >= AvgFPS);
	}

	// Test frame time calculation
	{
		float AvgFrameTime = Telemetry->GetAverageFrameTime();
		TestTrue(TEXT("Average frame time should be > 0"), AvgFrameTime > 0.0f);
	}

	// Test memory usage
	{
		float MemoryUsage = Telemetry->GetMemoryUsageMB();
		TestTrue(TEXT("Memory usage should be > 0"), MemoryUsage > 0.0f);
	}

	// Test snapshot
	{
		FPerformanceTelemetrySnapshot Snapshot = Telemetry->GetCurrentSnapshot();
		TestTrue(TEXT("Snapshot should have valid FPS"), Snapshot.FPS > 0.0f);
	}

	// Test reset
	{
		Telemetry->ResetStats();

		TArray<FPerformanceTelemetrySnapshot> History = Telemetry->GetPerformanceHistory();
		TestEqual(TEXT("History should be empty after reset"), History.Num(), 0);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameplayTelemetryTest, "HorrorProject.Analytics.GameplayTelemetry", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FGameplayTelemetryTest::RunTest(const FString& Parameters)
{
	UGameplayTelemetry* Telemetry = NewObject<UGameplayTelemetry>();

	// Test playtime tracking
	{
		Telemetry->Update(1.0f);

		float Playtime = Telemetry->GetTotalPlaytime();
		TestEqual(TEXT("Playtime should be 1 second"), Playtime, 1.0f);
	}

	// Test level completion
	{
		Telemetry->RecordLevelCompleted(TEXT("Level_1"));

		int32 LevelsCompleted = Telemetry->GetLevelsCompleted();
		TestEqual(TEXT("Levels completed should be 1"), LevelsCompleted, 1);
	}

	// Test death tracking
	{
		Telemetry->RecordDeath(TEXT("Enemy"));
		Telemetry->RecordDeath(TEXT("Fall"));

		int32 Deaths = Telemetry->GetDeathCount();
		TestEqual(TEXT("Death count should be 2"), Deaths, 2);

		TMap<FString, int32> DeathCauses = Telemetry->GetDeathCauses();
		TestEqual(TEXT("Should have 2 death causes"), DeathCauses.Num(), 2);
	}

	// Test achievement tracking
	{
		Telemetry->RecordAchievement(TEXT("ACH_001"));
		Telemetry->RecordAchievement(TEXT("ACH_002"));
		Telemetry->RecordAchievement(TEXT("ACH_001")); // Duplicate

		int32 Achievements = Telemetry->GetAchievementsUnlocked();
		TestEqual(TEXT("Should have 2 unique achievements"), Achievements, 2);
	}

	// Test item collection
	{
		Telemetry->RecordItemCollected(TEXT("Key"));
		Telemetry->RecordItemCollected(TEXT("Coin"));
		Telemetry->RecordItemCollected(TEXT("Key"));

		FGameplayTelemetryMetrics Metrics = Telemetry->GetMetrics();
		TestEqual(TEXT("Should have collected 3 items"), Metrics.ItemsCollected, 3);
	}

	// Test enemy defeats
	{
		Telemetry->RecordEnemyDefeated(TEXT("Zombie"));
		Telemetry->RecordEnemyDefeated(TEXT("Ghost"));

		FGameplayTelemetryMetrics Metrics = Telemetry->GetMetrics();
		TestEqual(TEXT("Should have defeated 2 enemies"), Metrics.EnemiesDefeated, 2);
	}

	// Test distance tracking
	{
		Telemetry->RecordDistanceTraveled(100.0f);
		Telemetry->RecordDistanceTraveled(50.0f);

		FGameplayTelemetryMetrics Metrics = Telemetry->GetMetrics();
		TestEqual(TEXT("Total distance should be 150"), Metrics.DistanceTraveled, 150.0f);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FErrorTelemetryTest, "HorrorProject.Analytics.ErrorTelemetry", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FErrorTelemetryTest::RunTest(const FString& Parameters)
{
	UErrorTelemetry* Telemetry = NewObject<UErrorTelemetry>();

	// Test error reporting
	{
		Telemetry->ReportError(TEXT("Test error"), TEXT("ERR_001"), EErrorSeverity::Warning);

		int32 ErrorCount = Telemetry->GetErrorCount();
		TestEqual(TEXT("Error count should be 1"), ErrorCount, 1);
	}

	// Test multiple errors
	{
		Telemetry->ReportError(TEXT("Error 2"), TEXT("ERR_002"), EErrorSeverity::Error);
		Telemetry->ReportError(TEXT("Error 3"), TEXT("ERR_003"), EErrorSeverity::Critical);

		int32 ErrorCount = Telemetry->GetErrorCount();
		TestEqual(TEXT("Error count should be 3"), ErrorCount, 3);
	}

	// Test error severity filtering
	{
		int32 WarningCount = Telemetry->GetErrorCountBySeverity(EErrorSeverity::Warning);
		int32 ErrorCount = Telemetry->GetErrorCountBySeverity(EErrorSeverity::Error);
		int32 CriticalCount = Telemetry->GetErrorCountBySeverity(EErrorSeverity::Critical);

		TestEqual(TEXT("Should have 1 warning"), WarningCount, 1);
		TestEqual(TEXT("Should have 1 error"), ErrorCount, 1);
		TestEqual(TEXT("Should have 1 critical"), CriticalCount, 1);
	}

	// Test recent errors
	{
		TArray<FString> RecentErrors = Telemetry->GetRecentErrors(2);
		TestEqual(TEXT("Should return 2 recent errors"), RecentErrors.Num(), 2);
	}

	// Test exception reporting
	{
		Telemetry->ReportException(TEXT("Null pointer exception"), TEXT("Stack trace here"));

		int32 CriticalCount = Telemetry->GetErrorCountBySeverity(EErrorSeverity::Critical);
		TestEqual(TEXT("Should have 2 critical errors"), CriticalCount, 2);
	}

	// Test error history
	{
		TArray<FErrorReport> History = Telemetry->GetErrorHistory();
		TestTrue(TEXT("Should have error history"), History.Num() > 0);

		for (const FErrorReport& Error : History)
		{
			TestTrue(TEXT("Error should have ID"), !Error.ErrorId.IsEmpty());
			TestTrue(TEXT("Error should have message"), !Error.ErrorMessage.IsEmpty());
		}
	}

	// Test clear history
	{
		Telemetry->ClearErrorHistory();

		int32 ErrorCount = Telemetry->GetErrorCount();
		TestEqual(TEXT("Error count should be 0 after clear"), ErrorCount, 0);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTelemetrySubsystemTest, "HorrorProject.Analytics.TelemetrySubsystem", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTelemetrySubsystemTest::RunTest(const FString& Parameters)
{
	UTelemetrySubsystem* Telemetry = NewObject<UTelemetrySubsystem>();

	// Test initialization
	{
		TestTrue(TEXT("Telemetry should be enabled by default"), Telemetry->IsTelemetryEnabled());
	}

	// Test component access
	{
		UPerformanceTelemetry* PerfTelemetry = Telemetry->GetPerformanceTelemetry();
		UGameplayTelemetry* GameplayTelemetry = Telemetry->GetGameplayTelemetry();
		UErrorTelemetry* ErrorTelemetry = Telemetry->GetErrorTelemetry();

		TestNotNull(TEXT("Performance telemetry should exist"), PerfTelemetry);
		TestNotNull(TEXT("Gameplay telemetry should exist"), GameplayTelemetry);
		TestNotNull(TEXT("Error telemetry should exist"), ErrorTelemetry);
	}

	// Test start/stop
	{
		Telemetry->StartTelemetryCollection();
		TestTrue(TEXT("Telemetry should be enabled"), Telemetry->IsTelemetryEnabled());

		Telemetry->StopTelemetryCollection();
		TestFalse(TEXT("Telemetry should be disabled"), Telemetry->IsTelemetryEnabled());
	}

	// Test report generation
	{
		Telemetry->StartTelemetryCollection();

		FTelemetryReport Report = Telemetry->GenerateReport();

		TestTrue(TEXT("Report should have ID"), !Report.ReportId.IsEmpty());
		TestTrue(TEXT("Report should have timestamp"), Report.GeneratedAt.GetTicks() > 0);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
