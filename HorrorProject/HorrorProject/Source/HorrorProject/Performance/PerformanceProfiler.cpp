// Copyright Epic Games, Inc. All Rights Reserved.

#include "Performance/PerformanceProfiler.h"
#include "HAL/PlatformTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

#if !UE_BUILD_SHIPPING

FPerformanceScope::FPerformanceScope(const FName& InScopeName)
	: ScopeName(InScopeName)
	, StartTime(FPlatformTime::Seconds())
{
}

FPerformanceScope::~FPerformanceScope()
{
	const double EndTime = FPlatformTime::Seconds();
	const double DurationMs = (EndTime - StartTime) * 1000.0;

	if (GEngine && GEngine->GetWorld())
	{
		UPerformanceProfiler* Profiler = UPerformanceProfiler::Get(GEngine->GetWorld());
		if (Profiler && Profiler->IsProfilingEnabled())
		{
			Profiler->RecordSample(ScopeName, DurationMs);
		}
	}
}

#endif

void FHorrorPerformanceScopeStats::AddSample(double DurationMs)
{
	if (SampleCount == 0)
	{
		MinMs = DurationMs;
		MaxMs = DurationMs;
		AverageMs = DurationMs;
		TotalMs = DurationMs;
		SampleCount = 1;
	}
	else
	{
		MinMs = FMath::Min(MinMs, DurationMs);
		MaxMs = FMath::Max(MaxMs, DurationMs);
		TotalMs += DurationMs;
		SampleCount++;
		AverageMs = TotalMs / SampleCount;
	}
}

void FHorrorPerformanceScopeStats::Reset()
{
	SampleCount = 0;
	MinMs = 0.0;
	MaxMs = 0.0;
	AverageMs = 0.0;
	TotalMs = 0.0;
}

UPerformanceProfiler::UPerformanceProfiler()
{
	bProfilingEnabled = false;
}

void UPerformanceProfiler::SetProfilingEnabled(bool bEnabled)
{
	FScopeLock Lock(&DataMutex);
	bProfilingEnabled = bEnabled;

	if (bEnabled)
	{
		UE_LOG(LogTemp, Log, TEXT("Performance profiling enabled"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Performance profiling disabled"));
	}
}

void UPerformanceProfiler::RecordSample(const FName& ScopeName, double DurationMs)
{
	if (!bProfilingEnabled)
	{
		return;
	}

	FScopeLock Lock(&DataMutex);

	// Update statistics
	FHorrorPerformanceScopeStats& Stats = ScopeStats.FindOrAdd(ScopeName);
	Stats.ScopeName = ScopeName;
	Stats.AddSample(DurationMs);

	// Store sample for CSV export
	FPerformanceSample Sample;
	Sample.ScopeName = ScopeName;
	Sample.DurationMs = DurationMs;
	Sample.Timestamp = FDateTime::Now();
	Sample.FrameNumber = GFrameCounter;

	RecentSamples.Add(Sample);

	// Limit sample count
	if (RecentSamples.Num() > MaxSamples)
	{
		RecentSamples.RemoveAt(0, RecentSamples.Num() - MaxSamples);
	}
}

FHorrorPerformanceScopeStats UPerformanceProfiler::GetStats(const FName& ScopeName) const
{
	FScopeLock Lock(&const_cast<FCriticalSection&>(DataMutex));

	const FHorrorPerformanceScopeStats* Stats = ScopeStats.Find(ScopeName);
	return Stats ? *Stats : FHorrorPerformanceScopeStats();
}

TArray<FHorrorPerformanceScopeStats> UPerformanceProfiler::GetAllStats() const
{
	FScopeLock Lock(&const_cast<FCriticalSection&>(DataMutex));

	TArray<FHorrorPerformanceScopeStats> AllStats;
	ScopeStats.GenerateValueArray(AllStats);

	// Sort by average duration (descending)
	AllStats.Sort([](const FHorrorPerformanceScopeStats& A, const FHorrorPerformanceScopeStats& B)
	{
		return A.AverageMs > B.AverageMs;
	});

	return AllStats;
}

void UPerformanceProfiler::ClearStats()
{
	FScopeLock Lock(&DataMutex);

	ScopeStats.Empty();
	RecentSamples.Empty();

	UE_LOG(LogTemp, Log, TEXT("Performance stats cleared"));
}

bool UPerformanceProfiler::ExportToCSV(const FString& FilePath)
{
	FScopeLock Lock(&DataMutex);

	if (RecentSamples.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No performance samples to export"));
		return false;
	}

	FString CSVContent;
	CSVContent += TEXT("Timestamp,FrameNumber,ScopeName,DurationMs\n");

	for (const FPerformanceSample& Sample : RecentSamples)
	{
		CSVContent += FString::Printf(TEXT("%s,%d,%s,%.4f\n"),
			*Sample.Timestamp.ToString(),
			Sample.FrameNumber,
			*Sample.ScopeName.ToString(),
			Sample.DurationMs);
	}

	const FString FullPath = FPaths::ProjectSavedDir() / TEXT("Profiling") / FilePath;
	const FString Directory = FPaths::GetPath(FullPath);

	if (!FPaths::DirectoryExists(Directory))
	{
		IFileManager::Get().MakeDirectory(*Directory, true);
	}

	if (FFileHelper::SaveStringToFile(CSVContent, *FullPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Performance data exported to: %s"), *FullPath);
		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to export performance data to: %s"), *FullPath);
	return false;
}

UPerformanceProfiler* UPerformanceProfiler::Get(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	return World->GetSubsystem<UPerformanceProfiler>();
}
