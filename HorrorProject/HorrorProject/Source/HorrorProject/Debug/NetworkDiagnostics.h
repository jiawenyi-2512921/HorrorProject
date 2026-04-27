// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NetworkDiagnostics.generated.h"

namespace HorrorNetworkDiagnosticsDefaults
{
	inline constexpr float HighPingThresholdMs = 150.0f;
}

USTRUCT(BlueprintType)
struct FNetworkSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly)
	float PingMS = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float PacketLossPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 BytesSent = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 BytesReceived = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 ConnectedPlayers = 0;
};

USTRUCT(BlueprintType)
struct FNetworkStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	float AveragePing = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float MinPing = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float MaxPing = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float AveragePacketLoss = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalBytesSent = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalBytesReceived = 0;
};

/**
 * Network diagnostics and monitoring
 */
UCLASS(BlueprintType)
class HORRORPROJECT_API UNetworkDiagnostics : public UObject
{
	GENERATED_BODY()

public:
	UNetworkDiagnostics();

	UFUNCTION(BlueprintCallable, Category = "Network")
	void StartMonitoring(float Interval = 2.0f);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void StopMonitoring();

	UFUNCTION(BlueprintCallable, Category = "Network")
	FNetworkStats GetCurrentStats() const;

	UFUNCTION(BlueprintCallable, Category = "Network")
	void GenerateNetworkReport(const FString& FilePath = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Network")
	void ClearHistory();

	UFUNCTION(BlueprintCallable, Category = "Network")
	void TestConnection();

	UFUNCTION(BlueprintCallable, Category = "Network")
	void DumpNetworkStats();

	UFUNCTION(BlueprintPure, Category = "Network")
	bool IsMonitoring() const { return bIsMonitoring; }

	UFUNCTION(BlueprintPure, Category = "Network")
	bool IsNetworked() const;

	UFUNCTION(BlueprintPure, Category = "Network")
	TArray<FNetworkSnapshot> GetNetworkHistory() const { return NetworkHistory; }

protected:
	void CollectNetworkData();
	void CheckNetworkThresholds(const FNetworkSnapshot& Snapshot);

	UPROPERTY()
	TArray<FNetworkSnapshot> NetworkHistory;

	UPROPERTY()
	bool bIsMonitoring;

	UPROPERTY()
	float MonitoringInterval;

	FTimerHandle MonitoringTimer;

	// Thresholds
	UPROPERTY(EditDefaultsOnly, Category = "Network")
	float HighPingThreshold = HorrorNetworkDiagnosticsDefaults::HighPingThresholdMs;

	UPROPERTY(EditDefaultsOnly, Category = "Network")
	float PacketLossThreshold = 5.0f;
};
