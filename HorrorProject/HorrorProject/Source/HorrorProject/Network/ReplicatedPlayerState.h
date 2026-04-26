// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ReplicatedPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float, NewStamina, float, MaxStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFearChanged, float, NewFear);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHidingStateChanged, bool, bIsHiding);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryChanged, const TArray<FString>&, Items);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeathEvent);

UCLASS()
class HORRORPROJECT_API AReplicatedPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AReplicatedPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Player stats
	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Player Stats")
	float Health;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player Stats")
	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_Stamina, BlueprintReadOnly, Category = "Player Stats")
	float Stamina;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player Stats")
	float MaxStamina;

	UPROPERTY(ReplicatedUsing = OnRep_Fear, BlueprintReadOnly, Category = "Player Stats")
	float Fear;

	UPROPERTY(ReplicatedUsing = OnRep_IsAlive, BlueprintReadOnly, Category = "Player Stats")
	bool bIsAlive;

	UPROPERTY(ReplicatedUsing = OnRep_IsHiding, BlueprintReadOnly, Category = "Player State")
	bool bIsHiding;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Team")
	int32 TeamId;

	UPROPERTY(ReplicatedUsing = OnRep_InventoryItems, BlueprintReadOnly, Category = "Inventory")
	TArray<FString> InventoryItems;

	// Stat modification
	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void SetHealth(float NewHealth);

	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void SetStamina(float NewStamina);

	UFUNCTION(BlueprintCallable, Category = "Player Stats")
	void SetFear(float NewFear);

	// Inventory
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddInventoryItem(const FString& ItemName);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveInventoryItem(const FString& ItemName);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasInventoryItem(const FString& ItemName) const;

	// Server RPCs
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetHiding(bool bHiding);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStaminaChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFearChanged OnFearChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHidingStateChanged OnHidingStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnInventoryChanged OnInventoryChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDeathEvent OnPlayerDeathEvent;

protected:
	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void OnRep_Stamina();

	UFUNCTION()
	void OnRep_Fear();

	UFUNCTION()
	void OnRep_IsAlive();

	UFUNCTION()
	void OnRep_IsHiding();

	UFUNCTION()
	void OnRep_InventoryItems();

	void OnPlayerDied();
};
