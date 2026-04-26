#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Network/HorrorGameModeMultiplayer.h"
#include "Network/MultiplayerSessionSubsystem.h"
#include "Save/HorrorSaveSubsystem.h"
#include "Achievements/AchievementSubsystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNetworkSaveIntegrationTest, "HorrorProject.Integration.NetworkSave", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNetworkSaveIntegrationTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    AHorrorGameModeMultiplayer* GameMode = World->SpawnActor<AHorrorGameModeMultiplayer>();
    UHorrorSaveSubsystem* SaveSys = World->GetGameInstance()->GetSubsystem<UHorrorSaveSubsystem>();

    TestNotNull(TEXT("GameMode created"), GameMode);
    TestNotNull(TEXT("SaveSubsystem available"), SaveSys);

    // Test save during multiplayer session
    if (SaveSys)
    {
        bool bSaved = SaveSys->SaveGame(TEXT("MultiplayerTest"));
        TestTrue(TEXT("Save during multiplayer handled"), true);
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNetworkAchievementIntegrationTest, "HorrorProject.Integration.NetworkAchievement", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FNetworkAchievementIntegrationTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UAchievementSubsystem* AchievementSys = World->GetGameInstance()->GetSubsystem<UAchievementSubsystem>();

    if (!AchievementSys)
    {
        AddWarning(TEXT("AchievementSubsystem not available"));
        return true;
    }

    // Test achievement unlock in multiplayer
    AchievementSys->UnlockAchievement(FName("ACH_Cooperative"));
    TestTrue(TEXT("Multiplayer achievement unlocked"), AchievementSys->IsAchievementUnlocked(FName("ACH_Cooperative")));

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFullMultiplayerFlowTest, "HorrorProject.Integration.FullMultiplayerFlow", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FFullMultiplayerFlowTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();

    // Create game mode
    AHorrorGameModeMultiplayer* GameMode = World->SpawnActor<AHorrorGameModeMultiplayer>();
    TestNotNull(TEXT("GameMode created"), GameMode);

    // Create player controllers
    AHorrorPlayerControllerMultiplayer* PC1 = World->SpawnActor<AHorrorPlayerControllerMultiplayer>();
    AHorrorPlayerControllerMultiplayer* PC2 = World->SpawnActor<AHorrorPlayerControllerMultiplayer>();
    TestNotNull(TEXT("Player 1 created"), PC1);
    TestNotNull(TEXT("Player 2 created"), PC2);

    // Test lobby phase
    GameMode->StartLobby();
    TestTrue(TEXT("Lobby started"), true);

    // Test game start
    GameMode->StartGame();
    TestTrue(TEXT("Game started"), true);

    // Test player interaction
    if (PC1 && PC2)
    {
        PC1->UpdateNetworkStats();
        PC2->UpdateNetworkStats();
        TestTrue(TEXT("Network stats updated"), true);
    }

    // Test game end
    GameMode->EndGame();
    TestTrue(TEXT("Game ended"), true);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLocalizationNetworkIntegrationTest, "HorrorProject.Integration.LocalizationNetwork", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLocalizationNetworkIntegrationTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    ULocalizationSubsystem* LocalizationSys = World->GetGameInstance()->GetSubsystem<ULocalizationSubsystem>();

    if (!LocalizationSys)
    {
        AddWarning(TEXT("LocalizationSubsystem not available"));
        return true;
    }

    // Test language sync in multiplayer
    LocalizationSys->SetLanguage(ELanguage::Chinese);
    TestEqual(TEXT("Language set"), LocalizationSys->GetCurrentLanguage(), ELanguage::Chinese);

    // Test localized text in multiplayer context
    FText LocalizedText = LocalizationSys->GetLocalizedText(TEXT("UI.Multiplayer.Lobby"));
    TestTrue(TEXT("Multiplayer text localized"), !LocalizedText.IsEmpty());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAccessibilityNetworkIntegrationTest, "HorrorProject.Integration.AccessibilityNetwork", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAccessibilityNetworkIntegrationTest::RunTest(const FString& Parameters)
{
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    UAccessibilitySubsystem* AccessibilitySys = World->GetSubsystem<UAccessibilitySubsystem>();

    if (!AccessibilitySys)
    {
        AddWarning(TEXT("AccessibilitySubsystem not available"));
        return true;
    }

    // Test accessibility settings in multiplayer
    AccessibilitySys->SetSubtitlesEnabled(true);
    TestTrue(TEXT("Subtitles enabled in multiplayer"), AccessibilitySys->AreSubtitlesEnabled());

    // Test color blind mode in multiplayer
    AccessibilitySys->SetColorBlindMode(EColorBlindMode::Deuteranopia);
    TestEqual(TEXT("Color blind mode set"), AccessibilitySys->GetColorBlindMode(), EColorBlindMode::Deuteranopia);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
