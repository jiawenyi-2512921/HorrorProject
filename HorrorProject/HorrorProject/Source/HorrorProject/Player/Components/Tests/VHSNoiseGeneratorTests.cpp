#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/VHSNoiseGenerator.h"
#include "Misc/AutomationTest.h"
#include "Materials/MaterialInstanceDynamic.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVHSNoiseGeneratorInitializationTest,
	"HorrorProject.Player.VHSNoise.Initialization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVHSNoiseGeneratorInitializationTest::RunTest(const FString& Parameters)
{
	UVHSNoiseGenerator* NoiseGen = NewObject<UVHSNoiseGenerator>();

	TestNotNull(TEXT("VHS noise generator should be created"), NoiseGen);
	TestEqual(TEXT("Initial noise intensity should be 0"), NoiseGen->GetCurrentNoiseIntensity(), 0.0f);
	TestEqual(TEXT("Initial chromatic aberration should be 0"), NoiseGen->GetCurrentChromaticAberration(), 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVHSNoiseGeneratorParamsTest,
	"HorrorProject.Player.VHSNoise.Params",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVHSNoiseGeneratorParamsTest::RunTest(const FString& Parameters)
{
	UVHSNoiseGenerator* NoiseGen = NewObject<UVHSNoiseGenerator>();

	FVHSNoiseParams Params;
	Params.BaseNoiseIntensity = 0.25f;
	Params.ScanlineIntensity = 0.5f;
	Params.ChromaticAberration = 0.05f;
	Params.ScanlineCount = 600.0f;

	NoiseGen->SetNoiseParams(Params);

	FVHSNoiseParams RetrievedParams = NoiseGen->GetNoiseParams();
	TestEqual(TEXT("Base noise intensity should be set"), RetrievedParams.BaseNoiseIntensity, 0.25f);
	TestEqual(TEXT("Scanline intensity should be set"), RetrievedParams.ScanlineIntensity, 0.5f);
	TestEqual(TEXT("Chromatic aberration should be set"), RetrievedParams.ChromaticAberration, 0.05f);
	TestEqual(TEXT("Scanline count should be set"), RetrievedParams.ScanlineCount, 600.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVHSNoiseGeneratorUpdateTest,
	"HorrorProject.Player.VHSNoise.Update",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVHSNoiseGeneratorUpdateTest::RunTest(const FString& Parameters)
{
	UVHSNoiseGenerator* NoiseGen = NewObject<UVHSNoiseGenerator>();

	NoiseGen->UpdateNoise(0.016f, 0.0f, 1.0f);
	float LowStressNoise = NoiseGen->GetCurrentNoiseIntensity();

	NoiseGen->UpdateNoise(0.016f, 1.0f, 1.0f);
	float HighStressNoise = NoiseGen->GetCurrentNoiseIntensity();

	TestTrue(TEXT("High stress should increase noise"), HighStressNoise > LowStressNoise);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVHSNoiseGeneratorBatteryEffectTest,
	"HorrorProject.Player.VHSNoise.BatteryEffect",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVHSNoiseGeneratorBatteryEffectTest::RunTest(const FString& Parameters)
{
	UVHSNoiseGenerator* NoiseGen = NewObject<UVHSNoiseGenerator>();

	NoiseGen->UpdateNoise(0.016f, 0.0f, 1.0f);
	float FullBatteryNoise = NoiseGen->GetCurrentNoiseIntensity();

	NoiseGen->UpdateNoise(0.016f, 0.0f, 0.1f);
	float LowBatteryNoise = NoiseGen->GetCurrentNoiseIntensity();

	TestTrue(TEXT("Low battery should increase noise"), LowBatteryNoise > FullBatteryNoise);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVHSNoiseGeneratorChromaticAberrationTest,
	"HorrorProject.Player.VHSNoise.ChromaticAberration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVHSNoiseGeneratorChromaticAberrationTest::RunTest(const FString& Parameters)
{
	UVHSNoiseGenerator* NoiseGen = NewObject<UVHSNoiseGenerator>();

	NoiseGen->SetChromaticAberration(0.1f);
	NoiseGen->UpdateNoise(0.016f, 0.0f, 1.0f);
	float LowStressChromatic = NoiseGen->GetCurrentChromaticAberration();

	NoiseGen->UpdateNoise(0.016f, 1.0f, 1.0f);
	float HighStressChromatic = NoiseGen->GetCurrentChromaticAberration();

	TestTrue(TEXT("High stress should increase chromatic aberration"), HighStressChromatic > LowStressChromatic);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVHSNoiseGeneratorScanlineTest,
	"HorrorProject.Player.VHSNoise.Scanline",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVHSNoiseGeneratorScanlineTest::RunTest(const FString& Parameters)
{
	UVHSNoiseGenerator* NoiseGen = NewObject<UVHSNoiseGenerator>();

	NoiseGen->SetScanlineIntensity(0.6f);
	FVHSNoiseParams Params = NoiseGen->GetNoiseParams();
	TestEqual(TEXT("Scanline intensity should be set"), Params.ScanlineIntensity, 0.6f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVHSNoiseGeneratorBaseIntensityTest,
	"HorrorProject.Player.VHSNoise.BaseIntensity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVHSNoiseGeneratorBaseIntensityTest::RunTest(const FString& Parameters)
{
	UVHSNoiseGenerator* NoiseGen = NewObject<UVHSNoiseGenerator>();

	NoiseGen->SetBaseNoiseIntensity(0.3f);
	FVHSNoiseParams Params = NoiseGen->GetNoiseParams();
	TestEqual(TEXT("Base noise intensity should be set"), Params.BaseNoiseIntensity, 0.3f);

	return true;
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
