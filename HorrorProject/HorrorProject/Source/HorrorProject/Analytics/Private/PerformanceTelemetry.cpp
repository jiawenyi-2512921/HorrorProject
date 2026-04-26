// Copyright Epic Games, Inc. All Rights Reserved.

#include "PerformanceTelemetry.h"
#include "HAL/PlatformMemory.h"
#include "Engine/Engine.h"

void UPerformanceTelemetry::Update(float DeltaTime)
{
	FPerformanceTelemetrySnapshot Snapshot;

	// Calculate FPS
	Snapshot.FPS = DeltaTime > 0.0f ? 1.0f / DeltaTime : 0.0f;
	Snapshot.FrameTimeMs = DeltaTime * 1000.0f;
	Snapshot.MemoryUsageMB = CalculateMemoryUsage();
	Snapshot.Timestamp = FDateTime::UtcNow();

	// Update stats
	MinFPS = FMath::Min(MinFPS, Snapshot.FPS);
	MaxFPS = FMath::Max(MaxFPS, Snapshot.FPS);
	TotalFPS += Snapshot.FPS;
	TotalFrameTime += Snapshot.FrameTimeMs;
	SampleCount++;

	RecordSnapshot(Snapshot);
}

float UPerformanceTelemetry::GetAverageFPS() const
{
	return SampleCount > 0 ? TotalFPS / SampleCount : 0.0f;
}

float UPerformanceTelemetry::GetAverageFrameTime() const
{
	return SampleCount > 0 ? TotalFrameTime / SampleCount : 0.0f;
}

float UPerformanceTelemetry::GetMemoryUsageMB() const
{
	return CalculateMemoryUsage();
}

FPerformanceTelemetrySnapshot UPerformanceTelemetry::GetCurrentSnapshot() const
{
	if (PerformanceHistory.Num() > 0)
	{
		return PerformanceHistory.Last();
	}
	return FPerformanceTelemetrySnapshot();
}

void UPerformanceTelemetry::ResetStats()
{
	PerformanceHistory.Empty();
	MinFPS = TNumericLimits<float>::Max();
	MaxFPS = 0.0f;
	TotalFPS = 0.0f;
	TotalFrameTime = 0.0f;
	SampleCount = 0;
}

void UPerformanceTelemetry::RecordSnapshot(const FPerformanceTelemetrySnapshot& Snapshot)
{
	PerformanceHistory.Add(Snapshot);

	if (PerformanceHistory.Num() > MaxHistorySize)
	{
		PerformanceHistory.RemoveAt(0, PerformanceHistory.Num() - MaxHistorySize);
	}
}

float UPerformanceTelemetry::CalculateMemoryUsage() const
{
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	return static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f);
}
