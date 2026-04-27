// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHorrorCreateSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHorrorFindSessionsComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHorrorJoinSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHorrorDestroySessionComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHorrorStartSessionComplete);

/**
 * Coordinates Multiplayer Session Subsystem services for the Network module.
 */
UCLASS()
class HORRORPROJECT_API UMultiplayerSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiplayerSessionSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Session management
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void CreateSession(int32 NumPublicConnections, bool bIsLAN, const FString& SessionName);

	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void FindSessions(int32 MaxSearchResults, bool bIsLAN);

	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void JoinSession(int32 SessionIndex);

	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void DestroySession();

	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void StartSession();

	// Session info
	UFUNCTION(BlueprintPure, Category = "Multiplayer")
	int32 GetFoundSessionsCount() const;

	UFUNCTION(BlueprintPure, Category = "Multiplayer")
	FString GetSessionName(int32 SessionIndex) const;

	UFUNCTION(BlueprintPure, Category = "Multiplayer")
	int32 GetSessionPing(int32 SessionIndex) const;

	UFUNCTION(BlueprintPure, Category = "Multiplayer")
	int32 GetSessionCurrentPlayers(int32 SessionIndex) const;

	UFUNCTION(BlueprintPure, Category = "Multiplayer")
	int32 GetSessionMaxPlayers(int32 SessionIndex) const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Multiplayer")
	FHorrorCreateSessionComplete OnCreateSessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Multiplayer")
	FHorrorFindSessionsComplete OnFindSessionsComplete;

	UPROPERTY(BlueprintAssignable, Category = "Multiplayer")
	FHorrorJoinSessionComplete OnJoinSessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Multiplayer")
	FHorrorDestroySessionComplete OnDestroySessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Multiplayer")
	FHorrorStartSessionComplete OnStartSessionComplete;

protected:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;

	// Delegate handles
	FDelegateHandle CreateSessionCompleteHandle;
	FDelegateHandle FindSessionsCompleteHandle;
	FDelegateHandle JoinSessionCompleteHandle;
	FDelegateHandle DestroySessionCompleteHandle;
	FDelegateHandle StartSessionCompleteHandle;

	// Callbacks
	void OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsCompleted(bool bWasSuccessful);
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionCompleted(FName SessionName, bool bWasSuccessful);
	void OnStartSessionCompleted(FName SessionName, bool bWasSuccessful);

	// Helper
	bool ValidateCreateSessionRequest(int32 NumPublicConnections, const FString& SessionName) const;
	FString SanitizeSessionName(const FString& SessionName) const;
	void ResetExistingGameSession();
	void ConfigureSessionSettings(int32 NumPublicConnections, bool bIsLAN, const FString& SessionName);
	bool TryGetPreferredUserId(const TCHAR* OperationName, FUniqueNetIdRepl& OutUserId) const;
	bool IsValidSessionIndex(int32 Index) const;
};
