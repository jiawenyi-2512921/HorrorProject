// Copyright HorrorProject. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Game/HorrorDirectorSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHorrorDirectorTensionTest,
	"HorrorProject.Game.Director.AddTensionIncreasesValue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorDirectorTensionTest::RunTest(const FString& Parameters)
{
	UHorrorDirectorSubsystem* Director = NewObject<UHorrorDirectorSubsystem>();

	TestEqual(TEXT("Initial tension should be 0"), Director->GetTension(), 0.0f);

	Director->AddTension(0.5f);
	TestTrue(TEXT("Tension should increase"), Director->GetTension() > 0.0f);
	TestTrue(TEXT("Tension should be 0.5"), FMath::IsNearlyEqual(Director->GetTension(), 0.5f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHorrorDirectorTensionClampTest,
	"HorrorProject.Game.Director.TensionClampedToOne",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorDirectorTensionClampTest::RunTest(const FString& Parameters)
{
	UHorrorDirectorSubsystem* Director = NewObject<UHorrorDirectorSubsystem>();

	Director->AddTension(5.0f);
	TestTrue(TEXT("Tension should clamp to 1.0"), FMath::IsNearlyEqual(Director->GetTension(), 1.0f));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHorrorDirectorReduceTensionTest,
	"HorrorProject.Game.Director.ReduceTensionDecreasesValue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorDirectorReduceTensionTest::RunTest(const FString& Parameters)
{
	UHorrorDirectorSubsystem* Director = NewObject<UHorrorDirectorSubsystem>();

	Director->AddTension(0.8f);
	Director->ReduceTension(0.3f);
	TestTrue(TEXT("Tension should be 0.5"), FMath::IsNearlyEqual(Director->GetTension(), 0.5f));

	Director->ReduceTension(1.0f);
	TestEqual(TEXT("Tension should clamp to 0"), Director->GetTension(), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHorrorDirectorTensionLevelTest,
	"HorrorProject.Game.Director.TensionLevelTransitions",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorDirectorTensionLevelTest::RunTest(const FString& Parameters)
{
	UHorrorDirectorSubsystem* Director = NewObject<UHorrorDirectorSubsystem>();

	TestEqual(TEXT("Initial level should be Silent"), Director->GetTensionLevel(), ETensionLevel::Silent);

	Director->AddTension(0.2f);
	TestEqual(TEXT("Should be Uneasy at 0.2"), Director->GetTensionLevel(), ETensionLevel::Uneasy);

	Director->AddTension(0.2f);
	TestEqual(TEXT("Should be Tense at 0.4"), Director->GetTensionLevel(), ETensionLevel::Tense);

	Director->AddTension(0.2f);
	TestEqual(TEXT("Should be Dread at 0.6"), Director->GetTensionLevel(), ETensionLevel::Dread);

	Director->AddTension(0.2f);
	TestEqual(TEXT("Should be Peak at 0.8"), Director->GetTensionLevel(), ETensionLevel::Peak);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHorrorDirectorEnableDisableTest,
	"HorrorProject.Game.Director.EnableDisableState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorDirectorEnableDisableTest::RunTest(const FString& Parameters)
{
	UHorrorDirectorSubsystem* Director = NewObject<UHorrorDirectorSubsystem>();

	TestTrue(TEXT("Director should be enabled by default"), Director->IsDirectorEnabled());

	Director->SetDirectorEnabled(false);
	TestFalse(TEXT("Director should be disabled"), Director->IsDirectorEnabled());

	Director->SetDirectorEnabled(true);
	TestTrue(TEXT("Director should be re-enabled"), Director->IsDirectorEnabled());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
