// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TestHelpers.h"
#include "Tests/HorrorTestMacros.h"
#include "Game/HorrorEventBusSubsystem.h"
#include "Blueprint/UserWidget.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUISystemIntegrationTest,
    "HorrorProject.Integration.Systems.UI",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FUISystemIntegrationTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test 1: UI state transitions
    FString CurrentUIState = "None";

    EventBus->Subscribe("UI.StateChanged", [&CurrentUIState](const FHorrorEventPayload& Payload) {
        CurrentUIState = Payload.StringData.FindRef("NewState");
    });

    TArray<FString> UIStates = {"MainMenu", "InGame", "Pause", "Inventory", "InGame"};
    for (const FString& State : UIStates)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "UI.StateChanged";
        Payload.StringData.Add("NewState", State);
        EventBus->Publish(Payload);

        TestEqual(FString::Printf(TEXT("UI state: %s"), *State), CurrentUIState, State);
    }

    // Test 2: HUD updates
    int32 HealthValue = 100;
    int32 EvidenceCount = 0;
    float BatteryLevel = 1.0f;

    EventBus->Subscribe("UI.HUD.Update", [&](const FHorrorEventPayload& Payload) {
        HealthValue = Payload.IntData.FindRef("Health");
        EvidenceCount = Payload.IntData.FindRef("Evidence");
        BatteryLevel = Payload.FloatData.FindRef("Battery");
    });

    FHorrorEventPayload HUDPayload;
    HUDPayload.EventName = "UI.HUD.Update";
    HUDPayload.IntData.Add("Health", 75);
    HUDPayload.IntData.Add("Evidence", 5);
    HUDPayload.FloatData.Add("Battery", 0.6f);
    EventBus->Publish(HUDPayload);

    TestEqual("Health updated", HealthValue, 75);
    TestEqual("Evidence updated", EvidenceCount, 5);
    TestTrue("Battery updated", FMath::IsNearlyEqual(BatteryLevel, 0.6f, 0.01f));

    // Test 3: Notification system
    TArray<FString> Notifications;

    EventBus->Subscribe("UI.Notification.Show", [&Notifications](const FHorrorEventPayload& Payload) {
        Notifications.Add(Payload.StringData.FindRef("Message"));
    });

    TArray<FString> TestNotifications = {
        "Objective completed",
        "Evidence collected",
        "Battery low"
    };

    for (const FString& Notification : TestNotifications)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "UI.Notification.Show";
        Payload.StringData.Add("Message", Notification);
        EventBus->Publish(Payload);
    }

    TestEqual("All notifications received", Notifications.Num(), TestNotifications.Num());

    // Test 4: Menu navigation
    TArray<FString> MenuStack;

    EventBus->Subscribe("UI.Menu.Push", [&MenuStack](const FHorrorEventPayload& Payload) {
        MenuStack.Add(Payload.StringData.FindRef("MenuName"));
    });

    EventBus->Subscribe("UI.Menu.Pop", [&MenuStack](const FHorrorEventPayload& Payload) {
        if (MenuStack.Num() > 0)
        {
            MenuStack.RemoveAt(MenuStack.Num() - 1);
        }
    });

    // Push menus
    FHorrorEventPayload PushPayload;
    PushPayload.EventName = "UI.Menu.Push";
    PushPayload.StringData.Add("MenuName", "Settings");
    EventBus->Publish(PushPayload);

    TestEqual("Menu pushed", MenuStack.Num(), 1);

    PushPayload.StringData["MenuName"] = "Audio";
    EventBus->Publish(PushPayload);

    TestEqual("Second menu pushed", MenuStack.Num(), 2);

    // Pop menu
    FHorrorEventPayload PopPayload;
    PopPayload.EventName = "UI.Menu.Pop";
    EventBus->Publish(PopPayload);

    TestEqual("Menu popped", MenuStack.Num(), 1);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUIInteractionTest,
    "HorrorProject.Integration.Systems.UIInteraction",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FUIInteractionTest::RunTest(const FString& Parameters)
{
    using namespace HorrorTestHelpers;
    FTestWorldScope TestWorld;
    HORROR_TEST_REQUIRE_WORLD(TestWorld.GetWorld(), TestWorld, "Test world creation failed");

    UWorld* World = TestWorld.GetWorld();
    UHorrorEventBusSubsystem* EventBus = World->GetSubsystem<UHorrorEventBusSubsystem>();
    HORROR_TEST_REQUIRE(EventBus != nullptr, "EventBus not found");

    // Test button interactions
    int32 ButtonClicks = 0;
    EventBus->Subscribe("UI.Button.Clicked", [&ButtonClicks](const FHorrorEventPayload& Payload) {
        ButtonClicks++;
    });

    for (int32 i = 0; i < 5; ++i)
    {
        FHorrorEventPayload Payload;
        Payload.EventName = "UI.Button.Clicked";
        Payload.StringData.Add("ButtonID", FString::Printf(TEXT("Button_%d"), i));
        EventBus->Publish(Payload);
    }

    TestEqual("Button clicks registered", ButtonClicks, 5);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
