#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Blueprint/UserWidget.h"
#include "Misc/AutomationTest.h"
#include "UI/EndingCreditsWidget.h"
#include "UI/MainMenuWidget.h"
#include "UI/UIEventManager.h"
#include "UI/UIManagerSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUIEventManagerRegistrationTest,
	"HorrorProject.UI.EventManagerRegistration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUIEventManagerRegistrationTest::RunTest(const FString& Parameters)
{
	UUIEventManager* EventManager = NewObject<UUIEventManager>();
	TestNotNull(TEXT("UI event manager should be constructible."), EventManager);
	if (!EventManager)
	{
		return false;
	}

	const FName WidgetName(TEXT("Inventory"));
	EventManager->RegisterWidget(nullptr, WidgetName);
	TestFalse(TEXT("Null widgets should not be registered."), EventManager->IsWidgetRegistered(WidgetName));

	UUserWidget* Widget = NewObject<UUserWidget>();
	TestNotNull(TEXT("Test widget should be constructible."), Widget);
	if (!Widget)
	{
		return false;
	}

	EventManager->RegisterWidget(Widget, WidgetName);
	TestTrue(TEXT("Valid widgets should be registered."), EventManager->IsWidgetRegistered(WidgetName));
	TestTrue(TEXT("Registered widget should be returned by name."), EventManager->GetRegisteredWidget(WidgetName) == Widget);

	EventManager->UnregisterWidget(WidgetName);
	TestFalse(TEXT("Unregistered widget should no longer be tracked."), EventManager->IsWidgetRegistered(WidgetName));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUIManagerSubsystemStateTest,
	"HorrorProject.UI.ManagerSubsystemState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUIManagerSubsystemStateTest::RunTest(const FString& Parameters)
{
	UUIManagerSubsystem* UIManager = NewObject<UUIManagerSubsystem>();
	TestNotNull(TEXT("UI manager subsystem should be constructible."), UIManager);
	if (!UIManager)
	{
		return false;
	}

	const FName StateName(TEXT("Pause"));
	UIManager->SetUIState(StateName);

	TestEqual(TEXT("UI state should be stored."), UIManager->GetCurrentUIState(), StateName);
	TestNotNull(TEXT("UI manager should provide an event manager for lightweight test instances."), UIManager->GetEventManager());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMainMenuWidgetDefaultsTest,
	"HorrorProject.UI.MainMenuWidgetDefaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FMainMenuWidgetDefaultsTest::RunTest(const FString& Parameters)
{
	UMainMenuWidget* Widget = NewObject<UMainMenuWidget>();
	TestNotNull(TEXT("Main menu widget should be constructible."), Widget);
	if (!Widget)
	{
		return false;
	}

	TestFalse(TEXT("A lightweight main menu widget without a world should report no save game."), Widget->HasAnySaveGame());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEndingCreditsWidgetConstructionTest,
	"HorrorProject.UI.EndingCreditsWidgetConstruction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEndingCreditsWidgetConstructionTest::RunTest(const FString& Parameters)
{
	UEndingCreditsWidget* Widget = NewObject<UEndingCreditsWidget>();
	TestNotNull(TEXT("Ending credits widget should be constructible."), Widget);
	if (!Widget)
	{
		return false;
	}

	Widget->StartCreditsRoll();
	TestTrue(TEXT("Starting credits should be safe for lightweight widget instances."), true);

	return true;
}

#endif
