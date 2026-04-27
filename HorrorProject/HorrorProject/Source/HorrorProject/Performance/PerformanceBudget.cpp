// Copyright Epic Games, Inc. All Rights Reserved.

#include "Performance/PerformanceBudget.h"
#include "Engine/Engine.h"
#include "HAL/PlatformTime.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "DynamicRHI.h"

namespace HorrorPerformanceBudget
{
	constexpr float PercentMultiplier = 100.0f;
	constexpr float UpdateIntervalSeconds = 1.0f;
	constexpr float GameThreadBudgetMs = 10.0f;
	constexpr float RenderThreadBudgetMs = 12.0f;
	constexpr float GpuBudgetMs = 14.0f;
	constexpr float MemoryBudgetMB = 4096.0f;
	constexpr float DrawCallBudget = 3000.0f;
	constexpr float TriangleBudget = 5000000.0f;
	constexpr float LumenBudgetMs = 8.0f;
	constexpr float VsmBudgetMs = 3.0f;
}

void UPerformanceBudget::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadDefaultBudgets();

	UE_LOG(LogTemp, Log, TEXT("PerformanceBudget initialized"));
}

void UPerformanceBudget::Deinitialize()
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

void UPerformanceBudget::SetBudget(FName BudgetName, EPerformanceBudgetCategory Category, float BudgetValue)
{
	FPerformanceBudgetEntry& Entry = Budgets.FindOrAdd(BudgetName);
	Entry.BudgetName = BudgetName;
	Entry.Category = Category;
	Entry.BudgetValue = FMath::Max(0.0f, BudgetValue);

	UE_LOG(LogTemp, Log, TEXT("Budget set: %s = %.2f"), *BudgetName.ToString(), BudgetValue);
}

void UPerformanceBudget::UpdateBudgetValue(FName BudgetName, float CurrentValue)
{
	FPerformanceBudgetEntry* Entry = Budgets.Find(BudgetName);
	if (!Entry)
	{
		return;
	}

	Entry->CurrentValue = CurrentValue;

	if (Entry->BudgetValue > 0.0f)
	{
		Entry->PercentUsed = (CurrentValue / Entry->BudgetValue) * HorrorPerformanceBudget::PercentMultiplier;
		Entry->bExceeded = CurrentValue > Entry->BudgetValue;

		if (Entry->bExceeded && !WarningTriggered.Contains(BudgetName))
		{
			UE_LOG(LogTemp, Warning, TEXT("Budget exceeded: %s (%.2f / %.2f = %.1f%%)"),
				*BudgetName.ToString(), CurrentValue, Entry->BudgetValue, Entry->PercentUsed);

			OnBudgetExceeded.Broadcast(BudgetName);
			WarningTriggered.Add(BudgetName);
		}
		else if (!Entry->bExceeded && WarningTriggered.Contains(BudgetName))
		{
			WarningTriggered.Remove(BudgetName);
		}
	}
}

FPerformanceBudgetEntry UPerformanceBudget::GetBudget(FName BudgetName) const
{
	const FPerformanceBudgetEntry* Entry = Budgets.Find(BudgetName);
	return Entry ? *Entry : FPerformanceBudgetEntry();
}

TArray<FPerformanceBudgetEntry> UPerformanceBudget::GetAllBudgets() const
{
	TArray<FPerformanceBudgetEntry> AllBudgets;
	Budgets.GenerateValueArray(AllBudgets);

	// Sort by percent used (descending)
	AllBudgets.Sort([](const FPerformanceBudgetEntry& A, const FPerformanceBudgetEntry& B)
	{
		return A.PercentUsed > B.PercentUsed;
	});

	return AllBudgets;
}

bool UPerformanceBudget::IsAnyBudgetExceeded() const
{
	for (const auto& Pair : Budgets)
	{
		if (Pair.Value.bExceeded)
		{
			return true;
		}
	}
	return false;
}

TArray<FPerformanceBudgetEntry> UPerformanceBudget::GetExceededBudgets() const
{
	TArray<FPerformanceBudgetEntry> ExceededBudgets;

	for (const auto& Pair : Budgets)
	{
		if (Pair.Value.bExceeded)
		{
			ExceededBudgets.Add(Pair.Value);
		}
	}

	return ExceededBudgets;
}

void UPerformanceBudget::SetBudgetTrackingEnabled(bool bEnabled)
{
	if (bTrackingEnabled == bEnabled)
	{
		return;
	}

	bTrackingEnabled = bEnabled;

	if (bEnabled)
	{
		// Update automatic budgets every second
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				UpdateTimerHandle,
				this,
				&UPerformanceBudget::UpdateAutomaticBudgets,
				HorrorPerformanceBudget::UpdateIntervalSeconds,
				true);
		}

		UE_LOG(LogTemp, Log, TEXT("Budget tracking enabled"));
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

		UE_LOG(LogTemp, Log, TEXT("Budget tracking disabled"));
	}
}

void UPerformanceBudget::ResetBudgets()
{
	Budgets.Empty();
	WarningTriggered.Empty();
	LoadDefaultBudgets();

	UE_LOG(LogTemp, Log, TEXT("Budgets reset"));
}

void UPerformanceBudget::LoadDefaultBudgets()
{
	// Target: 60 FPS @ Epic quality
	// Frame budget: 16.67ms

	// Game thread budget: 10ms (60% of frame)
	SetBudget(FName("GameThread"), EPerformanceBudgetCategory::GameThread, HorrorPerformanceBudget::GameThreadBudgetMs);

	// Render thread budget: 12ms (72% of frame)
	SetBudget(FName("RenderThread"), EPerformanceBudgetCategory::RenderThread, HorrorPerformanceBudget::RenderThreadBudgetMs);

	// GPU budget: 14ms (84% of frame)
	SetBudget(FName("GPU"), EPerformanceBudgetCategory::GPU, HorrorPerformanceBudget::GpuBudgetMs);

	// Memory budget: 4GB
	SetBudget(FName("Memory"), EPerformanceBudgetCategory::Memory, HorrorPerformanceBudget::MemoryBudgetMB);

	// Draw calls budget: 3000
	SetBudget(FName("DrawCalls"), EPerformanceBudgetCategory::DrawCalls, HorrorPerformanceBudget::DrawCallBudget);

	// Triangle budget: 5M
	SetBudget(FName("Triangles"), EPerformanceBudgetCategory::Triangles, HorrorPerformanceBudget::TriangleBudget);

	// Lumen budget: 8ms
	SetBudget(FName("Lumen"), EPerformanceBudgetCategory::GPU, HorrorPerformanceBudget::LumenBudgetMs);

	// VSM budget: 3ms
	SetBudget(FName("VSM"), EPerformanceBudgetCategory::GPU, HorrorPerformanceBudget::VsmBudgetMs);

	UE_LOG(LogTemp, Log, TEXT("Default budgets loaded for 60 FPS @ Epic quality"));
}

void UPerformanceBudget::UpdateAutomaticBudgets()
{
	if (!bTrackingEnabled || !GEngine)
	{
		return;
	}

	// Update game thread time
	const float GameThreadMs = FPlatformTime::ToMilliseconds(GGameThreadTime);
	UpdateBudgetValue(FName("GameThread"), GameThreadMs);

	// Update render thread time
	const float RenderThreadMs = FPlatformTime::ToMilliseconds(GRenderThreadTime);
	UpdateBudgetValue(FName("RenderThread"), RenderThreadMs);

	// Update GPU time
	const float GPUMs = FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());
	UpdateBudgetValue(FName("GPU"), GPUMs);

	// Memory would need to be updated from MemoryTracker
	// Draw calls and triangles would need RHI queries
}
