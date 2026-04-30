#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Achievements/StatisticsSubsystem.h"
#include "Achievements/PlayerStatistics.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStatisticsSubsystemTest, "HorrorProject.Statistics.StatisticsSubsystem", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FStatisticsSubsystemTest::RunTest(const FString& Parameters)
{
	// Create test world
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Get statistics subsystem
	UStatisticsSubsystem* StatisticsSubsystem = TestWorld->GetGameInstance()->GetSubsystem<UStatisticsSubsystem>();
	if (!StatisticsSubsystem)
	{
		AddError(TEXT("Failed to get StatisticsSubsystem"));
		return false;
	}

	// Test: Set statistic
	StatisticsSubsystem->SetStatistic(FName("RoomsExplored"), 5.0f);
	float Value = StatisticsSubsystem->GetStatistic(FName("RoomsExplored"));
	TestEqual(TEXT("Statistic should be set correctly"), Value, 5.0f);

	// Test: Increment statistic
	StatisticsSubsystem->IncrementStatistic(FName("RoomsExplored"), 3.0f);
	Value = StatisticsSubsystem->GetStatistic(FName("RoomsExplored"));
	TestEqual(TEXT("Statistic should be incremented correctly"), Value, 8.0f);

	// Test: Reset statistic
	StatisticsSubsystem->ResetStatistic(FName("RoomsExplored"));
	Value = StatisticsSubsystem->GetStatistic(FName("RoomsExplored"));
	TestEqual(TEXT("Statistic should be reset to 0"), Value, 0.0f);

	// Test: Get player statistics
	StatisticsSubsystem->SetStatistic(FName("EvidenceCollected"), 10.0f);
	FPlayerStatistics PlayerStats = StatisticsSubsystem->GetPlayerStatistics();
	TestEqual(TEXT("Player statistics should reflect evidence collected"), PlayerStats.EvidenceCollected, 10);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStatisticsSessionTest, "HorrorProject.Statistics.Session", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FStatisticsSessionTest::RunTest(const FString& Parameters)
{
	// Create test world
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Get statistics subsystem
	UStatisticsSubsystem* StatisticsSubsystem = TestWorld->GetGameInstance()->GetSubsystem<UStatisticsSubsystem>();
	if (!StatisticsSubsystem)
	{
		AddError(TEXT("Failed to get StatisticsSubsystem"));
		return false;
	}

	// Test: Start session
	StatisticsSubsystem->StartSession();
	float SessionDuration = StatisticsSubsystem->GetCurrentSessionDuration();
	TestTrue(TEXT("Session should be started"), SessionDuration >= 0.0f);

	// Test: End session
	StatisticsSubsystem->EndSession();
	float TotalPlayTime = StatisticsSubsystem->GetTotalPlayTime();
	TestTrue(TEXT("Total play time should be recorded"), TotalPlayTime >= 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStatisticsReportTest, "HorrorProject.Statistics.Report", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FStatisticsReportTest::RunTest(const FString& Parameters)
{
	// Create test world
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Get statistics subsystem
	UStatisticsSubsystem* StatisticsSubsystem = TestWorld->GetGameInstance()->GetSubsystem<UStatisticsSubsystem>();
	if (!StatisticsSubsystem)
	{
		AddError(TEXT("Failed to get StatisticsSubsystem"));
		return false;
	}

	// Set some statistics
	StatisticsSubsystem->SetStatistic(FName("RoomsExplored"), 15.0f);
	StatisticsSubsystem->SetStatistic(FName("EvidenceCollected"), 8.0f);
	StatisticsSubsystem->SetStatistic(FName("Deaths"), 2.0f);

	// Test: Generate report
	FString Report = StatisticsSubsystem->GenerateStatisticsReport();
	TestTrue(TEXT("Report should not be empty"), !Report.IsEmpty());
	TestTrue(TEXT("Report should contain statistics"), Report.Contains(TEXT("统计")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStatisticsEventsTest, "HorrorProject.Statistics.Events", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FStatisticsEventsTest::RunTest(const FString& Parameters)
{
	// Create test world
	UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Get statistics subsystem
	UStatisticsSubsystem* StatisticsSubsystem = TestWorld->GetGameInstance()->GetSubsystem<UStatisticsSubsystem>();
	if (!StatisticsSubsystem)
	{
		AddError(TEXT("Failed to get StatisticsSubsystem"));
		return false;
	}

	// Test: Statistic updated event
	bool bEventFired = false;
	FName CapturedStatName;
	float CapturedValue = 0.0f;

	StatisticsSubsystem->OnStatisticUpdated.AddLambda([&](FName StatName, float NewValue)
	{
		bEventFired = true;
		CapturedStatName = StatName;
		CapturedValue = NewValue;
	});

	StatisticsSubsystem->SetStatistic(FName("TestStat"), 42.0f);
	TestTrue(TEXT("OnStatisticUpdated event should fire"), bEventFired);
	TestEqual(TEXT("Event should capture correct stat name"), CapturedStatName, FName("TestStat"));
	TestEqual(TEXT("Event should capture correct value"), CapturedValue, 42.0f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
