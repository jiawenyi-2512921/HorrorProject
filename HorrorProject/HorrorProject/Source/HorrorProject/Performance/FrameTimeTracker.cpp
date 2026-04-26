// Copyright Epic Games, Inc. All Rights Reserved.

#include "Performance/FrameTimeTracker.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformTime.h"

void UFrameTimeTracker::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FrameTimeHistory.Reserve(MaxHistorySize);

	UE_LOG(LogTemp, Log, TEXT("FrameTimeTracker initialized"));
}

void UFrameTimeTracker::Deinitialize()
{
	if (FrameEndHandle.IsValid())
	{
		FCoreDelegates::OnEndFrame.Remove(FrameEndHandle);
	}

	Super::Deinitialize();
}

void UFrameTimeTracker::SetTrackingEnabled(bool bEnabled)
{
	if (bTrackingEnabled == bEnabled)
	{
		return;
	}

	bTrackingEnabled = bEnabled;

	if (bEnabled)
	{
		FrameEndHandle = FCoreDelegates::OnEndFrame.AddUObject(this, &UFrameTimeTracker::OnFrameEnd);
		ResetStats();
		UE_LOG(LogTemp, Log, TEXT("Frame time tracking enabled"));
	}
	else
	{
		if (FrameEndHandle.IsValid())
		{
			FCoreDelegates::OnEndFrame.Remove(FrameEndHandle);
			FrameEndHandle.Reset();
		}
		UE_LOG(LogTemp, Log, TEXT("Frame time tracking disabled"));
	}
}

FFrameTimeStats UFrameTimeTracker::GetFrameTimeStats() const
{
	FFrameTimeStats Stats;

	if (FrameCount == 0)
	{
		return Stats;
	}

	const float AverageFrameTime = TotalFrameTime / FrameCount;
	Stats.AverageFrameTimeMs = AverageFrameTime * 1000.0f;
	Stats.AverageFPS = AverageFrameTime > 0.0f ? 1.0f / AverageFrameTime : 0.0f;

	if (FrameTimeHistory.Num() > 0)
	{
		const float CurrentFrameTime = FrameTimeHistory.Last();
		Stats.CurrentFrameTimeMs = CurrentFrameTime * 1000.0f;
		Stats.CurrentFPS = CurrentFrameTime > 0.0f ? 1.0f / CurrentFrameTime : 0.0f;
	}

	Stats.MinFPS = MaxFrameTime > 0.0f ? 1.0f / MaxFrameTime : 0.0f;
	Stats.MaxFPS = MinFrameTime > 0.0f ? 1.0f / MinFrameTime : 0.0f;

	Stats.FramesBelowTarget = FramesBelowTarget;
	Stats.PercentBelowTarget = FrameCount > 0 ? (float)FramesBelowTarget / FrameCount * 100.0f : 0.0f;

	// Get engine stats if available
	if (GEngine)
	{
		Stats.GameThreadTimeMs = FPlatformTime::ToMilliseconds(GGameThreadTime);
		Stats.RenderThreadTimeMs = FPlatformTime::ToMilliseconds(GRenderThreadTime);
		Stats.GPUTimeMs = FPlatformTime::ToMilliseconds(GGPUFrameTime);
	}

	return Stats;
}

void UFrameTimeTracker::SetTargetFPS(float InTargetFPS)
{
	TargetFPS = FMath::Max(1.0f, InTargetFPS);
	UE_LOG(LogTemp, Log, TEXT("Target FPS set to: %.1f"), TargetFPS);
}

bool UFrameTimeTracker::IsFrameWithinBudget() const
{
	if (FrameTimeHistory.Num() == 0)
	{
		return true;
	}

	const float CurrentFrameTime = FrameTimeHistory.Last();
	const float TargetFrameTime = 1.0f / TargetFPS;
	return CurrentFrameTime <= TargetFrameTime;
}

void UFrameTimeTracker::ResetStats()
{
	FrameTimeHistory.Empty();
	TotalFrameTime = 0.0f;
	FrameCount = 0;
	MinFrameTime = FLT_MAX;
	MaxFrameTime = 0.0f;
	FramesBelowTarget = 0;

	UE_LOG(LogTemp, Log, TEXT("Frame time stats reset"));
}

bool UFrameTimeTracker::ExportToCSV(const FString& FilePath)
{
	if (FrameTimeHistory.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No frame time data to export"));
		return false;
	}

	FString CSVContent;
	CSVContent += TEXT("FrameNumber,FrameTimeMs,FPS,WithinBudget\n");

	const float TargetFrameTime = 1.0f / TargetFPS;

	for (int32 i = 0; i < FrameTimeHistory.Num(); ++i)
	{
		const float FrameTime = FrameTimeHistory[i];
		const float FPS = FrameTime > 0.0f ? 1.0f / FrameTime : 0.0f;
		const bool bWithinBudget = FrameTime <= TargetFrameTime;

		CSVContent += FString::Printf(TEXT("%d,%.4f,%.2f,%s\n"),
			i,
			FrameTime * 1000.0f,
			FPS,
			bWithinBudget ? TEXT("Yes") : TEXT("No"));
	}

	const FString FullPath = FPaths::ProjectSavedDir() / TEXT("Profiling") / FilePath;
	const FString Directory = FPaths::GetPath(FullPath);

	if (!FPaths::DirectoryExists(Directory))
	{
		IFileManager::Get().MakeDirectory(*Directory, true);
	}

	if (FFileHelper::SaveStringToFile(CSVContent, *FullPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Frame time data exported to: %s"), *FullPath);
		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to export frame time data to: %s"), *FullPath);
	return false;
}

void UFrameTimeTracker::OnFrameEnd()
{
	if (!bTrackingEnabled)
	{
		return;
	}

	const float DeltaTime = FApp::GetDeltaTime();
	UpdateStats(DeltaTime);
}

void UFrameTimeTracker::UpdateStats(float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return;
	}

	// Add to history
	FrameTimeHistory.Add(DeltaTime);
	if (FrameTimeHistory.Num() > MaxHistorySize)
	{
		FrameTimeHistory.RemoveAt(0);
	}

	// Update statistics
	TotalFrameTime += DeltaTime;
	FrameCount++;
	MinFrameTime = FMath::Min(MinFrameTime, DeltaTime);
	MaxFrameTime = FMath::Max(MaxFrameTime, DeltaTime);

	// Check if below target
	const float TargetFrameTime = 1.0f / TargetFPS;
	if (DeltaTime > TargetFrameTime)
	{
		FramesBelowTarget++;
	}
}
