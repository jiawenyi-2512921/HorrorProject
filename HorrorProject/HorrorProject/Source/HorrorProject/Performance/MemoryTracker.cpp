// Copyright Epic Games, Inc. All Rights Reserved.

#include "Performance/MemoryTracker.h"
#include "HAL/PlatformMemory.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "Engine/World.h"

namespace HorrorMemoryTracker
{
	constexpr float BytesPerMegabyte = 1024.0f * 1024.0f;
	constexpr float PercentMultiplier = 100.0f;
	constexpr float MinMemoryBudgetMB = 100.0f;
	constexpr float UpdateIntervalSeconds = 1.0f;

	float BytesToMegabytes(uint64 Bytes)
	{
		return static_cast<float>(Bytes) / BytesPerMegabyte;
	}
}

void UMemoryTracker::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MemoryHistory.Reserve(MaxHistorySize);

	UE_LOG(LogTemp, Log, TEXT("MemoryTracker initialized"));
}

void UMemoryTracker::Deinitialize()
{
	if (UpdateTimerHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(UpdateTimerHandle);
		}
	}

	Super::Deinitialize();
}

void UMemoryTracker::SetTrackingEnabled(bool bEnabled)
{
	if (bTrackingEnabled == bEnabled)
	{
		return;
	}

	bTrackingEnabled = bEnabled;

	if (bEnabled)
	{
		ResetStats();

		// Update memory stats every second
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				UpdateTimerHandle,
				this,
				&UMemoryTracker::UpdateMemoryStats,
				HorrorMemoryTracker::UpdateIntervalSeconds,
				true);
		}

		UE_LOG(LogTemp, Log, TEXT("Memory tracking enabled"));
	}
	else
	{
		if (UpdateTimerHandle.IsValid())
		{
			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().ClearTimer(UpdateTimerHandle);
			}
		}

		UE_LOG(LogTemp, Log, TEXT("Memory tracking disabled"));
	}
}

FHorrorMemoryTrackerStats UMemoryTracker::GetMemoryStats() const
{
	FHorrorMemoryTrackerStats Stats;

	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();

	Stats.UsedPhysicalMB = HorrorMemoryTracker::BytesToMegabytes(MemStats.UsedPhysical);
	Stats.UsedVirtualMB = HorrorMemoryTracker::BytesToMegabytes(MemStats.UsedVirtual);
	Stats.PeakUsedPhysicalMB = HorrorMemoryTracker::BytesToMegabytes(MemStats.PeakUsedPhysical);
	Stats.PeakUsedVirtualMB = HorrorMemoryTracker::BytesToMegabytes(MemStats.PeakUsedVirtual);
	Stats.AvailablePhysicalMB = HorrorMemoryTracker::BytesToMegabytes(MemStats.AvailablePhysical);
	Stats.TotalPhysicalMB = HorrorMemoryTracker::BytesToMegabytes(MemStats.TotalPhysical);

	// Get texture memory if available
	if (GEngine && GEngine->GetWorld())
	{
		// These are approximations - actual values require RHI queries
		Stats.TextureMemoryMB = 0.0f; // Would need RHI query
		Stats.RenderTargetMemoryMB = 0.0f; // Would need RHI query
	}

	// Get object count
	Stats.AllocatedObjects = GUObjectArray.GetObjectArrayNum();

	return Stats;
}

void UMemoryTracker::SetMemoryBudgetMB(float BudgetMB)
{
	MemoryBudgetMB = FMath::Max(HorrorMemoryTracker::MinMemoryBudgetMB, BudgetMB);
	bBudgetExceededWarningShown = false;
	UE_LOG(LogTemp, Log, TEXT("Memory budget set to: %.1f MB"), MemoryBudgetMB);
}

bool UMemoryTracker::IsMemoryWithinBudget() const
{
	const FHorrorMemoryTrackerStats Stats = GetMemoryStats();
	return Stats.UsedPhysicalMB <= MemoryBudgetMB;
}

float UMemoryTracker::GetMemoryBudgetPercent() const
{
	const FHorrorMemoryTrackerStats Stats = GetMemoryStats();
	return MemoryBudgetMB > 0.0f
		? (Stats.UsedPhysicalMB / MemoryBudgetMB) * HorrorMemoryTracker::PercentMultiplier
		: 0.0f;
}

void UMemoryTracker::MarkCheckpoint(const FString& CheckpointName)
{
	const FHorrorMemoryTrackerStats Stats = GetMemoryStats();
	CheckpointMemoryMB = Stats.UsedPhysicalMB;
	LastCheckpointName = CheckpointName;

	UE_LOG(LogTemp, Log, TEXT("Memory checkpoint '%s': %.2f MB"), *CheckpointName, CheckpointMemoryMB);
}

float UMemoryTracker::GetMemoryDeltaSinceCheckpoint() const
{
	const FHorrorMemoryTrackerStats Stats = GetMemoryStats();
	return Stats.UsedPhysicalMB - CheckpointMemoryMB;
}

void UMemoryTracker::ResetStats()
{
	MemoryHistory.Empty();
	PeakPhysicalMB = 0.0f;
	PeakVirtualMB = 0.0f;
	CheckpointMemoryMB = 0.0f;
	LastCheckpointName.Empty();
	bBudgetExceededWarningShown = false;

	UE_LOG(LogTemp, Log, TEXT("Memory stats reset"));
}

bool UMemoryTracker::ExportToCSV(const FString& FilePath)
{
	if (MemoryHistory.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No memory data to export"));
		return false;
	}

	FString CSVContent;
	CSVContent += TEXT("Timestamp,MemoryDeltaMB,Context\n");

	for (const FMemoryAllocationEvent& Event : MemoryHistory)
	{
		CSVContent += FString::Printf(TEXT("%s,%.4f,%s\n"),
			*Event.Timestamp.ToString(),
			Event.MemoryDeltaMB,
			*Event.Context);
	}

	const FString FullPath = FPaths::ProjectSavedDir() / TEXT("Profiling") / FilePath;
	const FString Directory = FPaths::GetPath(FullPath);

	if (!FPaths::DirectoryExists(Directory))
	{
		IFileManager::Get().MakeDirectory(*Directory, true);
	}

	if (FFileHelper::SaveStringToFile(CSVContent, *FullPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Memory data exported to: %s"), *FullPath);
		return true;
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to export memory data to: %s"), *FullPath);
	return false;
}

void UMemoryTracker::UpdateMemoryStats()
{
	if (!bTrackingEnabled)
	{
		return;
	}

	const FHorrorMemoryTrackerStats Stats = GetMemoryStats();

	// Update peaks
	PeakPhysicalMB = FMath::Max(PeakPhysicalMB, Stats.UsedPhysicalMB);
	PeakVirtualMB = FMath::Max(PeakVirtualMB, Stats.UsedVirtualMB);

	// Record event if significant change
	const float MemoryDelta = Stats.UsedPhysicalMB - LastRecordedMemoryMB;

	if (FMath::Abs(MemoryDelta) > 10.0f) // 10MB threshold
	{
		FMemoryAllocationEvent Event;
		Event.Timestamp = FDateTime::Now();
		Event.MemoryDeltaMB = MemoryDelta;
		Event.Context = FString::Printf(TEXT("Total: %.2f MB"), Stats.UsedPhysicalMB);

		MemoryHistory.Add(Event);

		if (MemoryHistory.Num() > MaxHistorySize)
		{
			MemoryHistory.RemoveAt(0);
		}

		LastRecordedMemoryMB = Stats.UsedPhysicalMB;
	}

	CheckMemoryBudget();
}

void UMemoryTracker::CheckMemoryBudget()
{
	if (!IsMemoryWithinBudget() && !bBudgetExceededWarningShown)
	{
		const FHorrorMemoryTrackerStats Stats = GetMemoryStats();

		UE_LOG(LogTemp, Warning, TEXT("Memory budget exceeded! Current: %.2f MB, Budget: %.2f MB"),
			Stats.UsedPhysicalMB, MemoryBudgetMB);

		OnMemoryBudgetExceeded.Broadcast(Stats.UsedPhysicalMB);
		bBudgetExceededWarningShown = true;
	}
	else if (IsMemoryWithinBudget())
	{
		bBudgetExceededWarningShown = false;
	}
}
