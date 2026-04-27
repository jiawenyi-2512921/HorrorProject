// Copyright Epic Games, Inc. All Rights Reserved.

#include "HorrorPlayerControllerMultiplayer.h"
#include "ReplicatedPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"

namespace
{
	constexpr int32 MaxChatMessageLength = 256;
}

AHorrorPlayerControllerMultiplayer::AHorrorPlayerControllerMultiplayer()
{
	PrimaryActorTick.bCanEverTick = true;

	Ping = 0.0f;
	PacketLoss = 0.0f;
	bIsConnected = false;
	bVoiceChatActive = false;
	bLagCompensationEnabled = true;
	MaxPredictionTime = 0.5f;
	LastNetworkStatsUpdate = 0.0f;
	NetworkStatsUpdateInterval = 1.0f;
	bEnableMotionControls = false;
}

void AHorrorPlayerControllerMultiplayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHorrorPlayerControllerMultiplayer, Ping);
	DOREPLIFETIME(AHorrorPlayerControllerMultiplayer, PacketLoss);
}

void AHorrorPlayerControllerMultiplayer::BeginPlay()
{
	Super::BeginPlay();

	bIsConnected = true;

	if (IsLocalController())
	{
		UE_LOG(LogTemp, Log, TEXT("Local player controller initialized"));
	}
}

void AHorrorPlayerControllerMultiplayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocalController())
	{
		UWorld* World = GetWorld();
		if (!World)
		{
			return;
		}

		const float CurrentTime = World->GetTimeSeconds();
		if (CurrentTime - LastNetworkStatsUpdate >= NetworkStatsUpdateInterval)
		{
			UpdateNetworkStats();
			LastNetworkStatsUpdate = CurrentTime;
		}

		if (bLagCompensationEnabled)
		{
			ApplyLagCompensation(DeltaTime);
		}
	}
}

void AHorrorPlayerControllerMultiplayer::ServerSendInput_Implementation(FVector MovementInput, FRotator ViewRotation, float DeltaTime)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		// Process input on server
		ControlledPawn->AddMovementInput(MovementInput, 1.0f);
		ControlledPawn->SetActorRotation(ViewRotation);
	}
}

bool AHorrorPlayerControllerMultiplayer::ServerSendInput_Validate(FVector MovementInput, FRotator ViewRotation, float DeltaTime)
{
	return DeltaTime > 0.0f && DeltaTime < 1.0f;
}

void AHorrorPlayerControllerMultiplayer::ServerInteract_Implementation(AActor* TargetActor)
{
	if (TargetActor)
	{
		UE_LOG(LogTemp, Log, TEXT("Player interacting with: %s"), *TargetActor->GetName());
		// Implement interaction logic
	}
}

bool AHorrorPlayerControllerMultiplayer::ServerInteract_Validate(AActor* TargetActor)
{
	return TargetActor != nullptr;
}

void AHorrorPlayerControllerMultiplayer::ServerUseItem_Implementation(const FString& ItemName)
{
	if (AReplicatedPlayerState* PS = GetPlayerState<AReplicatedPlayerState>())
	{
		if (PS->HasInventoryItem(ItemName))
		{
			UE_LOG(LogTemp, Log, TEXT("Player using item: %s"), *ItemName);
			// Implement item usage logic
		}
	}
}

bool AHorrorPlayerControllerMultiplayer::ServerUseItem_Validate(const FString& ItemName)
{
	return !ItemName.IsEmpty();
}

void AHorrorPlayerControllerMultiplayer::ServerSendChatMessage_Implementation(const FString& Message)
{
	if (!Message.IsEmpty())
	{
		APlayerState* ChatPlayerState = GetPlayerState<APlayerState>();
		const FString PlayerName = ChatPlayerState ? ChatPlayerState->GetPlayerName() : TEXT("Unknown");
		MulticastReceiveChatMessage(PlayerName, Message);
	}
}

bool AHorrorPlayerControllerMultiplayer::ServerSendChatMessage_Validate(const FString& Message)
{
	return !Message.IsEmpty() && Message.Len() <= MaxChatMessageLength;
}

void AHorrorPlayerControllerMultiplayer::ClientReceiveChatMessage_Implementation(const FString& PlayerName, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("[Chat] %s: %s"), *PlayerName, *Message);
}

void AHorrorPlayerControllerMultiplayer::MulticastReceiveChatMessage_Implementation(const FString& PlayerName, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("[Chat] %s: %s"), *PlayerName, *Message);
}

void AHorrorPlayerControllerMultiplayer::StartVoiceChat()
{
	bVoiceChatActive = true;
	UE_LOG(LogTemp, Log, TEXT("Voice chat started"));
}

void AHorrorPlayerControllerMultiplayer::StopVoiceChat()
{
	bVoiceChatActive = false;
	UE_LOG(LogTemp, Log, TEXT("Voice chat stopped"));
}

void AHorrorPlayerControllerMultiplayer::UpdateNetworkStats()
{
	UpdatePingAndPacketLoss();
}

void AHorrorPlayerControllerMultiplayer::EnableLagCompensation(bool bEnable)
{
	bLagCompensationEnabled = bEnable;
}

void AHorrorPlayerControllerMultiplayer::UpdatePingAndPacketLoss()
{
	if (PlayerState)
	{
		Ping = PlayerState->GetPingInMilliseconds();
	}

	// Calculate packet loss (simplified)
	if (GetNetConnection())
	{
		PacketLoss = 0.0f; // Would need actual network stats
	}
}

void AHorrorPlayerControllerMultiplayer::ApplyLagCompensation(float DeltaTime)
{
	// Implement client-side prediction and lag compensation
	float CompensationTime = FMath::Min(Ping / 1000.0f, MaxPredictionTime);

	// Apply compensation logic here
}
