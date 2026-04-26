// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiplayerSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionSubsystem::UMultiplayerSessionSubsystem()
{
}

void UMultiplayerSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		SessionInterface = OnlineSubsystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMultiplayerSessionSubsystem::OnCreateSessionCompleted);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMultiplayerSessionSubsystem::OnFindSessionsCompleted);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMultiplayerSessionSubsystem::OnJoinSessionCompleted);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMultiplayerSessionSubsystem::OnDestroySessionCompleted);
			SessionInterface->OnStartSessionCompleteDelegates.AddUObject(this, &UMultiplayerSessionSubsystem::OnStartSessionCompleted);

			UE_LOG(LogTemp, Log, TEXT("Multiplayer Session Subsystem initialized"));
		}
	}
}

void UMultiplayerSessionSubsystem::Deinitialize()
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegates(this);
		SessionInterface->ClearOnFindSessionsCompleteDelegates(this);
		SessionInterface->ClearOnJoinSessionCompleteDelegates(this);
		SessionInterface->ClearOnDestroySessionCompleteDelegates(this);
		SessionInterface->ClearOnStartSessionCompleteDelegates(this);
	}

	Super::Deinitialize();
}

void UMultiplayerSessionSubsystem::CreateSession(int32 NumPublicConnections, bool bIsLAN, const FString& SessionName)
{
	if (!SessionInterface.IsValid())
	{
		OnCreateSessionComplete.Broadcast(false);
		return;
	}

	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession)
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = bIsLAN;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->Set(FName("SessionName"), SessionName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		OnCreateSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionSubsystem::FindSessions(int32 MaxSearchResults, bool bIsLAN)
{
	if (!SessionInterface.IsValid())
	{
		OnFindSessionsComplete.Broadcast(false);
		return;
	}

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = bIsLAN;
	LastSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		OnFindSessionsComplete.Broadcast(false);
	}
}

void UMultiplayerSessionSubsystem::JoinSession(int32 SessionIndex)
{
	if (!SessionInterface.IsValid() || !IsValidSessionIndex(SessionIndex))
	{
		OnJoinSessionComplete.Broadcast(false);
		return;
	}

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, LastSessionSearch->SearchResults[SessionIndex]))
	{
		OnJoinSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		OnDestroySessionComplete.Broadcast();
		return;
	}

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		OnDestroySessionComplete.Broadcast();
	}
}

void UMultiplayerSessionSubsystem::StartSession()
{
	if (!SessionInterface.IsValid())
	{
		OnStartSessionComplete.Broadcast();
		return;
	}

	SessionInterface->StartSession(NAME_GameSession);
}

int32 UMultiplayerSessionSubsystem::GetFoundSessionsCount() const
{
	return LastSessionSearch.IsValid() ? LastSessionSearch->SearchResults.Num() : 0;
}

FString UMultiplayerSessionSubsystem::GetSessionName(int32 SessionIndex) const
{
	if (IsValidSessionIndex(SessionIndex))
	{
		FString SessionName;
		LastSessionSearch->SearchResults[SessionIndex].Session.SessionSettings.Get(FName("SessionName"), SessionName);
		return SessionName;
	}
	return FString();
}

int32 UMultiplayerSessionSubsystem::GetSessionPing(int32 SessionIndex) const
{
	return IsValidSessionIndex(SessionIndex) ? LastSessionSearch->SearchResults[SessionIndex].PingInMs : 0;
}

int32 UMultiplayerSessionSubsystem::GetSessionCurrentPlayers(int32 SessionIndex) const
{
	return IsValidSessionIndex(SessionIndex) ? LastSessionSearch->SearchResults[SessionIndex].Session.SessionSettings.NumPublicConnections - LastSessionSearch->SearchResults[SessionIndex].Session.NumOpenPublicConnections : 0;
}

int32 UMultiplayerSessionSubsystem::GetSessionMaxPlayers(int32 SessionIndex) const
{
	return IsValidSessionIndex(SessionIndex) ? LastSessionSearch->SearchResults[SessionIndex].Session.SessionSettings.NumPublicConnections : 0;
}

void UMultiplayerSessionSubsystem::OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("Session created: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
	OnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionSubsystem::OnFindSessionsCompleted(bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("Find sessions: %s - Found %d sessions"),
		bWasSuccessful ? TEXT("Success") : TEXT("Failed"), GetFoundSessionsCount());
	OnFindSessionsComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionSubsystem::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	bool bSuccess = (Result == EOnJoinSessionCompleteResult::Success);
	UE_LOG(LogTemp, Log, TEXT("Join session: %s"), bSuccess ? TEXT("Success") : TEXT("Failed"));

	if (bSuccess && SessionInterface.IsValid())
	{
		FString ConnectInfo;
		if (SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectInfo))
		{
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				PC->ClientTravel(ConnectInfo, TRAVEL_Absolute);
			}
		}
	}

	OnJoinSessionComplete.Broadcast(bSuccess);
}

void UMultiplayerSessionSubsystem::OnDestroySessionCompleted(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("Session destroyed: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
	OnDestroySessionComplete.Broadcast();
}

void UMultiplayerSessionSubsystem::OnStartSessionCompleted(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("Session started: %s"), bWasSuccessful ? TEXT("Success") : TEXT("Failed"));
	OnStartSessionComplete.Broadcast();
}

bool UMultiplayerSessionSubsystem::IsValidSessionIndex(int32 Index) const
{
	return LastSessionSearch.IsValid() && LastSessionSearch->SearchResults.IsValidIndex(Index);
}
