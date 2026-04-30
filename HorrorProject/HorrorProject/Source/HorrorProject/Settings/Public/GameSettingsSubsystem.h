// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameSettingsSubsystem.generated.h"

class UGraphicsSettings;
class UHorrorAudioSettings;
class UControlSettings;
class UGameplaySettings;
class UConfigManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettingsChanged, FName, SettingsCategory);

/**
 * Game Settings Subsystem
 * Manages all game settings including graphics, audio, controls, and gameplay
 */
UCLASS()
class HORRORPROJECT_API UGameSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Settings access
	UFUNCTION(BlueprintPure, Category = "Settings")
	UGraphicsSettings* GetGraphicsSettings() const { return GraphicsSettings; }

	UFUNCTION(BlueprintPure, Category = "Settings")
	UHorrorAudioSettings* GetAudioSettings() const { return AudioSettings; }

	UFUNCTION(BlueprintPure, Category = "Settings")
	UControlSettings* GetControlSettings() const { return ControlSettings; }

	UFUNCTION(BlueprintPure, Category = "Settings")
	UGameplaySettings* GetGameplaySettings() const { return GameplaySettings; }

	// Settings operations
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadSettings();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SaveSettings();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ResetToDefaults(FName Category = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplySettings(FName Category = NAME_None);

	// Import/Export
	UFUNCTION(BlueprintCallable, Category = "Settings")
	bool ExportSettings(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "Settings")
	bool ImportSettings(const FString& FilePath);

	// Validation
	UFUNCTION(BlueprintCallable, Category = "Settings")
	bool ValidateSettings(FName Category = NAME_None);

	// Auto-detection
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void AutoDetectSettings();

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnSettingsChanged OnSettingsChanged;

	// Quality presets
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyQualityPreset(int32 PresetLevel);

	UFUNCTION(BlueprintPure, Category = "Settings")
	TArray<FString> GetAvailableQualityPresets() const;

	// Dirty tracking
	UFUNCTION(BlueprintPure, Category = "Settings")
	bool HasUnsavedChanges() const { return bHasUnsavedChanges; }

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void MarkDirty(FName Category);

private:
	void InitializeSettings();
	void LoadDefaultSettings();
	void MigrateSettings();
	void NotifySettingsChanged(FName Category);

	UPROPERTY()
	TObjectPtr<UGraphicsSettings> GraphicsSettings;

	UPROPERTY()
	TObjectPtr<UHorrorAudioSettings> AudioSettings;

	UPROPERTY()
	TObjectPtr<UControlSettings> ControlSettings;

	UPROPERTY()
	TObjectPtr<UGameplaySettings> GameplaySettings;

	UPROPERTY()
	TObjectPtr<UConfigManager> ConfigManager;

	bool bHasUnsavedChanges = false;
	TSet<FName> DirtyCategories;
};
