#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Analytics/AnalyticsSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAnalyticsUserIdAnonymizationUsesExpectedHashTest, "HorrorProject.Analytics.UserIdAnonymizationUsesExpectedHash", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAnalyticsUserIdAnonymizationUsesExpectedHashTest::RunTest(const FString& Parameters)
{
	UAnalyticsSubsystem* Analytics = NewObject<UAnalyticsSubsystem>();

	const FString UserId = TEXT("user@example.com");
	const FString DifferentUserId = TEXT("other-user@example.com");
	const FString ExpectedSHA256 = TEXT("b4c9a289323b21a01c3e940f150eb9b8c542587f1abfd8f0e1cc1ffc5e475514");

	Analytics->SetUserId(UserId, true);
	const FString FirstAnonymizedId = Analytics->GetAnonymizedUserId();

	Analytics->SetUserId(UserId, true);
	const FString SecondAnonymizedId = Analytics->GetAnonymizedUserId();

	Analytics->SetUserId(DifferentUserId, true);
	const FString DifferentAnonymizedId = Analytics->GetAnonymizedUserId();

	TestEqual(TEXT("Anonymized ID should be a 64-character SHA-256 hex digest"), FirstAnonymizedId.Len(), 64);
	TestNotEqual(TEXT("Anonymized ID should differ from original"), FirstAnonymizedId, UserId);
	TestEqual(TEXT("Anonymized ID should be stable for the same user"), FirstAnonymizedId, SecondAnonymizedId);
	TestNotEqual(TEXT("Different users should produce different anonymized IDs"), FirstAnonymizedId, DifferentAnonymizedId);
	TestEqual(TEXT("Anonymized ID should match the expected SHA-256 digest"), FirstAnonymizedId, ExpectedSHA256);

	for (const TCHAR Character : FirstAnonymizedId)
	{
		const bool bIsHexDigit =
			(Character >= TEXT('0') && Character <= TEXT('9')) ||
			(Character >= TEXT('a') && Character <= TEXT('f')) ||
			(Character >= TEXT('A') && Character <= TEXT('F'));

		TestTrue(TEXT("Anonymized ID should contain only hexadecimal characters"), bIsHexDigit);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS
