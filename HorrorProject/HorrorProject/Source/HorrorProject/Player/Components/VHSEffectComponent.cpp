// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/Components/VHSEffectComponent.h"

#include "Camera/CameraComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Player/Components/VHSNoiseGenerator.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/Package.h"

namespace HorrorVHSDefaults
{
	constexpr TCHAR DefaultPostProcessMaterialPath[] = TEXT("/Game/Bodycam_VHS_Effect/Materials/Instances/PostProcess/MI_OldVHS.MI_OldVHS");
	constexpr TCHAR MissingDefaultPostProcessMaterialReason[] = TEXT("Default VHS post-process material is not available; set VHSPostProcessMaterial on the component or Blueprint.");
}

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVHSEffectComponentTracksBodycamStateTest,
	"HorrorProject.Player.VHS.TracksBodycamState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVHSEffectComponentTracksBodycamStateTest::RunTest(const FString& Parameters)
{
	UVHSEffectComponent* VHSEffectComponent = NewObject<UVHSEffectComponent>();

	TestFalse(TEXT("Bodycam should start unacquired."), VHSEffectComponent->IsBodycamAcquired());
	TestFalse(TEXT("Bodycam should start disabled."), VHSEffectComponent->IsBodycamEnabled());
	TestFalse(TEXT("Bodycam cannot be enabled before it is acquired."), VHSEffectComponent->SetBodycamEnabled(true));
	TestTrue(TEXT("Acquiring the bodycam should change the runtime state."), VHSEffectComponent->SetBodycamAcquired(true));
	TestFalse(TEXT("Duplicate acquisition changes should be ignored."), VHSEffectComponent->SetBodycamAcquired(true));
	TestTrue(TEXT("Acquired bodycams can be enabled."), VHSEffectComponent->SetBodycamEnabled(true));
	TestTrue(TEXT("Enabled bodycam state should be queryable."), VHSEffectComponent->IsBodycamEnabled());
	TestTrue(TEXT("Enabled bodycams should activate VHS feedback."), VHSEffectComponent->IsFeedbackActive());
	TestEqual(TEXT("Enabled bodycams should start feedback in viewfinder mode."), VHSEffectComponent->GetFeedbackCameraMode(), EQuantumCameraMode::Viewfinder);
	TestTrue(TEXT("Recording mode should update VHS feedback."), VHSEffectComponent->SetFeedbackCameraMode(EQuantumCameraMode::Recording));
	TestEqual(TEXT("VHS feedback should expose the current camera mode."), VHSEffectComponent->GetFeedbackCameraMode(), EQuantumCameraMode::Recording);
	TestFalse(TEXT("Duplicate feedback mode changes should be ignored."), VHSEffectComponent->SetFeedbackCameraMode(EQuantumCameraMode::Recording));
	TestTrue(TEXT("Disabling while recording should change the runtime state."), VHSEffectComponent->SetBodycamEnabled(false));
	TestFalse(TEXT("Disabling while recording should deactivate VHS feedback."), VHSEffectComponent->IsFeedbackActive());
	TestEqual(TEXT("Disabling while recording should reset VHS feedback mode."), VHSEffectComponent->GetFeedbackCameraMode(), EQuantumCameraMode::Disabled);
	TestFalse(TEXT("Disabled VHS feedback should reject stale active modes."), VHSEffectComponent->SetFeedbackCameraMode(EQuantumCameraMode::Recording));
	TestTrue(TEXT("Re-enabling after recording disable should change runtime state."), VHSEffectComponent->SetBodycamEnabled(true));
	TestTrue(TEXT("Re-enabled bodycams should reactivate VHS feedback."), VHSEffectComponent->IsFeedbackActive());
	TestEqual(TEXT("Re-enabling after recording disable should return VHS to viewfinder."), VHSEffectComponent->GetFeedbackCameraMode(), EQuantumCameraMode::Viewfinder);
	TestTrue(TEXT("Rewind mode should update VHS feedback."), VHSEffectComponent->SetFeedbackCameraMode(EQuantumCameraMode::Rewind));
	TestTrue(TEXT("Clearing acquisition should update the acquired flag."), VHSEffectComponent->SetBodycamAcquired(false));
	TestFalse(TEXT("Losing acquisition should also disable the bodycam."), VHSEffectComponent->IsBodycamEnabled());
	TestFalse(TEXT("Losing acquisition should also deactivate VHS feedback."), VHSEffectComponent->IsFeedbackActive());
	TestEqual(TEXT("Disabled VHS feedback should reset camera mode."), VHSEffectComponent->GetFeedbackCameraMode(), EQuantumCameraMode::Disabled);
	TestFalse(TEXT("Acquisition loss should reject stale rewind feedback."), VHSEffectComponent->SetFeedbackCameraMode(EQuantumCameraMode::Rewind));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FVHSEffectComponentPostProcessBindingTest,
	"HorrorProject.Player.VHS.PostProcessBindingFollowsFeedbackState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FVHSEffectComponentPostProcessBindingTest::RunTest(const FString& Parameters)
{
	UVHSEffectComponent* VHSEffectComponent = NewObject<UVHSEffectComponent>();
	UCameraComponent* CameraComponent = NewObject<UCameraComponent>();
	UMaterialInterface* TestMaterial = NewObject<UMaterial>(GetTransientPackage());

	TestNotNull(TEXT("VHS component should have the bodycam post-process material as a native default."), VHSEffectComponent->VHSPostProcessMaterial.Get());
	TestFalse(TEXT("Already assigned default VHS material should not resolve duplicate defaults."), VHSEffectComponent->ResolveDefaultPostProcessMaterial());
	VHSEffectComponent->VHSPostProcessMaterial = TestMaterial;
	TestFalse(TEXT("Already assigned VHS material should not resolve duplicate defaults."), VHSEffectComponent->ResolveDefaultPostProcessMaterial());
	VHSEffectComponent->VHSPostProcessBlendWeight = 0.75f;

	TestTrue(TEXT("Binding a VHS post-process camera should succeed."), VHSEffectComponent->BindPostProcessCamera(CameraComponent));
	TestEqual(TEXT("Bound post-process camera should be queryable."), VHSEffectComponent->GetBoundPostProcessCamera(), CameraComponent);
	TestEqual(TEXT("Initial inactive VHS blend should be zero."), CameraComponent->PostProcessSettings.WeightedBlendables.Array[0].Weight, 0.0f);

	TestTrue(TEXT("Acquiring the bodycam should change the runtime state."), VHSEffectComponent->SetBodycamAcquired(true));
	TestTrue(TEXT("Enabling the bodycam should activate VHS feedback."), VHSEffectComponent->SetBodycamEnabled(true));
	TestEqual(TEXT("Active VHS feedback should apply the configured blend weight."), CameraComponent->PostProcessSettings.WeightedBlendables.Array[0].Weight, 0.75f);

	TestTrue(TEXT("Disabling the bodycam should deactivate VHS feedback."), VHSEffectComponent->SetBodycamEnabled(false));
	TestEqual(TEXT("Inactive VHS feedback should keep the material bound with zero blend weight."), CameraComponent->PostProcessSettings.WeightedBlendables.Array[0].Weight, 0.0f);
	TestTrue(TEXT("Re-enabling the bodycam should reactivate VHS feedback."), VHSEffectComponent->SetBodycamEnabled(true));
	TestEqual(TEXT("Reactivated VHS feedback should restore the configured blend weight."), CameraComponent->PostProcessSettings.WeightedBlendables.Array[0].Weight, 0.75f);
	VHSEffectComponent->VHSPostProcessBlendWeight = 1.75f;
	VHSEffectComponent->RefreshPostProcessBinding();
	TestEqual(TEXT("Active VHS feedback should clamp excessive blend weights."), CameraComponent->PostProcessSettings.WeightedBlendables.Array[0].Weight, 1.0f);
	VHSEffectComponent->VHSPostProcessBlendWeight = -0.25f;
	VHSEffectComponent->RefreshPostProcessBinding();
	TestEqual(TEXT("Active VHS feedback should clamp negative blend weights."), CameraComponent->PostProcessSettings.WeightedBlendables.Array[0].Weight, 0.0f);
	TestTrue(TEXT("Losing bodycam acquisition should deactivate VHS feedback."), VHSEffectComponent->SetBodycamAcquired(false));
	TestEqual(TEXT("Acquisition loss should clear the VHS blend weight."), CameraComponent->PostProcessSettings.WeightedBlendables.Array[0].Weight, 0.0f);

	return true;
}

#endif

UVHSEffectComponent::UVHSEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultPostProcessMaterial(
		HorrorVHSDefaults::DefaultPostProcessMaterialPath);
	if (DefaultPostProcessMaterial.Succeeded())
	{
		VHSPostProcessMaterial = DefaultPostProcessMaterial.Object;
	}
}

void UVHSEffectComponent::InitializeNoiseGenerator()
{
	if (!NoiseGenerator)
	{
		NoiseGenerator = NewObject<UVHSNoiseGenerator>(this);
	}

	if (NoiseGenerator && DynamicPostProcessMaterial)
	{
		NoiseGenerator->Initialize(DynamicPostProcessMaterial);
	}
}

void UVHSEffectComponent::UpdateNoiseGenerator(float DeltaTime, float StressLevel, float BatteryLevel)
{
	if (!NoiseGenerator)
	{
		InitializeNoiseGenerator();
	}

	if (NoiseGenerator && bFeedbackActive)
	{
		NoiseGenerator->UpdateNoise(DeltaTime, StressLevel, BatteryLevel);
	}
}

UVHSNoiseGenerator* UVHSEffectComponent::GetNoiseGenerator() const
{
	return NoiseGenerator;
}

bool UVHSEffectComponent::SetBodycamAcquired(bool bNewAcquired)
{
	if (bBodycamAcquired == bNewAcquired)
	{
		return false;
	}

	bBodycamAcquired = bNewAcquired;
	OnBodycamAcquiredChanged.Broadcast(bBodycamAcquired);

	if (!bBodycamAcquired && bBodycamEnabled)
	{
		bBodycamEnabled = false;
		OnBodycamEnabledChanged.Broadcast(false);
	}

	UpdateFeedbackState(bBodycamEnabled ? FeedbackCameraMode : EQuantumCameraMode::Disabled);
	return true;
}

bool UVHSEffectComponent::IsBodycamAcquired() const
{
	return bBodycamAcquired;
}

bool UVHSEffectComponent::SetBodycamEnabled(bool bNewEnabled)
{
	const bool bResolvedEnabled = bBodycamAcquired && bNewEnabled;
	if (bBodycamEnabled == bResolvedEnabled)
	{
		return false;
	}

	bBodycamEnabled = bResolvedEnabled;
	OnBodycamEnabledChanged.Broadcast(bBodycamEnabled);
	UpdateFeedbackState(bBodycamEnabled ? EQuantumCameraMode::Viewfinder : EQuantumCameraMode::Disabled);
	return true;
}

bool UVHSEffectComponent::IsBodycamEnabled() const
{
	return bBodycamEnabled;
}

bool UVHSEffectComponent::SetFeedbackCameraMode(EQuantumCameraMode NewMode)
{
	if (!bBodycamEnabled && NewMode != EQuantumCameraMode::Disabled)
	{
		return false;
	}

	const EQuantumCameraMode ResolvedMode = bBodycamEnabled ? NewMode : EQuantumCameraMode::Disabled;
	if (FeedbackCameraMode == ResolvedMode)
	{
		return false;
	}

	UpdateFeedbackState(ResolvedMode);
	return true;
}

bool UVHSEffectComponent::IsFeedbackActive() const
{
	return bFeedbackActive;
}

EQuantumCameraMode UVHSEffectComponent::GetFeedbackCameraMode() const
{
	return FeedbackCameraMode;
}

bool UVHSEffectComponent::BindPostProcessCamera(UCameraComponent* CameraComponent)
{
	if (BoundPostProcessCamera == CameraComponent)
	{
		return false;
	}

	BoundPostProcessCamera = CameraComponent;
	CreateDynamicMaterialInstance();
	ApplyPostProcessBlendWeight();
	return BoundPostProcessCamera != nullptr;
}

void UVHSEffectComponent::RefreshPostProcessBinding()
{
	ApplyPostProcessBlendWeight();
}

bool UVHSEffectComponent::ResolveDefaultPostProcessMaterial()
{
	if (VHSPostProcessMaterial)
	{
		return false;
	}

	if (UMaterialInterface* ResolvedMaterial = LoadObject<UMaterialInterface>(nullptr, HorrorVHSDefaults::DefaultPostProcessMaterialPath))
	{
		VHSPostProcessMaterial = ResolvedMaterial;
		CreateDynamicMaterialInstance();
		ApplyPostProcessBlendWeight();
		return true;
	}

	UE_LOG(LogTemp, Verbose, TEXT("%s"), HorrorVHSDefaults::MissingDefaultPostProcessMaterialReason);
	return false;
}

UCameraComponent* UVHSEffectComponent::GetBoundPostProcessCamera() const
{
	return BoundPostProcessCamera.Get();
}

void UVHSEffectComponent::CreateDynamicMaterialInstance()
{
	if (!VHSPostProcessMaterial || DynamicPostProcessMaterial)
	{
		return;
	}

	DynamicPostProcessMaterial = UMaterialInstanceDynamic::Create(VHSPostProcessMaterial, this);

	if (DynamicPostProcessMaterial && NoiseGenerator)
	{
		NoiseGenerator->Initialize(DynamicPostProcessMaterial);
	}
}

void UVHSEffectComponent::UpdateFeedbackState(EQuantumCameraMode NewMode)
{
	const bool bNewFeedbackActive = bBodycamEnabled && NewMode != EQuantumCameraMode::Disabled;
	const EQuantumCameraMode ResolvedMode = bNewFeedbackActive ? NewMode : EQuantumCameraMode::Disabled;
	if (bFeedbackActive == bNewFeedbackActive && FeedbackCameraMode == ResolvedMode)
	{
		return;
	}

	bFeedbackActive = bNewFeedbackActive;
	FeedbackCameraMode = ResolvedMode;
	ApplyPostProcessBlendWeight();
	OnVHSFeedbackChanged.Broadcast(bFeedbackActive, FeedbackCameraMode);
}

void UVHSEffectComponent::ApplyPostProcessBlendWeight()
{
	if (!BoundPostProcessCamera)
	{
		return;
	}

	UMaterialInterface* MaterialToUse = DynamicPostProcessMaterial ? Cast<UMaterialInterface>(DynamicPostProcessMaterial) : VHSPostProcessMaterial.Get();
	if (!MaterialToUse)
	{
		return;
	}

	BoundPostProcessCamera->PostProcessSettings.AddBlendable(
		MaterialToUse,
		bFeedbackActive ? FMath::Clamp(VHSPostProcessBlendWeight, 0.0f, 1.0f) : 0.0f);
}
