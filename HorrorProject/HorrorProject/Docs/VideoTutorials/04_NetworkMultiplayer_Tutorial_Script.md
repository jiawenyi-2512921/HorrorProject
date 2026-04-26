# Video Tutorial Script: Network Multiplayer System

**Duration:** 10-12 minutes  
**Target Audience:** Developers implementing multiplayer

---

## Introduction (0:00 - 0:30)

"Welcome to the HorrorProject Network Multiplayer tutorial. Learn how to create, find, and join multiplayer sessions, and implement network replication for your horror game."

**Show:** Multiple players in game together

---

## Part 1: Setup (0:30 - 2:00)

"First, enable the Online Subsystem in your DefaultEngine.ini:"

**Show:** Config file

```ini
[OnlineSubsystem]
DefaultPlatformService=NULL

[OnlineSubsystemNull]
bEnabled=true
```

"For testing, we use NULL subsystem. For production, use Steam or Epic Online Services."

**Show:** Different subsystem options

---

## Part 2: Creating a Session (2:00 - 4:00)

"Let's create a multiplayer session."

**Show:** Host menu UI

```cpp
UMultiplayerSessionSubsystem* SessionSubsystem = 
    GetGameInstance()->GetSubsystem<UMultiplayerSessionSubsystem>();

// Listen for completion
SessionSubsystem->OnCreateSessionComplete.AddDynamic(
    this, &UMyClass::OnSessionCreated);

// Create session
SessionSubsystem->CreateSession(
    4,      // Max players
    false,  // Not LAN
    TEXT("MyGameSession")
);
```

**Show:** Session being created

"Handle the callback:"

```cpp
void UMyClass::OnSessionCreated(bool bSuccess)
{
    if (bSuccess)
    {
        SessionSubsystem->StartSession();
    }
}
```

**Show:** Session started, waiting for players

---

## Part 3: Finding Sessions (4:00 - 5:30)

"Now let's find available sessions."

**Show:** Server browser UI

```cpp
SessionSubsystem->OnFindSessionsComplete.AddDynamic(
    this, &UMyClass::OnSessionsFound);

SessionSubsystem->FindSessions(10, false);
```

**Show:** Searching for sessions

"Display found sessions:"

```cpp
void UMyClass::OnSessionsFound(bool bSuccess)
{
    if (bSuccess)
    {
        int32 Count = SessionSubsystem->GetFoundSessionsCount();
        for (int32 i = 0; i < Count; i++)
        {
            FString Name = SessionSubsystem->GetSessionName(i);
            int32 Ping = SessionSubsystem->GetSessionPing(i);
            int32 Players = SessionSubsystem->GetSessionCurrentPlayers(i);
            int32 MaxPlayers = SessionSubsystem->GetSessionMaxPlayers(i);
        }
    }
}
```

**Show:** Session list populating

---

## Part 4: Joining a Session (5:30 - 6:30)

"Join a session from the list."

**Show:** Clicking join button

```cpp
SessionSubsystem->OnJoinSessionComplete.AddDynamic(
    this, &UMyClass::OnSessionJoined);

SessionSubsystem->JoinSession(0); // Join first session
```

**Show:** Connecting to session

"On successful join, travel to the game map:"

```cpp
void UMyClass::OnSessionJoined(bool bSuccess)
{
    if (bSuccess)
    {
        // Client automatically travels to host's map
    }
}
```

**Show:** Client joining host's game

---

## Part 5: Network Replication (6:30 - 8:30)

"Replicate game state across clients."

**Show:** Code editor

```cpp
// In ReplicatedGameState.h
UPROPERTY(Replicated)
int32 RemainingTime;

UPROPERTY(Replicated)
TArray<FString> CollectedEvidence;

// In ReplicatedGameState.cpp
void AReplicatedGameState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AReplicatedGameState, RemainingTime);
    DOREPLIFETIME(AReplicatedGameState, CollectedEvidence);
}
```

**Show:** Game state syncing between clients

"Replicate player state:"

```cpp
// In ReplicatedPlayerState.h
UPROPERTY(Replicated)
int32 Score;

UPROPERTY(Replicated)
bool bIsAlive;
```

**Show:** Player stats syncing

---

## Part 6: RPCs (8:30 - 9:30)

"Use Remote Procedure Calls for client-server communication."

**Show:** Code examples

```cpp
// Server RPC
UFUNCTION(Server, Reliable)
void ServerCollectItem(const FString& ItemID);

void AMyCharacter::ServerCollectItem_Implementation(const FString& ItemID)
{
    // Server validates and processes
    CollectedItems.Add(ItemID);
}

// Multicast RPC
UFUNCTION(NetMulticast, Reliable)
void MulticastPlaySound(USoundBase* Sound);

void AMyCharacter::MulticastPlaySound_Implementation(USoundBase* Sound)
{
    // All clients play sound
    UGameplayStatics::PlaySound2D(this, Sound);
}
```

**Show:** RPCs in action during gameplay

---

## Part 7: Testing Locally (9:30 - 10:30)

"Test multiplayer on one machine."

**Show:** Command line

```bash
# Launch host
HorrorProject.exe -log

# Launch client
HorrorProject.exe 127.0.0.1 -log
```

**Show:** Two game windows side by side

"Or use Unreal's multiplayer testing:"

**Show:** Editor settings

"Play → Net Mode → Client/Server
Number of Players: 2"

**Show:** Multiple editor windows running

---

## Part 8: Troubleshooting (10:30 - 11:30)

"Common issues and solutions:"

**Show:** Troubleshooting checklist

"Session not found:
- Check firewall settings
- Verify online subsystem is enabled
- Ensure session is started"

**Show:** Firewall settings

"Replication not working:
- Add GetLifetimeReplicatedProps
- Mark properties as Replicated
- Check network relevancy"

**Show:** Debugging replication

"High latency:
- Reduce replication frequency
- Use relevancy optimization
- Compress replicated data"

**Show:** Network profiler

---

## Conclusion (11:30 - 12:00)

"You now know how to:
- Create multiplayer sessions
- Find and join sessions
- Replicate game state
- Use RPCs
- Test locally
- Troubleshoot issues"

**Show:** Full multiplayer game session

"Check the documentation for advanced topics like lag compensation and prediction. Thanks for watching!"

---

## Visual Notes

- Show multiple game instances simultaneously
- Use network visualization tools
- Highlight replication in real-time
- Show latency indicators
- Use split screen for host/client comparison
- Include network traffic graphs

## B-Roll Suggestions

- Multiple players cooperating
- Session browser UI
- Network replication visualization
- RPC calls in action
- Local testing setup
- Network profiler data
