// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HorrorProjectPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

/**
 *  DEPRECATED: legacy template PlayerController from the FirstPerson template.
 *  Live PlayerController is `AHorrorPlayerController` (Player/HorrorPlayerController.h)
 *  which inherits directly from APlayerController. Kept only as a graveyard so
 *  old BP class references don't fail to load.
 */
UCLASS(abstract, Hidden, NotBlueprintable, NotPlaceable)
class HORRORPROJECT_API AHorrorProjectPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	AHorrorProjectPlayerController();

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

};
