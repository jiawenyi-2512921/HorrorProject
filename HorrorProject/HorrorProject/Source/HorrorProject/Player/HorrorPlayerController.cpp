// Copyright Epic Games, Inc. All Rights Reserved.


#include "Player/HorrorPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "HorrorProjectCameraManager.h"
#include "Variant_Horror/HorrorCharacter.h"
#include "Variant_Horror/UI/HorrorUI.h"
#include "HorrorProject.h"
#include "Widgets/Input/SVirtualJoystick.h"

AHorrorPlayerController::AHorrorPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AHorrorProjectCameraManager::StaticClass();
}

void AHorrorPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (SVirtualJoystick::ShouldDisplayTouchInterface() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogHorrorProject, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}

	ApplyInputMappingContexts();
}

void AHorrorPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	// only spawn UI on local player controllers
	if (IsLocalPlayerController())
	{
		// set up the UI for the character
		if (AHorrorCharacter* HorrorCharacter = Cast<AHorrorCharacter>(aPawn))
		{
			// create the UI
			if (!HorrorUI)
			{
				HorrorUI = CreateWidget<UHorrorUI>(this, HorrorUIClass);
				if (HorrorUI)
				{
					HorrorUI->AddToViewport(0);
				}
				else
				{
					UE_LOG(LogHorrorProject, Error, TEXT("Could not spawn horror UI widget."));
					return;
				}
			}

			HorrorUI->SetupCharacter(HorrorCharacter);
		}
	}

}

void AHorrorPlayerController::RestampCheckpointLoadedUIState()
{
	if (HorrorUI)
	{
		HorrorUI->RestampCheckpointLoadedState();
	}
}

void AHorrorPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	ApplyInputMappingContexts();
}

void AHorrorPlayerController::ApplyInputMappingContexts()
{
	if (bInputContextsApplied || !IsLocalPlayerController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			if (CurrentContext)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}

		if (!SVirtualJoystick::ShouldDisplayTouchInterface())
		{
			for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
			{
				if (CurrentContext)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}

		bInputContextsApplied = true;
	}
}
