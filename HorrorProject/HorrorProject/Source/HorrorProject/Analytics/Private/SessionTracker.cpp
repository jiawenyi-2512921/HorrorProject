// Copyright Epic Games, Inc. All Rights Reserved.

#include "SessionTracker.h"
#include "Misc/Guid.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void USessionTracker::StartSession()
{
	CurrentSession = FSessionData();
	CurrentSession.SessionId = GenerateSessionId();
	CurrentSession.StartTime = FDateTime::UtcNow();
	CurrentSession.Platform = UGameplayStatics::GetPlatformName();
	CurrentSession.AppVersion = FApp::GetProjectVersion();

	LoadSessionHistory();

	UE_LOG(LogTemp, Log, TEXT("Session started: %s"), *CurrentSession.SessionId);
}

void USessionTracker::EndSession()
{
	if (CurrentSession.SessionId.IsEmpty())
	{
		return;
	}

	CurrentSession.EndTime = FDateTime::UtcNow();
	CurrentSession.Duration = (CurrentSession.EndTime - CurrentSession.StartTime).GetTotalSeconds();

	// Add to history
	SessionHistory.Add(CurrentSession);
	if (SessionHistory.Num() > MaxSessionHistory)
	{
		SessionHistory.RemoveAt(0, SessionHistory.Num() - MaxSessionHistory);
	}

	SaveSessionData();

	UE_LOG(LogTemp, Log, TEXT("Session ended: %s (Duration: %.2f seconds)"),
		*CurrentSession.SessionId, CurrentSession.Duration);

	CurrentSession = FSessionData();
}

void USessionTracker::ClearSessionHistory()
{
	SessionHistory.Empty();
	CurrentSession = FSessionData();

	FString SessionPath = FPaths::ProjectSavedDir() / TEXT("Analytics/sessions.json");
	IFileManager::Get().Delete(*SessionPath);
}

float USessionTracker::GetSessionDuration() const
{
	if (CurrentSession.SessionId.IsEmpty())
	{
		return 0.0f;
	}

	return (FDateTime::UtcNow() - CurrentSession.StartTime).GetTotalSeconds();
}

float USessionTracker::GetAverageSessionDuration() const
{
	if (SessionHistory.Num() == 0)
	{
		return 0.0f;
	}

	float TotalDuration = 0.0f;
	for (const FSessionData& Session : SessionHistory)
	{
		TotalDuration += Session.Duration;
	}

	return TotalDuration / SessionHistory.Num();
}

FString USessionTracker::GenerateSessionId() const
{
	return FGuid::NewGuid().ToString();
}

void USessionTracker::SaveSessionData()
{
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);

	TArray<TSharedPtr<FJsonValue>> SessionsArray;
	for (const FSessionData& Session : SessionHistory)
	{
		TSharedPtr<FJsonObject> SessionObj = MakeShareable(new FJsonObject);
		SessionObj->SetStringField(TEXT("session_id"), Session.SessionId);
		SessionObj->SetStringField(TEXT("start_time"), Session.StartTime.ToIso8601());
		SessionObj->SetStringField(TEXT("end_time"), Session.EndTime.ToIso8601());
		SessionObj->SetNumberField(TEXT("duration"), Session.Duration);
		SessionObj->SetNumberField(TEXT("event_count"), Session.EventCount);
		SessionObj->SetStringField(TEXT("platform"), Session.Platform);
		SessionObj->SetStringField(TEXT("app_version"), Session.AppVersion);

		SessionsArray.Add(MakeShareable(new FJsonValueObject(SessionObj)));
	}
	RootObject->SetArrayField(TEXT("sessions"), SessionsArray);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	FString SessionPath = FPaths::ProjectSavedDir() / TEXT("Analytics/sessions.json");
	FFileHelper::SaveStringToFile(JsonString, *SessionPath);
}

void USessionTracker::LoadSessionHistory()
{
	FString SessionPath = FPaths::ProjectSavedDir() / TEXT("Analytics/sessions.json");

	FString JsonString;
	if (FFileHelper::LoadFileToString(JsonString, *SessionPath))
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			const TArray<TSharedPtr<FJsonValue>>* SessionsArray;
			if (JsonObject->TryGetArrayField(TEXT("sessions"), SessionsArray))
			{
				SessionHistory.Empty();
				for (const TSharedPtr<FJsonValue>& SessionValue : *SessionsArray)
				{
					TSharedPtr<FJsonObject> SessionObj = SessionValue->AsObject();
					if (SessionObj.IsValid())
					{
						FSessionData Session;
						Session.SessionId = SessionObj->GetStringField(TEXT("session_id"));
						FDateTime::ParseIso8601(*SessionObj->GetStringField(TEXT("start_time")), Session.StartTime);
						FDateTime::ParseIso8601(*SessionObj->GetStringField(TEXT("end_time")), Session.EndTime);
						Session.Duration = SessionObj->GetNumberField(TEXT("duration"));
						Session.EventCount = SessionObj->GetIntegerField(TEXT("event_count"));
						Session.Platform = SessionObj->GetStringField(TEXT("platform"));
						Session.AppVersion = SessionObj->GetStringField(TEXT("app_version"));

						SessionHistory.Add(Session);
					}
				}
			}
		}
	}
}
