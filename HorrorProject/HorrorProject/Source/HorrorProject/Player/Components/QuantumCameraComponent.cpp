// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/QuantumCameraComponent.h"
#include "Player/Components/CameraBatteryComponent.h"
#include "GameFramework/Actor.h"

void UQuantumCameraRuntimeDelegateProbe::HandleCameraAcquiredChanged(bool bNewAcquired)
{
	++AcquiredBroadcastCount;
	AcquiredValues.Add(bNewAcquired);
}

void UQuantumCameraRuntimeDelegateProbe::HandleCameraEnabledChanged(bool bNewEnabled)
{
	++EnabledBroadcastCount;
	EnabledValues.Add(bNewEnabled);
}

void UQuantumCameraRuntimeDelegateProbe::HandleCameraModeChanged(EQuantumCameraMode NewMode)
{
	++ModeBroadcastCount;
	ModeValues.Add(NewMode);
}

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuantumCameraComponentTracksRuntimeModesTest,
	"HorrorProject.Player.QuantumCamera.TracksRuntimeModes",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FQuantumCameraComponentTracksRuntimeModesTest::RunTest(const FString& Parameters)
{
	UQuantumCameraComponent* QuantumCameraComponent = NewObject<UQuantumCameraComponent>();

	TestFalse(TEXT("Quantum camera should start unacquired."), QuantumCameraComponent->IsCameraAcquired());
	TestFalse(TEXT("Quantum camera should start disabled."), QuantumCameraComponent->IsCameraEnabled());
	TestEqual(TEXT("Quantum camera should start in the disabled mode."), QuantumCameraComponent->GetCameraMode(), EQuantumCameraMode::Disabled);
	TestFalse(TEXT("Taking a photo while disabled should be rejected."), QuantumCameraComponent->TakePhoto());
	TestFalse(TEXT("Quantum camera cannot be enabled before it is acquired."), QuantumCameraComponent->SetCameraEnabled(true));
	TestFalse(TEXT("Disabled quantum cameras should reject active modes."), QuantumCameraComponent->SetCameraMode(EQuantumCameraMode::Recording));
	TestTrue(TEXT("Acquiring the quantum camera should change the runtime state."), QuantumCameraComponent->SetCameraAcquired(true));
	TestTrue(TEXT("Acquired quantum cameras can be enabled."), QuantumCameraComponent->SetCameraEnabled(true));
	TestEqual(TEXT("Enabling the camera should restore the default viewfinder mode."), QuantumCameraComponent->GetCameraMode(), EQuantumCameraMode::Viewfinder);
	TestTrue(TEXT("Recording should switch the camera into recording mode."), QuantumCameraComponent->StartRecording());
	TestEqual(TEXT("Recording mode should be queryable."), QuantumCameraComponent->GetCameraMode(), EQuantumCameraMode::Recording);
	TestTrue(TEXT("Stopping recording should restore the viewfinder mode."), QuantumCameraComponent->StopRecording());
	TestTrue(TEXT("Taking a photo should trigger the photo capture hook."), QuantumCameraComponent->TakePhoto());
	TestEqual(TEXT("Photo capture should return to viewfinder after the one-shot hook."), QuantumCameraComponent->GetCameraMode(), EQuantumCameraMode::Viewfinder);
	TestTrue(TEXT("Taking another photo should not be blocked by the prior capture."), QuantumCameraComponent->TakePhoto());
	TestEqual(TEXT("Repeated photo capture should still return to viewfinder."), QuantumCameraComponent->GetCameraMode(), EQuantumCameraMode::Viewfinder);
	TestTrue(TEXT("Rewind should switch the camera into rewind mode."), QuantumCameraComponent->StartRewind());
	TestEqual(TEXT("Rewind mode should be queryable."), QuantumCameraComponent->GetCameraMode(), EQuantumCameraMode::Rewind);
	TestTrue(TEXT("Stopping rewind should restore the viewfinder mode."), QuantumCameraComponent->StopRewind());

	TestTrue(TEXT("Recording should restart after returning to viewfinder."), QuantumCameraComponent->StartRecording());
	TestTrue(TEXT("Disabling while recording should change the runtime state."), QuantumCameraComponent->SetCameraEnabled(false));
	TestFalse(TEXT("Disabled recording camera should report disabled state."), QuantumCameraComponent->IsCameraEnabled());
	TestEqual(TEXT("Disabling while recording should reset to disabled mode."), QuantumCameraComponent->GetCameraMode(), EQuantumCameraMode::Disabled);
	TestFalse(TEXT("Stopping recording after disable should be a no-op."), QuantumCameraComponent->StopRecording());
	TestTrue(TEXT("Re-enabling after recording disable should change runtime state."), QuantumCameraComponent->SetCameraEnabled(true));
	TestEqual(TEXT("Re-enabling after recording disable should return to viewfinder."), QuantumCameraComponent->GetCameraMode(), EQuantumCameraMode::Viewfinder);

	TestTrue(TEXT("Rewind should restart after re-enabling."), QuantumCameraComponent->StartRewind());
	TestTrue(TEXT("Dropping acquisition while rewinding should update the acquired flag."), QuantumCameraComponent->SetCameraAcquired(false));
	TestFalse(TEXT("Losing acquisition while rewinding should also disable the camera."), QuantumCameraComponent->IsCameraEnabled());
	TestEqual(TEXT("Losing acquisition while rewinding should reset the camera mode."), QuantumCameraComponent->GetCameraMode(), EQuantumCameraMode::Disabled);
	TestFalse(TEXT("Stopping rewind after acquisition loss should be a no-op."), QuantumCameraComponent->StopRewind());
	TestTrue(TEXT("Reacquiring after acquisition loss should change runtime state."), QuantumCameraComponent->SetCameraAcquired(true));
	TestTrue(TEXT("Re-enabling after reacquire should change runtime state."), QuantumCameraComponent->SetCameraEnabled(true));
	TestEqual(TEXT("Re-enabling after reacquire should return to viewfinder."), QuantumCameraComponent->GetCameraMode(), EQuantumCameraMode::Viewfinder);
	TestTrue(TEXT("Dropping acquisition should update the acquired flag."), QuantumCameraComponent->SetCameraAcquired(false));
	TestFalse(TEXT("Losing acquisition should also disable the camera."), QuantumCameraComponent->IsCameraEnabled());
	TestEqual(TEXT("Losing acquisition should reset the camera mode."), QuantumCameraComponent->GetCameraMode(), EQuantumCameraMode::Disabled);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FQuantumCameraComponentBroadcastsRuntimeDelegatesTest,
	"HorrorProject.Player.QuantumCamera.BroadcastsRuntimeDelegates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FQuantumCameraComponentBroadcastsRuntimeDelegatesTest::RunTest(const FString& Parameters)
{
	UQuantumCameraComponent* QuantumCameraComponent = NewObject<UQuantumCameraComponent>();
	UQuantumCameraRuntimeDelegateProbe* DelegateProbe = NewObject<UQuantumCameraRuntimeDelegateProbe>();
	QuantumCameraComponent->OnCameraAcquiredChanged.AddDynamic(DelegateProbe, &UQuantumCameraRuntimeDelegateProbe::HandleCameraAcquiredChanged);
	QuantumCameraComponent->OnCameraEnabledChanged.AddDynamic(DelegateProbe, &UQuantumCameraRuntimeDelegateProbe::HandleCameraEnabledChanged);
	QuantumCameraComponent->OnCameraModeChanged.AddDynamic(DelegateProbe, &UQuantumCameraRuntimeDelegateProbe::HandleCameraModeChanged);

	TestFalse(TEXT("Repeated unacquired state should be a no-op."), QuantumCameraComponent->SetCameraAcquired(false));
	TestFalse(TEXT("Enabling before acquisition should be a no-op."), QuantumCameraComponent->SetCameraEnabled(true));
	TestEqual(TEXT("No-op runtime calls should not broadcast acquired delegates."), DelegateProbe->AcquiredBroadcastCount, 0);
	TestEqual(TEXT("No-op runtime calls should not broadcast enabled delegates."), DelegateProbe->EnabledBroadcastCount, 0);

	TestTrue(TEXT("Acquiring should broadcast the acquired delegate."), QuantumCameraComponent->SetCameraAcquired(true));
	TestEqual(TEXT("Acquiring should broadcast acquired once."), DelegateProbe->AcquiredBroadcastCount, 1);
	TestEqual(TEXT("Acquiring should publish true payload."), DelegateProbe->AcquiredValues.Last(), true);
	TestEqual(TEXT("Acquiring alone should not broadcast enabled."), DelegateProbe->EnabledBroadcastCount, 0);
	TestFalse(TEXT("Repeated acquire should be a no-op."), QuantumCameraComponent->SetCameraAcquired(true));
	TestEqual(TEXT("Repeated acquire should not rebroadcast."), DelegateProbe->AcquiredBroadcastCount, 1);

	TestTrue(TEXT("Enabling acquired camera should broadcast enabled delegate."), QuantumCameraComponent->SetCameraEnabled(true));
	TestEqual(TEXT("Enabled delegate should broadcast once."), DelegateProbe->EnabledBroadcastCount, 1);
	TestEqual(TEXT("Enabled delegate should publish true payload."), DelegateProbe->EnabledValues.Last(), true);
	TestEqual(TEXT("Enabling acquired camera should broadcast viewfinder mode."), DelegateProbe->ModeValues.Last(), EQuantumCameraMode::Viewfinder);
	TestFalse(TEXT("Repeated enable should be a no-op."), QuantumCameraComponent->SetCameraEnabled(true));
	TestEqual(TEXT("Repeated enable should not rebroadcast."), DelegateProbe->EnabledBroadcastCount, 1);
	TestEqual(TEXT("Repeated enable should not rebroadcast mode."), DelegateProbe->ModeBroadcastCount, 1);

	TestTrue(TEXT("Photo capture should broadcast a one-shot mode transition."), QuantumCameraComponent->TakePhoto());
	TestEqual(TEXT("Photo capture should broadcast photo then viewfinder."), DelegateProbe->ModeValues, TArray<EQuantumCameraMode>({ EQuantumCameraMode::Viewfinder, EQuantumCameraMode::Photo, EQuantumCameraMode::Viewfinder }));

	TestTrue(TEXT("Dropping acquisition should broadcast acquired false."), QuantumCameraComponent->SetCameraAcquired(false));
	TestEqual(TEXT("Acquisition loss should broadcast acquired twice total."), DelegateProbe->AcquiredBroadcastCount, 2);
	TestEqual(TEXT("Acquisition loss should publish false payload."), DelegateProbe->AcquiredValues.Last(), false);
	TestEqual(TEXT("Acquisition loss should force enabled false broadcast."), DelegateProbe->EnabledBroadcastCount, 2);
	TestEqual(TEXT("Forced disable should publish false payload."), DelegateProbe->EnabledValues.Last(), false);
	TestEqual(TEXT("Acquired delegate payload order should be true then false."), DelegateProbe->AcquiredValues, TArray<bool>({ true, false }));
	TestEqual(TEXT("Enabled delegate payload order should be true then false."), DelegateProbe->EnabledValues, TArray<bool>({ true, false }));

	TestFalse(TEXT("Repeated acquisition loss should be a no-op."), QuantumCameraComponent->SetCameraAcquired(false));
	TestFalse(TEXT("Repeated disable after acquisition loss should be a no-op."), QuantumCameraComponent->SetCameraEnabled(false));
	TestEqual(TEXT("Repeated acquisition loss should not rebroadcast acquired."), DelegateProbe->AcquiredBroadcastCount, 2);
	TestEqual(TEXT("Repeated disable should not rebroadcast enabled."), DelegateProbe->EnabledBroadcastCount, 2);

	return true;
}

#endif

UQuantumCameraComponent::UQuantumCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UQuantumCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	FindOrCreateBatteryComponent();

	if (CachedBatteryComponent)
	{
		CachedBatteryComponent->OnBatteryDepleted.AddDynamic(this, &UQuantumCameraComponent::OnBatteryDepleted);
	}
}

UCameraBatteryComponent* UQuantumCameraComponent::GetBatteryComponent() const
{
	return CachedBatteryComponent;
}

void UQuantumCameraComponent::FindOrCreateBatteryComponent()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	CachedBatteryComponent = Owner->FindComponentByClass<UCameraBatteryComponent>();
}

void UQuantumCameraComponent::OnBatteryDepleted()
{
	if (CameraMode == EQuantumCameraMode::Recording)
	{
		StopRecording();
	}
}

bool UQuantumCameraComponent::SetCameraAcquired(bool bNewAcquired)
{
	if (bCameraAcquired == bNewAcquired)
	{
		return false;
	}

	bCameraAcquired = bNewAcquired;
	OnCameraAcquiredChanged.Broadcast(bCameraAcquired);
	if (!bCameraAcquired)
	{
		SetCameraEnabled(false);
	}

	return true;
}

bool UQuantumCameraComponent::IsCameraAcquired() const
{
	return bCameraAcquired;
}

bool UQuantumCameraComponent::SetCameraEnabled(bool bNewEnabled)
{
	const bool bResolvedEnabled = bCameraAcquired && bNewEnabled;
	if (bCameraEnabled == bResolvedEnabled)
	{
		return false;
	}

	bCameraEnabled = bResolvedEnabled;
	OnCameraEnabledChanged.Broadcast(bCameraEnabled);
	if (bCameraEnabled)
	{
		if (CameraMode == EQuantumCameraMode::Disabled)
		{
			SetCameraMode(EQuantumCameraMode::Viewfinder);
		}
	}
	else
	{
		SetCameraMode(EQuantumCameraMode::Disabled);
	}

	return true;
}

bool UQuantumCameraComponent::IsCameraEnabled() const
{
	return bCameraEnabled;
}

bool UQuantumCameraComponent::SetCameraMode(EQuantumCameraMode NewMode)
{
	if (NewMode != EQuantumCameraMode::Disabled && !CanUseCamera())
	{
		return false;
	}

	if (CameraMode == NewMode)
	{
		return false;
	}

	CameraMode = NewMode;
	OnCameraModeChanged.Broadcast(CameraMode);
	return true;
}

EQuantumCameraMode UQuantumCameraComponent::GetCameraMode() const
{
	return CameraMode;
}

bool UQuantumCameraComponent::IsCameraMode(EQuantumCameraMode QueryMode) const
{
	return CameraMode == QueryMode;
}

bool UQuantumCameraComponent::StartRecording()
{
	if (CachedBatteryComponent && CachedBatteryComponent->IsBatteryDepleted())
	{
		return false;
	}

	bool bSuccess = SetCameraMode(EQuantumCameraMode::Recording);

	if (bSuccess && CachedBatteryComponent)
	{
		CachedBatteryComponent->StartRecordingDrain();
	}

	return bSuccess;
}

bool UQuantumCameraComponent::StopRecording()
{
	if (CameraMode != EQuantumCameraMode::Recording)
	{
		return false;
	}

	if (CachedBatteryComponent)
	{
		CachedBatteryComponent->StopRecordingDrain();
	}

	return SetCameraMode(CanUseCamera() ? EQuantumCameraMode::Viewfinder : EQuantumCameraMode::Disabled);
}

bool UQuantumCameraComponent::TakePhoto()
{
	if (!SetCameraMode(EQuantumCameraMode::Photo))
	{
		return false;
	}

	SetCameraMode(CanUseCamera() ? EQuantumCameraMode::Viewfinder : EQuantumCameraMode::Disabled);
	return true;
}

bool UQuantumCameraComponent::StartRewind()
{
	return SetCameraMode(EQuantumCameraMode::Rewind);
}

bool UQuantumCameraComponent::StopRewind()
{
	if (CameraMode != EQuantumCameraMode::Rewind)
	{
		return false;
	}

	return SetCameraMode(CanUseCamera() ? EQuantumCameraMode::Viewfinder : EQuantumCameraMode::Disabled);
}

bool UQuantumCameraComponent::CanUseCamera() const
{
	return bCameraAcquired && bCameraEnabled;
}
