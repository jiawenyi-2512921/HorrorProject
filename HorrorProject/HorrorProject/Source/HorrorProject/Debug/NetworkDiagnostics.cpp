// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkDiagnostics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/NetDriver.h"

namespace HorrorNetworkDiagnostics
{
	constexpr float DefaultHighPingThresholdMs = 150.0f;
	constexpr float ExcellentPingThresholdMs = 50.0f;
	constexpr float GoodPingThresholdMs = 100.0f;
	constexpr float PercentMultiplier = 100.0f;
	constexpr int32 MaxHistorySamples = 300;
}

UNetworkDiagnostics::UNetworkDiagnostics()
{
	bIsMonitoring = false;
	MonitoringInterval = 2.0f;
	HighPingThreshold = HorrorNetworkDiagnostics::DefaultHighPingThresholdMs;
	PacketLossThreshold = 5.0f;
}

void UNetworkDiagnostics::StartMonitoring(float Interval)
{
	if (bIsMonitoring) return;

	if (!IsNetworked())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start network monitoring: Not in networked game"));
		return;
	}

	MonitoringInterval = FMath::Max(1.0f, Interval);
	bIsMonitoring = true;

	UWorld* World = GetWorld();
	if (!World)
	{
		bIsMonitoring = false;
		return;
	}

	World->GetTimerManager().SetTimer(MonitoringTimer,
		this, &UNetworkDiagnostics::CollectNetworkData,
		MonitoringInterval, true);

	UE_LOG(LogTemp, Log, TEXT("Network monitoring started (interval: %.2f seconds)"), MonitoringInterval);
}

void UNetworkDiagnostics::StopMonitoring()
{
	if (!bIsMonitoring) return;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MonitoringTimer);
	}
	bIsMonitoring = false;

	UE_LOG(LogTemp, Log, TEXT("Network monitoring stopped"));
}

void UNetworkDiagnostics::CollectNetworkData()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FNetworkSnapshot Snapshot;
	Snapshot.Timestamp = FDateTime::Now();

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (PC && PC->PlayerState)
	{
		Snapshot.PingMS = PC->PlayerState->GetPingInMilliseconds();
	}

	if (const UNetDriver* NetDriver = World->GetNetDriver())
	{
		Snapshot.BytesSent = static_cast<int32>(FMath::Min<uint32>(NetDriver->OutBytesPerSecond, MAX_int32));
		Snapshot.BytesReceived = static_cast<int32>(FMath::Min<uint32>(NetDriver->InBytesPerSecond, MAX_int32));

		const uint32 LostPackets = NetDriver->InPacketsLost + NetDriver->OutPacketsLost;
		const uint32 TotalPackets = NetDriver->InPackets + NetDriver->OutPackets + LostPackets;
		Snapshot.PacketLossPercent = TotalPackets > 0
			? (static_cast<float>(LostPackets) / static_cast<float>(TotalPackets)) * HorrorNetworkDiagnostics::PercentMultiplier
			: 0.0f;
	}
	else
	{
		Snapshot.PacketLossPercent = 0.0f;
		Snapshot.BytesSent = 0;
		Snapshot.BytesReceived = 0;
	}
	Snapshot.ConnectedPlayers = World->GetNumPlayerControllers();

	NetworkHistory.Add(Snapshot);

	// Keep only the most recent samples.
	if (NetworkHistory.Num() > HorrorNetworkDiagnostics::MaxHistorySamples)
	{
		NetworkHistory.RemoveAt(0);
	}

	CheckNetworkThresholds(Snapshot);
}

void UNetworkDiagnostics::CheckNetworkThresholds(const FNetworkSnapshot& Snapshot)
{
	if (Snapshot.PingMS > HighPingThreshold)
	{
		UE_LOG(LogTemp, Warning, TEXT("High ping detected: %.0f ms"), Snapshot.PingMS);
	}

	if (Snapshot.PacketLossPercent > PacketLossThreshold)
	{
		UE_LOG(LogTemp, Warning, TEXT("High packet loss: %.2f%%"), Snapshot.PacketLossPercent);
	}
}

FNetworkStats UNetworkDiagnostics::GetCurrentStats() const
{
	FNetworkStats Stats;

	if (NetworkHistory.Num() == 0)
	{
		return Stats;
	}

	float TotalPing = 0.0f;
	float TotalPacketLoss = 0.0f;
	float MinPing = FLT_MAX;
	float MaxPing = 0.0f;
	int32 TotalBytesSent = 0;
	int32 TotalBytesReceived = 0;

	for (const FNetworkSnapshot& Snapshot : NetworkHistory)
	{
		TotalPing += Snapshot.PingMS;
		TotalPacketLoss += Snapshot.PacketLossPercent;
		MinPing = FMath::Min(MinPing, Snapshot.PingMS);
		MaxPing = FMath::Max(MaxPing, Snapshot.PingMS);
		TotalBytesSent += Snapshot.BytesSent;
		TotalBytesReceived += Snapshot.BytesReceived;
	}

	int32 Count = NetworkHistory.Num();
	Stats.AveragePing = TotalPing / Count;
	Stats.MinPing = MinPing;
	Stats.MaxPing = MaxPing;
	Stats.AveragePacketLoss = TotalPacketLoss / Count;
	Stats.TotalBytesSent = TotalBytesSent;
	Stats.TotalBytesReceived = TotalBytesReceived;

	return Stats;
}

void UNetworkDiagnostics::GenerateNetworkReport(const FString& FilePath)
{
	FString OutputPath = FilePath.IsEmpty() ?
		FPaths::ProjectSavedDir() / TEXT("Diagnostics") / FString::Printf(TEXT("Network_%s.txt"),
			*FDateTime::Now().ToString()) : FilePath;

	FString Content = TEXT("=== NETWORK DIAGNOSTIC REPORT ===\n");
	Content += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
	Content += FString::Printf(TEXT("Samples: %d\n"), NetworkHistory.Num());
	Content += FString::Printf(TEXT("Networked: %s\n\n"), IsNetworked() ? TEXT("Yes") : TEXT("No"));

	FNetworkStats Stats = GetCurrentStats();

	Content += TEXT("=== SUMMARY ===\n");
	Content += FString::Printf(TEXT("Average Ping: %.2f ms\n"), Stats.AveragePing);
	Content += FString::Printf(TEXT("Min Ping: %.2f ms\n"), Stats.MinPing);
	Content += FString::Printf(TEXT("Max Ping: %.2f ms\n"), Stats.MaxPing);
	Content += FString::Printf(TEXT("Average Packet Loss: %.2f%%\n"), Stats.AveragePacketLoss);
	Content += FString::Printf(TEXT("Total Bytes Sent: %d\n"), Stats.TotalBytesSent);
	Content += FString::Printf(TEXT("Total Bytes Received: %d\n\n"), Stats.TotalBytesReceived);

	Content += TEXT("=== DETAILED HISTORY ===\n");
	for (const FNetworkSnapshot& Snapshot : NetworkHistory)
	{
		Content += FString::Printf(TEXT("[%s] Ping: %.0f ms | Loss: %.2f%% | Sent: %d B | Recv: %d B | Players: %d\n"),
			*Snapshot.Timestamp.ToString(),
			Snapshot.PingMS,
			Snapshot.PacketLossPercent,
			Snapshot.BytesSent,
			Snapshot.BytesReceived,
			Snapshot.ConnectedPlayers);
	}

	if (FFileHelper::SaveStringToFile(Content, *OutputPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Network report saved to: %s"), *OutputPath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save network report to: %s"), *OutputPath);
	}
}

void UNetworkDiagnostics::ClearHistory()
{
	NetworkHistory.Empty();
	UE_LOG(LogTemp, Log, TEXT("Network history cleared"));
}

void UNetworkDiagnostics::TestConnection()
{
	UE_LOG(LogTemp, Warning, TEXT("Testing network connection..."));

	if (!IsNetworked())
	{
		UE_LOG(LogTemp, Warning, TEXT("Not in networked game"));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (PC && PC->PlayerState)
	{
		float Ping = PC->PlayerState->GetPingInMilliseconds();
		UE_LOG(LogTemp, Log, TEXT("Current Ping: %.0f ms"), Ping);

		if (Ping < HorrorNetworkDiagnostics::ExcellentPingThresholdMs)
		{
			UE_LOG(LogTemp, Log, TEXT("Connection Quality: Excellent"));
		}
		else if (Ping < HorrorNetworkDiagnostics::GoodPingThresholdMs)
		{
			UE_LOG(LogTemp, Log, TEXT("Connection Quality: Good"));
		}
		else if (Ping < HorrorNetworkDiagnostics::DefaultHighPingThresholdMs)
		{
			UE_LOG(LogTemp, Warning, TEXT("Connection Quality: Fair"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Connection Quality: Poor"));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Connected Players: %d"), World->GetNumPlayerControllers());
	UE_LOG(LogTemp, Log, TEXT("Net Mode: %d"), (int32)World->GetNetMode());
}

void UNetworkDiagnostics::DumpNetworkStats()
{
	UE_LOG(LogTemp, Warning, TEXT("Dumping network statistics..."));

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("=== NETWORK STATISTICS ==="));
	const ENetMode NetMode = World->GetNetMode();
	const bool bIsServer = NetMode == NM_ListenServer || NetMode == NM_DedicatedServer;
	const bool bIsClient = NetMode == NM_Client;
	UE_LOG(LogTemp, Log, TEXT("Net Mode: %d"), (int32)NetMode);
	UE_LOG(LogTemp, Log, TEXT("Is Server: %s"), bIsServer ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogTemp, Log, TEXT("Is Client: %s"), bIsClient ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogTemp, Log, TEXT("Player Controllers: %d"), World->GetNumPlayerControllers());

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
	if (PC && PC->PlayerState)
	{
		UE_LOG(LogTemp, Log, TEXT("Ping: %.0f ms"), PC->PlayerState->GetPingInMilliseconds());
	}

	// Execute engine network commands
	World->Exec(World, TEXT("stat net"));
}

bool UNetworkDiagnostics::IsNetworked() const
{
	const UWorld* World = GetWorld();
	return World && World->GetNetMode() != NM_Standalone;
}
