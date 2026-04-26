// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkDiagnostics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UNetworkDiagnostics::UNetworkDiagnostics()
{
	bIsMonitoring = false;
	MonitoringInterval = 2.0f;
	HighPingThreshold = 150.0f;
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

	GetWorld()->GetTimerManager().SetTimer(MonitoringTimer,
		this, &UNetworkDiagnostics::CollectNetworkData,
		MonitoringInterval, true);

	UE_LOG(LogTemp, Log, TEXT("Network monitoring started (interval: %.2f seconds)"), MonitoringInterval);
}

void UNetworkDiagnostics::StopMonitoring()
{
	if (!bIsMonitoring) return;

	GetWorld()->GetTimerManager().ClearTimer(MonitoringTimer);
	bIsMonitoring = false;

	UE_LOG(LogTemp, Log, TEXT("Network monitoring stopped"));
}

void UNetworkDiagnostics::CollectNetworkData()
{
	FNetworkSnapshot Snapshot;
	Snapshot.Timestamp = FDateTime::Now();

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && PC->PlayerState)
	{
		Snapshot.PingMS = PC->PlayerState->GetPingInMilliseconds();
	}

	// TODO: Collect actual packet loss and bandwidth data
	Snapshot.PacketLossPercent = 0.0f;
	Snapshot.BytesSent = 0;
	Snapshot.BytesReceived = 0;
	Snapshot.ConnectedPlayers = GetWorld()->GetNumPlayerControllers();

	NetworkHistory.Add(Snapshot);

	// Keep only last 300 samples
	if (NetworkHistory.Num() > 300)
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

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && PC->PlayerState)
	{
		float Ping = PC->PlayerState->GetPingInMilliseconds();
		UE_LOG(LogTemp, Log, TEXT("Current Ping: %.0f ms"), Ping);

		if (Ping < 50.0f)
		{
			UE_LOG(LogTemp, Log, TEXT("Connection Quality: Excellent"));
		}
		else if (Ping < 100.0f)
		{
			UE_LOG(LogTemp, Log, TEXT("Connection Quality: Good"));
		}
		else if (Ping < 150.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT("Connection Quality: Fair"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Connection Quality: Poor"));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Connected Players: %d"), GetWorld()->GetNumPlayerControllers());
	UE_LOG(LogTemp, Log, TEXT("Net Mode: %d"), (int32)GetWorld()->GetNetMode());
}

void UNetworkDiagnostics::DumpNetworkStats()
{
	UE_LOG(LogTemp, Warning, TEXT("Dumping network statistics..."));

	UE_LOG(LogTemp, Log, TEXT("=== NETWORK STATISTICS ==="));
	UE_LOG(LogTemp, Log, TEXT("Net Mode: %d"), (int32)GetWorld()->GetNetMode());
	UE_LOG(LogTemp, Log, TEXT("Is Server: %s"), GetWorld()->IsServer() ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogTemp, Log, TEXT("Is Client: %s"), GetWorld()->IsClient() ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogTemp, Log, TEXT("Player Controllers: %d"), GetWorld()->GetNumPlayerControllers());

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && PC->PlayerState)
	{
		UE_LOG(LogTemp, Log, TEXT("Ping: %.0f ms"), PC->PlayerState->GetPingInMilliseconds());
	}

	// Execute engine network commands
	GetWorld()->Exec(GetWorld(), TEXT("stat net"));
}

bool UNetworkDiagnostics::IsNetworked() const
{
	return GetWorld()->GetNetMode() != NM_Standalone;
}
