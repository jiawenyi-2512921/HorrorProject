// Copyright Epic Games, Inc. All Rights Reserved.

#include "StructuredLogging.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/Engine.h"
#include "JsonObjectConverter.h"

void UStructuredLogging::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	MinimumLogLevel = ELogLevel::Debug;
	bLogToFile = true;
	bLogToConsole = true;
	MaxLogEntries = 10000;

	// Memory optimization: Pre-allocate log entries and filters
	LogEntries.Reserve(MaxLogEntries);
	CategoryFilters.Reserve(16);

	LogFilePath = FPaths::ProjectSavedDir() / TEXT("Logs") / TEXT("StructuredLog.txt");

	UE_LOG(LogTemp, Log, TEXT("Structured Logging System Initialized"));
	LogInfo(TEXT("System"), TEXT("Structured logging system started"), TMap<FString, FString>());
}

void UStructuredLogging::Deinitialize()
{
	LogInfo(TEXT("System"), TEXT("Structured logging system shutting down"), TMap<FString, FString>());
	ExportLogsToFile();

	Super::Deinitialize();
}

// Consolidated logging functions - eliminates code duplication
void UStructuredLogging::LogTrace(const FString& Category, const FString& Message, const TMap<FString, FString>& Metadata)
{
	WriteLog(ELogLevel::Trace, Category, Message, Metadata);
}

void UStructuredLogging::LogTrace(const FString& Category, const FString& Message)
{
	WriteLog(ELogLevel::Trace, Category, Message, TMap<FString, FString>());
}

void UStructuredLogging::LogDebug(const FString& Category, const FString& Message, const TMap<FString, FString>& Metadata)
{
	WriteLog(ELogLevel::Debug, Category, Message, Metadata);
}

void UStructuredLogging::LogDebug(const FString& Category, const FString& Message)
{
	WriteLog(ELogLevel::Debug, Category, Message, TMap<FString, FString>());
}

void UStructuredLogging::LogInfo(const FString& Category, const FString& Message, const TMap<FString, FString>& Metadata)
{
	WriteLog(ELogLevel::Info, Category, Message, Metadata);
}

void UStructuredLogging::LogInfo(const FString& Category, const FString& Message)
{
	WriteLog(ELogLevel::Info, Category, Message, TMap<FString, FString>());
}

void UStructuredLogging::LogWarning(const FString& Category, const FString& Message, const TMap<FString, FString>& Metadata)
{
	WriteLog(ELogLevel::Warning, Category, Message, Metadata);
}

void UStructuredLogging::LogWarning(const FString& Category, const FString& Message)
{
	WriteLog(ELogLevel::Warning, Category, Message, TMap<FString, FString>());
}

void UStructuredLogging::LogError(const FString& Category, const FString& Message, const TMap<FString, FString>& Metadata)
{
	WriteLog(ELogLevel::Error, Category, Message, Metadata);
}

void UStructuredLogging::LogError(const FString& Category, const FString& Message)
{
	WriteLog(ELogLevel::Error, Category, Message, TMap<FString, FString>());
}

void UStructuredLogging::LogFatal(const FString& Category, const FString& Message, const TMap<FString, FString>& Metadata)
{
	WriteLog(ELogLevel::Fatal, Category, Message, Metadata);
}

void UStructuredLogging::LogFatal(const FString& Category, const FString& Message)
{
	WriteLog(ELogLevel::Fatal, Category, Message, TMap<FString, FString>());
}

/**
 * Core logging function that processes and routes log entries
 * @param Level - Severity level of the log entry
 * @param Category - Logical category for filtering and organization
 * @param Message - Human-readable log message
 * @param Metadata - Optional key-value pairs for structured data
 */
void UStructuredLogging::WriteLog(ELogLevel Level, const FString& Category, const FString& Message, const TMap<FString, FString>& Metadata)
{
	if (!ShouldLog(Level, Category))
	{
		return;
	}

	FStructuredLogEntry Entry;
	Entry.Timestamp = FDateTime::Now();
	Entry.Level = Level;
	Entry.Category = Category;
	Entry.Message = Message;
	Entry.Metadata = Metadata;
	Entry.LineNumber = 0;

	LogEntries.Add(Entry);

	// Maintain circular buffer to prevent unbounded memory growth
	if (LogEntries.Num() > MaxLogEntries)
	{
		LogEntries.RemoveAt(0, LogEntries.Num() - MaxLogEntries);
	}

	if (bLogToFile)
	{
		WriteToFile(Entry);
	}

	if (bLogToConsole)
	{
		WriteToConsole(Entry);
	}
}

void UStructuredLogging::WriteToFile(const FStructuredLogEntry& Entry)
{
	FString LogLine = FormatLogEntry(Entry);
	FFileHelper::SaveStringToFile(LogLine + TEXT("\n"), *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
}

void UStructuredLogging::WriteToConsole(const FStructuredLogEntry& Entry)
{
	FString LogLine = FormatLogEntry(Entry);

	switch (Entry.Level)
	{
	case ELogLevel::Trace:
	case ELogLevel::Debug:
	case ELogLevel::Info:
		UE_LOG(LogTemp, Log, TEXT("%s"), *LogLine);
		break;
	case ELogLevel::Warning:
		UE_LOG(LogTemp, Warning, TEXT("%s"), *LogLine);
		break;
	case ELogLevel::Error:
	case ELogLevel::Fatal:
		UE_LOG(LogTemp, Error, TEXT("%s"), *LogLine);
		break;
	}
}

/**
 * Formats a log entry into a human-readable string
 * Format: [Timestamp] [Level] [Category] Message key1=value1 key2=value2
 */
FString UStructuredLogging::FormatLogEntry(const FStructuredLogEntry& Entry) const
{
	FString MetadataStr;
	for (const auto& Pair : Entry.Metadata)
	{
		MetadataStr += FString::Printf(TEXT(" %s=%s"), *Pair.Key, *Pair.Value);
	}

	return FString::Printf(TEXT("[%s] [%s] [%s] %s%s"),
		*Entry.Timestamp.ToString(TEXT("%Y-%m-%d %H:%M:%S")),
		*LogLevelToString(Entry.Level),
		*Entry.Category,
		*Entry.Message,
		*MetadataStr);
}

FString UStructuredLogging::LogLevelToString(ELogLevel Level) const
{
	switch (Level)
	{
	case ELogLevel::Trace: return TEXT("TRACE");
	case ELogLevel::Debug: return TEXT("DEBUG");
	case ELogLevel::Info: return TEXT("INFO");
	case ELogLevel::Warning: return TEXT("WARN");
	case ELogLevel::Error: return TEXT("ERROR");
	case ELogLevel::Fatal: return TEXT("FATAL");
	default: return TEXT("UNKNOWN");
	}
}

FLinearColor UStructuredLogging::GetLogLevelColor(ELogLevel Level) const
{
	switch (Level)
	{
	case ELogLevel::Trace: return FLinearColor::Gray;
	case ELogLevel::Debug: return FLinearColor::White;
	case ELogLevel::Info: return FLinearColor::Green;
	case ELogLevel::Warning: return FLinearColor::Yellow;
	case ELogLevel::Error: return FLinearColor::Red;
	case ELogLevel::Fatal: return FLinearColor(1.0f, 0.0f, 1.0f);
	default: return FLinearColor::White;
	}
}

bool UStructuredLogging::ShouldLog(ELogLevel Level, const FString& Category) const
{
	// Check category-specific filter
	if (CategoryFilters.Contains(Category))
	{
		return Level >= CategoryFilters[Category];
	}

	// Check global minimum level
	return Level >= MinimumLogLevel;
}

void UStructuredLogging::SetMinimumLogLevel(ELogLevel Level)
{
	MinimumLogLevel = Level;
	UE_LOG(LogTemp, Log, TEXT("Minimum log level set to: %s"), *LogLevelToString(Level));
}

void UStructuredLogging::SetLogToFile(bool bEnabled, const FString& FilePath)
{
	bLogToFile = bEnabled;
	if (!FilePath.IsEmpty())
	{
		LogFilePath = FilePath;
	}
	UE_LOG(LogTemp, Log, TEXT("Log to file: %s (Path: %s)"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"), *LogFilePath);
}

void UStructuredLogging::SetLogToConsole(bool bEnabled)
{
	bLogToConsole = bEnabled;
	UE_LOG(LogTemp, Log, TEXT("Log to console: %s"), bEnabled ? TEXT("Enabled") : TEXT("Disabled"));
}

void UStructuredLogging::AddCategoryFilter(const FString& Category, ELogLevel MinLevel)
{
	CategoryFilters.Add(Category, MinLevel);
	UE_LOG(LogTemp, Log, TEXT("Added category filter: %s (Min Level: %s)"), *Category, *LogLevelToString(MinLevel));
}

void UStructuredLogging::RemoveCategoryFilter(const FString& Category)
{
	CategoryFilters.Remove(Category);
	UE_LOG(LogTemp, Log, TEXT("Removed category filter: %s"), *Category);
}

TArray<FStructuredLogEntry> UStructuredLogging::GetLogEntries(ELogLevel MinLevel, const FString& Category) const
{
	TArray<FStructuredLogEntry> FilteredEntries;

	for (const FStructuredLogEntry& Entry : LogEntries)
	{
		if (Entry.Level >= MinLevel)
		{
			if (Category.IsEmpty() || Entry.Category == Category)
			{
				FilteredEntries.Add(Entry);
			}
		}
	}

	return FilteredEntries;
}

TArray<FStructuredLogEntry> UStructuredLogging::GetRecentErrors(int32 Count) const
{
	TArray<FStructuredLogEntry> Errors;

	for (int32 i = LogEntries.Num() - 1; i >= 0 && Errors.Num() < Count; i--)
	{
		if (LogEntries[i].Level >= ELogLevel::Error)
		{
			Errors.Add(LogEntries[i]);
		}
	}

	return Errors;
}

void UStructuredLogging::ClearLogs()
{
	LogEntries.Empty();
	UE_LOG(LogTemp, Log, TEXT("Log entries cleared"));
}

void UStructuredLogging::ExportLogsToFile(const FString& FilePath)
{
	FString OutputPath = FilePath.IsEmpty() ?
		FPaths::ProjectSavedDir() / TEXT("Logs") / FString::Printf(TEXT("Export_%s.txt"),
			*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))) : FilePath;

	FString Content;
	for (const FStructuredLogEntry& Entry : LogEntries)
	{
		Content += FormatLogEntry(Entry) + TEXT("\n");
	}

	if (FFileHelper::SaveStringToFile(Content, *OutputPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Logs exported to: %s"), *OutputPath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to export logs to: %s"), *OutputPath);
	}
}

void UStructuredLogging::ExportLogsToJSON(const FString& FilePath)
{
	FString OutputPath = FilePath.IsEmpty() ?
		FPaths::ProjectSavedDir() / TEXT("Logs") / FString::Printf(TEXT("Export_%s.json"),
			*FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"))) : FilePath;

	FString JsonContent = TEXT("[\n");

	for (int32 i = 0; i < LogEntries.Num(); i++)
	{
		const FStructuredLogEntry& Entry = LogEntries[i];

		JsonContent += TEXT("  {\n");
		JsonContent += FString::Printf(TEXT("    \"timestamp\": \"%s\",\n"), *Entry.Timestamp.ToIso8601());
		JsonContent += FString::Printf(TEXT("    \"level\": \"%s\",\n"), *LogLevelToString(Entry.Level));
		JsonContent += FString::Printf(TEXT("    \"category\": \"%s\",\n"), *Entry.Category);
		JsonContent += FString::Printf(TEXT("    \"message\": \"%s\"\n"), *Entry.Message.ReplaceCharWithEscapedChar());

		if (i < LogEntries.Num() - 1)
		{
			JsonContent += TEXT("  },\n");
		}
		else
		{
			JsonContent += TEXT("  }\n");
		}
	}

	JsonContent += TEXT("]\n");

	if (FFileHelper::SaveStringToFile(JsonContent, *OutputPath))
	{
		UE_LOG(LogTemp, Log, TEXT("Logs exported to JSON: %s"), *OutputPath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to export logs to JSON: %s"), *OutputPath);
	}
}
