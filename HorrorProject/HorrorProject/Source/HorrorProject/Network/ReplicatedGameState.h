// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ReplicatedGameState.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	Lobby UMETA(DisplayName = "Lobby"),
	Playing UMETA(DisplayName = "Playing"),
	Victory UMETA(DisplayName = "Victory"),
	Defeat UMETA(DisplayName = "Defeat")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, EGamePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeUpdated, float, RemainingTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStartedEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameEnded, bool, bVictory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveProgressChanged, int32, Completed, int32, Total);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllObjectivesCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerListChanged, const TArray<FString>&, PlayerNames);

/**
 * Defines Replicated Game State behavior for the Network module.
 */
UCLASS()
class HORRORPROJECT_API AReplicatedGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AReplicatedGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Game state
	UPROPERTY(ReplicatedUsing = OnRep_GamePhase, BlueprintReadOnly, Category = "Game State")
	EGamePhase GamePhase;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingTime, BlueprintReadOnly, Category = "Game State")
	float RemainingTime;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	int32 MaxPlayers;

	UPROPERTY(ReplicatedUsing = OnRep_GameStarted, BlueprintReadOnly, Category = "Game State")
	bool bGameStarted;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	bool bGameEnded;

	// Objectives
	UPROPERTY(ReplicatedUsing = OnRep_ObjectivesCompleted, BlueprintReadOnly, Category = "Objectives")
	int32 ObjectivesCompleted;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Objectives")
	int32 TotalObjectives;

	// Players
	UPROPERTY(ReplicatedUsing = OnRep_ConnectedPlayerNames, BlueprintReadOnly, Category = "Players")
	TArray<FString> ConnectedPlayerNames;

	// Game control
	UFUNCTION(BlueprintCallable, Category = "Game State")
	void SetGamePhase(EGamePhase NewPhase);

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void EndGame(bool bVictory);

	UFUNCTION(BlueprintCallable, Category = "Game State")
	void UpdateRemainingTime(float NewTime);

	// Objectives
	UFUNCTION(BlueprintCallable, Category = "Objectives")
	void AddObjectiveProgress(int32 Amount = 1);

	// Player management
	UFUNCTION(BlueprintCallable, Category = "Players")
	void AddPlayerName(const FString& PlayerName);

	UFUNCTION(BlueprintCallable, Category = "Players")
	void RemovePlayerName(const FString& PlayerName);

	UFUNCTION(BlueprintPure, Category = "Players")
	int32 GetConnectedPlayerCount() const;

	UFUNCTION(BlueprintPure, Category = "Game State")
	bool CanStartGame() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGamePhaseChanged OnGamePhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTimeUpdated OnTimeUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGameStartedEvent OnGameStartedEvent;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGameEnded OnGameEnded;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnObjectiveProgressChanged OnObjectiveProgressChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAllObjectivesCompleted OnAllObjectivesCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerListChanged OnPlayerListChanged;

protected:
	UFUNCTION()
	void OnRep_GamePhase();

	UFUNCTION()
	void OnRep_RemainingTime();

	UFUNCTION()
	void OnRep_GameStarted();

	UFUNCTION()
	void OnRep_ObjectivesCompleted();

	UFUNCTION()
	void OnRep_ConnectedPlayerNames();
};
