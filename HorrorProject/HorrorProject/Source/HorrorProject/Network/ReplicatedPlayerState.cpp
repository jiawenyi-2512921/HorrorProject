// Copyright Epic Games, Inc. All Rights Reserved.

#include "ReplicatedPlayerState.h"
#include "Net/UnrealNetwork.h"

AReplicatedPlayerState::AReplicatedPlayerState()
{
	SetNetUpdateFrequency(10.0f);
	SetMinNetUpdateFrequency(2.0f);

	Health = 100.0f;
	MaxHealth = 100.0f;
	Stamina = 100.0f;
	MaxStamina = 100.0f;
	Fear = 0.0f;
	bIsAlive = true;
	bIsHiding = false;
	TeamId = 0;
}

void AReplicatedPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AReplicatedPlayerState, Health);
	DOREPLIFETIME(AReplicatedPlayerState, MaxHealth);
	DOREPLIFETIME(AReplicatedPlayerState, Stamina);
	DOREPLIFETIME(AReplicatedPlayerState, MaxStamina);
	DOREPLIFETIME(AReplicatedPlayerState, Fear);
	DOREPLIFETIME(AReplicatedPlayerState, bIsAlive);
	DOREPLIFETIME(AReplicatedPlayerState, bIsHiding);
	DOREPLIFETIME(AReplicatedPlayerState, TeamId);
	DOREPLIFETIME(AReplicatedPlayerState, InventoryItems);
}

void AReplicatedPlayerState::SetHealth(float NewHealth)
{
	if (HasAuthority())
	{
		float OldHealth = Health;
		Health = FMath::Clamp(NewHealth, 0.0f, MaxHealth);

		if (Health <= 0.0f && bIsAlive)
		{
			bIsAlive = false;
			OnPlayerDied();
		}

		OnRep_Health();
	}
}

void AReplicatedPlayerState::SetStamina(float NewStamina)
{
	if (HasAuthority())
	{
		Stamina = FMath::Clamp(NewStamina, 0.0f, MaxStamina);
		OnRep_Stamina();
	}
}

void AReplicatedPlayerState::SetFear(float NewFear)
{
	if (HasAuthority())
	{
		Fear = FMath::Clamp(NewFear, 0.0f, 100.0f);
		OnRep_Fear();
	}
}

void AReplicatedPlayerState::AddInventoryItem(const FString& ItemName)
{
	if (HasAuthority())
	{
		InventoryItems.Add(ItemName);
		OnRep_InventoryItems();
	}
}

void AReplicatedPlayerState::RemoveInventoryItem(const FString& ItemName)
{
	if (HasAuthority())
	{
		InventoryItems.Remove(ItemName);
		OnRep_InventoryItems();
	}
}

bool AReplicatedPlayerState::HasInventoryItem(const FString& ItemName) const
{
	return InventoryItems.Contains(ItemName);
}

void AReplicatedPlayerState::ServerSetHiding_Implementation(bool bHiding)
{
	bIsHiding = bHiding;
	OnRep_IsHiding();
}

bool AReplicatedPlayerState::ServerSetHiding_Validate(bool bHiding)
{
	return true;
}

void AReplicatedPlayerState::OnRep_Health()
{
	OnHealthChanged.Broadcast(Health, MaxHealth);
}

void AReplicatedPlayerState::OnRep_Stamina()
{
	OnStaminaChanged.Broadcast(Stamina, MaxStamina);
}

void AReplicatedPlayerState::OnRep_Fear()
{
	OnFearChanged.Broadcast(Fear);
}

void AReplicatedPlayerState::OnRep_IsAlive()
{
	if (!bIsAlive)
	{
		OnPlayerDied();
	}
}

void AReplicatedPlayerState::OnRep_IsHiding()
{
	OnHidingStateChanged.Broadcast(bIsHiding);
}

void AReplicatedPlayerState::OnRep_InventoryItems()
{
	OnInventoryChanged.Broadcast(InventoryItems);
}

void AReplicatedPlayerState::OnPlayerDied()
{
	OnPlayerDeathEvent.Broadcast();
}
