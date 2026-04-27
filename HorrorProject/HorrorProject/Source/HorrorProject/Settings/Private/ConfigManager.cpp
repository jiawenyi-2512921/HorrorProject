// Copyright Epic Games, Inc. All Rights Reserved.

#include "ConfigManager.h"
#include "GraphicsSettings.h"
#include "HorrorAudioSettings.h"
#include "ControlSettings.h"
#include "GameplaySettings.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "HAL/PlatformFileManager.h"

UConfigManager::UConfigManager()
{
	ConfigVersion = CurrentConfigVersion;
}

bool UConfigManager::LoadConfig(const FString& Category, UObject* SettingsObject)
{
	if (!SettingsObject)
	{
		return false;
	}

	FString FilePath = GetConfigFilePath(Category);
	FString JsonString;

	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return false;
	}

	return DeserializeFromJson(JsonObject, SettingsObject);
}

bool UConfigManager::SaveConfig(const FString& Category, UObject* SettingsObject)
{
	if (!SettingsObject)
	{
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());

	if (!SerializeToJson(SettingsObject, JsonObject))
	{
		return false;
	}

	JsonObject->SetNumberField(TEXT("ConfigVersion"), ConfigVersion);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);

	if (!FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer))
	{
		return false;
	}

	FString FilePath = GetConfigFilePath(Category);
	return FFileHelper::SaveStringToFile(JsonString, *FilePath);
}

bool UConfigManager::DeleteConfig(const FString& Category)
{
	FString FilePath = GetConfigFilePath(Category);
	return IFileManager::Get().Delete(*FilePath);
}

bool UConfigManager::ConfigExists(const FString& Category) const
{
	FString FilePath = GetConfigFilePath(Category);
	return FPaths::FileExists(FilePath);
}

bool UConfigManager::ExportSettings(const FString& FilePath, UGraphicsSettings* Graphics,
	UHorrorAudioSettings* Audio, UControlSettings* Controls, UGameplaySettings* Gameplay)
{
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject());

	if (Graphics)
	{
		TSharedPtr<FJsonObject> GraphicsJson = MakeShareable(new FJsonObject());
		if (SerializeToJson(Graphics, GraphicsJson))
		{
			RootObject->SetObjectField(TEXT("Graphics"), GraphicsJson);
		}
	}

	if (Audio)
	{
		TSharedPtr<FJsonObject> AudioJson = MakeShareable(new FJsonObject());
		if (SerializeToJson(Audio, AudioJson))
		{
			RootObject->SetObjectField(TEXT("Audio"), AudioJson);
		}
	}

	if (Controls)
	{
		TSharedPtr<FJsonObject> ControlsJson = MakeShareable(new FJsonObject());
		if (SerializeToJson(Controls, ControlsJson))
		{
			RootObject->SetObjectField(TEXT("Controls"), ControlsJson);
		}
	}

	if (Gameplay)
	{
		TSharedPtr<FJsonObject> GameplayJson = MakeShareable(new FJsonObject());
		if (SerializeToJson(Gameplay, GameplayJson))
		{
			RootObject->SetObjectField(TEXT("Gameplay"), GameplayJson);
		}
	}

	RootObject->SetNumberField(TEXT("ConfigVersion"), ConfigVersion);

	FString JsonString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);

	if (!FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer))
	{
		return false;
	}

	return FFileHelper::SaveStringToFile(JsonString, *FilePath);
}

bool UConfigManager::ImportSettings(const FString& FilePath, UGraphicsSettings* Graphics,
	UHorrorAudioSettings* Audio, UControlSettings* Controls, UGameplaySettings* Gameplay)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		return false;
	}

	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		return false;
	}

	bool bSuccess = true;

	if (Graphics && RootObject->HasField(TEXT("Graphics")))
	{
		TSharedPtr<FJsonObject> GraphicsJson = RootObject->GetObjectField(TEXT("Graphics"));
		bSuccess &= DeserializeFromJson(GraphicsJson, Graphics);
	}

	if (Audio && RootObject->HasField(TEXT("Audio")))
	{
		TSharedPtr<FJsonObject> AudioJson = RootObject->GetObjectField(TEXT("Audio"));
		bSuccess &= DeserializeFromJson(AudioJson, Audio);
	}

	if (Controls && RootObject->HasField(TEXT("Controls")))
	{
		TSharedPtr<FJsonObject> ControlsJson = RootObject->GetObjectField(TEXT("Controls"));
		bSuccess &= DeserializeFromJson(ControlsJson, Controls);
	}

	if (Gameplay && RootObject->HasField(TEXT("Gameplay")))
	{
		TSharedPtr<FJsonObject> GameplayJson = RootObject->GetObjectField(TEXT("Gameplay"));
		bSuccess &= DeserializeFromJson(GameplayJson, Gameplay);
	}

	return bSuccess;
}

bool UConfigManager::CreateBackup(const FString& BackupName)
{
	FString BackupPath = GetBackupFilePath(BackupName);
	FString ConfigDir = GetConfigDirectory();

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*GetBackupDirectory()))
	{
		PlatformFile.CreateDirectory(*GetBackupDirectory());
	}

	return PlatformFile.CopyDirectoryTree(*BackupPath, *ConfigDir, true);
}

bool UConfigManager::RestoreBackup(const FString& BackupName)
{
	FString BackupPath = GetBackupFilePath(BackupName);
	FString ConfigDir = GetConfigDirectory();

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	if (!PlatformFile.DirectoryExists(*BackupPath))
	{
		return false;
	}

	return PlatformFile.CopyDirectoryTree(*ConfigDir, *BackupPath, true);
}

TArray<FString> UConfigManager::GetAvailableBackups() const
{
	TArray<FString> Backups;
	FString BackupDir = GetBackupDirectory();

	IFileManager::Get().FindFiles(Backups, *BackupDir, nullptr);

	return Backups;
}

FString UConfigManager::GetConfigDirectory() const
{
	return FPaths::ProjectSavedDir() / TEXT("Config");
}

FString UConfigManager::GetBackupDirectory() const
{
	return FPaths::ProjectSavedDir() / TEXT("Config") / TEXT("Backups");
}

FString UConfigManager::GetConfigFilePath(const FString& Category) const
{
	return GetConfigDirectory() / (Category + TEXT(".json"));
}

FString UConfigManager::GetBackupFilePath(const FString& BackupName) const
{
	return GetBackupDirectory() / BackupName;
}

bool UConfigManager::SerializeToJson(UObject* Object, TSharedPtr<FJsonObject>& OutJson)
{
	if (!Object || !OutJson.IsValid())
	{
		return false;
	}

	FString PropertyName;
	for (TFieldIterator<FProperty> PropIt(Object->GetClass()); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		if (!Property || !Property->HasAnyPropertyFlags(CPF_Config))
		{
			continue;
		}

		PropertyName = Property->GetName();
		void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Object);

		if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
		{
			OutJson->SetBoolField(PropertyName, BoolProp->GetPropertyValue(ValuePtr));
		}
		else if (FIntProperty* IntProp = CastField<FIntProperty>(Property))
		{
			OutJson->SetNumberField(PropertyName, IntProp->GetPropertyValue(ValuePtr));
		}
		else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
		{
			OutJson->SetNumberField(PropertyName, FloatProp->GetPropertyValue(ValuePtr));
		}
		else if (FStrProperty* StrProp = CastField<FStrProperty>(Property))
		{
			OutJson->SetStringField(PropertyName, StrProp->GetPropertyValue(ValuePtr));
		}
		else if (FEnumProperty* EnumProp = CastField<FEnumProperty>(Property))
		{
			OutJson->SetNumberField(PropertyName, EnumProp->GetUnderlyingProperty()->GetSignedIntPropertyValue(ValuePtr));
		}
	}

	return true;
}

bool UConfigManager::DeserializeFromJson(const TSharedPtr<FJsonObject>& Json, UObject* Object)
{
	if (!Json.IsValid() || !Object)
	{
		return false;
	}

	FString PropertyName;
	for (TFieldIterator<FProperty> PropIt(Object->GetClass()); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		if (!Property || !Property->HasAnyPropertyFlags(CPF_Config))
		{
			continue;
		}

		PropertyName = Property->GetName();
		if (!Json->HasField(PropertyName))
		{
			continue;
		}

		void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Object);

		if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
		{
			BoolProp->SetPropertyValue(ValuePtr, Json->GetBoolField(PropertyName));
		}
		else if (FIntProperty* IntProp = CastField<FIntProperty>(Property))
		{
			IntProp->SetPropertyValue(ValuePtr, Json->GetIntegerField(PropertyName));
		}
		else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
		{
			FloatProp->SetPropertyValue(ValuePtr, Json->GetNumberField(PropertyName));
		}
		else if (FStrProperty* StrProp = CastField<FStrProperty>(Property))
		{
			StrProp->SetPropertyValue(ValuePtr, Json->GetStringField(PropertyName));
		}
		else if (FEnumProperty* EnumProp = CastField<FEnumProperty>(Property))
		{
			EnumProp->GetUnderlyingProperty()->SetIntPropertyValue(ValuePtr, static_cast<int64>(Json->GetIntegerField(PropertyName)));
		}
	}

	return true;
}
