// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkReplicationComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

namespace
{
	constexpr float MaxReplicatedCoordinate = 1000000.0f;
}

UNetworkReplicationComponent::UNetworkReplicationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	LastReplicationTime = 0.0f;
	CurrentPing = 0.0f;
}

void UNetworkReplicationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UNetworkReplicationComponent, ReplicatedTransform);
	DOREPLIFETIME(UNetworkReplicationComponent, ServerTimestamp);
}

void UNetworkReplicationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		TargetLocation = Owner->GetActorLocation();
		TargetRotation = Owner->GetActorRotation();
	}
}

void UNetworkReplicationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (!Owner) return;

	if (Owner->HasAuthority())
	{
		UWorld* World = GetWorld();
		if (!World)
		{
			return;
		}

		// Server: Update replicated transform at specified rate
		float CurrentTime = World->GetTimeSeconds();
		if (CurrentTime - LastReplicationTime >= 1.0f / ReplicationRate)
		{
			ReplicatedTransform.Location = Owner->GetActorLocation();
			ReplicatedTransform.Rotation = Owner->GetActorRotation();
			ReplicatedTransform.Timestamp = CurrentTime;
			ServerTimestamp = CurrentTime;
			LastReplicationTime = CurrentTime;
		}
	}
	else
	{
		// Client: Interpolate to target transform
		UpdateInterpolation(DeltaTime);
		CalculatePing();
	}
}

void UNetworkReplicationComponent::ServerSendTransform_Implementation(FVector Location, FRotator Rotation, float ClientTimestamp)
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (Owner && World)
	{
		// Apply client prediction with server reconciliation
		float ServerTime = World->GetTimeSeconds();
		float Latency = ServerTime - ClientTimestamp;

		if (bEnableClientPrediction && Latency < 0.5f)
		{
			Owner->SetActorLocation(Location);
			Owner->SetActorRotation(Rotation);
		}
	}
}

bool UNetworkReplicationComponent::ServerSendTransform_Validate(FVector Location, FRotator Rotation, float ClientTimestamp)
{
	// Security: Validate transform data
	if (FMath::IsNaN(Location.X) || FMath::IsNaN(Location.Y) || FMath::IsNaN(Location.Z))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid location received: NaN values"));
		return false;
	}

	if (FMath::Abs(Location.X) > MaxReplicatedCoordinate
		|| FMath::Abs(Location.Y) > MaxReplicatedCoordinate
		|| FMath::Abs(Location.Z) > MaxReplicatedCoordinate)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid location received: out of bounds"));
		return false;
	}

	if (FMath::IsNaN(Rotation.Pitch) || FMath::IsNaN(Rotation.Yaw) || FMath::IsNaN(Rotation.Roll))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid rotation received: NaN values"));
		return false;
	}

	// Security: Validate timestamp
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid timestamp: world unavailable"));
		return false;
	}

	float ServerTime = World->GetTimeSeconds();
	float Latency = FMath::Abs(ServerTime - ClientTimestamp);
	if (Latency > 5.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid timestamp: latency %.2f exceeds maximum"), Latency);
		return false;
	}

	return true;
}

void UNetworkReplicationComponent::MulticastBroadcastEvent_Implementation(const FString& EventName, const TArray<uint8>& EventData)
{
	// Handle replicated events
	UE_LOG(LogTemp, Log, TEXT("Network Event: %s"), *EventName);
}

void UNetworkReplicationComponent::SetTargetTransform(FVector Location, FRotator Rotation)
{
	TargetLocation = Location;
	TargetRotation = Rotation;
}

bool UNetworkReplicationComponent::IsNetworkReady() const
{
	AActor* Owner = GetOwner();
	UWorld* World = Owner ? Owner->GetWorld() : nullptr;
	return World && World->GetNetMode() != NM_Standalone;
}

void UNetworkReplicationComponent::OnRep_ReplicatedTransform()
{
	SetTargetTransform(ReplicatedTransform.Location, ReplicatedTransform.Rotation);
}

void UNetworkReplicationComponent::UpdateInterpolation(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector CurrentLocation = Owner->GetActorLocation();
	FRotator CurrentRotation = Owner->GetActorRotation();

	FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, InterpolationSpeed);
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, InterpolationSpeed);

	Owner->SetActorLocation(NewLocation);
	Owner->SetActorRotation(NewRotation);
}

void UNetworkReplicationComponent::CalculatePing()
{
	if (UWorld* World = GetWorld())
	{
		float ServerTime = ServerTimestamp;
		float ClientTime = World->GetTimeSeconds();
		CurrentPing = FMath::Abs(ClientTime - ServerTime) * 1000.0f; // Convert to ms
	}
}
