// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "StructuredLogging.generated.h"

UENUM(BlueprintType)
enum class ELogLevel : uint8
{
	Trace,
	Debug,
	Info,
	Warning,
	Error,
	Fatal
};

USTRUCT(BlueprintType)
struct FStructuredLogEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly)
	ELogLevel Level;

	UPROPERTY(BlueprintReadOnly)
	FString Category;

	UPROPERTY(BlueprintReadOnly)
	FString Message;

	UPROPERTY(BlueprintReadOnly)
	FString SourceFile;

	UPROPERTY(BlueprintReadOnly)
	int32 LineNumber;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FString> Metadata;

	UPROPERTY(BlueprintReadOnly)
	FString StackTrace;
};

/**
 * Structured logging system with metadata and filtering
 */
UCLASS()
class HORRORPROJECT_API UStructuredLogging : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Logging Functions
	UFUNCTION(BlueprintCallable, Category = "Logging")
	void LogTrace(const FString& Category, const FString& Message, const TMap<FString, FString>& Metadata);

	UFUNCTION(BlueprintCallable, Category = "Logging")
	void LogDebug(const FString& Category, const FString& Message, const TMap<FString, FString>& Metadata);

	UFUNCTION(BlueprintCallable, Category = "Logging")
	void LogInfo(const FString& Category, const FString& Message, const TMap<FString, FString>& Metadata);

	UFUNCTION(BlueprintCallable, Category = "Logging")
	void LogWarning(const FString& Category, const FString& Message, const TMap<FString, FString>& Metadata);

	UFUNCTION(BlueprintCallable, Category = "Logging")
	void LogError(const FString& Category, const FString& Message, const TMap<FString, FString>& Metadata);

	UFUNCTION(BlueprintCallable, Category = "Logging")
	void LogFatal(const FString& Category, const FString& Message, const TMap<FString, FString>& Metadata);

	void LogTrace(const FString& Category, const FString& Message);
	void LogDebug(const FString& Category, const FString& Message);
	void LogInfo(const FString& Category, const FString& Message);
	void LogWarning(const FString& Category, const FString& Message);
	void LogError(const FString& Category, const FString& Message);
	void LogFatal(const FString& Category, const FString& Message);

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Logging")
	void SetMinimumLogLevel(ELogLevel Level);

	UFUNCTION(BlueprintCallable, Category = "Logging")
	void SetLogToFile(bool bEnabled, const FString& FilePath = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Logging")
	void SetLogToConsole(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Logging")
	void AddCategoryFilter(const FString& Category, ELogLevel MinLevel);

	UFUNCTION(BlueprintCallable, Category = "Logging")
	void RemoveCategoryFilter(const FString& Category);

	// Query Functions
	UFUNCTION(BlueprintCallable, Category = "Logging")
	TArray<FStructuredLogEntry> GetLogEntries(ELogLevel MinLevel = ELogLevel::Trace, const FString& Category = TEXT("")) const;

	UFUNCTION(BlueprintCallable, Category = "Logging")
	TArray<FStructuredLogEntry> GetRecentErrors(int32 Count = 10) const;

	UFUNCTION(BlueprintCallable, Category = "Logging")
	void ClearLogs();

	UFUNCTION(BlueprintCallable, Category = "Logging")
	void ExportLogsToFile(const FString& FilePath = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Logging")
	void ExportLogsToJSON(const FString& FilePath = TEXT(""));

protected:
	void WriteLog(ELogLevel Level, const FString& Category, const FString& Message, const TMap<FString, FString>& Metadata);
	void WriteToFile(const FStructuredLogEntry& Entry);
	void WriteToConsole(const FStructuredLogEntry& Entry);
	FString FormatLogEntry(const FStructuredLogEntry& Entry) const;
	FString LogLevelToString(ELogLevel Level) const;
	FLinearColor GetLogLevelColor(ELogLevel Level) const;
	bool ShouldLog(ELogLevel Level, const FString& Category) const;

	UPROPERTY()
	TArray<FStructuredLogEntry> LogEntries;

	UPROPERTY()
	ELogLevel MinimumLogLevel;

	UPROPERTY()
	bool bLogToFile;

	UPROPERTY()
	bool bLogToConsole;

	UPROPERTY()
	FString LogFilePath;

	UPROPERTY()
	TMap<FString, ELogLevel> CategoryFilters;

	UPROPERTY()
	int32 MaxLogEntries;
};
