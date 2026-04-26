// Copyright Epic Games, Inc. All Rights Reserved.

#include "MemoryDiagnostics.h"
#include "HAL/PlatformMemory.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/Engine.h"

UMemoryDiagnostics::UMemoryDiagnostics()
{
	bIsMonitoring = false;
	MonitoringInterval = 5.0f;
	BaselineMemoryMB = 0.0f;
	MemoryLeakThresholdMB = 100.0f;
}

void UMemoryDiagnostics::StartMonitoring(float Interval)
{
	if (bIsMonitoring) return;

	MonitoringInterval = FMath::Max(1.0f, Interval);
	bIsMonitoring = true;

	// Set baseline
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	BaselineMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);

	GetWorld()->GetTimerManager().SetTimer(MonitoringTimer,
		this, &UMemoryDiagnostics::CollectMemoryData,
		MonitoringInterval, true);

	UE_LOG(LogTemp, Log, TEXT("Memory monitoring started (baseline: %.2f MB, interval: %.2f seconds)"),
		BaselineMemoryMB, MonitoringInterval);
}

void UMemoryDiagnostics::StopMonitoring()
{
	if (!bIsMonitoring) return;

	GetWorld()->GetTimerManager().ClearTimer(MonitoringTimer);
	bIsMonitoring = false;

	UE_LOG(LogTemp, Log, TEXT("Memory monitoring stopped"));
}

void UMemoryDiagnostics::CollectMemoryData()
{
	FMemorySnapshot Snapshot;
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();

	Snapshot.Timestamp = FDateTime::Now();
	Snapshot.UsedPhysicalMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
	Snapshot.UsedVirtualMB = MemStats.UsedVirtual / (1024.0f * 1024.0f);
	Snapshot.AvailablePhysicalMB = MemStats.AvailablePhysical / (1024.0f * 1024.0f);
	Snapshot.PeakUsedPhysicalMB = MemStats.PeakUsedPhysical / (1024.0f * 1024.0f);
	Snapshot.TotalAllocations = MemStats.TotalPhysical / (1024 * 1024);

	MemoryHistory.Add(Snapshot);

	// Keep only last 300 samples
	if (MemoryHistory.Num() > 300)
	{
		MemoryHistory.RemoveAt(0);
	}

	CheckMemoryThresholds(Snapshot);

	// Periodic leak detection
	if (MemoryHistory.Num() % 10 == 0)
	{
		AnalyzeMemoryTrend();
	}
}

void UMemoryDiagnostics::CheckMemoryThresholds(const FMemorySnapshot& Snapshot)
{
	float UsagePercent = (Snapshot.UsedPhysicalMB / (Snapshot.UsedPhysicalMB + Snapshot.AvailablePhysicalMB)) * 100.0f;

	if (UsagePercent > 90.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("CRITICAL: Memory usage at %.1f%% (%.2f MB used)"),
			UsagePercent, Snapshot.UsedPhysicalMB);
	}
	else if (UsagePercent > 80.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("WARNING: High memory usage at %.1f%% (%.2f MB used)"),
			UsagePercent, Snapshot.UsedPhysicalMB);
	}

	// Check for sudden spikes
	if (MemoryHistory.Num() > 1)
	{
		const FMemorySnapshot& Previous = MemoryHistory[MemoryHistory.Num() - 2];
		float MemoryIncrease = Snapshot.UsedPhysicalMB - Previous.UsedPhysicalMB;

		if (MemoryIncrease > 50.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("Memory spike detected: +%.2f MB in %.2f seconds"),
				MemoryIncrease, MonitoringInterval);
		}
	}
}

void UMemoryDiagnostics::AnalyzeMemoryTrend()
{
	if (MemoryHistory.Num() < 10) return;

	// Check for consistent memory growth (potential leak)
	int32 StartIndex = FMath::Max(0, MemoryHistory.Num() - 10);
	float StartMemory = MemoryHistory[StartIndex].UsedPhysicalMB;
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
		Stats.CurrentUsedMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
		Stats.AvailableMB = MemStats.AvailablePhysical / (1024.0f * 1024.0f);
		Stats.UsagePercent = (Stats.CurrentUsedMB / (Stats.CurrentUsedMB + Stats.AvailableMB)) * 100.0f;
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
	Stats.UsagePercent = (Stats.CurrentUsedMB / (Stats.CurrentUsedMB + Stats.AvailableMB)) * 100.0f;

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
	UE_LOG(LogTemp, Log, TEXT("Total Physical: %.2f MB"), MemStats.TotalPhysical / (1024.0f * 1024.0f));
	UE_LOG(LogTemp, Log, TEXT("Used Physical: %.2f MB"), MemStats.UsedPhysical / (1024.0f * 1024.0f));
	UE_LOG(LogTemp, Log, TEXT("Peak Used Physical: %.2f MB"), MemStats.PeakUsedPhysical / (1024.0f * 1024.0f));
	UE_LOG(LogTemp, Log, TEXT("Available Physical: %.2f MB"), MemStats.AvailablePhysical / (1024.0f * 1024.0f));
	UE_LOG(LogTemp, Log, TEXT("Total Virtual: %.2f MB"), MemStats.TotalVirtual / (1024.0f * 1024.0f));
	UE_LOG(LogTemp, Log, TEXT("Used Virtual: %.2f MB"), MemStats.UsedVirtual / (1024.0f * 1024.0f));
	UE_LOG(LogTemp, Log, TEXT("Peak Used Virtual: %.2f MB"), MemStats.PeakUsedVirtual / (1024.0f * 1024.0f));
	UE_LOG(LogTemp, Log, TEXT("Available Virtual: %.2f MB"), MemStats.AvailableVirtual / (1024.0f * 1024.0f));

	// Execute engine memory commands
	GetWorld()->Exec(GetWorld(), TEXT("stat memory"));
	GetWorld()->Exec(GetWorld(), TEXT("memreport"));
}
