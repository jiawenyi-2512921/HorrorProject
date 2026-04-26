// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsMenuWidget.generated.h"

UENUM(BlueprintType)
enum class ESettingsCategory : uint8
{
	Graphics,
	Audio,
	Controls,
	Gameplay,
	Accessibility
};

/**
 * Settings menu with graphics, audio, controls, and accessibility options
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class HORRORPROJECT_API USettingsMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category="Settings|UI")
	void OpenSettings();

	UFUNCTION(BlueprintCallable, Category="Settings|UI")
	void CloseSettings();

	UFUNCTION(BlueprintCallable, Category="Settings|UI")
	void ApplySettings();

	UFUNCTION(BlueprintCallable, Category="Settings|UI")
	void ResetToDefaults();

	UFUNCTION(BlueprintCallable, Category="Settings|UI")
	void SelectCategory(ESettingsCategory Category);

	UFUNCTION(BlueprintPure, Category="Settings|UI")
	bool IsOpen() const { return bIsOpen; }

	UFUNCTION(BlueprintPure, Category="Settings|UI")
	bool HasUnsavedChanges() const { return bHasUnsavedChanges; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category="Settings|UI", meta=(DisplayName="Settings Opened"))
	void BP_SettingsOpened();

	UFUNCTION(BlueprintImplementableEvent, Category="Settings|UI", meta=(DisplayName="Settings Closed"))
	void BP_SettingsClosed();

	UFUNCTION(BlueprintImplementableEvent, Category="Settings|UI", meta=(DisplayName="Settings Applied"))
	void BP_SettingsApplied();

	UFUNCTION(BlueprintImplementableEvent, Category="Settings|UI", meta=(DisplayName="Category Selected"))
	void BP_CategorySelected(ESettingsCategory Category);

	UPROPERTY(BlueprintReadOnly, Category="Settings|UI")
	bool bIsOpen = false;

	UPROPERTY(BlueprintReadOnly, Category="Settings|UI")
	bool bHasUnsavedChanges = false;

	UPROPERTY(BlueprintReadOnly, Category="Settings|UI")
	ESettingsCategory CurrentCategory = ESettingsCategory::Graphics;
};
