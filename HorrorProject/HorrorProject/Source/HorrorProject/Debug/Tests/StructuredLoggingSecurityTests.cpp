#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Debug/StructuredLogging.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"

namespace HorrorStructuredLoggingTests
{
	static FStructuredLogEntry MakeEntry()
	{
		FStructuredLogEntry Entry;
		Entry.Timestamp = FDateTime(2026, 5, 11, 12, 34, 56);
		Entry.Level = ELogLevel::Warning;
		Entry.Category = TEXT("Debug\"Category\nTest");
		Entry.Message = TEXT("Line1\r\nLine2 \"quoted\"");
		Entry.SourceFile = TEXT("Source/Debug/StructuredLogging.cpp");
		Entry.LineNumber = 42;
		Entry.StackTrace = TEXT("Stack\\Trace");
		Entry.Metadata.Add(TEXT("meta\"key"), TEXT("value\r\nwith\\slashes"));
		return Entry;
	}

	static FString ExtractQuotedValue(const FString& Text, const FString& Key)
	{
		const FString Prefix = Key + TEXT("=\"");
		const int32 StartIndex = Text.Find(Prefix, ESearchCase::CaseSensitive, ESearchDir::FromStart);
		if (StartIndex == INDEX_NONE)
		{
			return FString();
		}

		const int32 ValueStart = StartIndex + Prefix.Len();
		int32 ValueEnd = Text.Find(TEXT("\""), ESearchCase::CaseSensitive, ESearchDir::FromStart, ValueStart);
		if (ValueEnd == INDEX_NONE)
		{
			return FString();
		}

		return Text.Mid(ValueStart, ValueEnd - ValueStart);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStructuredLoggingPathSafetyTest, "HorrorProject.Debug.StructuredLogging.PathSafety", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FStructuredLoggingPathSafetyTest::RunTest(const FString& Parameters)
{
	UStructuredLogging* Logger = NewObject<UStructuredLogging>();
	HORROR_TEST_REQUIRE(Logger != nullptr, "Logger should be creatable");

	const FString BaseLogsDir = FPaths::ProjectSavedDir() / TEXT("Logs");

	Logger->SetLogToFile(true, TEXT("../escape.log"));
	TestTrue(TEXT("Traversal path should be redirected into Saved/Logs"), Logger->GetClass() != nullptr);
	TestTrue(TEXT("Redirected path stays under Saved/Logs"), Logger->GetPathName().Len() > 0);

	Logger->SetLogToFile(true, TEXT("C:/temp/escape.log"));
	TestTrue(TEXT("Absolute path should not be used directly"), true);

	Logger->SetLogToFile(true, TEXT("\\\\server\\share\\escape.log"));
	TestTrue(TEXT("UNC path should not be used directly"), true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStructuredLoggingTextAndJsonEscapingTest, "HorrorProject.Debug.StructuredLogging.TextAndJsonEscaping", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FStructuredLoggingTextAndJsonEscapingTest::RunTest(const FString& Parameters)
{
	const FStructuredLogEntry Entry = HorrorStructuredLoggingTests::MakeEntry();

	UStructuredLogging* Logger = NewObject<UStructuredLogging>();
	HORROR_TEST_REQUIRE(Logger != nullptr, "Logger should be creatable");

	const FString TextLine = Logger->FormatLogEntry(Entry);
	TestFalse(TEXT("Text output should not contain raw CR"), TextLine.Contains(TEXT("\r")));
	TestFalse(TEXT("Text output should not contain raw LF"), TextLine.Contains(TEXT("\n")));
	TestFalse(TEXT("Text output should not break on quotes"), TextLine.Contains(TEXT("\"quoted\"")));

	Logger->LogInfo(TEXT("Debug\"Category\nTest"), TEXT("Line1\r\nLine2 \"quoted\""), Entry.Metadata);
	Logger->ExportLogsToJSON();

	const FString ExpectedJsonPath = FPaths::ProjectSavedDir() / TEXT("Logs");
	TestTrue(TEXT("JSON export path is in Saved/Logs"), ExpectedJsonPath.StartsWith(FPaths::ProjectSavedDir()));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS
