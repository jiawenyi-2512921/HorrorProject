// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/InteractionComponent.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Game/FoundFootageObjectiveInteractable.h"
#include "Game/HorrorGameModeBase.h"
#include "Game/HorrorCampaignObjectiveActor.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSettings.h"
#include "Interaction/DoorInteractable.h"
#include "Misc/AutomationTest.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/HorrorPlayerController.h"
#include "Player/Components/InventoryComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Tests/AutomationCommon.h"
#include "UI/Day1SliceHUD.h"

namespace
{
	bool ContainsLatinLetter(const FString& Text)
	{
		for (const TCHAR Character : Text)
		{
			if ((Character >= TEXT('A') && Character <= TEXT('Z'))
				|| (Character >= TEXT('a') && Character <= TEXT('z')))
			{
				return true;
			}
		}
		return false;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentBuildsStructuredContextFromCampaignPresentationTest,
	"HorrorProject.Player.Interaction.BuildsStructuredContextFromCampaignPresentation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInteractionComponentBuildsStructuredContextFromCampaignPresentationTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for campaign presentation interaction context coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Campaign context test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Campaign context test should spawn a campaign objective actor."), ObjectiveActor);
	if (!PlayerCharacter || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.CampaignPresentationCircuit");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	Objective.PromptText = FText::FromString(TEXT("接入测试电路"));
	Objective.Presentation.MechanicLabel = FText::FromString(TEXT("电路接线窗口"));
	Objective.Presentation.InputHint = FText::FromString(TEXT("按 A/S/D 接入对应端子。"));
	Objective.Presentation.FailureStakes = FText::FromString(TEXT("接错会回退进度。"));
	Objective.Presentation.RiskLevel = EHorrorCampaignObjectiveRiskLevel::High;
	Objective.Presentation.bUsesFocusedInteraction = true;
	Objective.Presentation.bOpensInteractionPanel = true;
	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	FHitResult Hit;
	Hit.HitObjectHandle = FActorInstanceHandle(ObjectiveActor);
	Hit.Component = ObjectiveActor->GetInteractionBoundsForTests();

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	TestNotNull(TEXT("Campaign context test player should expose interaction."), Interaction);
	if (!Interaction)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FHorrorInteractionContext Context = Interaction->BuildInteractionContextForTests(ObjectiveActor, Hit);
	TestTrue(TEXT("Campaign objective should expose a visible structured interaction context."), Context.bVisible);
	TestTrue(TEXT("Campaign objective presentation should mark panel interactions."), Context.bOpensPanel);
	TestEqual(TEXT("Campaign objective presentation should classify repair interactions."), Context.Verb, EHorrorInteractionVerb::Repair);
	TestEqual(TEXT("Campaign objective presentation should use modal input style."), Context.InputStyle, EHorrorInteractionInputStyle::Modal);
	TestEqual(TEXT("Campaign objective presentation should propagate high risk to interaction context."), Context.RiskLevel, EHorrorInteractionRiskLevel::High);
	TestEqual(TEXT("Campaign objective presentation should provide mechanic label as input text."), Context.InputText.ToString(), FString(TEXT("电路接线窗口")));
	TestEqual(TEXT("Campaign objective presentation should provide detailed input hint."), Context.DetailText.ToString(), FString(TEXT("按 A/S/D 接入对应端子。")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentUsesCampaignObjectiveRuntimeStateTest,
	"HorrorProject.Player.Interaction.CampaignObjectiveContextUsesRuntimeState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInteractionComponentUsesCampaignObjectiveRuntimeStateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for campaign runtime interaction context coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Runtime context test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Runtime context test should spawn a campaign objective actor."), ObjectiveActor);
	if (!PlayerCharacter || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.RuntimeContextCircuit");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	Objective.PromptText = FText::FromString(TEXT("修复冷却泵电路"));
	Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	FHitResult Hit;
	Hit.HitObjectHandle = FActorInstanceHandle(ObjectiveActor);
	Hit.Component = ObjectiveActor->GetInteractionBoundsForTests();

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	TestNotNull(TEXT("Runtime context test player should expose interaction."), Interaction);
	if (!Interaction)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FHorrorInteractionContext IdleContext = Interaction->BuildInteractionContextForTests(ObjectiveActor, Hit);
	TestTrue(TEXT("Default advanced objectives should still advertise a modal panel before interaction."), IdleContext.bOpensPanel);
	TestEqual(TEXT("Default advanced objectives should classify as repair interactions."), IdleContext.Verb, EHorrorInteractionVerb::Repair);
	TestEqual(TEXT("Default advanced objectives should use modal input style."), IdleContext.InputStyle, EHorrorInteractionInputStyle::Modal);
	TestTrue(TEXT("Default advanced objectives should expose localized runtime detail."), IdleContext.DetailText.ToString().Contains(TEXT("等待")) || IdleContext.DetailText.ToString().Contains(TEXT("接线")));

	TestTrue(TEXT("Interacting should open the advanced circuit panel."), ObjectiveActor->Interact_Implementation(PlayerCharacter, Hit));
	ObjectiveActor->Tick(0.9f);

	const FHorrorCampaignObjectiveRuntimeState RuntimeState = ObjectiveActor->BuildObjectiveRuntimeState();
	const FHorrorInteractionContext ActiveContext = Interaction->BuildInteractionContextForTests(ObjectiveActor, Hit);
	TestTrue(TEXT("Active advanced objectives should keep the modal panel advertised."), ActiveContext.bOpensPanel);
	TestTrue(TEXT("Active advanced objective context should remain interactable for panel input."), ActiveContext.bCanInteract);
	TestEqual(TEXT("Active interaction context should mirror runtime progress."), ActiveContext.ProgressFraction, RuntimeState.ProgressFraction);
	TestEqual(TEXT("Active interaction context should mirror runtime target id."), ActiveContext.TargetId, RuntimeState.ObjectiveId);
	TestEqual(TEXT("Active interaction context should mirror runtime phase text."), ActiveContext.DetailText.ToString(), RuntimeState.PhaseText.ToString());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentCampaignObjectiveContextShowsPreEntryPanelHintTest,
	"HorrorProject.Player.Interaction.CampaignObjectiveContextShowsPreEntryPanelHint",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInteractionComponentCampaignObjectiveContextShowsPreEntryPanelHintTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for pre-entry campaign interaction context coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Pre-entry context test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Pre-entry context test should spawn a campaign objective actor."), ObjectiveActor);
	if (!PlayerCharacter || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.PreEntryPanelCircuit");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::RestorePower;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::CircuitWiring;
	Objective.PromptText = FText::FromString(TEXT("重接冷却泵线路"));
	Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	FHitResult Hit;
	Hit.HitObjectHandle = FActorInstanceHandle(ObjectiveActor);
	Hit.Component = ObjectiveActor->GetInteractionBoundsForTests();

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	TestNotNull(TEXT("Pre-entry context test player should expose interaction."), Interaction);
	if (!Interaction)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FHorrorInteractionContext Context = Interaction->BuildInteractionContextForTests(ObjectiveActor, Hit);
	TestTrue(TEXT("Idle advanced objective should expose a visible pre-entry context."), Context.bVisible);
	TestTrue(TEXT("Idle advanced objective should advertise an interaction panel."), Context.bOpensPanel);
	TestEqual(TEXT("Circuit pre-entry context should use repair verb."), Context.Verb, EHorrorInteractionVerb::Repair);
	TestEqual(TEXT("Circuit pre-entry context should use modal input style."), Context.InputStyle, EHorrorInteractionInputStyle::Modal);
	TestTrue(TEXT("Circuit pre-entry input should name the target window."), Context.InputText.ToString().Contains(TEXT("电路接线窗口")));
	TestTrue(TEXT("Circuit pre-entry detail should tell the player how to open the panel."), Context.DetailText.ToString().Contains(TEXT("按互动键打开")));
	TestTrue(TEXT("Circuit pre-entry detail should repeat the target window name."), Context.DetailText.ToString().Contains(TEXT("电路接线窗口")));
	TestFalse(TEXT("Circuit pre-entry context should stay localized."), ContainsLatinLetter(Context.InputText.ToString() + Context.DetailText.ToString()));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentCampaignObjectiveContextSurfacesRetryableFailureTest,
	"HorrorProject.Player.Interaction.CampaignObjectiveContextSurfacesRetryableFailure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInteractionComponentCampaignObjectiveContextSurfacesRetryableFailureTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for retryable campaign interaction context coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Retryable context test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Retryable context test should spawn a campaign objective actor."), ObjectiveActor);
	if (!PlayerCharacter || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.RetryableContextSignal");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::AcquireSignal;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::SignalTuning;
	Objective.PromptText = FText::FromString(TEXT("调谐过载黑盒"));
	Objective.AdvancedInteractionTuning.RequiredStepCount = 3;
	Objective.AdvancedInteractionTuning.SuccessProgress = 0.34f;
	Objective.AdvancedInteractionTuning.FailureStabilityDamage = 1.0f;
	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	FHitResult Hit;
	Hit.HitObjectHandle = FActorInstanceHandle(ObjectiveActor);
	Hit.Component = ObjectiveActor->GetInteractionBoundsForTests();

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	TestNotNull(TEXT("Retryable context test player should expose interaction."), Interaction);
	if (!Interaction)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Opening the signal objective should start advanced interaction."), ObjectiveActor->Interact_Implementation(PlayerCharacter, Hit));
	ObjectiveActor->Tick(0.9f);
	TestTrue(TEXT("Left channel nudge should offset the signal before failure."), ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("左声道"), PlayerCharacter));
	TestTrue(TEXT("Confirming an offset signal should create a retryable failure."), ObjectiveActor->SubmitAdvancedInteractionInput(TEXT("中心频率"), PlayerCharacter));

	const FHorrorCampaignObjectiveRuntimeState RuntimeState = ObjectiveActor->BuildObjectiveRuntimeState();
	TestEqual(TEXT("Runtime state should be retryable before building interaction context."), RuntimeState.Status, EHorrorCampaignObjectiveRuntimeStatus::FailedRetryable);

	const FHorrorInteractionContext Context = Interaction->BuildInteractionContextForTests(ObjectiveActor, Hit);
	TestTrue(TEXT("Retryable campaign objective should keep a visible interaction context."), Context.bVisible);
	TestTrue(TEXT("Retryable campaign objective should stay interactable for restart."), Context.bCanInteract);
	TestTrue(TEXT("Retryable campaign objective should keep the panel affordance."), Context.bOpensPanel);
	TestEqual(TEXT("Retryable signal context should keep recording-style verb."), Context.Verb, EHorrorInteractionVerb::Record);
	TestEqual(TEXT("Retryable signal context should use modal input style."), Context.InputStyle, EHorrorInteractionInputStyle::Modal);
	TestEqual(TEXT("Retryable signal context should preserve target id."), Context.TargetId, RuntimeState.ObjectiveId);
	TestEqual(TEXT("Retryable signal context should mirror failed progress."), Context.ProgressFraction, RuntimeState.ProgressFraction);
	TestTrue(TEXT("Retryable signal context input should tell the player this is a restart."), Context.InputText.ToString().Contains(TEXT("重新互动")) || Context.InputText.ToString().Contains(TEXT("重试")));
	TestTrue(TEXT("Retryable signal context detail should include recovery guidance."), Context.DetailText.ToString().Contains(TEXT("重试")) || Context.DetailText.ToString().Contains(TEXT("重新")));
	TestTrue(TEXT("Retryable signal context should surface overload diagnostics."), Context.DetailText.ToString().Contains(TEXT("过载")) || Context.DetailText.ToString().Contains(TEXT("静音")));
	TestFalse(TEXT("Retryable signal context should remain localized."), ContainsLatinLetter(Context.InputText.ToString() + Context.DetailText.ToString() + Context.BlockedReason.ToString()));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentBuildsStructuredContextForSpectralScanTest,
	"HorrorProject.Player.Interaction.BuildsStructuredContextForSpectralScan",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInteractionComponentBuildsStructuredContextForSpectralScanTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for spectral scan interaction context coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
	TestNotNull(TEXT("Spectral scan context test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Spectral scan context test should spawn a campaign objective actor."), ObjectiveActor);
	if (!PlayerCharacter || !ObjectiveActor)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorCampaignObjectiveDefinition Objective;
	Objective.ObjectiveId = TEXT("Test.CampaignPresentationScan");
	Objective.ObjectiveType = EHorrorCampaignObjectiveType::ScanAnomaly;
	Objective.InteractionMode = EHorrorCampaignInteractionMode::SpectralScan;
	Objective.PromptText = FText::FromString(TEXT("扫描黑盒心跳频率"));
	Objective.Presentation.MechanicLabel = FText::FromString(TEXT("频谱扫描窗口"));
	Objective.Presentation.InputHint = FText::FromString(TEXT("按 A/D 扫频滤噪，再按 S 锁定异常峰。"));
	Objective.Presentation.FailureStakes = FText::FromString(TEXT("锁错波段会让噪点反冲。"));
	Objective.Presentation.RiskLevel = EHorrorCampaignObjectiveRiskLevel::High;
	Objective.Presentation.bUsesFocusedInteraction = true;
	Objective.Presentation.bOpensInteractionPanel = true;
	ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

	FHitResult Hit;
	Hit.HitObjectHandle = FActorInstanceHandle(ObjectiveActor);
	Hit.Component = ObjectiveActor->GetInteractionBoundsForTests();

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	TestNotNull(TEXT("Spectral scan context test player should expose interaction."), Interaction);
	if (!Interaction)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FHorrorInteractionContext Context = Interaction->BuildInteractionContextForTests(ObjectiveActor, Hit);
	TestTrue(TEXT("Spectral scan campaign objective should expose a visible structured interaction context."), Context.bVisible);
	TestTrue(TEXT("Spectral scan should mark panel interactions."), Context.bOpensPanel);
	TestEqual(TEXT("Spectral scan should classify as a recording-style interaction."), Context.Verb, EHorrorInteractionVerb::Record);
	TestEqual(TEXT("Spectral scan should use modal input style."), Context.InputStyle, EHorrorInteractionInputStyle::Modal);
	TestEqual(TEXT("Spectral scan should provide mechanic label as input text."), Context.InputText.ToString(), FString(TEXT("频谱扫描窗口")));
	TestTrue(TEXT("Spectral scan should expose localized sweep-and-lock detail."), Context.DetailText.ToString().Contains(TEXT("扫频")) && Context.DetailText.ToString().Contains(TEXT("锁定")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentBuildsStructuredContextForPressureWindowsTest,
	"HorrorProject.Player.Interaction.BuildsStructuredContextForPressureWindows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInteractionComponentBuildsStructuredContextForPressureWindowsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for pressure window interaction context coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	TestNotNull(TEXT("Pressure window context test should spawn a player character."), PlayerCharacter);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	TestNotNull(TEXT("Pressure window context test player should expose interaction."), Interaction);
	if (!Interaction)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	struct FPressureContextCase
	{
		EHorrorCampaignObjectiveType ObjectiveType = EHorrorCampaignObjectiveType::PlantBeacon;
		FName ObjectiveId = NAME_None;
		const TCHAR* PromptText = TEXT("");
		EHorrorInteractionVerb ExpectedVerb = EHorrorInteractionVerb::Interact;
		const TCHAR* ExpectedInputTextFragment = TEXT("");
	};

	const FPressureContextCase Cases[] = {
		{
			EHorrorCampaignObjectiveType::PlantBeacon,
			TEXT("Test.ContextBeaconWindow"),
			TEXT("锚定测试信标"),
			EHorrorInteractionVerb::Record,
			TEXT("调谐")
		},
		{
			EHorrorCampaignObjectiveType::BossWeakPoint,
			TEXT("Test.ContextWeakPointWindow"),
			TEXT("压制测试弱点"),
			EHorrorInteractionVerb::Record,
			TEXT("频谱")
		}
	};

	for (const FPressureContextCase& ContextCase : Cases)
	{
		AHorrorCampaignObjectiveActor* ObjectiveActor = World->SpawnActor<AHorrorCampaignObjectiveActor>();
		TestNotNull(
			FString::Printf(TEXT("Pressure window context test should spawn objective actor for %s."), *ContextCase.ObjectiveId.ToString()),
			ObjectiveActor);
		if (!ObjectiveActor)
		{
			TestWorld.DestroyTestWorld(false);
			return false;
		}

		FHorrorCampaignObjectiveDefinition Objective;
		Objective.ObjectiveId = ContextCase.ObjectiveId;
		Objective.ObjectiveType = ContextCase.ObjectiveType;
		Objective.PromptText = FText::FromString(ContextCase.PromptText);
		ObjectiveActor->ConfigureObjective(TEXT("Chapter.Test"), Objective);

		FHitResult Hit;
		Hit.HitObjectHandle = FActorInstanceHandle(ObjectiveActor);
		Hit.Component = ObjectiveActor->GetInteractionBoundsForTests();

		const FHorrorInteractionContext Context = Interaction->BuildInteractionContextForTests(ObjectiveActor, Hit);
		TestTrue(
			FString::Printf(TEXT("Pressure objective should expose a visible context for %s."), *ContextCase.ObjectiveId.ToString()),
			Context.bVisible);
		TestTrue(
			FString::Printf(TEXT("Pressure objective should open a panel for %s."), *ContextCase.ObjectiveId.ToString()),
			Context.bOpensPanel);
		TestEqual(
			FString::Printf(TEXT("Pressure objective should use an evidence/recording verb for %s."), *ContextCase.ObjectiveId.ToString()),
			Context.Verb,
			ContextCase.ExpectedVerb);
		TestEqual(
			FString::Printf(TEXT("Pressure objective should use modal input style for %s."), *ContextCase.ObjectiveId.ToString()),
			Context.InputStyle,
			EHorrorInteractionInputStyle::Modal);
		TestTrue(
			FString::Printf(TEXT("Pressure objective should name its concrete window for %s."), *ContextCase.ObjectiveId.ToString()),
			Context.InputText.ToString().Contains(ContextCase.ExpectedInputTextFragment));
		TestFalse(
			FString::Printf(TEXT("Pressure objective input text should remain localized for %s."), *ContextCase.ObjectiveId.ToString()),
			ContainsLatinLetter(Context.InputText.ToString()));
	}

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentPrioritizesCampaignNavigationWhenRouteKitExistsTest,
	"HorrorProject.Player.Navigation.PrioritizesCampaignWhenRouteKitExists",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInteractionComponentPrioritizesCampaignNavigationWhenRouteKitExistsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for navigation priority coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	AHorrorGameModeBase* GameMode = World->SpawnActor<AHorrorGameModeBase>();
	TestNotNull(TEXT("Navigation priority test should spawn a player controller."), PlayerController);
	TestNotNull(TEXT("Navigation priority test should spawn a campaign game mode."), GameMode);
	if (!PlayerController || !GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->ResetCampaignProgressForChapterForTests(TEXT("Chapter.DeepWaterStationFinale"));
	GameMode->ImportDay1CompleteState(false);
	FString ErrorMessage;
	GameMode->InitGame(TEXT("/Game/DeepWaterStation/Maps/DemoMap_VerticalSlice_Day1"), TEXT(""), ErrorMessage);
	TestFalse(
		TEXT("Day1 found-footage flow should keep campaign navigation hidden until the Day1 arc is complete."),
		PlayerController->ShouldPrioritizeCampaignNavigationForTests(GameMode));

	GameMode->ImportDay1CompleteState(true);
	TestTrue(
		TEXT("Campaign navigation should become available again once the Day1 flow is complete."),
		PlayerController->ShouldPrioritizeCampaignNavigationForTests(GameMode));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentBuildsStructuredContextForPasswordDoorTest,
	"HorrorProject.Player.Interaction.BuildsStructuredContextForPasswordDoor",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInteractionComponentBuildsStructuredContextForPasswordDoorTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for password interaction context coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	ADoorInteractable* Door = World->SpawnActor<ADoorInteractable>();
	TestNotNull(TEXT("Password context test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Password context test should spawn a door."), Door);
	if (!PlayerCharacter || !Door)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Door->ConfigurePassword(TEXT("1697"), FText::FromString(TEXT("备忘录上的四位数。")));

	FHitResult DoorHit;
	DoorHit.HitObjectHandle = FActorInstanceHandle(Door);

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	TestNotNull(TEXT("Password context test player should expose interaction."), Interaction);
	if (!Interaction)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorInteractionContext Context = Interaction->BuildInteractionContextForTests(Door, DoorHit);
	TestTrue(TEXT("Password door should expose a visible interaction context."), Context.bVisible);
	TestTrue(TEXT("Password door should be interactable so it can open the keypad panel."), Context.bCanInteract);
	TestTrue(TEXT("Password door should open a modal panel."), Context.bOpensPanel);
	TestEqual(TEXT("Password door should classify its verb as code entry."), Context.Verb, EHorrorInteractionVerb::EnterCode);
	TestEqual(TEXT("Password door should classify input as modal."), Context.InputStyle, EHorrorInteractionInputStyle::Modal);
	TestEqual(TEXT("Password door should carry a medium risk warning."), Context.RiskLevel, EHorrorInteractionRiskLevel::Medium);
	TestTrue(TEXT("Password door should surface password hint detail."), Context.DetailText.ToString().Contains(TEXT("备忘录上的四位数")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentBuildsStructuredContextForBlockedRecordingTest,
	"HorrorProject.Player.Interaction.BuildsStructuredContextForBlockedRecording",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInteractionComponentBuildsStructuredContextForBlockedRecordingTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for structured interaction context coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Day1 game mode for structured context coverage."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	AFoundFootageObjectiveInteractable* Recorder = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	TestNotNull(TEXT("Structured context test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Structured context test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Structured context test should spawn the recorder objective."), Recorder);
	if (!GameMode || !PlayerCharacter || !Recorder)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	Recorder->Objective = EFoundFootageInteractableObjective::FirstAnomalyRecord;
	Recorder->SourceId = TEXT("Evidence.Recorder");
	Recorder->bIsRecordingForFirstAnomalyRecord = true;

	FHitResult ObjectiveHit;
	ObjectiveHit.HitObjectHandle = FActorInstanceHandle(Recorder);
	ObjectiveHit.Component = Recorder->FindComponentByClass<UBoxComponent>();

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	TestNotNull(TEXT("Player should expose an interaction component."), Interaction);
	if (!Interaction)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	FHorrorInteractionContext Context = Interaction->BuildInteractionContextForTests(Recorder, ObjectiveHit);
	TestTrue(TEXT("Blocked recorder should still expose a visible structured interaction context."), Context.bVisible);
	TestFalse(TEXT("Blocked recorder should mark the action as currently unavailable."), Context.bCanInteract);
	TestTrue(TEXT("Blocked recorder should flag recording-dependent interaction."), Context.bRequiresRecording);
	TestEqual(TEXT("Blocked recorder should classify its verb as recording."), Context.Verb, EHorrorInteractionVerb::Record);
	TestEqual(TEXT("Blocked recorder should classify input as record aiming."), Context.InputStyle, EHorrorInteractionInputStyle::RecordAim);
	TestEqual(TEXT("Blocked recorder should carry high interaction risk."), Context.RiskLevel, EHorrorInteractionRiskLevel::High);
	TestEqual(TEXT("Blocked recorder should expose the source id as the target id."), Context.TargetId, FName(TEXT("Evidence.Recorder")));
	TestEqual(TEXT("Blocked recorder should explain the missing anomaly candidate."), Context.BlockedReason.ToString(), FString(TEXT("先对准异常。")));

	TestFalse(TEXT("Blocked recorder should reject interaction before candidate alignment."), Interaction->TryInteractWithHit(ObjectiveHit));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentBlockedObjectiveShowsHUDReasonTest,
	"HorrorProject.Player.Interaction.BlockedObjectiveCanInteractFalseShowsHUDReason",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInteractionComponentBlockedObjectiveShowsHUDReasonTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for blocked interaction feedback coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Day1 game mode for interaction feedback."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();

	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	AFoundFootageObjectiveInteractable* Recorder = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	TestNotNull(TEXT("Blocked interaction test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Blocked interaction test should spawn a player controller."), PlayerController);
	TestNotNull(TEXT("Blocked interaction test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Blocked interaction test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Blocked interaction test should spawn the blocked objective."), Recorder);
	if (!GameMode || !PlayerController || !HUD || !PlayerCharacter || !Recorder)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);

	Recorder->Objective = EFoundFootageInteractableObjective::FirstAnomalyRecord;
	Recorder->SourceId = TEXT("Evidence.Recorder");
	Recorder->bIsRecordingForFirstAnomalyRecord = true;

	FHitResult ObjectiveHit;
	ObjectiveHit.HitObjectHandle = FActorInstanceHandle(Recorder);
	ObjectiveHit.Component = Recorder->FindComponentByClass<UBoxComponent>();

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	TestNotNull(TEXT("Player should expose an interaction component."), Interaction);
	if (!Interaction)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestFalse(TEXT("Blocked anomaly recording should reject interaction before a candidate exists."), Interaction->TryInteractWithHit(ObjectiveHit));
	TestFalse(TEXT("Rejected interaction should not record the first anomaly."), GameMode->HasRecordedFirstAnomaly());
	TestTrue(TEXT("Rejected interaction should show a transient HUD reason."), HUD->IsTransientMessageVisibleForTests());
	TestEqual(
		TEXT("Rejected interaction should explain the missing anomaly candidate."),
		HUD->GetTransientMessageForTests().ToString(),
		FString(TEXT("先对准异常。")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FInteractionComponentRecordsAnomalyFromFocusedCandidateTest,
	"HorrorProject.Player.Interaction.RecordsAnomalyFromFocusedCandidate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInteractionComponentRecordsAnomalyFromFocusedCandidateTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for focused anomaly recording coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Day1 game mode for focused anomaly recording."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>();
	AFoundFootageObjectiveInteractable* AnomalyCandidate = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	TestNotNull(TEXT("Focused anomaly test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Focused anomaly test should spawn a player controller."), PlayerController);
	TestNotNull(TEXT("Focused anomaly test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Focused anomaly test should spawn an anomaly candidate."), AnomalyCandidate);
	if (!GameMode || !PlayerController || !PlayerCharacter || !AnomalyCandidate)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->Possess(PlayerCharacter);

	const FName AnomalyId(TEXT("Evidence.Anomaly01"));
	AnomalyCandidate->Objective = EFoundFootageInteractableObjective::FirstAnomalyCandidate;
	AnomalyCandidate->SourceId = AnomalyId;
	AnomalyCandidate->EvidenceMetadata.EvidenceId = AnomalyId;
	AnomalyCandidate->EvidenceMetadata.DisplayName = FText::FromString(TEXT("First Anomaly"));

	FHitResult ObjectiveHit;
	ObjectiveHit.HitObjectHandle = FActorInstanceHandle(AnomalyCandidate);
	ObjectiveHit.Component = AnomalyCandidate->FindComponentByClass<UBoxComponent>();

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	UInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent();
	TestNotNull(TEXT("Focused anomaly test player should expose interaction."), Interaction);
	TestNotNull(TEXT("Focused anomaly test player should expose quantum camera."), QuantumCamera);
	TestNotNull(TEXT("Focused anomaly test player should expose inventory."), Inventory);
	if (!Interaction || !QuantumCamera || !Inventory)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Bodycam acquisition should unlock the first note."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	TestTrue(TEXT("First note collection should unlock anomaly alignment."), GameMode->TryCollectFirstNote(TEXT("Note.Intro")));
	TestTrue(TEXT("First interaction with the focused anomaly should register the candidate."), Interaction->TryInteractWithHit(ObjectiveHit));
	TestTrue(TEXT("Focused anomaly should now be pending."), GameMode->HasPendingFirstAnomalyCandidate());
	TestFalse(TEXT("Focused anomaly should not be recorded before camera recording."), GameMode->HasRecordedFirstAnomaly());

	QuantumCamera->SetCameraAcquired(true);
	TestTrue(TEXT("Focused anomaly test should have an acquired camera."), QuantumCamera->IsCameraAcquired());
	QuantumCamera->SetCameraEnabled(true);
	TestTrue(TEXT("Focused anomaly test should have an enabled camera."), QuantumCamera->IsCameraEnabled());
	TestTrue(TEXT("Focused anomaly test should start recording."), QuantumCamera->StartRecording());
	TestTrue(TEXT("Second interaction with the same focused anomaly should record it while filming."), Interaction->TryInteractWithHit(ObjectiveHit));
	TestTrue(TEXT("Focused anomaly recording should complete the first anomaly objective."), GameMode->HasRecordedFirstAnomaly());
	TestFalse(TEXT("Focused anomaly recording should clear the pending candidate."), GameMode->HasPendingFirstAnomalyCandidate());
	TestTrue(TEXT("Focused anomaly recording should write anomaly evidence to inventory."), Inventory->HasCollectedEvidenceId(AnomalyId));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
