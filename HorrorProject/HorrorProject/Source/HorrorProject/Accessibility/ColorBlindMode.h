// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ColorBlindMode.generated.h"

USTRUCT(BlueprintType)
struct FColorBlindTransform
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "ColorBlind")
    FMatrix TransformMatrix;

    UPROPERTY(BlueprintReadWrite, Category = "ColorBlind")
    float Severity = 1.0f;

    FColorBlindTransform()
    {
        TransformMatrix = FMatrix::Identity;
    }
};

/**
 * Color Blind Mode Manager
 * Provides color transformation matrices for different types of color blindness
 */
UCLASS()
class HORRORPROJECT_API AColorBlindMode : public AActor
{
    GENERATED_BODY()

public:
    AColorBlindMode();

    UFUNCTION(BlueprintCallable, Category = "ColorBlind")
    static FLinearColor TransformColorProtanopia(const FLinearColor& Color, float Severity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "ColorBlind")
    static FLinearColor TransformColorDeuteranopia(const FLinearColor& Color, float Severity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "ColorBlind")
    static FLinearColor TransformColorTritanopia(const FLinearColor& Color, float Severity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "ColorBlind")
    static FLinearColor SimulateColorBlindness(const FLinearColor& Color, int32 ColorBlindType, float Severity = 1.0f);

    UFUNCTION(BlueprintPure, Category = "ColorBlind")
    static bool IsColorDistinguishable(const FLinearColor& Color1, const FLinearColor& Color2, int32 ColorBlindType);

    UFUNCTION(BlueprintCallable, Category = "ColorBlind")
    static FLinearColor GetAccessibleAlternative(const FLinearColor& Color, int32 ColorBlindType);

private:
    static FMatrix GetProtanopiaMatrix();
    static FMatrix GetDeuteranopiaMatrix();
    static FMatrix GetTritanopiaMatrix();
    static FLinearColor ApplyColorMatrix(const FLinearColor& Color, const FMatrix& Matrix);
};
