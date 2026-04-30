// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HorrorPlayerCharacter.h"
#include "Player/Components/InteractionComponent.h"
#include "Player/Components/InventoryComponent.h"
#include "Player/Components/VHSEffectComponent.h"
#include "Player/Components/NoteRecorderComponent.h"
#include "Player/Components/QuantumCameraComponent.h"
#include "Player/Components/CameraRecordingComponent.h"
#include "Player/Components/CameraPhotoComponent.h"
#include "Player/Components/FlashlightComponent.h"
#include "Player/Components/FearComponent.h"
#include "Player/Components/NoiseGeneratorComponent.h"
#include "Player/Components/SanityComponent.h"
#include "Player/Components/EnvironmentalStoryComponent.h"
#include "Evidence/EvidenceCollectionComponent.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "Game/HorrorGameModeBase.h"
#include "HorrorProject.h"
#include "InputAction.h"
#include "Kismet/GameplayStatics.h"
#include "Player/HorrorPlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/SoftObjectPath.h"

namespace HorrorPlayerCharacterDefaults
{
	constexpr TCHAR InteractActionAssetPath[] = TEXT("/Game/Input/Actions/IA_Interact.IA_Interact");
	constexpr TCHAR RecordActionAssetPath[] = TEXT("/Game/Input/Actions/IA_Record.IA_Record");
	constexpr TCHAR TakePhotoActionAssetPath[] = TEXT("/Game/Input/Actions/IA_TakePhoto.IA_TakePhoto");
	constexpr TCHAR RewindActionAssetPath[] = TEXT("/Game/Input/Actions/IA_Rewind.IA_Rewind");
	constexpr TCHAR OpenArchiveActionAssetPath[] = TEXT("/Game/Input/Actions/IA_OpenArchive.IA_OpenArchive");
	constexpr TCHAR ToggleFlashlightActionAssetPath[] = TEXT("/Game/Input/Actions/IA_Flashlight.IA_Flashlight");
}

namespace
{
	template <typename UserClass, typename HandlerType>
	void BindActionAndStoreHandle(
		UEnhancedInputComponent* EnhancedInputComponent,
		const UInputAction* Action,
		const ETriggerEvent TriggerEvent,
		UserClass* Object,
		HandlerType Handler,
		uint32& BindingHandle)
	{
		if (!EnhancedInputComponent)
		{
			return;
		}

		if (BindingHandle != 0)
		{
			EnhancedInputComponent->RemoveBindingByHandle(BindingHandle);
			BindingHandle = 0;
		}

		if (!Action)
		{
			return;
		}

		BindingHandle = EnhancedInputComponent->BindAction(Action, TriggerEvent, Object, Handler).GetHandle();
	}
}

AHorrorPlayerCharacter::AHorrorPlayerCharacter()
{
	Interaction = CreateDefaultSubobject<UInteractionComponent>(TEXT("Interaction"));
	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	VHSEffect = CreateDefaultSubobject<UVHSEffectComponent>(TEXT("VHSEffect"));
	NoteRecorder = CreateDefaultSubobject<UNoteRecorderComponent>(TEXT("NoteRecorder"));
	QuantumCamera = CreateDefaultSubobject<UQuantumCameraComponent>(TEXT("QuantumCamera"));
	CameraRecording = CreateDefaultSubobject<UCameraRecordingComponent>(TEXT("CameraRecording"));
	CameraPhoto = CreateDefaultSubobject<UCameraPhotoComponent>(TEXT("CameraPhoto"));
	Flashlight = CreateDefaultSubobject<UFlashlightComponent>(TEXT("Flashlight"));
	Fear = CreateDefaultSubobject<UFearComponent>(TEXT("Fear"));
	NoiseGenerator = CreateDefaultSubobject<UNoiseGeneratorComponent>(TEXT("NoiseGenerator"));
	EvidenceCollection = CreateDefaultSubobject<UEvidenceCollectionComponent>(TEXT("EvidenceCollection"));
	Sanity = CreateDefaultSubobject<USanityComponent>(TEXT("Sanity"));
	EnvironmentalStory = CreateDefaultSubobject<UEnvironmentalStoryComponent>(TEXT("EnvironmentalStory"));
	BindQuantumCameraDelegates();

	static ConstructorHelpers::FObjectFinder<UInputAction> InteractActionAsset(HorrorPlayerCharacterDefaults::InteractActionAssetPath);
	static ConstructorHelpers::FObjectFinder<UInputAction> RecordActionAsset(HorrorPlayerCharacterDefaults::RecordActionAssetPath);
	static ConstructorHelpers::FObjectFinder<UInputAction> TakePhotoActionAsset(HorrorPlayerCharacterDefaults::TakePhotoActionAssetPath);
	static ConstructorHelpers::FObjectFinder<UInputAction> RewindActionAsset(HorrorPlayerCharacterDefaults::RewindActionAssetPath);
	static ConstructorHelpers::FObjectFinder<UInputAction> OpenArchiveActionAsset(HorrorPlayerCharacterDefaults::OpenArchiveActionAssetPath);
	static ConstructorHelpers::FObjectFinder<UInputAction> ToggleFlashlightActionAsset(HorrorPlayerCharacterDefaults::ToggleFlashlightActionAssetPath);

	if (InteractActionAsset.Succeeded())
	{
		InteractAction = InteractActionAsset.Object;
	}

	if (RecordActionAsset.Succeeded())
	{
		RecordAction = RecordActionAsset.Object;
	}

	if (TakePhotoActionAsset.Succeeded())
	{
		TakePhotoAction = TakePhotoActionAsset.Object;
	}

	if (RewindActionAsset.Succeeded())
	{
		RewindAction = RewindActionAsset.Object;
	}

	if (OpenArchiveActionAsset.Succeeded())
	{
		OpenArchiveAction = OpenArchiveActionAsset.Object;
	}

	if (ToggleFlashlightActionAsset.Succeeded())
	{
		ToggleFlashlightAction = ToggleFlashlightActionAsset.Object;
	}
}

UInputAction* AHorrorPlayerCharacter::ResolveInteractAction()
{
	return ResolveInputAction(&AHorrorPlayerCharacter::InteractAction, HorrorPlayerCharacterDefaults::InteractActionAssetPath, TEXT("InteractAction"));
}

void AHorrorPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	BindQuantumCameraDelegates();
	if (VHSEffect)
	{
		VHSEffect->ResolveDefaultPostProcessMaterial();
		VHSEffect->BindPostProcessCamera(GetFirstPersonCameraComponent());
	}

	if (Flashlight && GetSpotLight())
	{
		Flashlight->BindSpotLight(GetSpotLight());
	}

	AHorrorGameModeBase* HorrorGameMode = Cast<AHorrorGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (HorrorGameMode && HorrorGameMode->ShouldAutoGrantBodycamOnPlayerBeginPlay())
	{
		HorrorGameMode->TryAcquireBodycam(TEXT("PlayerBootstrap"), true);
	}

	if (HorrorGameMode)
	{
		HorrorGameMode->SyncFoundFootageRuntimeStateToPlayer();
	}
}

void AHorrorPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	BindQuantumCameraDelegates();
}

UInputAction* AHorrorPlayerCharacter::ResolveInputAction(FInputActionMember ActionMember, const TCHAR* AssetPath, const TCHAR* DebugName)
{
	if (UInputAction* ExistingAction = (this->*ActionMember).Get())
	{
		return ExistingAction;
	}

	const AHorrorPlayerCharacter* const NativeDefaults = GetDefault<AHorrorPlayerCharacter>();
	const AHorrorPlayerCharacter* const ClassDefaults = GetClass() ? GetClass()->GetDefaultObject<AHorrorPlayerCharacter>() : nullptr;
	if (NativeDefaults
		&& ClassDefaults
		&& ClassDefaults != NativeDefaults
		&& (ClassDefaults->*ActionMember) == nullptr)
	{
		UE_LOG(LogHorrorProject, Verbose, TEXT("%s was intentionally cleared in class defaults for %s."), DebugName, *GetNameSafe(GetClass()));
		return nullptr;
	}

	const FSoftObjectPath DefaultActionPath(AssetPath);
	if (UInputAction* LoadedAction = Cast<UInputAction>(DefaultActionPath.TryLoad()))
	{
		(this->*ActionMember) = LoadedAction;
		UE_LOG(LogHorrorProject, Verbose, TEXT("Recovered %s from %s for %s."), DebugName, AssetPath, *GetNameSafe(this));
		return LoadedAction;
	}

	UE_LOG(LogHorrorProject, Warning, TEXT("Failed to resolve %s default %s for %s."), DebugName, AssetPath, *GetNameSafe(this));
	return nullptr;
}

void AHorrorPlayerCharacter::BindQuantumCameraDelegates()
{
	if (!QuantumCamera)
	{
		return;
	}

	QuantumCamera->OnCameraModeChanged.RemoveDynamic(this, &AHorrorPlayerCharacter::HandleQuantumCameraModeChanged);
	QuantumCamera->OnCameraModeChanged.AddDynamic(this, &AHorrorPlayerCharacter::HandleQuantumCameraModeChanged);
	QuantumCamera->OnCameraAcquiredChanged.RemoveDynamic(this, &AHorrorPlayerCharacter::HandleQuantumCameraAcquiredChanged);
	QuantumCamera->OnCameraAcquiredChanged.AddDynamic(this, &AHorrorPlayerCharacter::HandleQuantumCameraAcquiredChanged);
	QuantumCamera->OnCameraEnabledChanged.RemoveDynamic(this, &AHorrorPlayerCharacter::HandleQuantumCameraEnabledChanged);
	QuantumCamera->OnCameraEnabledChanged.AddDynamic(this, &AHorrorPlayerCharacter::HandleQuantumCameraEnabledChanged);
}

void AHorrorPlayerCharacter::RebindInteractStarted(UEnhancedInputComponent* EnhancedInputComponent, const UInputAction* Action)
{
	if (!EnhancedInputComponent)
	{
		return;
	}

	if (InteractStartedBindingHandle != 0)
	{
		EnhancedInputComponent->RemoveBindingByHandle(InteractStartedBindingHandle);
		InteractStartedBindingHandle = 0;
	}

	if (!Action)
	{
		return;
	}

	InteractStartedBindingHandle = EnhancedInputComponent
		->BindAction(Action, ETriggerEvent::Started, this, &AHorrorPlayerCharacter::DoInteract)
		.GetHandle();
}

void AHorrorPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		const UInputAction* ResolvedInteractAction = ResolveInteractAction();
		RebindInteractStarted(EnhancedInputComponent, ResolvedInteractAction);

		if (!ResolvedInteractAction)
		{
			UE_LOG(LogHorrorProject, Warning, TEXT("InteractAction is not available on %s."), *GetNameSafe(this));
		}

		const UInputAction* ResolvedRecordAction = ResolveInputAction(&AHorrorPlayerCharacter::RecordAction, HorrorPlayerCharacterDefaults::RecordActionAssetPath, TEXT("RecordAction"));
		BindActionAndStoreHandle(EnhancedInputComponent, ResolvedRecordAction, ETriggerEvent::Started, this, &AHorrorPlayerCharacter::DoStartRecord, RecordStartedBindingHandle);
		BindActionAndStoreHandle(EnhancedInputComponent, ResolvedRecordAction, ETriggerEvent::Completed, this, &AHorrorPlayerCharacter::DoStopRecord, RecordCompletedBindingHandle);
		BindActionAndStoreHandle(EnhancedInputComponent, ResolvedRecordAction, ETriggerEvent::Canceled, this, &AHorrorPlayerCharacter::DoStopRecord, RecordCanceledBindingHandle);

		const UInputAction* ResolvedTakePhotoAction = ResolveInputAction(&AHorrorPlayerCharacter::TakePhotoAction, HorrorPlayerCharacterDefaults::TakePhotoActionAssetPath, TEXT("TakePhotoAction"));
		BindActionAndStoreHandle(EnhancedInputComponent, ResolvedTakePhotoAction, ETriggerEvent::Started, this, &AHorrorPlayerCharacter::DoTakePhoto, TakePhotoStartedBindingHandle);

		const UInputAction* ResolvedRewindAction = ResolveInputAction(&AHorrorPlayerCharacter::RewindAction, HorrorPlayerCharacterDefaults::RewindActionAssetPath, TEXT("RewindAction"));
		BindActionAndStoreHandle(EnhancedInputComponent, ResolvedRewindAction, ETriggerEvent::Started, this, &AHorrorPlayerCharacter::DoStartRewind, RewindStartedBindingHandle);
		BindActionAndStoreHandle(EnhancedInputComponent, ResolvedRewindAction, ETriggerEvent::Completed, this, &AHorrorPlayerCharacter::DoStopRewind, RewindCompletedBindingHandle);
		BindActionAndStoreHandle(EnhancedInputComponent, ResolvedRewindAction, ETriggerEvent::Canceled, this, &AHorrorPlayerCharacter::DoStopRewind, RewindCanceledBindingHandle);

		const UInputAction* ResolvedOpenArchiveAction = ResolveInputAction(&AHorrorPlayerCharacter::OpenArchiveAction, HorrorPlayerCharacterDefaults::OpenArchiveActionAssetPath, TEXT("OpenArchiveAction"));
		BindActionAndStoreHandle(EnhancedInputComponent, ResolvedOpenArchiveAction, ETriggerEvent::Started, this, &AHorrorPlayerCharacter::DoOpenArchive, OpenArchiveStartedBindingHandle);

		const UInputAction* ResolvedToggleFlashlightAction = ResolveInputAction(&AHorrorPlayerCharacter::ToggleFlashlightAction, HorrorPlayerCharacterDefaults::ToggleFlashlightActionAssetPath, TEXT("ToggleFlashlightAction"));
		BindActionAndStoreHandle(EnhancedInputComponent, ResolvedToggleFlashlightAction, ETriggerEvent::Started, this, &AHorrorPlayerCharacter::DoToggleFlashlight, ToggleFlashlightStartedBindingHandle);
	}
}

void AHorrorPlayerCharacter::DoInteract()
{
	if (AHorrorPlayerController* HorrorPlayerController = Cast<AHorrorPlayerController>(GetController()))
	{
		if (HorrorPlayerController->TrySubmitActiveAdvancedInteractionExpectedInput())
		{
			return;
		}
	}

	if (Interaction)
	{
		Interaction->TryInteract();
	}
}

void AHorrorPlayerCharacter::HandleQuantumCameraModeChanged(EQuantumCameraMode NewMode)
{
	if (VHSEffect)
	{
		VHSEffect->SetFeedbackCameraMode(NewMode);
	}
}

void AHorrorPlayerCharacter::HandleQuantumCameraAcquiredChanged(bool bNewAcquired)
{
	if (VHSEffect)
	{
		VHSEffect->SetBodycamAcquired(bNewAcquired);
	}
}

void AHorrorPlayerCharacter::HandleQuantumCameraEnabledChanged(bool bNewEnabled)
{
	if (VHSEffect)
	{
		VHSEffect->SetBodycamEnabled(bNewEnabled);
	}
}

void AHorrorPlayerCharacter::DoStartRecord()
{
	if (QuantumCamera && QuantumCamera->StartRecording())
	{
		if (CameraRecording)
		{
			CameraRecording->StartRecording();
		}
		HandleQuantumCameraModeChanged(QuantumCamera->GetCameraMode());
	}
}

void AHorrorPlayerCharacter::DoStopRecord()
{
	if (QuantumCamera && QuantumCamera->StopRecording())
	{
		if (CameraRecording)
		{
			CameraRecording->StopRecording();
		}
		HandleQuantumCameraModeChanged(QuantumCamera->GetCameraMode());
	}
}

void AHorrorPlayerCharacter::DoTakePhoto()
{
	if (QuantumCamera && QuantumCamera->TakePhoto())
	{
		if (CameraPhoto)
		{
			CameraPhoto->TakePhoto(true);
		}
		HandleQuantumCameraModeChanged(QuantumCamera->GetCameraMode());
	}
}

void AHorrorPlayerCharacter::DoStartRewind()
{
	if (QuantumCamera && QuantumCamera->StartRewind())
	{
		if (CameraRecording)
		{
			CameraRecording->StartRewind();
		}
		HandleQuantumCameraModeChanged(QuantumCamera->GetCameraMode());
	}
}

void AHorrorPlayerCharacter::DoStopRewind()
{
	if (QuantumCamera && QuantumCamera->StopRewind())
	{
		if (CameraRecording)
		{
			CameraRecording->StopRewind();
		}
		HandleQuantumCameraModeChanged(QuantumCamera->GetCameraMode());
	}
}

void AHorrorPlayerCharacter::DoOpenArchive()
{
	BP_OpenArchive();
}

void AHorrorPlayerCharacter::DoToggleFlashlight()
{
	if (Flashlight)
	{
		Flashlight->ToggleFlashlight();
	}
}
