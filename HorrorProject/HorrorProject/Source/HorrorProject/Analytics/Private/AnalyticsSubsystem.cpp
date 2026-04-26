// Copyright Epic Games, Inc. All Rights Reserved.

#include "AnalyticsSubsystem.h"
#include "EventTracker.h"
#include "SessionTracker.h"
#include "MetricsCollector.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/SecureHash.h"

void UAnalyticsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Create subsystem components
	EventTracker = NewObject<UEventTracker>(this);
	SessionTracker = NewObject<USessionTracker>(this);
	MetricsCollector = NewObject<UMetricsCollector>(this);

	// Load user consent
	LoadUserConsent();

	// Start session
	if (SessionTracker)
	{
		SessionTracker->StartSession();
	}

	// Setup periodic flush
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FlushTimerHandle,
			this,
			&UAnalyticsSubsystem::FlushEvents,
			FlushInterval,
			true
		);
	}

	UE_LOG(LogTemp, Log, TEXT("AnalyticsSubsystem initialized. Consent: %s"),
		UserConsent.bAnalyticsEnabled ? TEXT("Granted") : TEXT("Not Granted"));
}

void UAnalyticsSubsystem::Deinitialize()
{
	// Flush remaining events
	FlushEvents();

	// End session
	if (SessionTracker)
	{
		SessionTracker->EndSession();
	}

	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FlushTimerHandle);
	}

	Super::Deinitialize();
}

void UAnalyticsSubsystem::TrackEvent(const FString& EventName, const TMap<FString, FString>& Parameters)
{
	if (!ShouldTrackEvent())
	{
		return;
	}

	FAnalyticsEvent Event;
	Event.EventName = EventName;
	Event.Parameters = Parameters;
	Event.Timestamp = FDateTime::UtcNow();
	Event.SessionId = GetCurrentSessionId();

	// Add to queue
	EventQueue.Add(Event);

	// Track with event tracker
	if (EventTracker)
	{
		EventTracker->TrackEvent(Event);
	}

	// Broadcast event
	OnEventTracked.Broadcast(Event);

	UE_LOG(LogTemp, Verbose, TEXT("Event tracked: %s"), *EventName);
}

void UAnalyticsSubsystem::TrackScreenView(const FString& ScreenName)
{
	TMap<FString, FString> Parameters;
	Parameters.Add(TEXT("screen_name"), ScreenName);
	TrackEvent(TEXT("screen_view"), Parameters);
}

void UAnalyticsSubsystem::TrackError(const FString& ErrorMessage, const FString& ErrorCode)
{
	TMap<FString, FString> Parameters;
	Parameters.Add(TEXT("error_message"), ErrorMessage);
	Parameters.Add(TEXT("error_code"), ErrorCode);
	TrackEvent(TEXT("error"), Parameters);
}

void UAnalyticsSubsystem::SetUserConsent(const FUserConsent& Consent)
{
	UserConsent = Consent;
	UserConsent.ConsentTimestamp = FDateTime::UtcNow();
	UserConsent.ConsentVersion = TEXT("1.0");

	SaveUserConsent();

	// Track consent change
	TMap<FString, FString> Parameters;
	Parameters.Add(TEXT("analytics_enabled"), UserConsent.bAnalyticsEnabled ? TEXT("true") : TEXT("false"));
	Parameters.Add(TEXT("ads_enabled"), UserConsent.bPersonalizedAdsEnabled ? TEXT("true") : TEXT("false"));
	Parameters.Add(TEXT("crash_reporting_enabled"), UserConsent.bCrashReportingEnabled ? TEXT("true") : TEXT("false"));
	TrackEvent(TEXT("consent_updated"), Parameters);

	UE_LOG(LogTemp, Log, TEXT("User consent updated: Analytics=%s"),
		UserConsent.bAnalyticsEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UAnalyticsSubsystem::SetUserId(const FString& UserId, bool bAnonymize)
{
	if (bAnonymize)
	{
		AnonymizedUserId = AnonymizeUserId(UserId);
	}
	else
	{
		AnonymizedUserId = UserId;
	}

	UE_LOG(LogTemp, Log, TEXT("User ID set (anonymized: %s)"), bAnonymize ? TEXT("Yes") : TEXT("No"));
}

void UAnalyticsSubsystem::RequestDataDeletion()
{
	// Clear all local data
	EventQueue.Empty();

	if (EventTracker)
	{
		EventTracker->ClearAllEvents();
	}

	if (SessionTracker)
	{
		SessionTracker->ClearSessionHistory();
	}

	if (MetricsCollector)
	{
		MetricsCollector->ClearAllMetrics();
	}

	// Delete consent file
	FString ConsentPath = FPaths::ProjectSavedDir() / TEXT("Analytics/user_consent.json");
	IFileManager::Get().Delete(*ConsentPath);

	// Reset consent
	UserConsent = FUserConsent();
	AnonymizedUserId.Empty();

	UE_LOG(LogTemp, Warning, TEXT("All user data deleted per GDPR/CCPA request"));
}

void UAnalyticsSubsystem::ExportUserData(const FString& OutputPath)
{
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);

	// User info
	RootObject->SetStringField(TEXT("user_id"), AnonymizedUserId);
	RootObject->SetStringField(TEXT("consent_version"), UserConsent.ConsentVersion);
	RootObject->SetStringField(TEXT("consent_timestamp"), UserConsent.ConsentTimestamp.ToString());

	// Events
	TArray<TSharedPtr<FJsonValue>> EventsArray;
	for (const FAnalyticsEvent& Event : EventQueue)
	{
		TSharedPtr<FJsonObject> EventObj = MakeShareable(new FJsonObject);
		EventObj->SetStringField(TEXT("event_name"), Event.EventName);
		EventObj->SetStringField(TEXT("timestamp"), Event.Timestamp.ToString());
		EventObj->SetStringField(TEXT("session_id"), Event.SessionId);

		TSharedPtr<FJsonObject> ParamsObj = MakeShareable(new FJsonObject);
		for (const auto& Param : Event.Parameters)
		{
			ParamsObj->SetStringField(Param.Key, Param.Value);
		}
		EventObj->SetObjectField(TEXT("parameters"), ParamsObj);

		EventsArray.Add(MakeShareable(new FJsonValueObject(EventObj)));
	}
	RootObject->SetArrayField(TEXT("events"), EventsArray);

	// Write to file
	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	FFileHelper::SaveStringToFile(JsonString, *OutputPath);

	UE_LOG(LogTemp, Log, TEXT("User data exported to: %s"), *OutputPath);
}

void UAnalyticsSubsystem::FlushEvents()
{
	if (EventQueue.Num() == 0)
	{
		return;
	}

	ProcessEventQueue();

	UE_LOG(LogTemp, Verbose, TEXT("Flushed %d analytics events"), EventQueue.Num());
	EventQueue.Empty();
}

FString UAnalyticsSubsystem::GetCurrentSessionId() const
{
	return SessionTracker ? SessionTracker->GetCurrentSessionId() : FString();
}

float UAnalyticsSubsystem::GetSessionDuration() const
{
	return SessionTracker ? SessionTracker->GetSessionDuration() : 0.0f;
}

void UAnalyticsSubsystem::LoadUserConsent()
{
	FString ConsentPath = FPaths::ProjectSavedDir() / TEXT("Analytics/user_consent.json");

	FString JsonString;
	if (FFileHelper::LoadFileToString(JsonString, *ConsentPath))
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			UserConsent.bAnalyticsEnabled = JsonObject->GetBoolField(TEXT("analytics_enabled"));
			UserConsent.bPersonalizedAdsEnabled = JsonObject->GetBoolField(TEXT("ads_enabled"));
			UserConsent.bCrashReportingEnabled = JsonObject->GetBoolField(TEXT("crash_reporting_enabled"));
			UserConsent.ConsentVersion = JsonObject->GetStringField(TEXT("consent_version"));

			FString TimestampStr = JsonObject->GetStringField(TEXT("consent_timestamp"));
			FDateTime::Parse(TimestampStr, UserConsent.ConsentTimestamp);
		}
	}
}

void UAnalyticsSubsystem::SaveUserConsent()
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetBoolField(TEXT("analytics_enabled"), UserConsent.bAnalyticsEnabled);
	JsonObject->SetBoolField(TEXT("ads_enabled"), UserConsent.bPersonalizedAdsEnabled);
	JsonObject->SetBoolField(TEXT("crash_reporting_enabled"), UserConsent.bCrashReportingEnabled);
	JsonObject->SetStringField(TEXT("consent_version"), UserConsent.ConsentVersion);
	JsonObject->SetStringField(TEXT("consent_timestamp"), UserConsent.ConsentTimestamp.ToString());

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	FString ConsentPath = FPaths::ProjectSavedDir() / TEXT("Analytics/user_consent.json");
	FFileHelper::SaveStringToFile(JsonString, *ConsentPath);
}

FString UAnalyticsSubsystem::AnonymizeUserId(const FString& UserId)
{
	// Use SHA-256 hash for anonymization
	return FMD5::HashAnsiString(*UserId);
}

void UAnalyticsSubsystem::ProcessEventQueue()
{
	// Write events to file
	FString EventsPath = FPaths::ProjectSavedDir() / TEXT("Analytics/events.jsonl");

	FString EventsData;
	for (const FAnalyticsEvent& Event : EventQueue)
	{
		TSharedPtr<FJsonObject> EventObj = MakeShareable(new FJsonObject);
		EventObj->SetStringField(TEXT("event_name"), Event.EventName);
		EventObj->SetStringField(TEXT("timestamp"), Event.Timestamp.ToIso8601());
		EventObj->SetStringField(TEXT("session_id"), Event.SessionId);
		EventObj->SetStringField(TEXT("user_id"), AnonymizedUserId);

		TSharedPtr<FJsonObject> ParamsObj = MakeShareable(new FJsonObject);
		for (const auto& Param : Event.Parameters)
		{
			ParamsObj->SetStringField(Param.Key, Param.Value);
		}
		EventObj->SetObjectField(TEXT("parameters"), ParamsObj);

		FString JsonLine;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonLine);
		FJsonSerializer::Serialize(EventObj.ToSharedRef(), Writer);
		EventsData += JsonLine + TEXT("\n");
	}

	FFileHelper::SaveStringToFile(EventsData, *EventsPath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
}

bool UAnalyticsSubsystem::ShouldTrackEvent() const
{
	return UserConsent.bAnalyticsEnabled;
}
