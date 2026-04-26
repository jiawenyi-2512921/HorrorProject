// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AnalyticsSubsystem.generated.h"

class UEventTracker;
class USessionTracker;
class UMetricsCollector;

USTRUCT(BlueprintType)
struct FAnalyticsEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString EventName;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	TMap<FString, FString> Parameters;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString SessionId;

	FAnalyticsEvent()
		: Timestamp(FDateTime::UtcNow())
	{}
};

USTRUCT(BlueprintType)
struct FUserConsent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	bool bAnalyticsEnabled = false;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	bool bPersonalizedAdsEnabled = false;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	bool bCrashReportingEnabled = true;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FDateTime ConsentTimestamp;

	UPROPERTY(BlueprintReadWrite, Category = "Analytics")
	FString ConsentVersion;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnalyticsEvent, const FAnalyticsEvent&, Event);

/**
 * Analytics Subsystem - GDPR/CCPA compliant analytics system
 */
UCLASS()
class HORRORPROJECT_API UAnalyticsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Event Tracking
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void TrackEvent(const FString& EventName, const TMap<FString, FString>& Parameters);

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void TrackScreenView(const FString& ScreenName);

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void TrackError(const FString& ErrorMessage, const FString& ErrorCode);

	// User Consent (GDPR/CCPA)
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void SetUserConsent(const FUserConsent& Consent);

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	FUserConsent GetUserConsent() const { return UserConsent; }

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	bool HasAnalyticsConsent() const { return UserConsent.bAnalyticsEnabled; }

	// User Identification (Anonymized)
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void SetUserId(const FString& UserId, bool bAnonymize = true);

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	FString GetAnonymizedUserId() const { return AnonymizedUserId; }

	// Data Management
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void RequestDataDeletion();

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void ExportUserData(const FString& OutputPath);

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	void FlushEvents();

	// Session Management
	UFUNCTION(BlueprintCallable, Category = "Analytics")
	FString GetCurrentSessionId() const;

	UFUNCTION(BlueprintCallable, Category = "Analytics")
	float GetSessionDuration() const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Analytics")
	FOnAnalyticsEvent OnEventTracked;

protected:
	UPROPERTY()
	TObjectPtr<UEventTracker> EventTracker;

	UPROPERTY()
	TObjectPtr<USessionTracker> SessionTracker;

	UPROPERTY()
	TObjectPtr<UMetricsCollector> MetricsCollector;

	UPROPERTY()
	FUserConsent UserConsent;

	UPROPERTY()
	FString AnonymizedUserId;

	UPROPERTY()
	TArray<FAnalyticsEvent> EventQueue;

private:
	void LoadUserConsent();
	void SaveUserConsent();
	FString AnonymizeUserId(const FString& UserId);
	void ProcessEventQueue();
	bool ShouldTrackEvent() const;

	FTimerHandle FlushTimerHandle;
	static constexpr float FlushInterval = 30.0f;
};
