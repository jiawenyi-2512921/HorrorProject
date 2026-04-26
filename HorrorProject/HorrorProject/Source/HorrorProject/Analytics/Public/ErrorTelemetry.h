// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ErrorTelemetry.generated.h"

UENUM(BlueprintType)
enum class EErrorSeverity : uint8
{
	Info,
	Warning,
	Error,
	Critical
};

USTRUCT(BlueprintType)
struct FErrorReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	FString ErrorId;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	FString ErrorCode;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	EErrorSeverity Severity = EErrorSeverity::Error;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	FString StackTrace;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadWrite, Category = "Telemetry")
	TMap<FString, FString> Context;
};

/**
 * Error Telemetry - Tracks and reports errors and crashes
 */
UCLASS()
class HORRORPROJECT_API UErrorTelemetry : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void ReportError(const FString& ErrorMessage, const FString& ErrorCode, EErrorSeverity Severity = EErrorSeverity::Error);

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void ReportException(const FString& ExceptionMessage, const FString& StackTrace);

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void ReportCrash(const FString& CrashReason);

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	TArray<FString> GetRecentErrors(int32 Count = 10) const;

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	int32 GetErrorCount() const { return ErrorHistory.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	int32 GetErrorCountBySeverity(EErrorSeverity Severity) const;

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	TArray<FErrorReport> GetErrorHistory() const { return ErrorHistory; }

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void ClearErrorHistory();

	UFUNCTION(BlueprintCallable, Category = "Telemetry")
	void ExportErrorLog(const FString& OutputPath);

protected:
	UPROPERTY()
	TArray<FErrorReport> ErrorHistory;

	UPROPERTY()
	TMap<FString, int32> ErrorCounts;

	static constexpr int32 MaxErrorHistory = 500;

private:
	void RecordError(const FErrorReport& Error);
	FString GenerateErrorId() const;
	FString GetStackTrace() const;
};
