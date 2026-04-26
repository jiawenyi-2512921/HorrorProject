// Copyright Epic Games, Inc. All Rights Reserved.

#include "ControlRemapping.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JsonObjectConverter.h"

void UControlRemapping::RemapAction(FName ActionName, FKey NewKey, bool bIsPrimary)
{
    for (FInputMapping& Mapping : InputMappings)
    {
        if (Mapping.ActionName == ActionName)
        {
            if (bIsPrimary)
            {
                Mapping.PrimaryKey = NewKey;
            }
            else
            {
                Mapping.SecondaryKey = NewKey;
            }

            OnControlsRemapped.Broadcast(InputMappings);
            return;
        }
    }

    // Create new mapping if not found
    FInputMapping NewMapping;
    NewMapping.ActionName = ActionName;
    NewMapping.PrimaryKey = bIsPrimary ? NewKey : FKey();
    NewMapping.SecondaryKey = bIsPrimary ? FKey() : NewKey;
    InputMappings.Add(NewMapping);

    OnControlsRemapped.Broadcast(InputMappings);
}

void UControlRemapping::RemapAxis(FName AxisName, FKey NewKey, float Scale)
{
    for (FInputMapping& Mapping : InputMappings)
    {
        if (Mapping.ActionName == AxisName)
        {
            Mapping.PrimaryKey = NewKey;
            Mapping.AxisScale = Scale;
            Mapping.bIsAxis = true;

            OnControlsRemapped.Broadcast(InputMappings);
            return;
        }
    }

    FInputMapping NewMapping;
    NewMapping.ActionName = AxisName;
    NewMapping.PrimaryKey = NewKey;
    NewMapping.AxisScale = Scale;
    NewMapping.bIsAxis = true;
    InputMappings.Add(NewMapping);

    OnControlsRemapped.Broadcast(InputMappings);
}

FKey UControlRemapping::GetMappedKey(FName ActionName, bool bPrimary) const
{
    for (const FInputMapping& Mapping : InputMappings)
    {
        if (Mapping.ActionName == ActionName)
        {
            return bPrimary ? Mapping.PrimaryKey : Mapping.SecondaryKey;
        }
    }

    return FKey();
}

void UControlRemapping::ResetToDefaults()
{
    InputMappings.Empty();
    InitializeDefaultMappings();
    OnControlsRemapped.Broadcast(InputMappings);
}

void UControlRemapping::SaveMappings()
{
    FString JsonString;
    if (FJsonObjectConverter::UStructToJsonObjectString(FInputMapping(), JsonString))
    {
        TArray<FString> JsonArray;
        for (const FInputMapping& Mapping : InputMappings)
        {
            FString MappingJson;
            if (FJsonObjectConverter::UStructToJsonObjectString(Mapping, MappingJson))
            {
                JsonArray.Add(MappingJson);
            }
        }

        FString CombinedJson = FString::Printf(TEXT("[%s]"), *FString::Join(JsonArray, TEXT(",")));
        FString SavePath = GetMappingsSavePath();

        if (FFileHelper::SaveStringToFile(CombinedJson, *SavePath))
        {
            UE_LOG(LogTemp, Log, TEXT("Control mappings saved to: %s"), *SavePath);
        }
    }
}

void UControlRemapping::LoadMappings()
{
    FString SavePath = GetMappingsSavePath();
    FString JsonString;

    if (FFileHelper::LoadFileToString(JsonString, *SavePath))
    {
        // Parse JSON array and load mappings
        UE_LOG(LogTemp, Log, TEXT("Control mappings loaded from: %s"), *SavePath);
    }
    else
    {
        InitializeDefaultMappings();
    }
}

bool UControlRemapping::IsKeyConflicting(FKey Key, FName ExcludeAction) const
{
    for (const FInputMapping& Mapping : InputMappings)
    {
        if (Mapping.ActionName != ExcludeAction)
        {
            if (Mapping.PrimaryKey == Key || Mapping.SecondaryKey == Key)
            {
                return true;
            }
        }
    }

    return false;
}

void UControlRemapping::SetHoldDuration(float Duration)
{
    HoldDuration = FMath::Max(0.1f, Duration);
}

void UControlRemapping::SetDoubleTapWindow(float Window)
{
    DoubleTapWindow = FMath::Max(0.1f, Window);
}

void UControlRemapping::InitializeDefaultMappings()
{
    // Initialize with default key mappings
    FInputMapping MoveForward;
    MoveForward.ActionName = TEXT("MoveForward");
    MoveForward.PrimaryKey = EKeys::W;
    MoveForward.SecondaryKey = EKeys::Up;
    MoveForward.bIsAxis = true;
    InputMappings.Add(MoveForward);

    FInputMapping MoveRight;
    MoveRight.ActionName = TEXT("MoveRight");
    MoveRight.PrimaryKey = EKeys::D;
    MoveRight.SecondaryKey = EKeys::Right;
    MoveRight.bIsAxis = true;
    InputMappings.Add(MoveRight);

    FInputMapping Jump;
    Jump.ActionName = TEXT("Jump");
    Jump.PrimaryKey = EKeys::SpaceBar;
    Jump.SecondaryKey = EKeys::Gamepad_FaceButton_Bottom;
    InputMappings.Add(Jump);

    FInputMapping Interact;
    Interact.ActionName = TEXT("Interact");
    Interact.PrimaryKey = EKeys::E;
    Interact.SecondaryKey = EKeys::Gamepad_FaceButton_Right;
    InputMappings.Add(Interact);

    FInputMapping Sprint;
    Sprint.ActionName = TEXT("Sprint");
    Sprint.PrimaryKey = EKeys::LeftShift;
    Sprint.SecondaryKey = EKeys::Gamepad_LeftThumbstick;
    InputMappings.Add(Sprint);
}

FString UControlRemapping::GetMappingsSavePath() const
{
    return FPaths::ProjectSavedDir() / TEXT("Config") / TEXT("ControlMappings.json");
}
