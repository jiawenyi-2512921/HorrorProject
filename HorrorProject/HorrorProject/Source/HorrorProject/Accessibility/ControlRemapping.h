// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InputCoreTypes.h"
#include "ControlRemapping.generated.h"

USTRUCT(BlueprintType)
struct FInputMapping
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Input")
    FName ActionName;

    UPROPERTY(BlueprintReadWrite, Category = "Input")
    FKey PrimaryKey;

    UPROPERTY(BlueprintReadWrite, Category = "Input")
    FKey SecondaryKey;

    UPROPERTY(BlueprintReadWrite, Category = "Input")
    bool bIsAxis = false;

    UPROPERTY(BlueprintReadWrite, Category = "Input")
    float AxisScale = 1.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnControlsRemapped, const TArray<FInputMapping>&, NewMappings);

/**
 * Control Remapping System
 * Allows full keyboard and gamepad remapping for accessibility
 */
UCLASS(BlueprintType)
class HORRORPROJECT_API UControlRemapping : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Input")
    void RemapAction(FName ActionName, FKey NewKey, bool bIsPrimary = true);

    UFUNCTION(BlueprintCallable, Category = "Input")
    void RemapAxis(FName AxisName, FKey NewKey, float Scale = 1.0f);

    UFUNCTION(BlueprintPure, Category = "Input")
    FKey GetMappedKey(FName ActionName, bool bPrimary = true) const;

    UFUNCTION(BlueprintCallable, Category = "Input")
    void ResetToDefaults();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void SaveMappings();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void LoadMappings();

    UFUNCTION(BlueprintPure, Category = "Input")
    TArray<FInputMapping> GetAllMappings() const { return InputMappings; }

    UFUNCTION(BlueprintPure, Category = "Input")
    bool IsKeyConflicting(FKey Key, FName ExcludeAction) const;

    UFUNCTION(BlueprintCallable, Category = "Input")
    void SetHoldDuration(float Duration);

    UFUNCTION(BlueprintPure, Category = "Input")
    float GetHoldDuration() const { return HoldDuration; }

    UFUNCTION(BlueprintCallable, Category = "Input")
    void SetDoubleTapWindow(float Window);

    UFUNCTION(BlueprintPure, Category = "Input")
    float GetDoubleTapWindow() const { return DoubleTapWindow; }

    UPROPERTY(BlueprintAssignable, Category = "Input")
    FOnControlsRemapped OnControlsRemapped;

private:
    UPROPERTY()
    TArray<FInputMapping> InputMappings;

    UPROPERTY()
    float HoldDuration = 0.5f;

    UPROPERTY()
    float DoubleTapWindow = 0.3f;

    void InitializeDefaultMappings();
    FString GetMappingsSavePath() const;
};
