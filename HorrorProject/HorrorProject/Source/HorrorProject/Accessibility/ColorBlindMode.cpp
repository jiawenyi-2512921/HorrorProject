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
    // Protanopia (Red-blind) transformation matrix
    FMatrix Matrix;
    Matrix.M[0][0] = 0.567f; Matrix.M[0][1] = 0.433f; Matrix.M[0][2] = 0.0f;   Matrix.M[0][3] = 0.0f;
    Matrix.M[1][0] = 0.558f; Matrix.M[1][1] = 0.442f; Matrix.M[1][2] = 0.0f;   Matrix.M[1][3] = 0.0f;
    Matrix.M[2][0] = 0.0f;   Matrix.M[2][1] = 0.242f; Matrix.M[2][2] = 0.758f; Matrix.M[2][3] = 0.0f;
    Matrix.M[3][0] = 0.0f;   Matrix.M[3][1] = 0.0f;   Matrix.M[3][2] = 0.0f;   Matrix.M[3][3] = 1.0f;
    return Matrix;
}

FMatrix AColorBlindMode::GetDeuteranopiaMatrix()
{
    // Deuteranopia (Green-blind) transformation matrix
    FMatrix Matrix;
    Matrix.M[0][0] = 0.625f; Matrix.M[0][1] = 0.375f; Matrix.M[0][2] = 0.0f;   Matrix.M[0][3] = 0.0f;
    Matrix.M[1][0] = 0.7f;   Matrix.M[1][1] = 0.3f;   Matrix.M[1][2] = 0.0f;   Matrix.M[1][3] = 0.0f;
    Matrix.M[2][0] = 0.0f;   Matrix.M[2][1] = 0.3f;   Matrix.M[2][2] = 0.7f;   Matrix.M[2][3] = 0.0f;
    Matrix.M[3][0] = 0.0f;   Matrix.M[3][1] = 0.0f;   Matrix.M[3][2] = 0.0f;   Matrix.M[3][3] = 1.0f;
    return Matrix;
}

FMatrix AColorBlindMode::GetTritanopiaMatrix()
{
    // Tritanopia (Blue-blind) transformation matrix
    FMatrix Matrix;
    Matrix.M[0][0] = 0.95f;  Matrix.M[0][1] = 0.05f;  Matrix.M[0][2] = 0.0f;   Matrix.M[0][3] = 0.0f;
    Matrix.M[1][0] = 0.0f;   Matrix.M[1][1] = 0.433f; Matrix.M[1][2] = 0.567f; Matrix.M[1][3] = 0.0f;
    Matrix.M[2][0] = 0.0f;   Matrix.M[2][1] = 0.475f; Matrix.M[2][2] = 0.525f; Matrix.M[2][3] = 0.0f;
    Matrix.M[3][0] = 0.0f;   Matrix.M[3][1] = 0.0f;   Matrix.M[3][2] = 0.0f;   Matrix.M[3][3] = 1.0f;
    return Matrix;
}

FLinearColor AColorBlindMode::ApplyColorMatrix(const FLinearColor& Color, const FMatrix& Matrix)
{
    FLinearColor Result;
    Result.R = Color.R * Matrix.M[0][0] + Color.G * Matrix.M[0][1] + Color.B * Matrix.M[0][2];
    Result.G = Color.R * Matrix.M[1][0] + Color.G * Matrix.M[1][1] + Color.B * Matrix.M[1][2];
    Result.B = Color.R * Matrix.M[2][0] + Color.G * Matrix.M[2][1] + Color.B * Matrix.M[2][2];
    Result.A = Color.A;
    return Result;
}
