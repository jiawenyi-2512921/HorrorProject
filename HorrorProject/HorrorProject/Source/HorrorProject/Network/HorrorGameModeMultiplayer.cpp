// Copyright Epic Games, Inc. All Rights Reserved.

#include "HorrorGameModeMultiplayer.h"
#include "ReplicatedGameState.h"
#include "ReplicatedPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AHorrorGameModeMultiplayer::AHorrorGameModeMultiplayer()
{
	PrimaryActorTick.bCanEverTick = true;

	LobbyTimer = 0.0f;
	GameTimer = 0.0f;
	bGameInProgress = false;

	GameStateClass = AReplicatedGameState::StaticClass();
	PlayerStateClass = AReplicatedPlayerState::StaticClass();
}

void AHorrorGameModeMultiplayer::BeginPlay()
{
	Super::BeginPlay();

	if (AReplicatedGameState* GS = GetGameState<AReplicatedGameState>())
	{
		GS->MaxPlayers = MaxPlayers;
		GS->TotalObjectives = 5; // Default objectives
		GS->SetGamePhase(EGamePhase::Lobby);
	}
}

void AHorrorGameModeMultiplayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bGameInProgress)
	{
		UpdateLobby(DeltaTime);
	}
	else
	{
		UpdateGameTimer(DeltaTime);
	}
}

void AHorrorGameModeMultiplayer::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer)
	{
		ConnectedPlayers.Add(NewPlayer);

		if (AReplicatedGameState* GS = GetGameState<AReplicatedGameState>())
		{
			FString PlayerName = NewPlayer->GetPlayerState<APlayerState>()->GetPlayerName();
			GS->AddPlayerName(PlayerName);

			UE_LOG(LogTemp, Log, TEXT("Player joined: %s (%d/%d)"),
				*PlayerName, GetConnectedPlayerCount(), MaxPlayers);
		}

		// Auto-start if conditions met
		if (bAutoStartGame && CanStartGame())
		{
			LobbyTimer = LobbyWaitTime;
		}
	}
}

void AHorrorGameModeMultiplayer::Logout(AController* Exiting)
{
	if (APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		ConnectedPlayers.Remove(PC);

		if (AReplicatedGameState* GS = GetGameState<AReplicatedGameState>())
		{
			FString PlayerName = PC->GetPlayerState<APlayerState>()->GetPlayerName();
			GS->RemovePlayerName(PlayerName);

			UE_LOG(LogTemp, Log, TEXT("Player left: %s (%d/%d)"),
				*PlayerName, GetConnectedPlayerCount(), MaxPlayers);
		}
	}

	Super::Logout(Exiting);
}

void AHorrorGameModeMultiplayer::StartMultiplayerGame()
{
	if (!bGameInProgress && CanStartGame())
	{
		bGameInProgress = true;
		GameTimer = GameDuration;

		if (AReplicatedGameState* GS = GetGameState<AReplicatedGameState>())
		{
			GS->StartGame();
			GS->UpdateRemainingTime(GameTimer);
		}

		AssignPlayerTeams();
		SpawnPlayersAtStartPoints();
		HandleMatchHasStarted();

		UE_LOG(LogTemp, Log, TEXT("Multiplayer game started with %d players"), GetConnectedPlayerCount());
	}
}

void AHorrorGameModeMultiplayer::EndMultiplayerGame(bool bVictory)
{
	if (bGameInProgress)
	{
		bGameInProgress = false;

		if (AReplicatedGameState* GS = GetGameState<AReplicatedGameState>())
		{
			GS->EndGame(bVictory);
		}

		HandleMatchHasEnded();

		UE_LOG(LogTemp, Log, TEXT("Multiplayer game ended - Victory: %s"), bVictory ? TEXT("Yes") : TEXT("No"));
	}
}

void AHorrorGameModeMultiplayer::RestartMultiplayerGame()
{
	EndMultiplayerGame(false);

	FTimerHandle RestartTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		RestartTimerHandle,
		[this]()
		{
			if (AReplicatedGameState* GS = GetGameState<AReplicatedGameState>())
			{
				GS->SetGamePhase(EGamePhase::Lobby);
			}
			LobbyTimer = 0.0f;
		},
		3.0f,
		false
	);
}

int32 AHorrorGameModeMultiplayer::GetConnectedPlayerCount() const
{
	return ConnectedPlayers.Num();
}

bool AHorrorGameModeMultiplayer::CanStartGame() const
{
	int32 PlayerCount = GetConnectedPlayerCount();
	return !bGameInProgress && PlayerCount >= MinPlayers && PlayerCount <= MaxPlayers;
}

void AHorrorGameModeMultiplayer::KickPlayer(APlayerController* PlayerToKick, const FString& Reason)
{
	if (PlayerToKick && HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Kicking player: %s - Reason: %s"),
			*PlayerToKick->GetPlayerState<APlayerState>()->GetPlayerName(), *Reason);

		PlayerToKick->ClientReturnToMainMenuWithTextReason(FText::FromString(Reason));
	}
}

void AHorrorGameModeMultiplayer::HandleMatchHasStarted()
{
	UE_LOG(LogTemp, Verbose, TEXT("Multiplayer match state entered Started"));
}

void AHorrorGameModeMultiplayer::HandleMatchHasEnded()
{
	UE_LOG(LogTemp, Verbose, TEXT("Multiplayer match state entered Ended"));
}

void AHorrorGameModeMultiplayer::UpdateLobby(float DeltaTime)
{
	if (bAutoStartGame && CanStartGame())
	{
		LobbyTimer += DeltaTime;

		if (LobbyTimer >= LobbyWaitTime)
		{
			StartMultiplayerGame();
		}
	}
	else
	{
		LobbyTimer = 0.0f;
	}
}

void AHorrorGameModeMultiplayer::UpdateGameTimer(float DeltaTime)
{
	GameTimer = FMath::Max(0.0f, GameTimer - DeltaTime);

	if (AReplicatedGameState* GS = GetGameState<AReplicatedGameState>())
	{
		GS->UpdateRemainingTime(GameTimer);
	}

	if (GameTimer <= 0.0f)
	{
		EndMultiplayerGame(false); // Time's up = defeat
	}
}

void AHorrorGameModeMultiplayer::AssignPlayerTeams()
{
	int32 TeamIndex = 0;
	for (APlayerController* PC : ConnectedPlayers)
	{
		if (AReplicatedPlayerState* PS = PC->GetPlayerState<AReplicatedPlayerState>())
		{
			PS->TeamId = TeamIndex++;
		}
	}
}

void AHorrorGameModeMultiplayer::SpawnPlayersAtStartPoints()
{
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

	int32 StartIndex = 0;
	for (APlayerController* PC : ConnectedPlayers)
	{
		if (PlayerStarts.IsValidIndex(StartIndex))
		{
			AActor* StartPoint = PlayerStarts[StartIndex];
			if (APawn* Pawn = PC->GetPawn())
			{
				Pawn->SetActorLocation(StartPoint->GetActorLocation());
				Pawn->SetActorRotation(StartPoint->GetActorRotation());
			}
			StartIndex = (StartIndex + 1) % PlayerStarts.Num();
		}
	}
}
