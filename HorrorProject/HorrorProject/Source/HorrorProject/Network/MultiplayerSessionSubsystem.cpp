// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiplayerSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

namespace
{
constexpr int32 MaxPublicSessionConnections = 64;
constexpr int32 MaxSessionNameLength = 64;

const ULocalPlayer* GetSessionLocalPlayer(const UObject* WorldContext)
{
	if (!WorldContext)
	{
		return nullptr;
	}

	const UWorld* World = WorldContext->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	return World->GetFirstLocalPlayerFromController();
}

const FOnlineSessionSearchResult* GetSessionSearchResult(const TSharedPtr<FOnlineSessionSearch>& SessionSearch, int32 Index)
{
	if (!SessionSearch.IsValid() || !SessionSearch->SearchResults.IsValidIndex(Index))
	{
		return nullptr;
	}

	return SessionSearch->SearchResults.GetData() + Index;
}
}

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

	if (!ValidateCreateSessionRequest(NumPublicConnections, SessionName))
	{
		OnCreateSessionComplete.Broadcast(false);
		return;
	}

	ResetExistingGameSession();
	ConfigureSessionSettings(NumPublicConnections, bIsLAN, SanitizeSessionName(SessionName));

	FUniqueNetIdRepl UserId;
	if (!TryGetPreferredUserId(TEXT("create session"), UserId))
	{
		OnCreateSessionComplete.Broadcast(false);
		return;
	}

	if (!SessionInterface->CreateSession(*UserId, NAME_GameSession, *LastSessionSettings))
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

	// Security: Validate search parameters
	if (MaxSearchResults < 1 || MaxSearchResults > 100)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid MaxSearchResults: %d (must be 1-100)"), MaxSearchResults);
		OnFindSessionsComplete.Broadcast(false);
		return;
	}

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = bIsLAN;
	LastSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

	FUniqueNetIdRepl UserId;
	if (!TryGetPreferredUserId(TEXT("find sessions"), UserId))
	{
		OnFindSessionsComplete.Broadcast(false);
		return;
	}

	if (!SessionInterface->FindSessions(*UserId, LastSessionSearch.ToSharedRef()))
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

	FUniqueNetIdRepl UserId;
	if (!TryGetPreferredUserId(TEXT("join session"), UserId))
	{
		OnJoinSessionComplete.Broadcast(false);
		return;
	}

	const FOnlineSessionSearchResult* SearchResult = GetSessionSearchResult(LastSessionSearch, SessionIndex);
	if (!SearchResult)
	{
		OnJoinSessionComplete.Broadcast(false);
		return;
	}

	if (!SessionInterface->JoinSession(*UserId, NAME_GameSession, *SearchResult))
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
	if (const FOnlineSessionSearchResult* SearchResult = GetSessionSearchResult(LastSessionSearch, SessionIndex))
	{
		FString SessionName;
		SearchResult->Session.SessionSettings.Get(FName("SessionName"), SessionName);
		return SessionName;
	}
	return FString();
}

int32 UMultiplayerSessionSubsystem::GetSessionPing(int32 SessionIndex) const
{
	const FOnlineSessionSearchResult* SearchResult = GetSessionSearchResult(LastSessionSearch, SessionIndex);
	return SearchResult ? SearchResult->PingInMs : 0;
}

int32 UMultiplayerSessionSubsystem::GetSessionCurrentPlayers(int32 SessionIndex) const
{
	const FOnlineSessionSearchResult* SearchResult = GetSessionSearchResult(LastSessionSearch, SessionIndex);
	return SearchResult
		? SearchResult->Session.SessionSettings.NumPublicConnections - SearchResult->Session.NumOpenPublicConnections
		: 0;
}

int32 UMultiplayerSessionSubsystem::GetSessionMaxPlayers(int32 SessionIndex) const
{
	const FOnlineSessionSearchResult* SearchResult = GetSessionSearchResult(LastSessionSearch, SessionIndex);
	return SearchResult ? SearchResult->Session.SessionSettings.NumPublicConnections : 0;
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
			UWorld* World = GetWorld();
			if (!World)
			{
				OnJoinSessionComplete.Broadcast(false);
				return;
			}

			if (APlayerController* PC = World->GetFirstPlayerController())
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

bool UMultiplayerSessionSubsystem::ValidateCreateSessionRequest(int32 NumPublicConnections, const FString& SessionName) const
{
	if (NumPublicConnections < 1 || NumPublicConnections > MaxPublicSessionConnections)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid NumPublicConnections: %d (must be 1-64)"), NumPublicConnections);
		return false;
	}

	if (SessionName.IsEmpty() || SessionName.Len() > MaxSessionNameLength)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid SessionName length: %d"), SessionName.Len());
		return false;
	}

	return true;
}

FString UMultiplayerSessionSubsystem::SanitizeSessionName(const FString& SessionName) const
{
	FString SanitizedName = SessionName;
	SanitizedName.ReplaceInline(TEXT("<"), TEXT(""));
	SanitizedName.ReplaceInline(TEXT(">"), TEXT(""));
	SanitizedName.ReplaceInline(TEXT("&"), TEXT(""));
	return SanitizedName;
}

void UMultiplayerSessionSubsystem::ResetExistingGameSession()
{
	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}
}

void UMultiplayerSessionSubsystem::ConfigureSessionSettings(int32 NumPublicConnections, bool bIsLAN, const FString& SessionName)
{
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = bIsLAN;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->Set(FName("SessionName"), SessionName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
}

bool UMultiplayerSessionSubsystem::TryGetPreferredUserId(const TCHAR* OperationName, FUniqueNetIdRepl& OutUserId) const
{
	const ULocalPlayer* LocalPlayer = GetSessionLocalPlayer(this);
	if (!LocalPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot %s: local player unavailable"), OperationName);
		return false;
	}

	OutUserId = LocalPlayer->GetPreferredUniqueNetId();
	if (!OutUserId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot %s: local player has no valid online id"), OperationName);
		return false;
	}

	return true;
}
