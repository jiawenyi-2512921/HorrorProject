// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HorrorGameModeMultiplayer.generated.h"

UCLASS()
class HORRORPROJECT_API AHorrorGameModeMultiplayer : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHorrorGameModeMultiplayer();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Game settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiplayer")
	int32 MinPlayers = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiplayer")
	int32 MaxPlayers = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiplayer")
	float LobbyWaitTime = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiplayer")
	float GameDuration = 1800.0f; // 30 minutes

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multiplayer")
	bool bAutoStartGame = true;

	// Game control
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void StartMultiplayerGame();

	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void EndMultiplayerGame(bool bVictory);

	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void RestartMultiplayerGame();

	UFUNCTION(BlueprintPure, Category = "Multiplayer")
	int32 GetConnectedPlayerCount() const;

	UFUNCTION(BlueprintPure, Category = "Multiplayer")
	bool CanStartGame() const;

	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void KickPlayer(APlayerController* PlayerToKick, const FString& Reason);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Multiplayer")
	TArray<APlayerController*> ConnectedPlayers;

	UPROPERTY(BlueprintReadOnly, Category = "Multiplayer")
	float LobbyTimer;

	UPROPERTY(BlueprintReadOnly, Category = "Multiplayer")
	float GameTimer;

	UPROPERTY(BlueprintReadOnly, Category = "Multiplayer")
	bool bGameInProgress;

	void HandleMatchHasStarted();
	void HandleMatchHasEnded();

	void UpdateLobby(float DeltaTime);
	void UpdateGameTimer(float DeltaTime);
	void AssignPlayerTeams();
	void SpawnPlayersAtStartPoints();
};
