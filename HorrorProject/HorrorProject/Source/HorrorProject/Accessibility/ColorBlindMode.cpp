// Copyright Epic Games, Inc. All Rights Reserved.

#include "ColorBlindMode.h"

AColorBlindMode::AColorBlindMode()
{
    PrimaryActorTick.bCanEverTick = false;
}

FLinearColor AColorBlindMode::TransformColorProtanopia(const FLinearColor& Color, float Severity)
{
    FMatrix Matrix = GetProtanopiaMatrix();
    FLinearColor Transformed = ApplyColorMatrix(Color, Matrix);
    return FMath::Lerp(Color, Transformed, Severity);
}

FLinearColor AColorBlindMode::TransformColorDeuteranopia(const FLinearColor& Color, float Severity)
{
    FMatrix Matrix = GetDeuteranopiaMatrix();
    FLinearColor Transformed = ApplyColorMatrix(Color, Matrix);
    return FMath::Lerp(Color, Transformed, Severity);
}

FLinearColor AColorBlindMode::TransformColorTritanopia(const FLinearColor& Color, float Severity)
{
    FMatrix Matrix = GetTritanopiaMatrix();
    FLinearColor Transformed = ApplyColorMatrix(Color, Matrix);
    return FMath::Lerp(Color, Transformed, Severity);
}

FLinearColor AColorBlindMode::SimulateColorBlindness(const FLinearColor& Color, int32 ColorBlindType, float Severity)
{
    switch (ColorBlindType)
    {
        case 1: return TransformColorProtanopia(Color, Severity);
        case 2: return TransformColorDeuteranopia(Color, Severity);
        case 3: return TransformColorTritanopia(Color, Severity);
        default: return Color;
    }
}

bool AColorBlindMode::IsColorDistinguishable(const FLinearColor& Color1, const FLinearColor& Color2, int32 ColorBlindType)
{
    FLinearColor Simulated1 = SimulateColorBlindness(Color1, ColorBlindType);
    FLinearColor Simulated2 = SimulateColorBlindness(Color2, ColorBlindType);

    float Distance = FVector(
        Simulated1.R - Simulated2.R,
        Simulated1.G - Simulated2.G,
        Simulated1.B - Simulated2.B
    ).Size();

    return Distance > 0.2f; // Threshold for distinguishability
}

FLinearColor AColorBlindMode::GetAccessibleAlternative(const FLinearColor& Color, int32 ColorBlindType)
{
    // Provide high-contrast alternatives
    float Luminance = Color.GetLuminance();

    if (Luminance > 0.5f)
    {
        return FLinearColor(0.1f, 0.1f, 0.1f, Color.A); // Dark alternative
    }
    else
    {
        return FLinearColor(0.9f, 0.9f, 0.9f, Color.A); // Light alternative
    }
}

FMatrix AColorBlindMode::GetProtanopiaMatrix()
{
    return FMatrix(
        FPlane(0.567f, 0.433f, 0.0f, 0.0f),
        FPlane(0.558f, 0.442f, 0.0f, 0.0f),
        FPlane(0.0f, 0.242f, 0.758f, 0.0f),
        FPlane(0.0f, 0.0f, 0.0f, 1.0f));
}

FMatrix AColorBlindMode::GetDeuteranopiaMatrix()
{
    return FMatrix(
        FPlane(0.625f, 0.375f, 0.0f, 0.0f),
        FPlane(0.7f, 0.3f, 0.0f, 0.0f),
        FPlane(0.0f, 0.3f, 0.7f, 0.0f),
        FPlane(0.0f, 0.0f, 0.0f, 1.0f));
}

FMatrix AColorBlindMode::GetTritanopiaMatrix()
{
    return FMatrix(
        FPlane(0.95f, 0.05f, 0.0f, 0.0f),
        FPlane(0.0f, 0.433f, 0.567f, 0.0f),
        FPlane(0.0f, 0.475f, 0.525f, 0.0f),
        FPlane(0.0f, 0.0f, 0.0f, 1.0f));
}

FLinearColor AColorBlindMode::ApplyColorMatrix(const FLinearColor& Color, const FMatrix& Matrix)
{
    const FVector4 Source(Color.R, Color.G, Color.B, Color.A);
    const FVector4 Transformed = Matrix.TransformFVector4(Source);
    return FLinearColor(Transformed.X, Transformed.Y, Transformed.Z, Color.A);
}
