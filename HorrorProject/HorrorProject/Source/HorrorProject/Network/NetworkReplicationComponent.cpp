// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkReplicationComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

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
		// Server: Update replicated transform at specified rate
		float CurrentTime = GetWorld()->GetTimeSeconds();
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
	if (AActor* Owner = GetOwner())
	{
		// Apply client prediction with server reconciliation
		float ServerTime = GetWorld()->GetTimeSeconds();
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
	return Owner && Owner->GetWorld() && Owner->GetWorld()->GetNetMode() != NM_Standalone;
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
	if (GetWorld())
	{
		float ServerTime = ServerTimestamp;
		float ClientTime = GetWorld()->GetTimeSeconds();
		CurrentPing = FMath::Abs(ClientTime - ServerTime) * 1000.0f; // Convert to ms
	}
}
