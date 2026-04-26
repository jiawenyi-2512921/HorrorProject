#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Achievements/AchievementSubsystem.h"
#include "Achievements/AchievementDefinition.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAchievementSystemTest, "HorrorProject.Achievements.AchievementSystem", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAchievementSystemTest::RunTest(const FString& Parameters)
{
	// Create test world
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Get achievement subsystem
	UAchievementSubsystem* AchievementSubsystem = TestWorld->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
	if (!AchievementSubsystem)
	{
		AddError(TEXT("Failed to get AchievementSubsystem"));
		return false;
	}

	// Test: Get all achievements
	TArray<FAchievementData> AllAchievements = AchievementSubsystem->GetAllAchievements();
	TestTrue(TEXT("Should have achievements defined"), AllAchievements.Num() > 0);

	// Test: Unlock achievement
	FName TestAchievementID = FName("ACH_FirstSteps");
	AchievementSubsystem->UnlockAchievement(TestAchievementID);
	TestTrue(TEXT("Achievement should be unlocked"), AchievementSubsystem->IsAchievementUnlocked(TestAchievementID));

	// Test: Get unlocked count
	int32 UnlockedCount = AchievementSubsystem->GetUnlockedCount();
	TestEqual(TEXT("Should have 1 unlocked achievement"), UnlockedCount, 1);

	// Test: Progressive achievement
	FName ProgressiveAchievementID = FName("ACH_Explorer");
	AchievementSubsystem->UpdateAchievementProgress(ProgressiveAchievementID, 5.0f);
	float Progress = AchievementSubsystem->GetAchievementProgress(ProgressiveAchievementID);
	TestEqual(TEXT("Progress should be 50%"), Progress, 0.5f);

	// Test: Complete progressive achievement
	AchievementSubsystem->UpdateAchievementProgress(ProgressiveAchievementID, 10.0f);
	TestTrue(TEXT("Progressive achievement should be unlocked"), AchievementSubsystem->IsAchievementUnlocked(ProgressiveAchievementID));

	// Test: Completion percentage
	float CompletionPercentage = AchievementSubsystem->GetCompletionPercentage();
	TestTrue(TEXT("Completion percentage should be > 0"), CompletionPercentage > 0.0f);

	// Test: Get locked achievements
	TArray<FAchievementData> LockedAchievements = AchievementSubsystem->GetLockedAchievements();
	TestTrue(TEXT("Should have locked achievements"), LockedAchievements.Num() > 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAchievementDefinitionTest, "HorrorProject.Achievements.AchievementDefinition", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAchievementDefinitionTest::RunTest(const FString& Parameters)
{
	// Test: Get all achievement definitions
	TArray<FAchievementDefinition> Definitions = UAchievementDefinition::GetAllAchievementDefinitions();
	TestTrue(TEXT("Should have at least 20 achievements"), Definitions.Num() >= 20);

	// Test: Check categories
	bool bHasExploration = false;
	bool bHasCollection = false;
	bool bHasSurvival = false;
	bool bHasSpeedrun = false;
	bool bHasSecret = false;

	for (const FAchievementDefinition& Def : Definitions)
	{
		if (Def.Category == EAchievementCategory::Exploration) bHasExploration = true;
		if (Def.Category == EAchievementCategory::Collection) bHasCollection = true;
		if (Def.Category == EAchievementCategory::Survival) bHasSurvival = true;
		if (Def.Category == EAchievementCategory::Speedrun) bHasSpeedrun = true;
		if (Def.Category == EAchievementCategory::Secret) bHasSecret = true;
	}

	TestTrue(TEXT("Should have Exploration achievements"), bHasExploration);
	TestTrue(TEXT("Should have Collection achievements"), bHasCollection);
	TestTrue(TEXT("Should have Survival achievements"), bHasSurvival);
	TestTrue(TEXT("Should have Speedrun achievements"), bHasSpeedrun);
	TestTrue(TEXT("Should have Secret achievements"), bHasSecret);

	// Test: Check progressive achievements
	int32 ProgressiveCount = 0;
	for (const FAchievementDefinition& Def : Definitions)
	{
		if (Def.bIsProgressive)
		{
			ProgressiveCount++;
			TestTrue(TEXT("Progressive achievement should have MaxProgress > 1"), Def.MaxProgress > 1.0f);
		}
	}

	TestTrue(TEXT("Should have progressive achievements"), ProgressiveCount > 0);

	// Test: Check hidden achievements
	int32 HiddenCount = 0;
	for (const FAchievementDefinition& Def : Definitions)
	{
		if (Def.bIsHidden)
		{
			HiddenCount++;
		}
	}

	TestTrue(TEXT("Should have hidden achievements"), HiddenCount > 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAchievementEventsTest, "HorrorProject.Achievements.Events", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAchievementEventsTest::RunTest(const FString& Parameters)
{
	// Create test world
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestWorld)
	{
		AddError(TEXT("Failed to create test world"));
		return false;
	}

	// Get achievement subsystem
	UAchievementSubsystem* AchievementSubsystem = TestWorld->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();
	if (!AchievementSubsystem)
	{
		AddError(TEXT("Failed to get AchievementSubsystem"));
		return false;
	}

	// Test: Achievement unlocked event
	bool bEventFired = false;
	AchievementSubsystem->OnAchievementUnlocked.AddLambda([&bEventFired](const FAchievementData& Achievement)
	{
		bEventFired = true;
	});

	AchievementSubsystem->UnlockAchievement(FName("ACH_FirstSteps"));
	TestTrue(TEXT("OnAchievementUnlocked event should fire"), bEventFired);

	// Test: Achievement progress event
	bool bProgressEventFired = false;
	AchievementSubsystem->OnAchievementProgress.AddLambda([&bProgressEventFired](FName AchievementID, float Progress)
	{
		bProgressEventFired = true;
	});

	AchievementSubsystem->UpdateAchievementProgress(FName("ACH_Explorer"), 5.0f);
	TestTrue(TEXT("OnAchievementProgress event should fire"), bProgressEventFired);

	return true;
}
