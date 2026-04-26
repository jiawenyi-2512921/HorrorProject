#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Achievements/AchievementSubsystem.h"
#include "Achievements/AchievementTracker.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAchievementTrackerTest, "HorrorProject.Achievements.Tracker", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAchievementTrackerTest::RunTest(const FString& Parameters)
{
	// Create test world
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Create test actor with tracker
	AActor* TestActor = TestWorld->SpawnActor<AActor>();
	UAchievementTracker* Tracker = NewObject<UAchievementTracker>(TestActor);
	Tracker->RegisterComponent();

	// Get achievement subsystem
	UAchievementSubsystem* AchievementSubsystem = TestWorld->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
	if (!AchievementSubsystem)
	{
		AddError(TEXT("Failed to get AchievementSubsystem"));
		return false;
	}

	// Test: Track room discovered
	Tracker->TrackRoomDiscovered(FName("Room_01"));
	TestTrue(TEXT("First Steps achievement should unlock"), AchievementSubsystem->IsAchievementUnlocked(FName("ACH_FirstSteps")));

	// Test: Track evidence collected
	Tracker->TrackEvidenceCollected(FName("Evidence_01"));
	TestTrue(TEXT("First Clue achievement should unlock"), AchievementSubsystem->IsAchievementUnlocked(FName("ACH_FirstClue")));

	// Test: Track ghost encounter
	Tracker->TrackGhostEncounter();
	TestTrue(TEXT("First Encounter achievement should unlock"), AchievementSubsystem->IsAchievementUnlocked(FName("ACH_FirstEncounter")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProgressiveAchievementTest, "HorrorProject.Achievements.Progressive", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FProgressiveAchievementTest::RunTest(const FString& Parameters)
{
	// Create test world
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Create test actor with tracker
	AActor* TestActor = TestWorld->SpawnActor<AActor>();
	UAchievementTracker* Tracker = NewObject<UAchievementTracker>(TestActor);
	Tracker->RegisterComponent();

	// Get achievement subsystem
	UAchievementSubsystem* AchievementSubsystem = TestWorld->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
	if (!AchievementSubsystem)
	{
		AddError(TEXT("Failed to get AchievementSubsystem"));
		return false;
	}

	// Test: Track multiple rooms for Explorer achievement
	for (int32 i = 0; i < 10; i++)
	{
		Tracker->TrackRoomDiscovered(FName(*FString::Printf(TEXT("Room_%02d"), i)));
	}

	TestTrue(TEXT("Explorer achievement should unlock after 10 rooms"),
		AchievementSubsystem->IsAchievementUnlocked(FName("ACH_Explorer")));

	// Test: Track multiple evidence for Detective achievement
	for (int32 i = 0; i < 10; i++)
	{
		Tracker->TrackEvidenceCollected(FName(*FString::Printf(TEXT("Evidence_%02d"), i)));
	}

	TestTrue(TEXT("Detective achievement should unlock after 10 evidence"),
		AchievementSubsystem->IsAchievementUnlocked(FName("ACH_Detective")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSecretAchievementTest, "HorrorProject.Achievements.Secret", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSecretAchievementTest::RunTest(const FString& Parameters)
{
	// Create test world
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Create test actor with tracker
	AActor* TestActor = TestWorld->SpawnActor<AActor>();
	UAchievementTracker* Tracker = NewObject<UAchievementTracker>(TestActor);
	Tracker->RegisterComponent();

	// Get achievement subsystem
	UAchievementSubsystem* AchievementSubsystem = TestWorld->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
	if (!AchievementSubsystem)
	{
		AddError(TEXT("Failed to get AchievementSubsystem"));
		return false;
	}

	// Test: Track secret passage
	Tracker->TrackSecretFound(FName("SecretPassage"));
	TestTrue(TEXT("Secret Passage achievement should unlock"),
		AchievementSubsystem->IsAchievementUnlocked(FName("ACH_SecretPassage")));

	// Test: Track hidden secrets
	Tracker->TrackSecretFound(FName("TheWatcher"));
	TestTrue(TEXT("The Watcher achievement should unlock"),
		AchievementSubsystem->IsAchievementUnlocked(FName("ACH_TheWatcher")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSurvivalAchievementTest, "HorrorProject.Achievements.Survival", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSurvivalAchievementTest::RunTest(const FString& Parameters)
{
	// Create test world
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Create test actor with tracker
	AActor* TestActor = TestWorld->SpawnActor<AActor>();
	UAchievementTracker* Tracker = NewObject<UAchievementTracker>(TestActor);
	Tracker->RegisterComponent();

	// Get achievement subsystem
	UAchievementSubsystem* AchievementSubsystem = TestWorld->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
	if (!AchievementSubsystem)
	{
		AddError(TEXT("Failed to get AchievementSubsystem"));
		return false;
	}

	// Test: Track multiple ghost encounters
	for (int32 i = 0; i < 5; i++)
	{
		Tracker->TrackGhostEncounter();
	}

	TestTrue(TEXT("Survivor achievement should unlock after 5 encounters"),
		AchievementSubsystem->IsAchievementUnlocked(FName("ACH_Survivor")));

	// Test: Track sanity level
	Tracker->TrackSanityLevel(75.0f);
	Tracker->TrackGameComplete(3600.0f);

	TestTrue(TEXT("Nerves of Steel achievement should unlock with high sanity"),
		AchievementSubsystem->IsAchievementUnlocked(FName("ACH_NervesOfSteel")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpeedrunAchievementTest, "HorrorProject.Achievements.Speedrun", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FSpeedrunAchievementTest::RunTest(const FString& Parameters)
{
	// Create test world
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Create test actor with tracker
	AActor* TestActor = TestWorld->SpawnActor<AActor>();
	UAchievementTracker* Tracker = NewObject<UAchievementTracker>(TestActor);
	Tracker->RegisterComponent();

	// Get achievement subsystem
	UAchievementSubsystem* AchievementSubsystem = TestWorld->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
	if (!AchievementSubsystem)
	{
		AddError(TEXT("Failed to get AchievementSubsystem"));
		return false;
	}

	// Test: Complete game under 2 hours
	Tracker->TrackGameComplete(7000.0f); // 1 hour 56 minutes
	TestTrue(TEXT("Speed Runner achievement should unlock"),
		AchievementSubsystem->IsAchievementUnlocked(FName("ACH_SpeedRunner")));

	// Test: Complete game under 1 hour
	AchievementSubsystem->ResetStatistics();
	Tracker->TrackGameComplete(3500.0f); // 58 minutes
	TestTrue(TEXT("Lightning Fast achievement should unlock"),
		AchievementSubsystem->IsAchievementUnlocked(FName("ACH_LightningFast")));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
