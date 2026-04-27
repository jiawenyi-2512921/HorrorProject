// Copyright Epic Games, Inc. All Rights Reserved.

#include "MemoryDiagnostics.h"
#include "HAL/PlatformMemory.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/Engine.h"

namespace HorrorMemoryDiagnostics
{
	constexpr float DefaultLeakThresholdMB = 100.0f;
	constexpr float BytesPerMegabyte = 1024.0f * 1024.0f;
	constexpr int64 BytesPerMegabyteInt = 1024 * 1024;
	constexpr int32 MaxHistorySamples = 300;
	constexpr int32 LeakTrendSampleCount = 10;
	constexpr float PercentMultiplier = 100.0f;
	constexpr float CriticalUsagePercent = 90.0f;
	constexpr float WarningUsagePercent = 80.0f;
	constexpr float SpikeThresholdMB = 50.0f;

	float BytesToMegabytes(uint64 Bytes)
	{
		return static_cast<float>(Bytes) / BytesPerMegabyte;
	}
}

UMemoryDiagnostics::UMemoryDiagnostics()
{
	bIsMonitoring = false;
	MonitoringInterval = 5.0f;
	BaselineMemoryMB = 0.0f;
	MemoryLeakThresholdMB = HorrorMemoryDiagnostics::DefaultLeakThresholdMB;
}

void UMemoryDiagnostics::StartMonitoring(float Interval)
{
	if (bIsMonitoring) return;

	MonitoringInterval = FMath::Max(1.0f, Interval);
	bIsMonitoring = true;

	// Set baseline
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	BaselineMemoryMB = HorrorMemoryDiagnostics::BytesToMegabytes(MemStats.UsedPhysical);

	UWorld* World = GetWorld();
	if (!World)
	{
		bIsMonitoring = false;
		return;
	}

	World->GetTimerManager().SetTimer(MonitoringTimer,
		this, &UMemoryDiagnostics::CollectMemoryData,
		MonitoringInterval, true);

	UE_LOG(LogTemp, Log, TEXT("Memory monitoring started (baseline: %.2f MB, interval: %.2f seconds)"),
		BaselineMemoryMB, MonitoringInterval);
}

void UMemoryDiagnostics::StopMonitoring()
{
	if (!bIsMonitoring) return;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MonitoringTimer);
	}
	bIsMonitoring = false;

	UE_LOG(LogTemp, Log, TEXT("Memory monitoring stopped"));
}

void UMemoryDiagnostics::CollectMemoryData()
{
	FMemorySnapshot Snapshot;
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();

	Snapshot.Timestamp = FDateTime::Now();
	Snapshot.UsedPhysicalMB = HorrorMemoryDiagnostics::BytesToMegabytes(MemStats.UsedPhysical);
	Snapshot.UsedVirtualMB = HorrorMemoryDiagnostics::BytesToMegabytes(MemStats.UsedVirtual);
	Snapshot.AvailablePhysicalMB = HorrorMemoryDiagnostics::BytesToMegabytes(MemStats.AvailablePhysical);
	Snapshot.PeakUsedPhysicalMB = HorrorMemoryDiagnostics::BytesToMegabytes(MemStats.PeakUsedPhysical);
	Snapshot.TotalAllocations = MemStats.TotalPhysical / HorrorMemoryDiagnostics::BytesPerMegabyteInt;

	MemoryHistory.Add(Snapshot);

	// Keep only the most recent samples.
	if (MemoryHistory.Num() > HorrorMemoryDiagnostics::MaxHistorySamples)
	{
		MemoryHistory.RemoveAt(0);
	}

	CheckMemoryThresholds(Snapshot);

	// Periodic leak detection
	if (MemoryHistory.Num() % HorrorMemoryDiagnostics::LeakTrendSampleCount == 0)
	{
		AnalyzeMemoryTrend();
	}
}

void UMemoryDiagnostics::CheckMemoryThresholds(const FMemorySnapshot& Snapshot)
{
	float UsagePercent = (Snapshot.UsedPhysicalMB / (Snapshot.UsedPhysicalMB + Snapshot.AvailablePhysicalMB)) * HorrorMemoryDiagnostics::PercentMultiplier;

	if (UsagePercent > HorrorMemoryDiagnostics::CriticalUsagePercent)
	{
		UE_LOG(LogTemp, Error, TEXT("CRITICAL: Memory usage at %.1f%% (%.2f MB used)"),
			UsagePercent, Snapshot.UsedPhysicalMB);
	}
	else if (UsagePercent > HorrorMemoryDiagnostics::WarningUsagePercent)
	{
		UE_LOG(LogTemp, Warning, TEXT("WARNING: High memory usage at %.1f%% (%.2f MB used)"),
			UsagePercent, Snapshot.UsedPhysicalMB);
	}

	// Check for sudden spikes
	if (MemoryHistory.Num() > 1)
	{
		const FMemorySnapshot& Previous = MemoryHistory.Last(1);
		float MemoryIncrease = Snapshot.UsedPhysicalMB - Previous.UsedPhysicalMB;

		if (MemoryIncrease > HorrorMemoryDiagnostics::SpikeThresholdMB)
		{
			UE_LOG(LogTemp, Warning, TEXT("Memory spike detected: +%.2f MB in %.2f seconds"),
				MemoryIncrease, MonitoringInterval);
		}
	}
}

void UMemoryDiagnostics::AnalyzeMemoryTrend()
{
	if (MemoryHistory.Num() < HorrorMemoryDiagnostics::LeakTrendSampleCount) return;

	// Check for consistent memory growth (potential leak)
	int32 StartIndex = FMath::Max(0, MemoryHistory.Num() - HorrorMemoryDiagnostics::LeakTrendSampleCount);
	float StartMemory = (MemoryHistory.GetData() + StartIndex)->UsedPhysicalMB;
	float CurrentMemory = MemoryHistory.Last().UsedPhysicalMB;
	float MemoryGrowth = CurrentMemory - StartMemory;

	if (MemoryGrowth > MemoryLeakThresholdMB)
	{
		UE_LOG(LogTemp, Error, TEXT("POTENTIAL MEMORY LEAK: Memory increased by %.2f MB over last 10 samples"),
			MemoryGrowth);
		UE_LOG(LogTemp, Error, TEXT("Consider running garbage collection or investigating memory allocations"));
	}
}

FHorrorMemoryDiagnosticsStats UMemoryDiagnostics::GetCurrentStats() const
{
	FHorrorMemoryDiagnosticsStats Stats;

	if (MemoryHistory.Num() == 0)
	{
		FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
		Stats.CurrentUsedMB = HorrorMemoryDiagnostics::BytesToMegabytes(MemStats.UsedPhysical);
		Stats.AvailableMB = HorrorMemoryDiagnostics::BytesToMegabytes(MemStats.AvailablePhysical);
		Stats.UsagePercent = (Stats.CurrentUsedMB / (Stats.CurrentUsedMB + Stats.AvailableMB)) * HorrorMemoryDiagnostics::PercentMultiplier;
		return Stats;
	}

	// Calculate stats from history
	float TotalUsed = 0.0f;
	float PeakUsed = 0.0f;

	for (const FMemorySnapshot& Snapshot : MemoryHistory)
	{
		TotalUsed += Snapshot.UsedPhysicalMB;
		PeakUsed = FMath::Max(PeakUsed, Snapshot.UsedPhysicalMB);
	}

	const FMemorySnapshot& Latest = MemoryHistory.Last();
	Stats.CurrentUsedMB = Latest.UsedPhysicalMB;
	Stats.PeakUsedMB = PeakUsed;
	Stats.AverageUsedMB = TotalUsed / MemoryHistory.Num();
	Stats.AvailableMB = Latest.AvailablePhysicalMB;
	Stats.UsagePercent = (Stats.CurrentUsedMB / (Stats.CurrentUsedMB + Stats.AvailableMB)) * HorrorMemoryDiagnostics::PercentMultiplier;

	return Stats;
}

void UMemoryDiagnostics::GenerateMemoryReport(const FString& FilePath)
{
	FString OutputPath = FilePath.IsEmpty() ?
		FPaths::ProjectSavedDir() / TEXT("Diagnostics") / FString::Printf(TEXT("Memory_%s.txt"),
			*FDateTime::Now().ToString()) : FilePath;

	FString Content = TEXT("=== MEMORY DIAGNOSTIC REPORT ===\n");
	Content += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
	Content += FString::Printf(TEXT("Samples: %d\n"), MemoryHistory.Num());
	Content += FString::Printf(TEXT("Baseline: %.2f MB\n\n"), BaselineMemoryMB);

	FHorrorMemoryDiagnosticsStats Stats = GetCurrentStats();

	Content += TEXT("=== SUMMARY ===\n");
	Content += FString::Printf(TEXT("Current Usage: %.2f MB (%.1f%%)\n"), Stats.CurrentUsedMB, Stats.UsagePercent);
	Content += FString::Printf(TEXT("Peak Usage: %.2f MB\n"), Stats.PeakUsedMB);
	Content += FString::Printf(TEXT("Average Usage: %.2f MB\n"), Stats.AverageUsedMB);
	Content += FString::Printf(TEXT("Available: %.2f MB\n"), Stats.AvailableMB);
	Content += FString::Printf(TEXT("Growth from Baseline: %.2f MB\n\n"), Stats.CurrentUsedMB - BaselineMemoryMB);

	Content += TEXT("=== DETAILED HISTORY ===\n");
	for (const FMemorySnapshot& Snapshot : MemoryHistory)
	{
		Content += FString::Printf(TEXT("[%s] Physical: %.2f MB | Virtual: %.2f MB | Available: %.2f MB | Peak: %.2f MB\n"),
			*Snapshot.Timestamp.ToString(),
			Snapshot.UsedPhysicalMB,
			Snapshot.UsedVirtualMB,
			Snapshot.AvailablePhysicalMB,
			Snapshot.PeakUsedPhysicalMB);
	}

	if (FFileHelper::SaveStringToFile(Content, *OutputPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Memory report saved to: %s"), *OutputPath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save memory report to: %s"), *OutputPath);
	}
}

void UMemoryDiagnostics::ClearHistory()
{
	MemoryHistory.Empty();
	UE_LOG(LogTemp, Log, TEXT("Memory history cleared"));
}

void UMemoryDiagnostics::TakeMemorySnapshot()
{
	CollectMemoryData();
	UE_LOG(LogTemp, Log, TEXT("Memory snapshot taken"));
}

void UMemoryDiagnostics::DetectMemoryLeaks()
{
	UE_LOG(LogTemp, Warning, TEXT("Running memory leak detection..."));

	if (MemoryHistory.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("Insufficient data for leak detection. Need at least 2 samples."));
		return;
	}

	AnalyzeMemoryTrend();

	// Additional leak detection logic
	FHorrorMemoryDiagnosticsStats Stats = GetCurrentStats();
	float MemoryGrowth = Stats.CurrentUsedMB - BaselineMemoryMB;

	if (MemoryGrowth > MemoryLeakThresholdMB)
	{
		UE_LOG(LogTemp, Error, TEXT("Memory leak suspected: %.2f MB growth from baseline"), MemoryGrowth);
		UE_LOG(LogTemp, Error, TEXT("Recommendations:"));
		UE_LOG(LogTemp, Error, TEXT("  1. Run garbage collection (ForceGarbageCollection)"));
		UE_LOG(LogTemp, Error, TEXT("  2. Check for circular references"));
		UE_LOG(LogTemp, Error, TEXT("  3. Review recent actor spawns"));
		UE_LOG(LogTemp, Error, TEXT("  4. Use 'memreport -full' for detailed analysis"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("No significant memory leaks detected (%.2f MB growth)"), MemoryGrowth);
	}
}

void UMemoryDiagnostics::ForceGarbageCollection()
{
	UE_LOG(LogTemp, Warning, TEXT("Forcing garbage collection..."));

	FHorrorMemoryDiagnosticsStats BeforeStats = GetCurrentStats();

	GEngine->ForceGarbageCollection(true);

	// Wait a frame for GC to complete
	FPlatformProcess::Sleep(0.1f);

	FHorrorMemoryDiagnosticsStats AfterStats = GetCurrentStats();
	float MemoryFreed = BeforeStats.CurrentUsedMB - AfterStats.CurrentUsedMB;

	UE_LOG(LogTemp, Log, TEXT("Garbage collection complete. Freed: %.2f MB"), MemoryFreed);
}

void UMemoryDiagnostics::DumpMemoryStats()
{
	UE_LOG(LogTemp, Warning, TEXT("Dumping memory statistics..."));

	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();

	UE_LOG(LogTemp, Log, TEXT("=== MEMORY STATISTICS ==="));
	UE_LOG(LogTemp, Log, TEXT("Total Physical: %.2f MB"), HorrorMemoryDiagnostics::BytesToMegabytes(MemStats.TotalPhysical));
	UE_LOG(LogTemp, Log, TEXT("Used Physical: %.2f MB"), HorrorMemoryDiagnostics::BytesToMegabytes(MemStats.UsedPhysical));
	UE_LOG(LogTemp, Log, TEXT("Peak Used Physical: %.2f MB"), HorrorMemoryDiagnostics::BytesToMegabytes(MemStats.PeakUsedPhysical));
	UE_LOG(LogTemp, Log, TEXT("Available Physical: %.2f MB"), HorrorMemoryDiagnostics::BytesToMegabytes(MemStats.AvailablePhysical));
	UE_LOG(LogTemp, Log, TEXT("Total Virtual: %.2f MB"), HorrorMemoryDiagnostics::BytesToMegabytes(MemStats.TotalVirtual));
	UE_LOG(LogTemp, Log, TEXT("Used Virtual: %.2f MB"), HorrorMemoryDiagnostics::BytesToMegabytes(MemStats.UsedVirtual));
	UE_LOG(LogTemp, Log, TEXT("Peak Used Virtual: %.2f MB"), HorrorMemoryDiagnostics::BytesToMegabytes(MemStats.PeakUsedVirtual));
	UE_LOG(LogTemp, Log, TEXT("Available Virtual: %.2f MB"), HorrorMemoryDiagnostics::BytesToMegabytes(MemStats.AvailableVirtual));

	// Execute engine memory commands
	if (UWorld* World = GetWorld())
	{
		World->Exec(World, TEXT("stat memory"));
		World->Exec(World, TEXT("memreport"));
	}
}
