// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HorrorPlayerController.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Camera/CameraComponent.h"
#include "Game/DeepWaterStationRouteKit.h"
#include "Game/FoundFootageObjectiveInteractable.h"
#include "Game/HorrorGameModeBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/AutomationTest.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/InteractionComponent.h"
#include "Player/Components/InventoryComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Tests/AutomationCommon.h"
#include "UI/Day1SliceHUD.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerAutoCapturesFocusedAnomalyWhileRecordingTest,
	"HorrorProject.Player.Controller.AutoCapturesFocusedAnomalyWhileRecording",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerAutoCapturesFocusedAnomalyWhileRecordingTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for focused anomaly auto-capture."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Day1 game mode for focused anomaly auto-capture."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector::ZeroVector, FRotator::ZeroRotator);
	AFoundFootageObjectiveInteractable* AnomalyCandidate = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	TestNotNull(TEXT("Auto-capture test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Auto-capture test should spawn a player controller."), PlayerController);
	TestNotNull(TEXT("Auto-capture test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Auto-capture test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Auto-capture test should spawn an anomaly candidate."), AnomalyCandidate);
	if (!GameMode || !PlayerController || !HUD || !PlayerCharacter || !AnomalyCandidate)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);
	PlayerController->SetControlRotation(FRotator::ZeroRotator);

	const FName AnomalyId(TEXT("Evidence.Anomaly01"));
	AnomalyCandidate->Objective = EFoundFootageInteractableObjective::FirstAnomalyCandidate;
	AnomalyCandidate->SourceId = AnomalyId;
	AnomalyCandidate->EvidenceMetadata.EvidenceId = AnomalyId;
	AnomalyCandidate->EvidenceMetadata.DisplayName = FText::FromString(TEXT("第一异常"));
	AnomalyCandidate->DebugLabel = FText::FromString(TEXT("第一个异常"));

	const UCameraComponent* FirstPersonCamera = PlayerCharacter->GetFirstPersonCameraComponent();
	TestNotNull(TEXT("Auto-capture player should expose a first-person camera."), FirstPersonCamera);
	if (!FirstPersonCamera)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FVector CandidateLocation = FirstPersonCamera->GetComponentLocation() + FirstPersonCamera->GetForwardVector() * 220.0f;
	AnomalyCandidate->SetActorLocation(CandidateLocation);
	AnomalyCandidate->RefreshVisualDefaults();

	UInteractionComponent* Interaction = PlayerCharacter->GetInteractionComponent();
	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	UInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent();
	TestNotNull(TEXT("Auto-capture player should expose interaction."), Interaction);
	TestNotNull(TEXT("Auto-capture player should expose quantum camera."), QuantumCamera);
	TestNotNull(TEXT("Auto-capture player should expose inventory."), Inventory);
	if (!Interaction || !QuantumCamera || !Inventory)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	TestTrue(TEXT("Bodycam acquisition should unlock the first note."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	TestTrue(TEXT("First note collection should unlock anomaly capture."), GameMode->TryCollectFirstNote(TEXT("Note.Intro")));
	TestTrue(TEXT("Auto-capture camera should be acquired by the objective state."), QuantumCamera->IsCameraAcquired());
	TestTrue(TEXT("Auto-capture camera should be enabled by the objective state."), QuantumCamera->IsCameraEnabled());
	TestFalse(TEXT("Focused anomaly should not be recorded before the controller update."), GameMode->HasRecordedFirstAnomaly());

	UObject* FocusedTarget = nullptr;
	FHitResult FocusedHit;
	TestTrue(TEXT("Auto-capture setup should place the anomaly under the player's camera trace."), Interaction->FindFocusedInteractable(FocusedHit, FocusedTarget));
	TestEqual(TEXT("Focused anomaly trace should resolve the candidate actor."), Cast<AActor>(FocusedTarget), Cast<AActor>(AnomalyCandidate));
	TestTrue(TEXT("Focused anomaly should remain tracked after it becomes the pending candidate."), GameMode->BeginFirstAnomalyCandidate(AnomalyId));
	TestTrue(TEXT("Focused anomaly should now be pending before recording starts."), GameMode->HasPendingFirstAnomalyCandidate());

	PlayerController->RefreshDay1HUDStateForTests();
	TestTrue(TEXT("Focused anomaly should show capture assistance in the HUD before recording."), HUD->IsAnomalyCaptureStatusVisibleForTests());
	TestEqual(
		TEXT("Focused anomaly capture assistance should explain the next action."),
		HUD->GetAnomalyCaptureStatusForTests().ToString(),
		FString(TEXT("异常已对准，开启录像锁定。")));
	TestTrue(TEXT("Focused anomaly capture assistance should require recording."), HUD->DoesAnomalyCaptureRequireRecordingForTests());
	TestFalse(TEXT("Focused anomaly capture assistance should not claim the clip is locked before recording."), HUD->IsAnomalyCaptureLockedForTests());

	TestTrue(TEXT("Auto-capture camera should start recording."), QuantumCamera->StartRecording());
	TestFalse(TEXT("Initial auto-capture runtime update should only begin the sustained lock."), PlayerController->UpdateDay1RuntimeStateForTests(0.016f));

	TestFalse(TEXT("Recording should require a sustained anomaly lock before completing."), GameMode->HasRecordedFirstAnomaly());
	TestTrue(TEXT("Sustained anomaly lock should show capture assistance while recording."), HUD->IsAnomalyCaptureStatusVisibleForTests());
	TestTrue(TEXT("Sustained anomaly lock should advance capture progress while recording."), HUD->GetAnomalyCaptureProgressForTests() > 0.0f);

	TestTrue(TEXT("Sustained auto-capture runtime update should complete the focused anomaly."), PlayerController->UpdateDay1RuntimeStateForTests(1.0f));

	TestTrue(TEXT("Recording while focused on the first anomaly should auto-complete the capture."), GameMode->HasRecordedFirstAnomaly());
	TestFalse(TEXT("Auto-capture should clear the pending anomaly candidate."), GameMode->HasPendingFirstAnomalyCandidate());
	TestTrue(TEXT("Auto-capture should write the anomaly evidence to the player inventory."), Inventory->HasCollectedEvidenceId(AnomalyId));
	TestTrue(TEXT("Auto-capture should show a confirmation message in the native HUD."), HUD->IsTransientMessageVisibleForTests());
	TestEqual(
		TEXT("Auto-capture confirmation should be Chinese and explicit."),
		HUD->GetTransientMessageForTests().ToString(),
		FString(TEXT("异常录像已锁定。")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerAutoCapturesRouteKitAnomalyWhileRecordingTest,
	"HorrorProject.Player.Controller.AutoCapturesRouteKitAnomalyWhileRecording",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerAutoCapturesRouteKitAnomalyWhileRecordingTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for route-kit anomaly auto-capture."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Day1 game mode for route-kit anomaly auto-capture."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector::ZeroVector, FRotator::ZeroRotator);
	ADeepWaterStationRouteKit* RouteKit = World->SpawnActor<ADeepWaterStationRouteKit>();
	TestNotNull(TEXT("Route-kit auto-capture should expose the game mode."), GameMode);
	TestNotNull(TEXT("Route-kit auto-capture should spawn a player controller."), PlayerController);
	TestNotNull(TEXT("Route-kit auto-capture should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Route-kit auto-capture should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Route-kit auto-capture should spawn a route kit."), RouteKit);
	if (!GameMode || !PlayerController || !HUD || !PlayerCharacter || !RouteKit)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);
	PlayerController->SetControlRotation(FRotator::ZeroRotator);

	const UCameraComponent* FirstPersonCamera = PlayerCharacter->GetFirstPersonCameraComponent();
	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	UInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent();
	TestNotNull(TEXT("Route-kit auto-capture player should expose a first-person camera."), FirstPersonCamera);
	TestNotNull(TEXT("Route-kit auto-capture player should expose quantum camera."), QuantumCamera);
	TestNotNull(TEXT("Route-kit auto-capture player should expose inventory."), Inventory);
	if (!FirstPersonCamera || !QuantumCamera || !Inventory)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	RouteKit->ObjectiveInteractableClass = AFoundFootageObjectiveInteractable::StaticClass();
	RouteKit->ConfigureDefaultFirstLoopObjectiveNodes();
	TestEqual(TEXT("Route kit should spawn the default Day1 objective actors."), RouteKit->SpawnObjectiveNodes(), 6);
	const TArray<AFoundFootageObjectiveInteractable*>& SpawnedInteractables = RouteKit->GetSpawnedObjectiveInteractablesForTests();
	TestTrue(TEXT("Route kit should expose the first anomaly candidate in its spawned objective cache."), SpawnedInteractables.IsValidIndex(2));
	TestTrue(TEXT("Route kit should expose the first anomaly recording node in its spawned objective cache."), SpawnedInteractables.IsValidIndex(3));
	if (!SpawnedInteractables.IsValidIndex(2) || !SpawnedInteractables.IsValidIndex(3))
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	AFoundFootageObjectiveInteractable* AnomalyCandidate = SpawnedInteractables[2];
	AnomalyCandidate->SetActorLocation(FirstPersonCamera->GetComponentLocation() + FirstPersonCamera->GetForwardVector() * 220.0f);
	AnomalyCandidate->RefreshVisualDefaults();

	TestTrue(TEXT("Bodycam acquisition should unlock route-kit anomaly capture."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	TestTrue(TEXT("First note collection should unlock route-kit anomaly capture."), GameMode->TryCollectFirstNote(TEXT("Note.Intro")));
	TestTrue(TEXT("Route-kit anomaly candidate should become pending before recording starts."), GameMode->BeginFirstAnomalyCandidate(TEXT("Evidence.Anomaly01")));
	TestTrue(TEXT("Route-kit auto-capture camera should start recording."), QuantumCamera->StartRecording());

	TestFalse(TEXT("Route-kit focused anomaly should require a sustained lock before completing."), PlayerController->UpdateDay1RuntimeStateForTests(0.25f));
	TestTrue(TEXT("Route-kit focused anomaly should complete after a sustained recording lock."), PlayerController->UpdateDay1RuntimeStateForTests(1.0f));
	TestTrue(TEXT("Route-kit focused anomaly should be recorded through the cached objective list."), GameMode->HasRecordedFirstAnomaly());
	TestTrue(TEXT("Route-kit focused anomaly should write evidence to inventory."), Inventory->HasCollectedEvidenceId(TEXT("Evidence.Anomaly01")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerAnomalyCaptureLosesProgressWhenFocusBreaksTest,
	"HorrorProject.Player.Controller.AnomalyCaptureLosesProgressWhenFocusBreaks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerAnomalyCaptureLosesProgressWhenFocusBreaksTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for anomaly lock loss coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Day1 game mode for anomaly lock loss coverage."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector::ZeroVector, FRotator::ZeroRotator);
	AFoundFootageObjectiveInteractable* AnomalyCandidate = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	if (!GameMode || !PlayerController || !HUD || !PlayerCharacter || !AnomalyCandidate)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);
	PlayerController->SetControlRotation(FRotator::ZeroRotator);

	const FName AnomalyId(TEXT("Evidence.Anomaly01"));
	AnomalyCandidate->Objective = EFoundFootageInteractableObjective::FirstAnomalyCandidate;
	AnomalyCandidate->SourceId = AnomalyId;
	AnomalyCandidate->EvidenceMetadata.EvidenceId = AnomalyId;
	AnomalyCandidate->DebugLabel = FText::FromString(TEXT("第一个异常"));

	const UCameraComponent* FirstPersonCamera = PlayerCharacter->GetFirstPersonCameraComponent();
	if (!FirstPersonCamera)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	AnomalyCandidate->SetActorLocation(FirstPersonCamera->GetComponentLocation() + FirstPersonCamera->GetForwardVector() * 220.0f);
	AnomalyCandidate->RefreshVisualDefaults();

	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	TestTrue(TEXT("Bodycam acquisition should unlock anomaly lock loss setup."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	TestTrue(TEXT("First note collection should unlock anomaly lock loss setup."), GameMode->TryCollectFirstNote(TEXT("Note.Intro")));
	TestTrue(TEXT("Anomaly candidate should become pending before lock loss setup."), GameMode->BeginFirstAnomalyCandidate(AnomalyId));
	TestTrue(TEXT("Camera should start recording for anomaly lock loss setup."), QuantumCamera && QuantumCamera->StartRecording());

	TestFalse(TEXT("Partial focused runtime update should accumulate lock progress without completing."), PlayerController->UpdateDay1RuntimeStateForTests(0.5f));
	const float ProgressBeforeFocusBreak = HUD->GetAnomalyCaptureProgressForTests();
	TestTrue(TEXT("Focused recording should accumulate some lock progress."), ProgressBeforeFocusBreak > 0.0f);
	TestFalse(TEXT("Partial lock should not complete the anomaly."), GameMode->HasRecordedFirstAnomaly());

	AnomalyCandidate->SetActorLocation(FirstPersonCamera->GetComponentLocation() - FirstPersonCamera->GetForwardVector() * 300.0f);
	TestFalse(TEXT("Runtime update after focus breaks should not complete the anomaly."), PlayerController->UpdateDay1RuntimeStateForTests(0.25f));

	TestFalse(TEXT("Breaking focus should still leave the anomaly incomplete."), GameMode->HasRecordedFirstAnomaly());
	TestTrue(TEXT("Breaking focus should reset or decay the lock progress."), HUD->GetAnomalyCaptureProgressForTests() < ProgressBeforeFocusBreak);
	TestEqual(
		TEXT("Breaking focus should return the capture HUD to search guidance."),
		HUD->GetAnomalyCaptureStatusForTests().ToString(),
		FString(TEXT("搜索异常信号。")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerControllerFirstAnomalyFallbackScanCachesWorldResultsTest,
	"HorrorProject.Player.Controller.FirstAnomalyFallbackScanCachesWorldResults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerControllerFirstAnomalyFallbackScanCachesWorldResultsTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for first anomaly fallback cache coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the Day1 game mode for first anomaly fallback cache coverage."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	AHorrorPlayerController* PlayerController = World->SpawnActor<AHorrorPlayerController>();
	ADay1SliceHUD* HUD = World->SpawnActor<ADay1SliceHUD>();
	AHorrorPlayerCharacter* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacter>(FVector::ZeroVector, FRotator::ZeroRotator);
	AFoundFootageObjectiveInteractable* InitialAnomaly = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	TestNotNull(TEXT("Fallback cache test should expose the game mode."), GameMode);
	TestNotNull(TEXT("Fallback cache test should spawn a player controller."), PlayerController);
	TestNotNull(TEXT("Fallback cache test should attach the native Day1 HUD."), HUD);
	TestNotNull(TEXT("Fallback cache test should spawn a player character."), PlayerCharacter);
	TestNotNull(TEXT("Fallback cache test should spawn an initial anomaly."), InitialAnomaly);
	if (!GameMode || !PlayerController || !HUD || !PlayerCharacter || !InitialAnomaly)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	PlayerController->PlayerState = World->SpawnActor<APlayerState>();
	World->AddController(PlayerController);
	PlayerController->MyHUD = HUD;
	PlayerController->Possess(PlayerCharacter);
	PlayerController->SetControlRotation(FRotator::ZeroRotator);

	const UCameraComponent* FirstPersonCamera = PlayerCharacter->GetFirstPersonCameraComponent();
	TestNotNull(TEXT("Fallback cache player should expose a first-person camera."), FirstPersonCamera);
	if (!FirstPersonCamera)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FName InitialAnomalyId(TEXT("Evidence.FallbackAnomaly01"));
	InitialAnomaly->Objective = EFoundFootageInteractableObjective::FirstAnomalyCandidate;
	InitialAnomaly->SourceId = InitialAnomalyId;
	InitialAnomaly->EvidenceMetadata.EvidenceId = InitialAnomalyId;
	InitialAnomaly->DebugLabel = FText::FromString(TEXT("兜底异常 01"));
	InitialAnomaly->SetActorLocation(FirstPersonCamera->GetComponentLocation() + FirstPersonCamera->GetForwardVector() * 220.0f);
	if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(InitialAnomaly->GetRootComponent()))
	{
		RootPrimitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	TestTrue(TEXT("Bodycam acquisition should unlock fallback anomaly capture."), GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true));
	TestTrue(TEXT("First note collection should unlock fallback anomaly capture."), GameMode->TryCollectFirstNote(TEXT("Note.Intro")));
	TestTrue(TEXT("Fallback anomaly should become pending before runtime updates."), GameMode->BeginFirstAnomalyCandidate(InitialAnomalyId));

	TestEqual(TEXT("Fallback world scans should start at zero."), PlayerController->GetFirstAnomalyFallbackWorldScanCountForTests(), 0);
	TestFalse(TEXT("Fallback cache warm-up should not complete without recording."), PlayerController->UpdateDay1RuntimeStateForTests(0.016f));
	TestEqual(TEXT("Auto-capture and HUD refresh should share one fallback world scan per cache window."), PlayerController->GetFirstAnomalyFallbackWorldScanCountForTests(), 1);
	TestTrue(TEXT("Fallback cache should retain the initial anomaly candidate."), PlayerController->GetCachedFirstAnomalyFallbackCandidateCountForTests() >= 1);
	TestEqual(
		TEXT("Fallback-resolved anomaly should still show capture guidance."),
		HUD->GetAnomalyCaptureStatusForTests().ToString(),
		FString(TEXT("异常已对准，开启录像锁定。")));

	InitialAnomaly->SetActorLocation(FirstPersonCamera->GetComponentLocation() - FirstPersonCamera->GetForwardVector() * 320.0f);

	AFoundFootageObjectiveInteractable* FreshAnomaly = World->SpawnActor<AFoundFootageObjectiveInteractable>();
	TestNotNull(TEXT("Fallback cache test should spawn a fresh anomaly."), FreshAnomaly);
	if (!FreshAnomaly)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	const FName FreshAnomalyId(TEXT("Evidence.FallbackAnomaly02"));
	FreshAnomaly->Objective = EFoundFootageInteractableObjective::FirstAnomalyCandidate;
	FreshAnomaly->SourceId = FreshAnomalyId;
	FreshAnomaly->EvidenceMetadata.EvidenceId = FreshAnomalyId;
	FreshAnomaly->DebugLabel = FText::FromString(TEXT("兜底异常 02"));
	FreshAnomaly->SetActorLocation(FirstPersonCamera->GetComponentLocation() + FirstPersonCamera->GetForwardVector() * 240.0f);
	if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(FreshAnomaly->GetRootComponent()))
	{
		RootPrimitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	TestFalse(TEXT("A fresh anomaly should not force another world scan inside the cache window."), PlayerController->UpdateDay1RuntimeStateForTests(0.016f));
	TestEqual(TEXT("Fallback scan count should stay stable inside the cache window."), PlayerController->GetFirstAnomalyFallbackWorldScanCountForTests(), 1);
	TestEqual(
		TEXT("Cached fallback candidates should keep HUD in search mode until the cache refreshes."),
		HUD->GetAnomalyCaptureStatusForTests().ToString(),
		FString(TEXT("搜索异常信号。")));

	PlayerController->ExpireFirstAnomalyFallbackCacheForTests();
	TestFalse(TEXT("Expired fallback cache should refresh without recording."), PlayerController->UpdateDay1RuntimeStateForTests(0.016f));
	TestEqual(TEXT("Expired fallback cache should allow exactly one more world scan."), PlayerController->GetFirstAnomalyFallbackWorldScanCountForTests(), 2);
	TestEqual(
		TEXT("Refreshed fallback cache should resolve the newly visible anomaly."),
		HUD->GetAnomalyCaptureStatusForTests().ToString(),
		FString(TEXT("异常已对准，开启录像锁定。")));

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif
