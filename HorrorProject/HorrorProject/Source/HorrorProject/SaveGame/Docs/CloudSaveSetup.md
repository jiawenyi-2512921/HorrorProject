# Cloud Save Setup Guide

## Overview

The cloud save system supports both Steam Cloud and Epic Online Services (EOS) for automatic save game synchronization across devices.

## Platform Detection

The system automatically detects the active platform at runtime:

```cpp
void UCloudSaveSubsystem::DetectCloudProvider()
{
    #if WITH_STEAMWORKS
    if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get(STEAM_SUBSYSTEM))
    {
        CloudProvider = ECloudSaveProvider::Steam;
        return;
    }
    #endif

    #if WITH_EOS_SDK
    if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get(EOS_SUBSYSTEM))
    {
        CloudProvider = ECloudSaveProvider::Epic;
        return;
    }
    #endif
}
```

## Steam Cloud Setup

### 1. Enable Steam Cloud in Steamworks

1. Log into Steamworks Partner site
2. Navigate to your app's configuration
3. Go to "Cloud" section
4. Enable "Steam Cloud"
5. Set quota (recommended: 100MB minimum)
6. Publish changes

### 2. Configure Steam Cloud Files

Add to `steam_appid.txt` in your project root:
```
YOUR_APP_ID
```

### 3. Project Configuration

Add to `DefaultEngine.ini`:
```ini
[OnlineSubsystem]
DefaultPlatformService=Steam

[OnlineSubsystemSteam]
bEnabled=true
SteamDevAppId=YOUR_APP_ID
bInitServerOnClient=true

[/Script/Engine.GameEngine]
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="OnlineSubsystemSteam.SteamNetDriver",DriverClassNameFallback="OnlineSubsystemUtils.IpNetDriver")
```

### 4. Build Configuration

Add to `HorrorProject.Build.cs`:
```csharp
if (Target.Platform == UnrealTargetPlatform.Win64)
{
    PublicDependencyModuleNames.AddRange(new string[] {
        "OnlineSubsystem",
        "OnlineSubsystemSteam"
    });
    
    PublicDefinitions.Add("WITH_STEAMWORKS=1");
}
```

### 5. Steam Cloud Implementation

The system uses Steam's ISteamRemoteStorage interface:

```cpp
bool UCloudSaveSubsystem::UploadToSteamCloud(int32 SlotIndex)
{
    #if WITH_STEAMWORKS
    ISteamRemoteStorage* RemoteStorage = SteamRemoteStorage();
    if (!RemoteStorage)
        return false;
    
    FString SlotName = GetSlotName(SlotIndex);
    TArray<uint8> SaveData;
    
    // Load save file data
    if (!FFileHelper::LoadFileToArray(SaveData, *GetSaveFilePath(SlotIndex)))
        return false;
    
    // Upload to Steam Cloud
    return RemoteStorage->FileWrite(
        TCHAR_TO_UTF8(*SlotName),
        SaveData.GetData(),
        SaveData.Num()
    );
    #else
    return false;
    #endif
}
```

## Epic Online Services Setup

### 1. Create EOS Application

1. Go to Epic Games Developer Portal
2. Create new application
3. Note your Product ID, Sandbox ID, and Deployment ID
4. Enable Player Data Storage service

### 2. Configure EOS Credentials

Add to `DefaultEngine.ini`:
```ini
[OnlineSubsystem]
DefaultPlatformService=EOS

[OnlineSubsystemEOS]
bEnabled=true
ProductId=YOUR_PRODUCT_ID
SandboxId=YOUR_SANDBOX_ID
DeploymentId=YOUR_DEPLOYMENT_ID
ClientId=YOUR_CLIENT_ID
ClientSecret=YOUR_CLIENT_SECRET
```

### 3. Build Configuration

Add to `HorrorProject.Build.cs`:
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "OnlineSubsystem",
    "OnlineSubsystemEOS"
});

PublicDefinitions.Add("WITH_EOS_SDK=1");
```

### 4. EOS Cloud Implementation

The system uses EOS Player Data Storage:

```cpp
bool UCloudSaveSubsystem::UploadToEpicCloud(int32 SlotIndex)
{
    #if WITH_EOS_SDK
    IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get(EOS_SUBSYSTEM);
    if (!OnlineSubsystem)
        return false;
    
    // Get player data storage interface
    IOnlineUserCloudPtr UserCloud = OnlineSubsystem->GetUserCloudInterface();
    if (!UserCloud.IsValid())
        return false;
    
    // Upload file
    FString SlotName = GetSlotName(SlotIndex);
    TArray<uint8> SaveData;
    
    if (!FFileHelper::LoadFileToArray(SaveData, *GetSaveFilePath(SlotIndex)))
        return false;
    
    return UserCloud->WriteUserFile(
        *GetLocalUserId(),
        SlotName,
        SaveData
    );
    #else
    return false;
    #endif
}
```

## Usage in Game

### Check Cloud Availability

```cpp
UCloudSaveSubsystem* CloudSave = GameInstance->GetSubsystem<UCloudSaveSubsystem>();

if (CloudSave->IsCloudSaveEnabled())
{
    ECloudSaveProvider Provider = CloudSave->GetCloudProvider();
    // Cloud saves available
}
```

### Upload Save to Cloud

```cpp
// Bind to completion callback
CloudSave->OnCloudSaveComplete.AddDynamic(this, &UMyClass::OnCloudSaveComplete);

// Upload slot 0
CloudSave->UploadSaveToCloud(0);

void UMyClass::OnCloudSaveComplete(int32 SlotIndex, ECloudSaveStatus Status, const FString& ErrorMessage)
{
    if (Status == ECloudSaveStatus::Success)
    {
        // Upload successful
    }
    else
    {
        // Handle error
        UE_LOG(LogTemp, Error, TEXT("Cloud save failed: %s"), *ErrorMessage);
    }
}
```

### Download Save from Cloud

```cpp
CloudSave->OnCloudSaveComplete.AddDynamic(this, &UMyClass::OnCloudDownloadComplete);
CloudSave->DownloadSaveFromCloud(0);

void UMyClass::OnCloudDownloadComplete(int32 SlotIndex, ECloudSaveStatus Status, const FString& ErrorMessage)
{
    if (Status == ECloudSaveStatus::Success)
    {
        // Download successful, load the save
        USaveSlotManager* SlotManager = GetSlotManager();
        UHorrorSaveGame* SaveGame = SlotManager->LoadFromSlot(SlotIndex);
    }
}
```

### Sync All Saves

```cpp
// Sync all save slots to cloud
CloudSave->SyncAllSaves();
```

## Best Practices

### When to Upload
- After successful save operation
- On game exit
- After major progress milestones
- Periodically (every 5-10 minutes of gameplay)

### When to Download
- On game start (before showing load menu)
- When user explicitly requests sync
- After detecting newer cloud save

### Conflict Resolution
```cpp
void ResolveCloudConflict(int32 SlotIndex)
{
    FSaveSlotMetadata LocalMeta = SlotManager->GetSlotMetadata(SlotIndex);
    FSaveSlotMetadata CloudMeta = GetCloudMetadata(SlotIndex);
    
    if (CloudMeta.SaveTime > LocalMeta.SaveTime)
    {
        // Cloud is newer, download
        CloudSave->DownloadSaveFromCloud(SlotIndex);
    }
    else
    {
        // Local is newer, upload
        CloudSave->UploadSaveToCloud(SlotIndex);
    }
}
```

### Error Handling
```cpp
void HandleCloudError(ECloudSaveStatus Status, const FString& ErrorMessage)
{
    switch (Status)
    {
    case ECloudSaveStatus::Failed:
        // Show error to user
        ShowNotification(TEXT("Cloud save failed. Playing offline."));
        break;
        
    case ECloudSaveStatus::Success:
        // Update UI
        UpdateCloudSyncIcon(true);
        break;
    }
}
```

## Testing Cloud Saves

### Steam Testing
1. Enable Steam Cloud in Steamworks
2. Run game with valid steam_appid.txt
3. Save game
4. Check Steam Cloud files in: `Steam/userdata/[USER_ID]/[APP_ID]/remote/`
5. Delete local saves
6. Restart game and verify download

### EOS Testing
1. Configure EOS credentials
2. Use EOS DevAuthTool for local testing
3. Monitor uploads in EOS Developer Portal
4. Test with multiple accounts

## Troubleshooting

### Steam Cloud Not Working
- Verify steam_appid.txt exists and is correct
- Check Steam Cloud is enabled in Steamworks
- Ensure user is logged into Steam
- Check Steam Cloud quota not exceeded

### EOS Not Working
- Verify all credentials are correct
- Check EOS services are enabled
- Ensure user is logged in
- Check network connectivity

### Common Issues
- **"Cloud save disabled"** - Platform not detected, check build configuration
- **"Upload failed"** - Check network, quotas, and permissions
- **"Download failed"** - Verify save exists in cloud
- **"Sync conflict"** - Implement conflict resolution logic

## Platform-Specific Notes

### Steam
- Max file size: 100MB per file (configurable)
- Total quota: Set in Steamworks (recommend 100MB+)
- Automatic sync on game exit
- Works offline (syncs when online)

### Epic
- Max file size: 64MB per file
- Total quota: 100MB per user
- Requires online connection
- Supports metadata queries

## Security Considerations

- Save files are not encrypted by default
- Consider encrypting sensitive data
- Validate downloaded saves before loading
- Implement checksum verification
- Handle malicious/corrupted cloud data gracefully
