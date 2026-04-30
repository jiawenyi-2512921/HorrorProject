// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LocalizationSubsystem.generated.h"

UENUM(BlueprintType)
enum class ELanguage : uint8
{
	English UMETA(DisplayName = "英语"),
	Chinese UMETA(DisplayName = "简体中文"),
	Japanese UMETA(DisplayName = "日语"),
	Korean UMETA(DisplayName = "韩语"),
	Spanish UMETA(DisplayName = "西班牙语")
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
	bool HasLocalizedText(const FString& Key) const;

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
