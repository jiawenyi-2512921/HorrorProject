// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HorrorPlayerControllerMultiplayer.generated.h"

UCLASS()
class HORRORPROJECT_API AHorrorPlayerControllerMultiplayer : public APlayerController
{
	GENERATED_BODY()

public:
	AHorrorPlayerControllerMultiplayer();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Network info
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Network")
	float Ping;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Network")
	float PacketLoss;

	UPROPERTY(BlueprintReadOnly, Category = "Network")
	bool bIsConnected;

	// Input replication
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSendInput(FVector MovementInput, FRotator ViewRotation, float DeltaTime);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerInteract(AActor* TargetActor);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUseItem(const FString& ItemName);

	// Chat system
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSendChatMessage(const FString& Message);

	UFUNCTION(Client, Reliable)
	void ClientReceiveChatMessage(const FString& PlayerName, const FString& Message);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastReceiveChatMessage(const FString& PlayerName, const FString& Message);

	// Voice chat
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void StartVoiceChat();

	UFUNCTION(BlueprintCallable, Category = "Voice")
	void StopVoiceChat();

	UFUNCTION(BlueprintPure, Category = "Voice")
	bool IsVoiceChatActive() const { return bVoiceChatActive; }

	// Network diagnostics
	UFUNCTION(BlueprintCallable, Category = "Network")
	void UpdateNetworkStats();

	UFUNCTION(BlueprintPure, Category = "Network")
	float GetCurrentPing() const { return Ping; }

	UFUNCTION(BlueprintPure, Category = "Network")
	float GetPacketLoss() const { return PacketLoss; }

	// Lag compensation
	UFUNCTION(BlueprintCallable, Category = "Network")
	void EnableLagCompensation(bool bEnable);

	UFUNCTION(BlueprintPure, Category = "Network")
	bool IsLagCompensationEnabled() const { return bLagCompensationEnabled; }

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Voice")
	bool bVoiceChatActive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	bool bLagCompensationEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	float MaxPredictionTime;

	float LastNetworkStatsUpdate;
	float NetworkStatsUpdateInterval;

	void UpdatePingAndPacketLoss();
	void ApplyLagCompensation(float DeltaTime);
};
