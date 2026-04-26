// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsMenuWidget.generated.h"

class UGameSettingsSubsystem;
class UGraphicsSettingsWidget;
class UAudioSettingsWidget;
class UControlsSettingsWidget;
class UButton;
class UWidgetSwitcher;

/**
 * Settings Menu Widget
 * Main settings menu UI
 */
UCLASS()
class HORRORPROJECT_API USettingsMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	// Tab buttons
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> GraphicsTabButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> AudioTabButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ControlsTabButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> GameplayTabButton;

	// Action buttons
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ApplyButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ResetButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BackButton;

	// Widget switcher for tabs
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> SettingsSwitcher;

	// Settings widgets
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UGraphicsSettingsWidget> GraphicsWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAudioSettingsWidget> AudioWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UControlsSettingsWidget> ControlsWidget;

	// Button callbacks
	UFUNCTION()
	void OnGraphicsTabClicked();

	UFUNCTION()
	void OnAudioTabClicked();

	UFUNCTION()
	void OnControlsTabClicked();

	UFUNCTION()
	void OnGameplayTabClicked();

	UFUNCTION()
	void OnApplyClicked();

	UFUNCTION()
	void OnResetClicked();

	UFUNCTION()
	void OnBackClicked();

	// Settings subsystem
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void InitializeSettings();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void RefreshSettings();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SwitchToTab(int32 TabIndex);

private:
	UPROPERTY()
	TObjectPtr<UGameSettingsSubsystem> SettingsSubsystem;

	int32 CurrentTabIndex;
};
