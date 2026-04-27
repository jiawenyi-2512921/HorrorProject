// Copyright Epic Games, Inc. All Rights Reserved.

#include "ErrorTelemetry.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void UErrorTelemetry::ReportError(const FString& ErrorMessage, const FString& ErrorCode, EErrorSeverity Severity)
{
	FErrorReport Error;
	Error.ErrorId = GenerateErrorId();
	Error.ErrorMessage = ErrorMessage;
	Error.ErrorCode = ErrorCode;
	Error.Severity = Severity;
	Error.Timestamp = FDateTime::UtcNow();
	Error.StackTrace = GetStackTrace();

	RecordError(Error);

	UE_LOG(LogTemp, Warning, TEXT("Error reported: [%s] %s"), *ErrorCode, *ErrorMessage);
}

void UErrorTelemetry::ReportException(const FString& ExceptionMessage, const FString& StackTrace)
{
	FErrorReport Error;
	Error.ErrorId = GenerateErrorId();
	Error.ErrorMessage = ExceptionMessage;
	Error.ErrorCode = TEXT("EXCEPTION");
	Error.Severity = EErrorSeverity::Critical;
	Error.StackTrace = StackTrace;
	Error.Timestamp = FDateTime::UtcNow();

	RecordError(Error);

	UE_LOG(LogTemp, Error, TEXT("Exception reported: %s"), *ExceptionMessage);
}

void UErrorTelemetry::ReportCrash(const FString& CrashReason)
{
	FErrorReport Error;
	Error.ErrorId = GenerateErrorId();
	Error.ErrorMessage = CrashReason;
	Error.ErrorCode = TEXT("CRASH");
	Error.Severity = EErrorSeverity::Critical;
	Error.StackTrace = GetStackTrace();
	Error.Timestamp = FDateTime::UtcNow();

	RecordError(Error);

	UE_LOG(LogTemp, Fatal, TEXT("Crash reported: %s"), *CrashReason);
}

TArray<FString> UErrorTelemetry::GetRecentErrors(int32 Count) const
{
	TArray<FString> RecentErrors;
	int32 StartIndex = FMath::Max(0, ErrorHistory.Num() - Count);

	int32 ErrorIndex = 0;
	for (const FErrorReport& Error : ErrorHistory)
	{
		if (ErrorIndex++ < StartIndex)
		{
			continue;
		}

		RecentErrors.Emplace(FString::Printf(TEXT("[%s] %s: %s"),
			*Error.Timestamp.ToString(),
			*Error.ErrorCode,
			*Error.ErrorMessage));
	}

	return RecentErrors;
}

int32 UErrorTelemetry::GetErrorCountBySeverity(EErrorSeverity Severity) const
{
	int32 Count = 0;
	for (const FErrorReport& Error : ErrorHistory)
	{
		if (Error.Severity == Severity)
		{
			Count++;
		}
	}
	return Count;
}

void UErrorTelemetry::ClearErrorHistory()
{
	ErrorHistory.Empty();
	ErrorCounts.Empty();
}

void UErrorTelemetry::ExportErrorLog(const FString& OutputPath)
{
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);

	TArray<TSharedPtr<FJsonValue>> ErrorsArray;
	for (const FErrorReport& Error : ErrorHistory)
	{
		TSharedPtr<FJsonObject> ErrorObj = MakeShareable(new FJsonObject);
		ErrorObj->SetStringField(TEXT("error_id"), Error.ErrorId);
		ErrorObj->SetStringField(TEXT("error_message"), Error.ErrorMessage);
		ErrorObj->SetStringField(TEXT("error_code"), Error.ErrorCode);
		ErrorObj->SetNumberField(TEXT("severity"), static_cast<int32>(Error.Severity));
		ErrorObj->SetStringField(TEXT("stack_trace"), Error.StackTrace);
		ErrorObj->SetStringField(TEXT("timestamp"), Error.Timestamp.ToIso8601());

		TSharedPtr<FJsonObject> ContextObj = MakeShareable(new FJsonObject);
		for (const auto& Pair : Error.Context)
		{
			ContextObj->SetStringField(Pair.Key, Pair.Value);
		}
		ErrorObj->SetObjectField(TEXT("context"), ContextObj);

		ErrorsArray.Add(MakeShareable(new FJsonValueObject(ErrorObj)));
	}
	RootObject->SetArrayField(TEXT("errors"), ErrorsArray);

	// Add summary
	TSharedPtr<FJsonObject> SummaryObj = MakeShareable(new FJsonObject);
	SummaryObj->SetNumberField(TEXT("total_errors"), ErrorHistory.Num());
	SummaryObj->SetNumberField(TEXT("info_count"), GetErrorCountBySeverity(EErrorSeverity::Info));
	SummaryObj->SetNumberField(TEXT("warning_count"), GetErrorCountBySeverity(EErrorSeverity::Warning));
	SummaryObj->SetNumberField(TEXT("error_count"), GetErrorCountBySeverity(EErrorSeverity::Error));
	SummaryObj->SetNumberField(TEXT("critical_count"), GetErrorCountBySeverity(EErrorSeverity::Critical));
	RootObject->SetObjectField(TEXT("summary"), SummaryObj);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

	FFileHelper::SaveStringToFile(JsonString, *OutputPath);

	UE_LOG(LogTemp, Log, TEXT("Error log exported to: %s"), *OutputPath);
}

void UErrorTelemetry::RecordError(const FErrorReport& Error)
{
	ErrorHistory.Add(Error);

	if (ErrorHistory.Num() > MaxErrorHistory)
	{
		ErrorHistory.RemoveAt(0, ErrorHistory.Num() - MaxErrorHistory);
	}

	int32& Count = ErrorCounts.FindOrAdd(Error.ErrorCode, 0);
	Count++;
}

FString UErrorTelemetry::GenerateErrorId() const
{
	return FGuid::NewGuid().ToString();
}

FString UErrorTelemetry::GetStackTrace() const
{
	// Simplified stack trace - in production, use proper stack walking
	return FString::Printf(TEXT("Stack trace at %s"), *FDateTime::UtcNow().ToString());
}
