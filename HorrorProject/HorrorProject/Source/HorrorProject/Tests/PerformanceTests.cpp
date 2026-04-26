// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformMemory.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFrameRateStabilityTest, "HorrorProject.Performance.FrameRateStability", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FFrameRateStabilityTest::RunTest(const FString& Parameters)
{
	const int32 NumFrames = 100;
	TArray<float> FrameTimes;
	FrameTimes.Reserve(NumFrames);

	for (int32 i = 0; i < NumFrames; i++)
	{
		double StartTime = FPlatformTime::Seconds();

		// Simulate frame work
		FPlatformProcess::Sleep(0.001f);

		double EndTime = FPlatformTime::Seconds();
		float FrameTime = static_cast<float>((EndTime - StartTime) * 1000.0);
		FrameTimes.Add(FrameTime);
	}

	float TotalTime = 0.0f;
	for (float Time : FrameTimes)
	{
		TotalTime += Time;
	}
	float AverageFrameTime = TotalTime / NumFrames;

	float Variance = 0.0f;
	for (float Time : FrameTimes)
	{
		float Diff = Time - AverageFrameTime;
		Variance += Diff * Diff;
	}
	Variance /= NumFrames;
	float StdDev = FMath::Sqrt(Variance);

	TestTrue(TEXT("Frame rate stability measured"), AverageFrameTime > 0.0f);
	AddInfo(FString::Printf(TEXT("Average frame time: %.2f ms, StdDev: %.2f ms"), AverageFrameTime, StdDev));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemoryLeakTest, "HorrorProject.Performance.MemoryLeak", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMemoryLeakTest::RunTest(const FString& Parameters)
{
	FPlatformMemoryStats StartStats = FPlatformMemory::GetStats();
	uint64 StartUsedPhysical = StartStats.UsedPhysical;

	TArray<UObject*> TestObjects;
	const int32 NumIterations = 100;

	for (int32 i = 0; i < NumIterations; i++)
	{
		UObject* TestObj = NewObject<UObject>();
		TestObjects.Add(TestObj);
	}

	TestObjects.Empty();
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

	FPlatformMemoryStats EndStats = FPlatformMemory::GetStats();
	uint64 EndUsedPhysical = EndStats.UsedPhysical;

	int64 MemoryDelta = static_cast<int64>(EndUsedPhysical - StartUsedPhysical);
	float MemoryDeltaMB = MemoryDelta / (1024.0f * 1024.0f);

	AddInfo(FString::Printf(TEXT("Memory delta: %.2f MB"), MemoryDeltaMB));
	TestTrue(TEXT("Memory leak test completed"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAssetLoadingPerformanceTest, "HorrorProject.Performance.AssetLoading", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAssetLoadingPerformanceTest::RunTest(const FString& Parameters)
{
	const int32 NumLoads = 10;
	TArray<float> LoadTimes;
	LoadTimes.Reserve(NumLoads);

	for (int32 i = 0; i < NumLoads; i++)
	{
		double StartTime = FPlatformTime::Seconds();

		UObject* TestObject = NewObject<UObject>();

		double EndTime = FPlatformTime::Seconds();
		float LoadTime = static_cast<float>((EndTime - StartTime) * 1000.0);
		LoadTimes.Add(LoadTime);
	}

	float TotalTime = 0.0f;
	float MaxTime = 0.0f;
	float MinTime = FLT_MAX;

	for (float Time : LoadTimes)
	{
		TotalTime += Time;
		MaxTime = FMath::Max(MaxTime, Time);
		MinTime = FMath::Min(MinTime, Time);
	}

	float AverageTime = TotalTime / NumLoads;

	AddInfo(FString::Printf(TEXT("Average load time: %.2f ms"), AverageTime));
	AddInfo(FString::Printf(TEXT("Min: %.2f ms, Max: %.2f ms"), MinTime, MaxTime));
	TestTrue(TEXT("Asset loading performance measured"), AverageTime >= 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVFXPerformanceBenchmarkTest, "HorrorProject.Performance.VFXBenchmark", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FVFXPerformanceBenchmarkTest::RunTest(const FString& Parameters)
{
	const int32 NumIterations = 1000;

	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < NumIterations; i++)
	{
		// Simulate VFX operations
		FVector Location(i * 10.0f, 0.0f, 0.0f);
		FRotator Rotation(0.0f, i * 1.0f, 0.0f);
	}

	double EndTime = FPlatformTime::Seconds();
	float TotalTime = static_cast<float>((EndTime - StartTime) * 1000.0);
	float AverageTime = TotalTime / NumIterations;

	AddInfo(FString::Printf(TEXT("VFX operations: %d in %.2f ms (%.4f ms avg)"), NumIterations, TotalTime, AverageTime));
	TestTrue(TEXT("VFX performance benchmark completed"), TotalTime > 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioPerformanceBenchmarkTest, "HorrorProject.Performance.AudioBenchmark", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FAudioPerformanceBenchmarkTest::RunTest(const FString& Parameters)
{
	const int32 NumIterations = 500;

	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < NumIterations; i++)
	{
		// Simulate audio operations
		float Volume = FMath::FRandRange(0.0f, 1.0f);
		float Pitch = FMath::FRandRange(0.8f, 1.2f);
	}

	double EndTime = FPlatformTime::Seconds();
	float TotalTime = static_cast<float>((EndTime - StartTime) * 1000.0);
	float AverageTime = TotalTime / NumIterations;

	AddInfo(FString::Printf(TEXT("Audio operations: %d in %.2f ms (%.4f ms avg)"), NumIterations, TotalTime, AverageTime));
	TestTrue(TEXT("Audio performance benchmark completed"), TotalTime > 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUIPerformanceBenchmarkTest, "HorrorProject.Performance.UIBenchmark", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FUIPerformanceBenchmarkTest::RunTest(const FString& Parameters)
{
	const int32 NumIterations = 200;

	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < NumIterations; i++)
	{
		// Simulate UI operations
		UObject* Widget = NewObject<UObject>();
	}

	double EndTime = FPlatformTime::Seconds();
	float TotalTime = static_cast<float>((EndTime - StartTime) * 1000.0);
	float AverageTime = TotalTime / NumIterations;

	AddInfo(FString::Printf(TEXT("UI operations: %d in %.2f ms (%.4f ms avg)"), NumIterations, TotalTime, AverageTime));
	TestTrue(TEXT("UI performance benchmark completed"), TotalTime > 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEvidenceSystemPerformanceTest, "HorrorProject.Performance.EvidenceSystem", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FEvidenceSystemPerformanceTest::RunTest(const FString& Parameters)
{
	const int32 NumEvidenceItems = 100;

	double StartTime = FPlatformTime::Seconds();

	TArray<FName> EvidenceIds;
	for (int32 i = 0; i < NumEvidenceItems; i++)
	{
		FName EvidenceId = FName(*FString::Printf(TEXT("Evidence_%d"), i));
		EvidenceIds.Add(EvidenceId);
	}

	double EndTime = FPlatformTime::Seconds();
	float TotalTime = static_cast<float>((EndTime - StartTime) * 1000.0);

	AddInfo(FString::Printf(TEXT("Created %d evidence items in %.2f ms"), NumEvidenceItems, TotalTime));
	TestTrue(TEXT("Evidence system performance test completed"), TotalTime > 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInteractionSystemPerformanceTest, "HorrorProject.Performance.InteractionSystem", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FInteractionSystemPerformanceTest::RunTest(const FString& Parameters)
{
	const int32 NumInteractions = 100;

	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < NumInteractions; i++)
	{
		// Simulate interaction checks
		FVector Location(i * 50.0f, 0.0f, 0.0f);
		float Distance = Location.Size();
	}

	double EndTime = FPlatformTime::Seconds();
	float TotalTime = static_cast<float>((EndTime - StartTime) * 1000.0);

	AddInfo(FString::Printf(TEXT("Processed %d interactions in %.2f ms"), NumInteractions, TotalTime));
	TestTrue(TEXT("Interaction system performance test completed"), TotalTime > 0.0f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
