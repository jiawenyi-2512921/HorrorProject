// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LocalizationSubsystem.generated.h"

UENUM(BlueprintType)
enum class ELanguage : uint8
{
	English UMETA(DisplayName = "English"),
	Chinese UMETA(DisplayName = "中文"),
	Japanese UMETA(DisplayName = "日本語"),
	Korean UMETA(DisplayName = "한국어"),
	Spanish UMETA(DisplayName = "Español")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLanguageChanged, ELanguage, NewLanguage);

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
