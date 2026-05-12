// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/UIManagerSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUIManagerSubsystemProvidesEventManagerBeforeInitializeTest,
	"HorrorProject.UI.UIManagerSubsystem.ProvidesEventManagerBeforeInitialize",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUIManagerSubsystemProvidesEventManagerBeforeInitializeTest::RunTest(const FString& Parameters)
{
	UUIManagerSubsystem* UIManager = NewObject<UUIManagerSubsystem>();
	TestNotNull(TEXT("UI manager subsystem should be constructible in lightweight tests."), UIManager);
	if (!UIManager)
	{
		return false;
	}

	TestNotNull(
		TEXT("UI manager subsystem should provide an event manager before GameInstance Initialize is called."),
		UIManager->GetEventManager());
	if (!UIManager->GetEventManager())
	{
		return false;
	}

	const FName ExpectedState(TEXT("Pause"));
	UIManager->SetUIState(ExpectedState);

	TestEqual(TEXT("UI manager should store the requested UI state."), UIManager->GetCurrentUIState(), ExpectedState);
	TestNotNull(TEXT("UI manager should keep its event manager after state changes."), UIManager->GetEventManager());

	return true;
}

#endif
