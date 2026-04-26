# Network Multiplayer - Quick Start Guide

Get multiplayer working in 5 minutes.

## Step 1: Enable Online Subsystem (1 min)

Edit `DefaultEngine.ini`:

```ini
[OnlineSubsystem]
DefaultPlatformService=NULL

[OnlineSubsystemNull]
bEnabled=true
```

## Step 2: Create Session (2 min)

```cpp
// Get subsystem
UMultiplayerSessionSubsystem* SessionSubsystem = 
    GetGameInstance()->GetSubsystem<UMultiplayerSessionSubsystem>();

// Listen for completion
SessionSubsystem->OnCreateSessionComplete.AddDynamic(this, &UMyClass::OnSessionCreated);

// Create session
SessionSubsystem->CreateSession(4, false, TEXT("MyGameSession"));
```

## Step 3: Find and Join Sessions (2 min)

```cpp
// Find sessions
SessionSubsystem->OnFindSessionsComplete.AddDynamic(this, &UMyClass::OnSessionsFound);
SessionSubsystem->FindSessions(10, false);

// Join session
void UMyClass::OnSessionsFound(bool bSuccess)
{
    if (bSuccess && SessionSubsystem->GetFoundSessionsCount() > 0)
    {
        SessionSubsystem->OnJoinSessionComplete.AddDynamic(this, &UMyClass::OnSessionJoined);
        SessionSubsystem->JoinSession(0); // Join first session
    }
}
```

## Blueprint Quick Start

1. Get Multiplayer Session Subsystem
2. Create Session (NumPlayers: 4, IsLAN: false, Name: "MyGame")
3. Bind to OnCreateSessionComplete
4. On success, call Start Session

## Testing Locally

Launch two instances:
```bash
# Host
HorrorProject.exe -log

# Client
HorrorProject.exe 127.0.0.1 -log
```

## Common Issues

**Session not found**: Check firewall settings
**Can't join**: Ensure session is started after creation
**Replication not working**: Add `GetLifetimeReplicatedProps()` to your actors

## Next Steps

- Read full documentation: `/Source/HorrorProject/Network/`
- Implement player state replication
- Add game state synchronization
- Test with multiple clients
