// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SessionTracker.generated.h"

USTRUCT(BlueprintType)
struct FSessionData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString SessionId;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FDateTime StartTime;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FDateTime EndTime;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	float Duration = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	int32 EventCount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString Platform;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString AppVersion;
};

/**
 * Session Tracker - Manages user session tracking
 */
UCLASS()
class HORRORPROJECT_API USessionTracker : public UObject
{
	GENERATED_BODY()

public:
	void StartSession();
	void EndSession();
	void ClearSessionHistory();

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	FString GetCurrentSessionId() const { return CurrentSession.SessionId; }

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	float GetSessionDuration() const;

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	int32 GetTotalSessions() const { return SessionHistory.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	float GetAverageSessionDuration() const;

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	FSessionData GetCurrentSession() const { return CurrentSession; }

protected:
	UPROPERTY()
	FSessionData CurrentSession;

	UPROPERTY()
	TArray<FSessionData> SessionHistory;

	static constexpr int32 MaxSessionHistory = 100;

private:
	FString GenerateSessionId() const;
	void SaveSessionData();
	void LoadSessionHistory();
};
