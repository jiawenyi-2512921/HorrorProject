// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LocalizationSubsystem.generated.h"

UENUM(BlueprintType)
enum class ELanguage : uint8
{
	English UMETA(DisplayName = "English"),
	Chinese UMETA(DisplayName = "Chinese"),
	Japanese UMETA(DisplayName = "Japanese"),
	Korean UMETA(DisplayName = "Korean"),
	Spanish UMETA(DisplayName = "Spanish")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLanguageChanged, ELanguage, NewLanguage);

/**
 * Coordinates Localization Subsystem services for the Localization module.
 */
UCLASS()
class HORRORPROJECT_API ULocalizationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Localization")
	void SetLanguage(ELanguage NewLanguage);

	UFUNCTION(BlueprintPure, Category = "Localization")
	ELanguage GetCurrentLanguage() const { return CurrentLanguage; }

	UFUNCTION(BlueprintPure, Category = "Localization")
	FString GetLanguageCode(ELanguage Language) const;

	UFUNCTION(BlueprintPure, Category = "Localization")
	FText GetLocalizedText(const FString& Key) const;

	UFUNCTION(BlueprintPure, Category = "Localization")
	bool IsRTLLanguage() const;

	UPROPERTY(BlueprintAssignable, Category = "Localization")
	FOnLanguageChanged OnLanguageChanged;

private:
	void LoadLanguageData(ELanguage Language);
	void ApplyLanguageSettings();

	UPROPERTY()
	ELanguage CurrentLanguage;

	UPROPERTY()
	TMap<FString, FText> LocalizedTexts;
};
