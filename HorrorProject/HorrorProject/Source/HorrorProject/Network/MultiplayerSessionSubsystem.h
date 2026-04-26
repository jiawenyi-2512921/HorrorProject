// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFindSessionsComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDestroySessionComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartSessionComplete);

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
	FOnCreateSessionComplete OnCreateSessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Multiplayer")
	FOnFindSessionsComplete OnFindSessionsComplete;

	UPROPERTY(BlueprintAssignable, Category = "Multiplayer")
	FOnJoinSessionComplete OnJoinSessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Multiplayer")
	FOnDestroySessionComplete OnDestroySessionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Multiplayer")
	FOnStartSessionComplete OnStartSessionComplete;

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
	bool IsValidSessionIndex(int32 Index) const;
};
