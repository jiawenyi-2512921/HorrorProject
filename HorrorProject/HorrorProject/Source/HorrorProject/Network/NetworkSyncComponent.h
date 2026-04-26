// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NetworkSyncComponent.generated.h"

USTRUCT(BlueprintType)
struct FSyncedVariable
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString VariableName;

	UPROPERTY(BlueprintReadWrite)
	float FloatValue;

	UPROPERTY(BlueprintReadWrite)
	int32 IntValue;

	UPROPERTY(BlueprintReadWrite)
	bool BoolValue;

	UPROPERTY(BlueprintReadWrite)
	FVector VectorValue;

	FSyncedVariable()
		: FloatValue(0.0f)
		, IntValue(0)
		, BoolValue(false)
		, VectorValue(FVector::ZeroVector)
	{}
};

UCLASS(ClassGroup=(Network), meta=(BlueprintSpawnableComponent))
class HORRORPROJECT_API UNetworkSyncComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNetworkSyncComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Sync settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sync")
	float SyncInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sync")
	bool bEnableDeltaCompression = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sync")
	float PositionTolerance = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sync")
	float RotationTolerance = 1.0f;

	// Synced variables
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Sync")
	TArray<FSyncedVariable> SyncedVariables;

	// Sync methods
	UFUNCTION(BlueprintCallable, Category = "Sync")
	void SyncFloat(const FString& VarName, float Value);

	UFUNCTION(BlueprintCallable, Category = "Sync")
	void SyncInt(const FString& VarName, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Sync")
	void SyncBool(const FString& VarName, bool Value);

	UFUNCTION(BlueprintCallable, Category = "Sync")
	void SyncVector(const FString& VarName, FVector Value);

	UFUNCTION(BlueprintPure, Category = "Sync")
	float GetSyncedFloat(const FString& VarName) const;

	UFUNCTION(BlueprintPure, Category = "Sync")
	int32 GetSyncedInt(const FString& VarName) const;

	UFUNCTION(BlueprintPure, Category = "Sync")
	bool GetSyncedBool(const FString& VarName) const;

	UFUNCTION(BlueprintPure, Category = "Sync")
	FVector GetSyncedVector(const FString& VarName) const;

	// Server RPCs
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSyncVariable(const FSyncedVariable& Variable);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSyncVariable(const FSyncedVariable& Variable);

	// Bandwidth optimization
	UFUNCTION(BlueprintPure, Category = "Sync")
	float GetBandwidthUsage() const { return CurrentBandwidth; }

	UFUNCTION(BlueprintCallable, Category = "Sync")
	void ResetBandwidthCounter();

protected:
	virtual void BeginPlay() override;

private:
	float LastSyncTime;
	float CurrentBandwidth;
	int32 BytesSentThisSecond;
	float BandwidthResetTimer;

	FVector LastSyncedPosition;
	FRotator LastSyncedRotation;

	FSyncedVariable* FindVariable(const FString& VarName);
	const FSyncedVariable* FindVariable(const FString& VarName) const;
	bool ShouldSyncTransform() const;
	void UpdateBandwidthTracking(int32 BytesSent);
};
