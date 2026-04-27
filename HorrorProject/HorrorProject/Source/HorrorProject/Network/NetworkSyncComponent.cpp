// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkSyncComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

namespace
{
	constexpr float BytesPerKilobyte = 1024.0f;
}

UNetworkSyncComponent::UNetworkSyncComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	LastSyncTime = 0.0f;
	CurrentBandwidth = 0.0f;
	BytesSentThisSecond = 0;
	BandwidthResetTimer = 0.0f;
	LastSyncedPosition = FVector::ZeroVector;
	LastSyncedRotation = FRotator::ZeroRotator;
}

void UNetworkSyncComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UNetworkSyncComponent, SyncedVariables);
}

void UNetworkSyncComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		LastSyncedPosition = Owner->GetActorLocation();
		LastSyncedRotation = Owner->GetActorRotation();
	}
}

void UNetworkSyncComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	BandwidthResetTimer += DeltaTime;
	if (BandwidthResetTimer >= 1.0f)
	{
		CurrentBandwidth = BytesSentThisSecond / BytesPerKilobyte; // KB/s
		BytesSentThisSecond = 0;
		BandwidthResetTimer = 0.0f;
	}

	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (Owner && Owner->HasAuthority() && World)
	{
		float CurrentTime = World->GetTimeSeconds();
		if (CurrentTime - LastSyncTime >= SyncInterval)
		{
			if (ShouldSyncTransform())
			{
				LastSyncedPosition = Owner->GetActorLocation();
				LastSyncedRotation = Owner->GetActorRotation();
			}
			LastSyncTime = CurrentTime;
		}
	}
}

void UNetworkSyncComponent::SyncFloat(const FString& VarName, float Value)
{
	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		FSyncedVariable* Var = FindVariable(VarName);
		if (!Var)
		{
			FSyncedVariable NewVar;
			NewVar.VariableName = VarName;
			NewVar.FloatValue = Value;
			SyncedVariables.Add(NewVar);
			UpdateBandwidthTracking(sizeof(FSyncedVariable));
		}
		else if (Var->FloatValue != Value)
		{
			Var->FloatValue = Value;
			UpdateBandwidthTracking(sizeof(float));
		}
	}
}

void UNetworkSyncComponent::SyncInt(const FString& VarName, int32 Value)
{
	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		FSyncedVariable* Var = FindVariable(VarName);
		if (!Var)
		{
			FSyncedVariable NewVar;
			NewVar.VariableName = VarName;
			NewVar.IntValue = Value;
			SyncedVariables.Add(NewVar);
			UpdateBandwidthTracking(sizeof(FSyncedVariable));
		}
		else if (Var->IntValue != Value)
		{
			Var->IntValue = Value;
			UpdateBandwidthTracking(sizeof(int32));
		}
	}
}

void UNetworkSyncComponent::SyncBool(const FString& VarName, bool Value)
{
	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		FSyncedVariable* Var = FindVariable(VarName);
		if (!Var)
		{
			FSyncedVariable NewVar;
			NewVar.VariableName = VarName;
			NewVar.BoolValue = Value;
			SyncedVariables.Add(NewVar);
			UpdateBandwidthTracking(sizeof(FSyncedVariable));
		}
		else if (Var->BoolValue != Value)
		{
			Var->BoolValue = Value;
			UpdateBandwidthTracking(sizeof(bool));
		}
	}
}

void UNetworkSyncComponent::SyncVector(const FString& VarName, FVector Value)
{
	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		FSyncedVariable* Var = FindVariable(VarName);
		if (!Var)
		{
			FSyncedVariable NewVar;
			NewVar.VariableName = VarName;
			NewVar.VectorValue = Value;
			SyncedVariables.Add(NewVar);
			UpdateBandwidthTracking(sizeof(FSyncedVariable));
		}
		else if (!Var->VectorValue.Equals(Value, PositionTolerance))
		{
			Var->VectorValue = Value;
			UpdateBandwidthTracking(sizeof(FVector));
		}
	}
}

float UNetworkSyncComponent::GetSyncedFloat(const FString& VarName) const
{
	const FSyncedVariable* Var = FindVariable(VarName);
	return Var ? Var->FloatValue : 0.0f;
}

int32 UNetworkSyncComponent::GetSyncedInt(const FString& VarName) const
{
	const FSyncedVariable* Var = FindVariable(VarName);
	return Var ? Var->IntValue : 0;
}

bool UNetworkSyncComponent::GetSyncedBool(const FString& VarName) const
{
	const FSyncedVariable* Var = FindVariable(VarName);
	return Var ? Var->BoolValue : false;
}

FVector UNetworkSyncComponent::GetSyncedVector(const FString& VarName) const
{
	const FSyncedVariable* Var = FindVariable(VarName);
	return Var ? Var->VectorValue : FVector::ZeroVector;
}

void UNetworkSyncComponent::ServerSyncVariable_Implementation(const FSyncedVariable& Variable)
{
	MulticastSyncVariable(Variable);
}

bool UNetworkSyncComponent::ServerSyncVariable_Validate(const FSyncedVariable& Variable)
{
	return true;
}

void UNetworkSyncComponent::MulticastSyncVariable_Implementation(const FSyncedVariable& Variable)
{
	FSyncedVariable* Var = FindVariable(Variable.VariableName);
	if (Var)
	{
		*Var = Variable;
	}
	else
	{
		SyncedVariables.Add(Variable);
	}
}

void UNetworkSyncComponent::ResetBandwidthCounter()
{
	BytesSentThisSecond = 0;
	CurrentBandwidth = 0.0f;
}

FSyncedVariable* UNetworkSyncComponent::FindVariable(const FString& VarName)
{
	for (FSyncedVariable& Var : SyncedVariables)
	{
		if (Var.VariableName == VarName)
		{
			return &Var;
		}
	}
	return nullptr;
}

const FSyncedVariable* UNetworkSyncComponent::FindVariable(const FString& VarName) const
{
	for (const FSyncedVariable& Var : SyncedVariables)
	{
		if (Var.VariableName == VarName)
		{
			return &Var;
		}
	}
	return nullptr;
}

bool UNetworkSyncComponent::ShouldSyncTransform() const
{
	const AActor* Owner = GetOwner();
	if (!bEnableDeltaCompression || !Owner)
	{
		return true;
	}

	FVector CurrentPos = Owner->GetActorLocation();
	FRotator CurrentRot = Owner->GetActorRotation();

	float PosDelta = FVector::Dist(CurrentPos, LastSyncedPosition);
	float RotDelta = FMath::Abs((CurrentRot - LastSyncedRotation).Yaw);

	return PosDelta > PositionTolerance || RotDelta > RotationTolerance;
}

void UNetworkSyncComponent::UpdateBandwidthTracking(int32 BytesSent)
{
	BytesSentThisSecond += BytesSent;
}
