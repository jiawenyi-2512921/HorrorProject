// Copyright Epic Games, Inc. All Rights Reserved.

#include "ControlRemapping.h"
#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

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
    TArray<FString> JsonArray;
    FString MappingJson;
    for (const FInputMapping& Mapping : InputMappings)
    {
        MappingJson.Reset();
        if (FJsonObjectConverter::UStructToJsonObjectString(Mapping, MappingJson))
        {
            JsonArray.Add(MappingJson);
        }
    }

    const FString CombinedJson = FString::Printf(TEXT("[%s]"), *FString::Join(JsonArray, TEXT(",")));
    const FString SavePath = GetMappingsSavePath();
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(SavePath), true);

    if (FFileHelper::SaveStringToFile(CombinedJson, *SavePath))
    {
        UE_LOG(LogTemp, Log, TEXT("Control mappings saved to: %s"), *SavePath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to save control mappings to: %s"), *SavePath);
    }
}

void UControlRemapping::LoadMappings()
{
    const FString SavePath = GetMappingsSavePath();
    FString JsonString;

    if (FFileHelper::LoadFileToString(JsonString, *SavePath))
    {
        TArray<TSharedPtr<FJsonValue>> MappingValues;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
        if (FJsonSerializer::Deserialize(Reader, MappingValues))
        {
            TArray<FInputMapping> LoadedMappings;
            for (const TSharedPtr<FJsonValue>& MappingValue : MappingValues)
            {
                if (!MappingValue.IsValid() || MappingValue->Type != EJson::Object)
                {
                    continue;
                }

                FInputMapping LoadedMapping;
                if (FJsonObjectConverter::JsonObjectToUStruct(
                    MappingValue->AsObject().ToSharedRef(),
                    FInputMapping::StaticStruct(),
                    &LoadedMapping,
                    0,
                    0) && !LoadedMapping.ActionName.IsNone())
                {
                    LoadedMappings.Add(LoadedMapping);
                }
            }

            if (LoadedMappings.Num() > 0)
            {
                InputMappings = MoveTemp(LoadedMappings);
                OnControlsRemapped.Broadcast(InputMappings);
                UE_LOG(LogTemp, Log, TEXT("Loaded %d control mappings from: %s"), InputMappings.Num(), *SavePath);
                return;
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("Control mapping file was invalid or empty, restoring defaults: %s"), *SavePath);
    }

    InputMappings.Empty();
    InitializeDefaultMappings();
    OnControlsRemapped.Broadcast(InputMappings);
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
    InputMappings.Empty();

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
