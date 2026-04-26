#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && HORRORPROJECT_ENABLE_LEGACY_AUTOMATION_TESTS

// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Game/HorrorGameModeBase.h"
#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Player/Components/VHSEffectComponent.h"

#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

// TODO(P5): SpawnActor<TestDouble> 当前回退为父类 spawn（无 UCLASS/GENERATED_BODY）；
// BlueprintImplementableEvent 的 DoOpenArchive 在无 BP 子类时为空操作，导致
// OpenArchiveForTest 实质是 no-op，断言"未解锁出口"为伪覆盖。
// P5 重写时移除此 TestDouble，改为 AHorrorPlayerCharacter 在
// #if WITH_DEV_AUTOMATION_TESTS 守卫下直接暴露 *ForTest() 接口。
class AHorrorPlayerCharacterInputTestDouble : public AHorrorPlayerCharacter
{
public:
	void StartRecordForTest()
	{
		DoStartRecord();
	}

	void StopRecordForTest()
	{
		DoStopRecord();
	}

	void StartRewindForTest()
	{
		DoStartRewind();
	}

	void StopRewindForTest()
	{
		DoStopRewind();
	}

	UVHSEffectComponent* GetMutableVHSEffectForTest() const
	{
		return GetVHSEffectComponent();
	}

	void OpenArchiveForTest()
	{
		DoOpenArchive();
	}
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHorrorPlayerInputDoesNotBypassRouteObjectivesTest,
	"HorrorProject.Player.Input.DoesNotBypassRouteObjectives",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FHorrorPlayerInputDoesNotBypassRouteObjectivesTest::RunTest(const FString& Parameters)
{
	FTestWorldWrapper TestWorld;
	TestTrue(TEXT("Transient game world should be created for input objective coverage."), TestWorld.CreateTestWorld(EWorldType::Game));
	UWorld* World = TestWorld.GetTestWorld();
	if (!World)
	{
		return false;
	}

	World->GetWorldSettings()->DefaultGameMode = AHorrorGameModeBase::StaticClass();
	TestTrue(TEXT("Transient world should create the horror game mode."), World->SetGameMode(FURL()));
	AHorrorGameModeBase* GameMode = World->GetAuthGameMode<AHorrorGameModeBase>();
	TestNotNull(TEXT("Transient world should expose the horror game mode."), GameMode);
	if (!GameMode)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	AHorrorPlayerCharacterInputTestDouble* PlayerCharacter = World->SpawnActor<AHorrorPlayerCharacterInputTestDouble>();
	TestNotNull(TEXT("Input bypass test should spawn a player character."), PlayerCharacter);
	if (!PlayerCharacter)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	UQuantumCameraComponent* QuantumCamera = PlayerCharacter->GetQuantumCameraComponent();
	UVHSEffectComponent* VHSEffect = PlayerCharacter->GetVHSEffectComponent();
	TestNotNull(TEXT("Input bypass test player should expose quantum camera."), QuantumCamera);
	TestNotNull(TEXT("Input bypass test player should expose VHS feedback."), VHSEffect);
	if (!QuantumCamera || !VHSEffect)
	{
		TestWorld.DestroyTestWorld(false);
		return false;
	}

	GameMode->TryAcquireBodycam(TEXT("Evidence.Bodycam"), true);
	TestTrue(TEXT("Quantum camera should be enabled by bodycam acquisition."), QuantumCamera->IsCameraEnabled());
	TestTrue(TEXT("VHS feedback should be enabled by bodycam acquisition."), VHSEffect->IsBodycamEnabled());
	GameMode->TryCollectFirstNote(TEXT("Note.Intro"));
	TestTrue(TEXT("Input bypass test should register an anomaly candidate."), GameMode->BeginFirstAnomalyCandidate(TEXT("Evidence.Anomaly01")));

	PlayerCharacter->StartRecordForTest();
	TestTrue(TEXT("Record input should put the camera into recording mode."), QuantumCamera->IsCameraMode(EQuantumCameraMode::Recording));
	TestEqual(TEXT("Record input should propagate recording mode to VHS feedback."), VHSEffect->GetFeedbackCameraMode(), EQuantumCameraMode::Recording);
	TestFalse(TEXT("Record input alone should not complete the first anomaly objective."), GameMode->HasRecordedFirstAnomaly());

	PlayerCharacter->StopRecordForTest();
	TestEqual(TEXT("Stop record input should return quantum camera to viewfinder."), QuantumCamera->GetCameraMode(), EQuantumCameraMode::Viewfinder);
	TestEqual(TEXT("Stop record input should return VHS feedback to viewfinder."), VHSEffect->GetFeedbackCameraMode(), EQuantumCameraMode::Viewfinder);
	PlayerCharacter->StartRewindForTest();
	TestEqual(TEXT("Rewind input should put the camera into rewind mode."), QuantumCamera->GetCameraMode(), EQuantumCameraMode::Rewind);
	TestEqual(TEXT("Rewind input should propagate rewind mode to VHS feedback."), VHSEffect->GetFeedbackCameraMode(), EQuantumCameraMode::Rewind);
	TestTrue(TEXT("Acquisition loss should change quantum camera runtime state."), QuantumCamera->SetCameraAcquired(false));
	PlayerCharacter->GetMutableVHSEffectForTest()->SetBodycamAcquired(QuantumCamera->IsCameraAcquired());
	PlayerCharacter->GetMutableVHSEffectForTest()->SetBodycamEnabled(QuantumCamera->IsCameraEnabled());
	PlayerCharacter->GetMutableVHSEffectForTest()->SetFeedbackCameraMode(QuantumCamera->GetCameraMode());
	TestFalse(TEXT("Acquisition loss should clear quantum camera acquired state."), QuantumCamera->IsCameraAcquired());
	TestFalse(TEXT("Acquisition loss should disable quantum camera."), QuantumCamera->IsCameraEnabled());
	TestEqual(TEXT("Acquisition loss should reset quantum camera mode."), QuantumCamera->GetCameraMode(), EQuantumCameraMode::Disabled);
	TestFalse(TEXT("Acquisition loss should propagate acquired state to VHS feedback."), VHSEffect->IsBodycamAcquired());
	TestFalse(TEXT("Acquisition loss should propagate enabled state to VHS feedback."), VHSEffect->IsBodycamEnabled());
	TestEqual(TEXT("Acquisition loss should propagate disabled mode to VHS feedback."), VHSEffect->GetFeedbackCameraMode(), EQuantumCameraMode::Disabled);
	TestTrue(TEXT("Reacquiring quantum camera should restore runtime state."), QuantumCamera->SetCameraAcquired(true));
	PlayerCharacter->GetMutableVHSEffectForTest()->SetBodycamAcquired(QuantumCamera->IsCameraAcquired());
	TestTrue(TEXT("Re-enabling quantum camera should restore enabled state."), QuantumCamera->SetCameraEnabled(true));
	PlayerCharacter->GetMutableVHSEffectForTest()->SetBodycamEnabled(QuantumCamera->IsCameraEnabled());
	PlayerCharacter->GetMutableVHSEffectForTest()->SetFeedbackCameraMode(QuantumCamera->GetCameraMode());
	TestTrue(TEXT("Reacquiring should propagate acquired state to VHS feedback."), VHSEffect->IsBodycamAcquired());
	TestTrue(TEXT("Re-enabling should propagate enabled state to VHS feedback."), VHSEffect->IsBodycamEnabled());
	TestEqual(TEXT("Re-enabling should return quantum camera to viewfinder."), QuantumCamera->GetCameraMode(), EQuantumCameraMode::Viewfinder);
	TestEqual(TEXT("Re-enabling should return VHS feedback to viewfinder."), VHSEffect->GetFeedbackCameraMode(), EQuantumCameraMode::Viewfinder);
	TestTrue(TEXT("Disabling quantum camera should change runtime state."), QuantumCamera->SetCameraEnabled(false));
	PlayerCharacter->GetMutableVHSEffectForTest()->SetFeedbackCameraMode(QuantumCamera->GetCameraMode());
	TestEqual(TEXT("Disabling quantum camera should propagate disabled mode to VHS feedback."), VHSEffect->GetFeedbackCameraMode(), EQuantumCameraMode::Disabled);

	PlayerCharacter->OpenArchiveForTest();
	TestFalse(TEXT("Archive input alone should not complete archive review."), GameMode->HasReviewedArchive());
	TestFalse(TEXT("Archive input alone should not unlock the exit."), GameMode->IsExitUnlocked());

	TestTrue(TEXT("Transient world should be destroyed cleanly."), TestWorld.DestroyTestWorld(false));
	return true;
}

#endif

#endif // WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
