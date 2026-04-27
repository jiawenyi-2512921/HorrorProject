// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HorrorPlayerController.generated.h"

class UInputMappingContext;
class UHorrorUI;
class UUserWidget;

/**
 *  Player Controller for a first person horror game
 *  Manages input mappings
 *  Manages UI
 */
UCLASS()
class HORRORPROJECT_API AHorrorPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Type of UI widget to spawn */
	UPROPERTY(EditAnywhere, Category="Horror|UI")
	TSubclassOf<UHorrorUI> HorrorUIClass;

	/** Temporary prototype HUD toggle. Keep disabled for playable smoke builds. */
	UPROPERTY(EditAnywhere, Category="Horror|UI")
	bool bAutoCreateHorrorUI = false;

	/** Pointer to the UI widget */
	TObjectPtr<UHorrorUI> HorrorUI;

public:

	/** Constructor */
	AHorrorPlayerController();

	/** Restamps state-derived UI after checkpoint restore without creating new widgets. */
	void RestampCheckpointLoadedUIState();

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** Gameplay Initialization */
	virtual void BeginPlay() override;

	/** Possessed pawn initialization */
	virtual void OnPossess(APawn* aPawn) override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Adds the configured mapping contexts to the local enhanced input subsystem once. */
	void ApplyInputMappingContexts();

private:

	/** Prevents duplicate IMC registration across BeginPlay / SetupInputComponent. */
	bool bInputContextsApplied = false;

};
