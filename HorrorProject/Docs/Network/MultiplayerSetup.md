# Multiplayer Setup Guide

## Prerequisites

### Engine Configuration
1. Enable Online Subsystem in `DefaultEngine.ini`:
```ini
[OnlineSubsystem]
DefaultPlatformService=NULL

[OnlineSubsystemNull]
bEnabled=true

[/Script/Engine.GameEngine]
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="/Script/OnlineSubsystemUtils.IpNetDriver",DriverClassNameFallback="/Script/OnlineSubsystemUtils.IpNetDriver")
```

2. Configure network settings:
```ini
[/Script/OnlineSubsystemUtils.IpNetDriver]
NetServerMaxTickRate=30
MaxNetTickRate=30
MaxInternetClientRate=10000
MaxClientRate=15000
```

### Project Setup
1. Add network modules to `HorrorProject.Build.cs`:
```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "OnlineSubsystem",
    "OnlineSubsystemUtils",
    "Sockets",
    "Networking"
});
```

2. Compile project

## Creating a Multiplayer Game

### Blueprint Setup

#### 1. Create Session Widget
```cpp
// In Blueprint or C++
UMultiplayerSessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSessionSubsystem>();

// Bind to delegates
SessionSubsystem->OnCreateSessionComplete.AddDynamic(this, &UMyWidget::OnSessionCreated);

// Create session
SessionSubsystem->CreateSession(4, true, "MyHorrorGame");
```

#### 2. Find Sessions Widget
```cpp
// Bind to delegates
SessionSubsystem->OnFindSessionsComplete.AddDynamic(this, &UMyWidget::OnSessionsFound);

// Find sessions
SessionSubsystem->FindSessions(10, true);

// Display results
int32 SessionCount = SessionSubsystem->GetFoundSessionsCount();
for (int32 i = 0; i < SessionCount; ++i)
{
    FString Name = SessionSubsystem->GetSessionName(i);
    int32 Ping = SessionSubsystem->GetSessionPing(i);
    int32 Players = SessionSubsystem->GetSessionCurrentPlayers(i);
    int32 MaxPlayers = SessionSubsystem->GetSessionMaxPlayers(i);
}
```

#### 3. Join Session
```cpp
// Bind to delegates
SessionSubsystem->OnJoinSessionComplete.AddDynamic(this, &UMyWidget::OnSessionJoined);

// Join selected session
SessionSubsystem->JoinSession(SelectedIndex);
```

### Game Mode Configuration

#### 1. Set Multiplayer Game Mode
In Project Settings → Maps & Modes:
- Default GameMode: `HorrorGameModeMultiplayer`
- GameState Class: `ReplicatedGameState`
- PlayerController Class: `HorrorPlayerControllerMultiplayer`
- PlayerState Class: `ReplicatedPlayerState`

#### 2. Configure Game Mode Settings
```cpp
// In Blueprint or C++
AHorrorGameModeMultiplayer* GameMode = GetWorld()->GetAuthGameMode<AHorrorGameModeMultiplayer>();
GameMode->MinPlayers = 2;
GameMode->MaxPlayers = 4;
GameMode->LobbyWaitTime = 30.0f;
GameMode->GameDuration = 1800.0f; // 30 minutes
GameMode->bAutoStartGame = true;
```

## Character Setup

### 1. Add Network Components
```cpp
// In Character constructor
NetworkReplication = CreateDefaultSubobject<UNetworkReplicationComponent>(TEXT("NetworkReplication"));
NetworkSync = CreateDefaultSubobject<UNetworkSyncComponent>(TEXT("NetworkSync"));
```

### 2. Configure Replication
```cpp
// Set replication settings
NetworkReplication->ReplicationRate = 30.0f;
NetworkReplication->InterpolationSpeed = 10.0f;
NetworkReplication->bEnableClientPrediction = true;

// Enable actor replication
bReplicates = true;
SetReplicateMovement(true);
```

### 3. Implement Input Replication
```cpp
void AMyCharacter::MoveForward(float Value)
{
    if (Controller && Value != 0.0f)
    {
        // Local prediction
        AddMovementInput(GetActorForwardVector(), Value);
        
        // Send to server
        if (AHorrorPlayerControllerMultiplayer* PC = Cast<AHorrorPlayerControllerMultiplayer>(Controller))
        {
            PC->ServerSendInput(GetActorForwardVector() * Value, GetControlRotation(), GetWorld()->GetDeltaSeconds());
        }
    }
}
```

## Testing Multiplayer

### Local Testing (PIE)

#### 1. Editor Settings
- Edit → Editor Preferences → Play
- Multiplayer Options:
  - Number of Players: 2-4
  - Net Mode: Play As Listen Server
  - Run Dedicated Server: Optional

#### 2. Launch Test
- Click Play dropdown → Number of Players → Select 2-4
- First window is server/host
- Additional windows are clients

### Standalone Testing

#### 1. Package Project
- File → Package Project → Windows (64-bit)
- Wait for packaging to complete

#### 2. Launch Server
```bash
HorrorProject.exe -server -log
```

#### 3. Launch Clients
```bash
HorrorProject.exe -game
```

#### 4. Connect to Server
```cpp
// In client
GetWorld()->GetFirstPlayerController()->ConsoleCommand("open 127.0.0.1");
```

### Command Line Arguments

#### Server
```bash
# Listen server
HorrorProject.exe /Game/Maps/MainMap?listen -log

# Dedicated server
HorrorProject.exe /Game/Maps/MainMap -server -log -port=7777
```

#### Client
```bash
# Connect to server
HorrorProject.exe 192.168.1.100:7777 -game -log

# LAN mode
HorrorProject.exe -game -log
```

## Common Issues

### Players Can't Connect
1. Check firewall settings (port 7777)
2. Verify IP address is correct
3. Ensure OnlineSubsystem is enabled
4. Check network mode in PIE settings

### Replication Not Working
1. Verify `bReplicates = true` on actors
2. Check `DOREPLIFETIME` in `GetLifetimeReplicatedProps`
3. Ensure `SetIsReplicatedByDefault(true)` on components
4. Verify server authority with `HasAuthority()`

### High Latency
1. Reduce replication rate
2. Enable delta compression
3. Increase position/rotation tolerance
4. Optimize replicated variables

### Desync Issues
1. Enable client prediction
2. Increase interpolation speed
3. Verify server reconciliation
4. Check timestamp synchronization

## Best Practices

1. **Always validate on server**: Use `_Validate` functions
2. **Minimize replicated data**: Only replicate what's necessary
3. **Use relevancy**: Implement distance-based culling
4. **Test with latency**: Use network emulation tools
5. **Handle disconnections**: Implement reconnection logic
6. **Log network events**: Debug with comprehensive logging
7. **Profile bandwidth**: Monitor network usage regularly

## Next Steps

- Read [Network Optimization](NetworkOptimization.md)
- Review [Network Testing Guide](NetworkTesting.md)
- Implement custom replication logic
- Add voice chat support
- Create matchmaking system
