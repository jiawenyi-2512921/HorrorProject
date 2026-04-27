// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LocalizationSubsystem.h"
#include "UILocalization.generated.h"

class UTextBlock;
class UImage;

USTRUCT(BlueprintType)
struct FLocalizedTextWidget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTextBlock* TextBlock = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LocalizationKey;
};

USTRUCT(BlueprintType)
struct FLocalizedImageWidget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UImage* Image = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LocalizationKey;
};

/**
 * Adds UILocalization Component behavior to its owning actor in the Localization module.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HORRORPROJECT_API UUILocalizationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Localization")
	void UpdateLocalization();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TArray<FLocalizedTextWidget> LocalizedTextWidgets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Localization")
	TArray<FLocalizedImageWidget> LocalizedImageWidgets;

private:
	UFUNCTION()
	void OnLanguageChanged(ELanguage NewLanguage);

	UTexture2D* GetLocalizedTexture(const FString& Key, const FString& LanguageCode) const;
};

/**
 * Exposes Blueprint helper functions for UILocalization Library workflows.
 */
UCLASS(BlueprintType)
class HORRORPROJECT_API UUILocalizationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Localization|UI")
	static void LocalizeTextBlock(UTextBlock* TextBlock, const FString& LocalizationKey);

	UFUNCTION(BlueprintCallable, Category = "Localization|UI")
	static void LocalizeImage(UImage* Image, const FString& LocalizationKey);

	UFUNCTION(BlueprintPure, Category = "Localization|UI")
	static FString GetFontPathForLanguage(ELanguage Language);

	UFUNCTION(BlueprintPure, Category = "Localization|UI")
	static float GetTextScaleForLanguage(ELanguage Language);
};
