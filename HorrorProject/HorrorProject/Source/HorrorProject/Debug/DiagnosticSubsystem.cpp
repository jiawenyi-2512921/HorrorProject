// Copyright Epic Games, Inc. All Rights Reserved.

#include "DiagnosticSubsystem.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "DynamicRHI.h"
#include "Engine/NetDriver.h"

namespace HorrorDiagnostics
{
	constexpr float FrameBudgetMs = 33.0f;
	constexpr float BytesPerMegabyte = 1024.0f * 1024.0f;
	constexpr float PercentMultiplier = 100.0f;
	constexpr float MemoryWarningPercent = 80.0f;
	constexpr float MemoryCriticalPercent = 90.0f;
	constexpr float PacketLossWarningPercent = 5.0f;
}

void UDiagnosticSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	bAutoDiagnosticsEnabled = false;
	AutoDiagnosticsInterval = HorrorDiagnosticsDefaults::AutoDiagnosticsIntervalSeconds;

	UE_LOG(LogTemp, Log, TEXT("Diagnostic Subsystem Initialized"));
}

void UDiagnosticSubsystem::Deinitialize()
{
	if (bAutoDiagnosticsEnabled)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(AutoDiagnosticsTimer);
		}
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
	if (GameThreadTime > HorrorDiagnostics::FrameBudgetMs)
	{
		AddDiagnosticReport(TEXT("Performance"),
			FString::Printf(TEXT("High game thread time: %.2f ms"), GameThreadTime),
			EDiagnosticSeverity::Warning);
	}

	// Check render thread time
	float RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
	if (RenderThreadTime > HorrorDiagnostics::FrameBudgetMs)
	{
		AddDiagnosticReport(TEXT("Performance"),
			FString::Printf(TEXT("High render thread time: %.2f ms"), RenderThreadTime),
			EDiagnosticSeverity::Warning);
	}

	// Check GPU time
	float GPUTime = FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles());
	if (GPUTime > HorrorDiagnostics::FrameBudgetMs)
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
	float UsedPhysicalMB = MemStats.UsedPhysical / HorrorDiagnostics::BytesPerMegabyte;
	float AvailablePhysicalMB = MemStats.AvailablePhysical / HorrorDiagnostics::BytesPerMegabyte;
	float UsagePercent = (UsedPhysicalMB / (UsedPhysicalMB + AvailablePhysicalMB)) * HorrorDiagnostics::PercentMultiplier;

	AddDiagnosticReport(TEXT("Memory"),
		FString::Printf(TEXT("Physical Memory: %.2f MB used (%.1f%%)"), UsedPhysicalMB, UsagePercent),
		UsagePercent > HorrorDiagnostics::MemoryWarningPercent ? EDiagnosticSeverity::Warning : EDiagnosticSeverity::Info);

	// Check for memory leaks (simplified)
	if (UsagePercent > HorrorDiagnostics::MemoryCriticalPercent)
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
	UWorld* World = GetWorld();
	if (!World)
	{
		AddDiagnosticReport(TEXT("Network"), TEXT("World unavailable"), EDiagnosticSeverity::Warning);
		return;
	}

	if (World->GetNetMode() == NM_Standalone)
	{
		AddDiagnosticReport(TEXT("Network"), TEXT("Running in standalone mode"), EDiagnosticSeverity::Info);
		return;
	}

	const UNetDriver* NetDriver = World->GetNetDriver();
	if (!NetDriver)
	{
		AddDiagnosticReport(TEXT("Network"), TEXT("World is networked but has no active NetDriver"), EDiagnosticSeverity::Warning);
		return;
	}

	AddDiagnosticReport(TEXT("Network"),
		FString::Printf(TEXT("NetDriver: %s | Clients: %d | In: %u B/s | Out: %u B/s"),
			*GetNameSafe(NetDriver),
			NetDriver->ClientConnections.Num(),
			NetDriver->InBytesPerSecond,
			NetDriver->OutBytesPerSecond),
		EDiagnosticSeverity::Info);

	const uint32 LostPackets = NetDriver->InPacketsLost + NetDriver->OutPacketsLost;
	const uint32 TotalPackets = NetDriver->InPackets + NetDriver->OutPackets + LostPackets;
	const float PacketLossPercent = TotalPackets > 0
		? (static_cast<float>(LostPackets) / static_cast<float>(TotalPackets)) * HorrorDiagnostics::PercentMultiplier
		: 0.0f;

	AddDiagnosticReport(TEXT("Network"),
		FString::Printf(TEXT("Packet loss estimate: %.2f%% (lost: %u)"), PacketLossPercent, LostPackets),
		PacketLossPercent > HorrorDiagnostics::PacketLossWarningPercent ? EDiagnosticSeverity::Warning : EDiagnosticSeverity::Info);
}

void UDiagnosticSubsystem::RunGameplayDiagnostics()
{
	AddDiagnosticReport(TEXT("Gameplay"), TEXT("Starting gameplay diagnostics"), EDiagnosticSeverity::Info);

	CheckActorCount();

	UWorld* World = GetWorld();
	if (!World)
	{
		AddDiagnosticReport(TEXT("Gameplay"), TEXT("World unavailable"), EDiagnosticSeverity::Warning);
		return;
	}

	// Check for null references
	int32 NullActorCount = 0;
	for (TActorIterator<AActor> It(World); It; ++It)
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
	float WorldTime = World->GetTimeSeconds();
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
		UWorld* World = GetWorld();
		if (!World)
		{
			bAutoDiagnosticsEnabled = false;
			AddDiagnosticReport(TEXT("Diagnostics"), TEXT("Cannot enable auto-diagnostics: world unavailable"), EDiagnosticSeverity::Warning);
			return;
		}

		World->GetTimerManager().SetTimer(AutoDiagnosticsTimer,
			this, &UDiagnosticSubsystem::PerformAutoDiagnostics,
			AutoDiagnosticsInterval, true);

		UE_LOG(LogTemp, Log, TEXT("Auto-diagnostics enabled (interval: %.2f seconds)"), Interval);
	}
	else
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(AutoDiagnosticsTimer);
		}
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
	UWorld* World = GetWorld();
	if (!World)
	{
		AddDiagnosticReport(TEXT("Performance"), TEXT("World unavailable for frame-rate check"), EDiagnosticSeverity::Warning);
		return;
	}

	float CurrentFPS = 1.0f / World->GetDeltaSeconds();

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
	float UsedPhysicalMB = MemStats.UsedPhysical / HorrorDiagnostics::BytesPerMegabyte;

	if (UsedPhysicalMB > MaxMemoryUsageMB)
	{
		AddDiagnosticReport(TEXT("Memory"),
			FString::Printf(TEXT("High memory usage: %.2f MB (threshold: %.2f MB)"), UsedPhysicalMB, MaxMemoryUsageMB),
			EDiagnosticSeverity::Warning);
	}
}

void UDiagnosticSubsystem::CheckActorCount()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		AddDiagnosticReport(TEXT("Gameplay"), TEXT("World unavailable for actor-count check"), EDiagnosticSeverity::Warning);
		return;
	}

	int32 ActorCount = World->GetActorCount();

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
