// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "../SubtitleSystem.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSubtitleSystemBasicTest, "HorrorProject.Accessibility.SubtitleBasic", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSubtitleSystemBasicTest::RunTest(const FString& Parameters)
{
    USubtitleSystem* SubtitleSystem = NewObject<USubtitleSystem>();
    TestNotNull(TEXT("Subtitle system created"), SubtitleSystem);

    // Test adding subtitle
    FText TestText = FText::FromString(TEXT("Test subtitle"));
    SubtitleSystem->AddSubtitle(TestText, TEXT("Speaker"), 3.0f);

    TArray<FSubtitleEntry> ActiveSubtitles = SubtitleSystem->GetActiveSubtitles();
    TestEqual(TEXT("One subtitle added"), ActiveSubtitles.Num(), 1);
    TestEqual(TEXT("Subtitle text matches"), ActiveSubtitles[0].Text.ToString(), TestText.ToString());
    TestEqual(TEXT("Speaker name matches"), ActiveSubtitles[0].SpeakerName, FString(TEXT("Speaker")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSubtitleSystemQueueTest, "HorrorProject.Accessibility.SubtitleQueue", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSubtitleSystemQueueTest::RunTest(const FString& Parameters)
{
    USubtitleSystem* SubtitleSystem = NewObject<USubtitleSystem>();
    SubtitleSystem->SetMaxSimultaneousSubtitles(2);

    // Add multiple subtitles
    SubtitleSystem->AddSubtitle(FText::FromString(TEXT("Subtitle 1")), TEXT(""), 3.0f, 1);
    SubtitleSystem->AddSubtitle(FText::FromString(TEXT("Subtitle 2")), TEXT(""), 3.0f, 2);
    SubtitleSystem->AddSubtitle(FText::FromString(TEXT("Subtitle 3")), TEXT(""), 3.0f, 3);

    TArray<FSubtitleEntry> ActiveSubtitles = SubtitleSystem->GetActiveSubtitles();
    TestEqual(TEXT("Queue limited to max simultaneous"), ActiveSubtitles.Num(), 2);

    // Check priority sorting (highest priority first)
    TestEqual(TEXT("Highest priority subtitle first"), ActiveSubtitles[0].Priority, 3);
    TestEqual(TEXT("Second highest priority subtitle second"), ActiveSubtitles[1].Priority, 2);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSubtitleSystemUpdateTest, "HorrorProject.Accessibility.SubtitleUpdate", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSubtitleSystemUpdateTest::RunTest(const FString& Parameters)
{
    USubtitleSystem* SubtitleSystem = NewObject<USubtitleSystem>();

    // Add subtitle with short duration
    SubtitleSystem->AddSubtitle(FText::FromString(TEXT("Short subtitle")), TEXT(""), 1.0f);
    TestEqual(TEXT("Subtitle added"), SubtitleSystem->GetActiveSubtitles().Num(), 1);

    // Update with time passing
    SubtitleSystem->UpdateSubtitles(0.5f);
    TestEqual(TEXT("Subtitle still active after 0.5s"), SubtitleSystem->GetActiveSubtitles().Num(), 1);

    // Update past duration
    SubtitleSystem->UpdateSubtitles(0.6f);
    TestEqual(TEXT("Subtitle removed after duration"), SubtitleSystem->GetActiveSubtitles().Num(), 0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSubtitleSystemClearTest, "HorrorProject.Accessibility.SubtitleClear", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSubtitleSystemClearTest::RunTest(const FString& Parameters)
{
    USubtitleSystem* SubtitleSystem = NewObject<USubtitleSystem>();

    // Add multiple subtitles
    SubtitleSystem->AddSubtitle(FText::FromString(TEXT("Subtitle 1")), TEXT(""), 3.0f);
    SubtitleSystem->AddSubtitle(FText::FromString(TEXT("Subtitle 2")), TEXT(""), 3.0f);
    TestEqual(TEXT("Two subtitles added"), SubtitleSystem->GetActiveSubtitles().Num(), 2);

    // Clear all
    SubtitleSystem->ClearAllSubtitles();
    TestEqual(TEXT("All subtitles cleared"), SubtitleSystem->GetActiveSubtitles().Num(), 0);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSubtitleSystemFormatTest, "HorrorProject.Accessibility.SubtitleFormat", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSubtitleSystemFormatTest::RunTest(const FString& Parameters)
{
    USubtitleSystem* SubtitleSystem = NewObject<USubtitleSystem>();

    // Test formatting with speaker name
    FSubtitleEntry Entry;
    Entry.Text = FText::FromString(TEXT("Hello world"));
    Entry.SpeakerName = TEXT("John");

    FText FormattedText = SubtitleSystem->FormatSubtitleText(Entry);
    TestTrue(TEXT("Formatted text contains speaker name"), FormattedText.ToString().Contains(TEXT("John")));
    TestTrue(TEXT("Formatted text contains subtitle text"), FormattedText.ToString().Contains(TEXT("Hello world")));

    // Test formatting without speaker name
    Entry.SpeakerName = TEXT("");
    FText UnformattedText = SubtitleSystem->FormatSubtitleText(Entry);
    TestEqual(TEXT("Text without speaker matches original"), UnformattedText.ToString(), Entry.Text.ToString());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSubtitleSystemPriorityTest, "HorrorProject.Accessibility.SubtitlePriority", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSubtitleSystemPriorityTest::RunTest(const FString& Parameters)
{
    USubtitleSystem* SubtitleSystem = NewObject<USubtitleSystem>();

    // Add subtitles with different priorities
    SubtitleSystem->AddSubtitle(FText::FromString(TEXT("Low priority")), TEXT(""), 3.0f, 1);
    SubtitleSystem->AddSubtitle(FText::FromString(TEXT("High priority")), TEXT(""), 3.0f, 10);
    SubtitleSystem->AddSubtitle(FText::FromString(TEXT("Medium priority")), TEXT(""), 3.0f, 5);

    TArray<FSubtitleEntry> ActiveSubtitles = SubtitleSystem->GetActiveSubtitles();

    // Check sorting by priority (descending)
    TestTrue(TEXT("Subtitles sorted by priority"),
        ActiveSubtitles[0].Priority >= ActiveSubtitles[1].Priority &&
        ActiveSubtitles[1].Priority >= ActiveSubtitles[2].Priority);

    TestEqual(TEXT("Highest priority first"), ActiveSubtitles[0].Priority, 10);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSubtitleSystemEnvironmentalTest, "HorrorProject.Accessibility.SubtitleEnvironmental", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FSubtitleSystemEnvironmentalTest::RunTest(const FString& Parameters)
{
    USubtitleSystem* SubtitleSystem = NewObject<USubtitleSystem>();

    // Add environmental subtitle
    SubtitleSystem->AddSubtitle(FText::FromString(TEXT("Door creaks")), TEXT(""), 2.0f, 0, true);

    TArray<FSubtitleEntry> ActiveSubtitles = SubtitleSystem->GetActiveSubtitles();
    TestEqual(TEXT("Environmental subtitle added"), ActiveSubtitles.Num(), 1);
    TestTrue(TEXT("Subtitle marked as environmental"), ActiveSubtitles[0].bIsEnvironmental);

    return true;
}
