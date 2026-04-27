// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TextLocalization.generated.h"

USTRUCT(BlueprintType)
struct FLocalizedString
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText English;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Chinese;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Japanese;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Korean;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Spanish;
};

/**
 * Exposes Blueprint helper functions for Text Localization Library workflows.
 */
UCLASS(BlueprintType)
class HORRORPROJECT_API UTextLocalizationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Localization|Text")
	static FText GetLocalizedText(const FString& Namespace, const FString& Key);

	UFUNCTION(BlueprintPure, Category = "Localization|Text")
	static FText FormatLocalizedText(const FString& Key, const TArray<FString>& Arguments);

	UFUNCTION(BlueprintPure, Category = "Localization|Text")
	static FText GetPluralText(const FString& Key, int32 Count);

	UFUNCTION(BlueprintCallable, Category = "Localization|Text")
	static void RegisterDynamicText(const FString& Key, const FText& Text);

	UFUNCTION(BlueprintPure, Category = "Localization|Text")
	static FString SanitizeTextForLocalization(const FString& Text);
};

/**
 * Defines Text Localization Data data used by the Localization module.
 */
UCLASS()
class HORRORPROJECT_API UTextLocalizationData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Localization")
	TArray<FLocalizedString> LocalizedStrings;

	UFUNCTION(BlueprintPure, Category = "Localization")
	FText GetText(const FString& Key, const FString& LanguageCode) const;
};
