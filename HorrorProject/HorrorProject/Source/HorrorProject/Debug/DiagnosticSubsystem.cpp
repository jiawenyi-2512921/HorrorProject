// Copyright Epic Games, Inc. All Rights Reserved.

#include "DiagnosticSubsystem.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TimerManager.h"

void UDiagnosticSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	bAutoDiagnosticsEnabled = false;
	AutoDiagnosticsInterval = 60.0f;

	UE_LOG(LogTemp, Log, TEXT("Diagnostic Subsystem Initialized"));
}

void UDiagnosticSubsystem::Deinitialize()
{
	if (bAutoDiagnosticsEnabled)
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoDiagnosticsTimer);
	}

	Super::Deinitialize();
}

void UDiagnosticSubsystem::RunFullDiagnostics()
{
	UE_LOG(LogTemp, Warning, TEXT("Running Full Diagnostics..."));

	ClearDiagnosticReports();

	RunPerformanceDiagnostics();
	RunMemoryDiagnostics();
	RunNetworkDiagnostics();
	RunGameplayDiagnostics();

	UE_LOG(LogTemp, Warning, TEXT("Full Diagnostics Complete: %d reports generated"), DiagnosticReports.Num());
}

void UDiagnosticSubsystem::RunPerformanceDiagnostics()
{
	AddDiagnosticReport(TEXT("Performance"), TEXT("Starting performance diagnostics"), EDiagnosticSeverity::Info);

	CheckFrameRate();
	CheckDrawCalls();

	// Check game thread time
	float GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
	if (GameThreadTime > 33.0f)
	{
		AddDiagnosticReport(TEXT("Performance"),
			FString::Printf(TEXT("High game thread time: %.2f ms"), GameThreadTime),
			EDiagnosticSeverity::Warning);
	}

	// Check render thread time
	float RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
	if (RenderThreadTime > 33.0f)
	{
		AddDiagnosticReport(TEXT("Performance"),
			FString::Printf(TEXT("High render thread time: %.2f ms"), RenderThreadTime),
			EDiagnosticSeverity::Warning);
	}

	// Check GPU time
	float GPUTime = FPlatformTime::ToMilliseconds(GGPUFrameTime);
	if (GPUTime > 33.0f)
	{
		AddDiagnosticReport(TEXT("Performance"),
			FString::Printf(TEXT("High GPU time: %.2f ms"), GPUTime),
			EDiagnosticSeverity::Warning);
	}
}

void UDiagnosticSubsystem::RunMemoryDiagnostics()
{
	AddDiagnosticReport(TEXT("Memory"), TEXT("Starting memory diagnostics"), EDiagnosticSeverity::Info);

	CheckMemoryUsage();

	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	float UsedPhysicalMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
	float AvailablePhysicalMB = MemStats.AvailablePhysical / (1024.0f * 1024.0f);
	float UsagePercent = (UsedPhysicalMB / (UsedPhysicalMB + AvailablePhysicalMB)) * 100.0f;

	AddDiagnosticReport(TEXT("Memory"),
		FString::Printf(TEXT("Physical Memory: %.2f MB used (%.1f%%)"), UsedPhysicalMB, UsagePercent),
		UsagePercent > 80.0f ? EDiagnosticSeverity::Warning : EDiagnosticSeverity::Info);

	// Check for memory leaks (simplified)
	if (UsagePercent > 90.0f)
	{
		AddDiagnosticReport(TEXT("Memory"),
			TEXT("Critical memory usage - possible memory leak"),
			EDiagnosticSeverity::Critical);
	}
}

void UDiagnosticSubsystem::RunNetworkDiagnostics()
{
	AddDiagnosticReport(TEXT("Network"), TEXT("Starting network diagnostics"), EDiagnosticSeverity::Info);

	// Check if networked
	if (GetWorld()->GetNetMode() == NM_Standalone)
	{
		AddDiagnosticReport(TEXT("Network"), TEXT("Running in standalone mode"), EDiagnosticSeverity::Info);
		return;
	}

	// TODO: Add network-specific checks
	AddDiagnosticReport(TEXT("Network"), TEXT("Network diagnostics not fully implemented"), EDiagnosticSeverity::Warning);
}

void UDiagnosticSubsystem::RunGameplayDiagnostics()
{
	AddDiagnosticReport(TEXT("Gameplay"), TEXT("Starting gameplay diagnostics"), EDiagnosticSeverity::Info);

	CheckActorCount();

	// Check for null references
	int32 NullActorCount = 0;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (!It->IsValidLowLevel())
		{
			NullActorCount++;
		}
	}

	if (NullActorCount > 0)
	{
		AddDiagnosticReport(TEXT("Gameplay"),
			FString::Printf(TEXT("Found %d invalid actors"), NullActorCount),
			EDiagnosticSeverity::Error);
	}

	// Check world time
	float WorldTime = GetWorld()->GetTimeSeconds();
	AddDiagnosticReport(TEXT("Gameplay"),
		FString::Printf(TEXT("World time: %.2f seconds"), WorldTime),
		EDiagnosticSeverity::Info);
}

void UDiagnosticSubsystem::ClearDiagnosticReports()
{
	DiagnosticReports.Empty();
	UE_LOG(LogTemp, Log, TEXT("Diagnostic reports cleared"));
}

void UDiagnosticSubsystem::ExportDiagnosticsToFile(const FString& FilePath)
{
	FString OutputPath = FilePath.IsEmpty() ?
		FPaths::ProjectSavedDir() / TEXT("Diagnostics") / FString::Printf(TEXT("Diagnostics_%s.txt"),
			*FDateTime::Now().ToString()) : FilePath;

	FString Content = TEXT("=== DIAGNOSTIC REPORT ===\n");
	Content += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());

	for (const FDiagnosticReport& Report : DiagnosticReports)
	{
		FString SeverityStr;
		switch (Report.Severity)
		{
		case EDiagnosticSeverity::Info: SeverityStr = TEXT("INFO"); break;
		case EDiagnosticSeverity::Warning: SeverityStr = TEXT("WARNING"); break;
		case EDiagnosticSeverity::Error: SeverityStr = TEXT("ERROR"); break;
		case EDiagnosticSeverity::Critical: SeverityStr = TEXT("CRITICAL"); break;
		}

		Content += FString::Printf(TEXT("[%s] [%s] %s: %s\n"),
			*Report.Timestamp.ToString(),
			*SeverityStr,
			*Report.Category,
			*Report.Message);
	}

	if (FFileHelper::SaveStringToFile(Content, *OutputPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Diagnostics exported to: %s"), *OutputPath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to export diagnostics to: %s"), *OutputPath);
	}
}

void UDiagnosticSubsystem::SetAutoDiagnostics(bool bEnabled, float Interval)
{
	bAutoDiagnosticsEnabled = bEnabled;
	AutoDiagnosticsInterval = Interval;

	if (bEnabled)
	{
		GetWorld()->GetTimerManager().SetTimer(AutoDiagnosticsTimer,
			this, &UDiagnosticSubsystem::PerformAutoDiagnostics,
			AutoDiagnosticsInterval, true);

		UE_LOG(LogTemp, Log, TEXT("Auto-diagnostics enabled (interval: %.2f seconds)"), Interval);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoDiagnosticsTimer);
		UE_LOG(LogTemp, Log, TEXT("Auto-diagnostics disabled"));
	}
}

void UDiagnosticSubsystem::AddDiagnosticReport(const FString& Category, const FString& Message, EDiagnosticSeverity Severity)
{
	FDiagnosticReport Report;
	Report.Category = Category;
	Report.Message = Message;
	Report.Severity = Severity;
	Report.Timestamp = FDateTime::Now();

	DiagnosticReports.Add(Report);

	// Log based on severity
	switch (Severity)
	{
	case EDiagnosticSeverity::Info:
		UE_LOG(LogTemp, Log, TEXT("[DIAGNOSTIC] %s: %s"), *Category, *Message);
		break;
	case EDiagnosticSeverity::Warning:
		UE_LOG(LogTemp, Warning, TEXT("[DIAGNOSTIC] %s: %s"), *Category, *Message);
		break;
	case EDiagnosticSeverity::Error:
	case EDiagnosticSeverity::Critical:
		UE_LOG(LogTemp, Error, TEXT("[DIAGNOSTIC] %s: %s"), *Category, *Message);
		break;
	}
}

void UDiagnosticSubsystem::PerformAutoDiagnostics()
{
	UE_LOG(LogTemp, Log, TEXT("Performing auto-diagnostics..."));
	RunFullDiagnostics();
}

void UDiagnosticSubsystem::CheckFrameRate()
{
	float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();

	if (CurrentFPS < MinAcceptableFPS)
	{
		AddDiagnosticReport(TEXT("Performance"),
			FString::Printf(TEXT("Low FPS detected: %.1f (threshold: %.1f)"), CurrentFPS, MinAcceptableFPS),
			EDiagnosticSeverity::Warning);
	}
	else
	{
		AddDiagnosticReport(TEXT("Performance"),
			FString::Printf(TEXT("FPS: %.1f"), CurrentFPS),
			EDiagnosticSeverity::Info);
	}
}

void UDiagnosticSubsystem::CheckMemoryUsage()
{
	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	float UsedPhysicalMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);

	if (UsedPhysicalMB > MaxMemoryUsageMB)
	{
		AddDiagnosticReport(TEXT("Memory"),
			FString::Printf(TEXT("High memory usage: %.2f MB (threshold: %.2f MB)"), UsedPhysicalMB, MaxMemoryUsageMB),
			EDiagnosticSeverity::Warning);
	}
}

void UDiagnosticSubsystem::CheckActorCount()
{
	int32 ActorCount = GetWorld()->GetActorCount();

	if (ActorCount > MaxActorCount)
	{
		AddDiagnosticReport(TEXT("Gameplay"),
			FString::Printf(TEXT("High actor count: %d (threshold: %d)"), ActorCount, MaxActorCount),
			EDiagnosticSeverity::Warning);
	}
	else
	{
		AddDiagnosticReport(TEXT("Gameplay"),
			FString::Printf(TEXT("Actor count: %d"), ActorCount),
			EDiagnosticSeverity::Info);
	}
}

void UDiagnosticSubsystem::CheckDrawCalls()
{
	// Note: Actual draw call count requires RHI stats
	AddDiagnosticReport(TEXT("Performance"),
		TEXT("Draw call check requires stat rhi"),
		EDiagnosticSeverity::Info);
}
