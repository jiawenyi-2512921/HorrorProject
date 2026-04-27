// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "NetworkReplicationComponent.generated.h"

USTRUCT(BlueprintType)
struct FReplicatedTransform
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FVector Location;

	UPROPERTY(BlueprintReadOnly)
	FRotator Rotation;

	UPROPERTY(BlueprintReadOnly)
	float Timestamp;

	FReplicatedTransform()
		: Location(FVector::ZeroVector)
		, Rotation(FRotator::ZeroRotator)
		, Timestamp(0.0f)
	{}
};

/**
 * Adds Network Replication Component behavior to its owning actor in the Network module.
 */
UCLASS(ClassGroup=(Network), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UNetworkReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	static constexpr float DefaultReplicationRate = 30.0f;

	UNetworkReplicationComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Replication settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	float ReplicationRate = DefaultReplicationRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	float InterpolationSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Network")
	bool bEnableClientPrediction = true;

	// Replicated data
	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedTransform)
	FReplicatedTransform ReplicatedTransform;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Network")
	float ServerTimestamp;

	// Client prediction
	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerSendTransform(FVector Location, FRotator Rotation, float ClientTimestamp);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBroadcastEvent(const FString& EventName, const TArray<uint8>& EventData);

	// Interpolation
	UFUNCTION(BlueprintCallable, Category = "Network")
	void SetTargetTransform(FVector Location, FRotator Rotation);

	UFUNCTION(BlueprintPure, Category = "Network")
	float GetPing() const { return CurrentPing; }

	UFUNCTION(BlueprintPure, Category = "Network")
	bool IsNetworkReady() const;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_ReplicatedTransform();

private:
	FVector TargetLocation;
	FRotator TargetRotation;
	float LastReplicationTime;
	float CurrentPing;

	void UpdateInterpolation(float DeltaTime);
	void CalculatePing();
};
