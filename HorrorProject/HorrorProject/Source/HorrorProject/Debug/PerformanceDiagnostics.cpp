// Copyright Epic Games, Inc. All Rights Reserved.

#include "PerformanceDiagnostics.h"
#include "Engine/Engine.h"
#include "HAL/PlatformTime.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "DynamicRHI.h"

namespace HorrorPerformanceDiagnostics
{
	constexpr int32 MaxHistorySamples = 300;
	constexpr float MillisecondsPerSecond = 1000.0f;
	constexpr float LowFpsThreshold = 30.0f;
	constexpr float LowFpsFrameTimeSeconds = 0.033f;
	constexpr float FrameThreadBudgetMs = 16.0f;
}

UPerformanceDiagnostics::UPerformanceDiagnostics()
{
	bIsMonitoring = false;
	MonitoringInterval = 1.0f;
	FrameTimeHistory.Reserve(HorrorPerformanceDiagnostics::MaxHistorySamples);
	FPSHistory.Reserve(HorrorPerformanceDiagnostics::MaxHistorySamples);
}

void UPerformanceDiagnostics::StartMonitoring(float Interval)
{
	if (bIsMonitoring) return;

	MonitoringInterval = FMath::Max(0.1f, Interval);
	bIsMonitoring = true;

	UWorld* World = GetWorld();
	if (!World)
	{
		bIsMonitoring = false;
		return;
	}

	World->GetTimerManager().SetTimer(MonitoringTimer,
		this, &UPerformanceDiagnostics::CollectPerformanceData,
		MonitoringInterval, true);

	UE_LOG(LogTemp, Log, TEXT("Performance monitoring started (interval: %.2f seconds)"), MonitoringInterval);
}

void UPerformanceDiagnostics::StopMonitoring()
{
	if (!bIsMonitoring) return;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MonitoringTimer);
	}
	bIsMonitoring = false;

	UE_LOG(LogTemp, Log, TEXT("Performance monitoring stopped"));
}

void UPerformanceDiagnostics::CollectPerformanceData()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FHorrorPerformanceDiagnosticsSnapshot Snapshot;
	Snapshot.Timestamp = FDateTime::Now();
	Snapshot.DeltaTime = World->GetDeltaSeconds();
	Snapshot.FPS = 1.0f / Snapshot.DeltaTime;
	Snapshot.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
	Snapshot.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
	Snapshot.GPUTime = FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());
	Snapshot.ActorCount = World->GetActorCount();

	PerformanceHistory.Add(Snapshot);

	// Keep only the most recent samples.
	if (PerformanceHistory.Num() > HorrorPerformanceDiagnostics::MaxHistorySamples)
	{
		PerformanceHistory.RemoveAt(0);
	}

	// Update frame time history
	FrameTimeHistory.Add(Snapshot.DeltaTime * HorrorPerformanceDiagnostics::MillisecondsPerSecond);
	if (FrameTimeHistory.Num() > HorrorPerformanceDiagnostics::MaxHistorySamples)
	{
		FrameTimeHistory.RemoveAt(0);
	}

	// Update FPS history
	FPSHistory.Add(Snapshot.FPS);
	if (FPSHistory.Num() > HorrorPerformanceDiagnostics::MaxHistorySamples)
	{
		FPSHistory.RemoveAt(0);
	}

	// Check for performance issues
	CheckPerformanceThresholds(Snapshot);
}

void UPerformanceDiagnostics::CheckPerformanceThresholds(const FHorrorPerformanceDiagnosticsSnapshot& Snapshot)
{
	// Check FPS
	if (Snapshot.FPS < HorrorPerformanceDiagnostics::LowFpsThreshold)
	{
		UE_LOG(LogTemp, Warning, TEXT("Low FPS detected: %.1f"), Snapshot.FPS);
	}

	// Check frame time
	if (Snapshot.DeltaTime > HorrorPerformanceDiagnostics::LowFpsFrameTimeSeconds)
	{
		UE_LOG(LogTemp, Warning, TEXT("High frame time: %.2f ms"), Snapshot.DeltaTime * HorrorPerformanceDiagnostics::MillisecondsPerSecond);
	}

	// Check game thread
	if (Snapshot.GameThreadTime > HorrorPerformanceDiagnostics::FrameThreadBudgetMs)
	{
		UE_LOG(LogTemp, Warning, TEXT("High game thread time: %.2f ms"), Snapshot.GameThreadTime);
	}

	// Check render thread
	if (Snapshot.RenderThreadTime > HorrorPerformanceDiagnostics::FrameThreadBudgetMs)
	{
		UE_LOG(LogTemp, Warning, TEXT("High render thread time: %.2f ms"), Snapshot.RenderThreadTime);
	}

	// Check GPU
	if (Snapshot.GPUTime > HorrorPerformanceDiagnostics::FrameThreadBudgetMs)
	{
		UE_LOG(LogTemp, Warning, TEXT("High GPU time: %.2f ms"), Snapshot.GPUTime);
	}
}

FHorrorPerformanceDiagnosticsStats UPerformanceDiagnostics::GetCurrentStats() const
{
	FHorrorPerformanceDiagnosticsStats Stats;

	if (PerformanceHistory.Num() == 0)
	{
		return Stats;
	}

	// Calculate averages
	float TotalFPS = 0.0f;
	float TotalGameThread = 0.0f;
	float TotalRenderThread = 0.0f;
	float TotalGPU = 0.0f;
	float MinFPS = FLT_MAX;
	float MaxFPS = 0.0f;

	for (const FHorrorPerformanceDiagnosticsSnapshot& Snapshot : PerformanceHistory)
	{
		TotalFPS += Snapshot.FPS;
		TotalGameThread += Snapshot.GameThreadTime;
		TotalRenderThread += Snapshot.RenderThreadTime;
		TotalGPU += Snapshot.GPUTime;

		MinFPS = FMath::Min(MinFPS, Snapshot.FPS);
		MaxFPS = FMath::Max(MaxFPS, Snapshot.FPS);
	}

	int32 Count = PerformanceHistory.Num();
	Stats.AverageFPS = TotalFPS / Count;
	Stats.MinFPS = MinFPS;
	Stats.MaxFPS = MaxFPS;
	Stats.AverageGameThreadTime = TotalGameThread / Count;
	Stats.AverageRenderThreadTime = TotalRenderThread / Count;
	Stats.AverageGPUTime = TotalGPU / Count;

	// Get current values
	const FHorrorPerformanceDiagnosticsSnapshot& Latest = PerformanceHistory.Last();
	Stats.CurrentFPS = Latest.FPS;
	Stats.CurrentFrameTime = Latest.DeltaTime * HorrorPerformanceDiagnostics::MillisecondsPerSecond;

	return Stats;
}

void UPerformanceDiagnostics::GeneratePerformanceReport(const FString& FilePath)
{
	FString OutputPath = FilePath.IsEmpty() ?
		FPaths::ProjectSavedDir() / TEXT("Diagnostics") / FString::Printf(TEXT("Performance_%s.txt"),
			*FDateTime::Now().ToString()) : FilePath;

	FString Content = TEXT("=== PERFORMANCE DIAGNOSTIC REPORT ===\n");
	Content += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
	Content += FString::Printf(TEXT("Samples: %d\n\n"), PerformanceHistory.Num());

	FHorrorPerformanceDiagnosticsStats Stats = GetCurrentStats();

	Content += TEXT("=== SUMMARY ===\n");
	Content += FString::Printf(TEXT("Average FPS: %.2f\n"), Stats.AverageFPS);
	Content += FString::Printf(TEXT("Min FPS: %.2f\n"), Stats.MinFPS);
	Content += FString::Printf(TEXT("Max FPS: %.2f\n"), Stats.MaxFPS);
	Content += FString::Printf(TEXT("Average Game Thread: %.2f ms\n"), Stats.AverageGameThreadTime);
	Content += FString::Printf(TEXT("Average Render Thread: %.2f ms\n"), Stats.AverageRenderThreadTime);
	Content += FString::Printf(TEXT("Average GPU: %.2f ms\n\n"), Stats.AverageGPUTime);

	Content += TEXT("=== DETAILED HISTORY ===\n");
	for (const FHorrorPerformanceDiagnosticsSnapshot& Snapshot : PerformanceHistory)
	{
		Content += FString::Printf(TEXT("[%s] FPS: %.1f | Frame: %.2f ms | Game: %.2f ms | Render: %.2f ms | GPU: %.2f ms | Actors: %d\n"),
			*Snapshot.Timestamp.ToString(),
			Snapshot.FPS,
			Snapshot.DeltaTime * HorrorPerformanceDiagnostics::MillisecondsPerSecond,
			Snapshot.GameThreadTime,
			Snapshot.RenderThreadTime,
			Snapshot.GPUTime,
			Snapshot.ActorCount);
	}

	if (FFileHelper::SaveStringToFile(Content, *OutputPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Performance report saved to: %s"), *OutputPath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save performance report to: %s"), *OutputPath);
	}
}

void UPerformanceDiagnostics::ClearHistory()
{
	PerformanceHistory.Empty();
	FrameTimeHistory.Empty();
	FPSHistory.Empty();
	UE_LOG(LogTemp, Log, TEXT("Performance history cleared"));
}

void UPerformanceDiagnostics::CaptureFrameProfile()
{
	UE_LOG(LogTemp, Warning, TEXT("Capturing frame profile..."));
	if (UWorld* World = GetWorld())
	{
		World->Exec(World, TEXT("profilegpu"));
	}
}

void UPerformanceDiagnostics::StartCPUProfiling()
{
	UE_LOG(LogTemp, Warning, TEXT("Starting CPU profiling..."));
	if (UWorld* World = GetWorld())
	{
		World->Exec(World, TEXT("stat startfile"));
	}
}

void UPerformanceDiagnostics::StopCPUProfiling()
{
	UE_LOG(LogTemp, Warning, TEXT("Stopping CPU profiling..."));
	if (UWorld* World = GetWorld())
	{
		World->Exec(World, TEXT("stat stopfile"));
	}
}
