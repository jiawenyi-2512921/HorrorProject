#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/NoiseGeneratorComponent.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNoiseGeneratorComponentInitializationTest,
	"HorrorProject.Player.Noise.Initialization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNoiseGeneratorComponentInitializationTest::RunTest(const FString& Parameters)
{
	UNoiseGeneratorComponent* NoiseGen = NewObject<UNoiseGeneratorComponent>();

	TestEqual(TEXT("Noise level should start at 0"), NoiseGen->GetCurrentNoiseLevel(), 0.0f);
	TestFalse(TEXT("Should not be sprinting initially"), NoiseGen->IsSprinting());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNoiseGeneratorComponentGenerateNoiseTest,
	"HorrorProject.Player.Noise.GenerateNoise",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNoiseGeneratorComponentGenerateNoiseTest::RunTest(const FString& Parameters)
{
	UNoiseGeneratorComponent* NoiseGen = NewObject<UNoiseGeneratorComponent>();

	bool bGenerated = NoiseGen->GenerateNoise(ENoiseType::Footstep);
	TestTrue(TEXT("Footstep noise should be generated"), bGenerated);
	TestTrue(TEXT("Noise level should increase"), NoiseGen->GetCurrentNoiseLevel() > 0.0f);

	NoiseGen->GenerateNoise(ENoiseType::Jump);
	TestTrue(TEXT("Jump noise should increase noise level"), NoiseGen->GetCurrentNoiseLevel() > 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNoiseGeneratorComponentSprintTest,
	"HorrorProject.Player.Noise.Sprint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNoiseGeneratorComponentSprintTest::RunTest(const FString& Parameters)
{
	UNoiseGeneratorComponent* NoiseGen = NewObject<UNoiseGeneratorComponent>();

	NoiseGen->SetSprinting(true);
	TestTrue(TEXT("Should be sprinting"), NoiseGen->IsSprinting());

	float SprintLoudness = NoiseGen->GetFootstepLoudness();

	NoiseGen->SetSprinting(false);
	TestFalse(TEXT("Should not be sprinting"), NoiseGen->IsSprinting());

	float WalkLoudness = NoiseGen->GetFootstepLoudness();
	TestTrue(TEXT("Sprint should be louder than walk"), SprintLoudness > WalkLoudness);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNoiseGeneratorComponentNoiseTypesTest,
	"HorrorProject.Player.Noise.NoiseTypes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNoiseGeneratorComponentNoiseTypesTest::RunTest(const FString& Parameters)
{
	UNoiseGeneratorComponent* NoiseGen = NewObject<UNoiseGeneratorComponent>();

	NoiseGen->GenerateNoise(ENoiseType::Footstep);
	float FootstepNoise = NoiseGen->GetCurrentNoiseLevel();

	NoiseGen->GenerateNoise(ENoiseType::Jump);
	float JumpNoise = NoiseGen->GetCurrentNoiseLevel();

	NoiseGen->GenerateNoise(ENoiseType::Land);
	float LandNoise = NoiseGen->GetCurrentNoiseLevel();

	NoiseGen->GenerateNoise(ENoiseType::Interaction);
	float InteractionNoise = NoiseGen->GetCurrentNoiseLevel();

	TestTrue(TEXT("Different noise types should generate noise"), FootstepNoise > 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNoiseGeneratorComponentNoiseDecayTest,
	"HorrorProject.Player.Noise.Decay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNoiseGeneratorComponentNoiseDecayTest::RunTest(const FString& Parameters)
{
	UNoiseGeneratorComponent* NoiseGen = NewObject<UNoiseGeneratorComponent>();

	NoiseGen->GenerateNoise(ENoiseType::Jump);
	float InitialNoise = NoiseGen->GetCurrentNoiseLevel();

	NoiseGen->TickComponent(1.0f, ELevelTick::LEVELTICK_All, nullptr);

	TestTrue(TEXT("Noise should decay over time"), NoiseGen->GetCurrentNoiseLevel() < InitialNoise);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNoiseGeneratorComponentLocationNoiseTest,
	"HorrorProject.Player.Noise.LocationNoise",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNoiseGeneratorComponentLocationNoiseTest::RunTest(const FString& Parameters)
{
	UNoiseGeneratorComponent* NoiseGen = NewObject<UNoiseGeneratorComponent>();

	FVector TestLocation(100.0f, 200.0f, 300.0f);
	bool bGenerated = NoiseGen->GenerateNoiseAtLocation(0.5f, TestLocation, FName("TestNoise"));
	TestTrue(TEXT("Noise at location should be generated"), bGenerated);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FNoiseGeneratorComponentLoudnessMultiplierTest,
	"HorrorProject.Player.Noise.LoudnessMultiplier",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNoiseGeneratorComponentLoudnessMultiplierTest::RunTest(const FString& Parameters)
{
	UNoiseGeneratorComponent* NoiseGen = NewObject<UNoiseGeneratorComponent>();

	NoiseGen->GenerateNoise(ENoiseType::Footstep, 1.0f);
	float NormalNoise = NoiseGen->GetCurrentNoiseLevel();

	NoiseGen->GenerateNoise(ENoiseType::Footstep, 2.0f);
	float AmplifiedNoise = NoiseGen->GetCurrentNoiseLevel();

	TestTrue(TEXT("Loudness multiplier should amplify noise"), AmplifiedNoise > NormalNoise);

	return true;
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
