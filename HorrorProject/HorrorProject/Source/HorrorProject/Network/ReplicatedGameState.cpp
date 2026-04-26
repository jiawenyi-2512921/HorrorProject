// Copyright Epic Games, Inc. All Rights Reserved.

#include "ReplicatedGameState.h"
#include "Net/UnrealNetwork.h"

AReplicatedGameState::AReplicatedGameState()
{
	GamePhase = EGamePhase::Lobby;
	RemainingTime = 0.0f;
	MaxPlayers = 4;
	bGameStarted = false;
	bGameEnded = false;
}

void AReplicatedGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AReplicatedGameState, GamePhase);
	DOREPLIFETIME(AReplicatedGameState, RemainingTime);
	DOREPLIFETIME(AReplicatedGameState, MaxPlayers);
	DOREPLIFETIME(AReplicatedGameState, bGameStarted);
	DOREPLIFETIME(AReplicatedGameState, bGameEnded);
	DOREPLIFETIME(AReplicatedGameState, ObjectivesCompleted);
	DOREPLIFETIME(AReplicatedGameState, TotalObjectives);
	DOREPLIFETIME(AReplicatedGameState, ConnectedPlayerNames);
}

void AReplicatedGameState::SetGamePhase(EGamePhase NewPhase)
{
	if (HasAuthority())
	{
		GamePhase = NewPhase;
		OnRep_GamePhase();
	}
}

void AReplicatedGameState::StartGame()
{
	if (HasAuthority() && !bGameStarted)
	{
		bGameStarted = true;
		SetGamePhase(EGamePhase::Playing);
		OnRep_GameStarted();
	}
}

void AReplicatedGameState::EndGame(bool bVictory)
{
	if (HasAuthority() && !bGameEnded)
	{
		bGameEnded = true;
		SetGamePhase(bVictory ? EGamePhase::Victory : EGamePhase::Defeat);
		OnGameEnded.Broadcast(bVictory);
	}
}

void AReplicatedGameState::UpdateRemainingTime(float NewTime)
{
	if (HasAuthority())
	{
		RemainingTime = FMath::Max(0.0f, NewTime);
		OnRep_RemainingTime();
	}
}

void AReplicatedGameState::AddObjectiveProgress(int32 Amount)
{
	if (HasAuthority())
	{
		ObjectivesCompleted = FMath::Clamp(ObjectivesCompleted + Amount, 0, TotalObjectives);
		OnRep_ObjectivesCompleted();

		if (ObjectivesCompleted >= TotalObjectives)
		{
			OnAllObjectivesCompleted.Broadcast();
		}
	}
}

void AReplicatedGameState::AddPlayerName(const FString& PlayerName)
{
	if (HasAuthority())
	{
		ConnectedPlayerNames.AddUnique(PlayerName);
		OnRep_ConnectedPlayerNames();
	}
}

void AReplicatedGameState::RemovePlayerName(const FString& PlayerName)
{
	if (HasAuthority())
	{
		ConnectedPlayerNames.Remove(PlayerName);
		OnRep_ConnectedPlayerNames();
	}
}

int32 AReplicatedGameState::GetConnectedPlayerCount() const
{
	return ConnectedPlayerNames.Num();
}

bool AReplicatedGameState::CanStartGame() const
{
	return !bGameStarted && GetConnectedPlayerCount() >= 2 && GetConnectedPlayerCount() <= MaxPlayers;
}

void AReplicatedGameState::OnRep_GamePhase()
{
	OnGamePhaseChanged.Broadcast(GamePhase);
}

void AReplicatedGameState::OnRep_RemainingTime()
{
	OnTimeUpdated.Broadcast(RemainingTime);
}

void AReplicatedGameState::OnRep_GameStarted()
{
	if (bGameStarted)
	{
		OnGameStartedEvent.Broadcast();
	}
}

void AReplicatedGameState::OnRep_ObjectivesCompleted()
{
	OnObjectiveProgressChanged.Broadcast(ObjectivesCompleted, TotalObjectives);
}

void AReplicatedGameState::OnRep_ConnectedPlayerNames()
{
	OnPlayerListChanged.Broadcast(ConnectedPlayerNames);
}
