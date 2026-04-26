# Code Examples: Network Multiplayer

Complete code examples for implementing multiplayer functionality.

---

## Example 1: Session Creation

```cpp
// MultiplayerGameMode.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Network/MultiplayerSessionSubsystem.h"
#include "MultiplayerGameMode.generated.h"

UCLASS()
class HORRORPROJECT_API AMultiplayerGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    void CreateMultiplayerSession();
    
    UFUNCTION()
    void OnSessionCreated(bool bSuccess);
};

// MultiplayerGameMode.cpp
#include "MultiplayerGameMode.h"

void AMultiplayerGameMode::CreateMultiplayerSession()
{
    UMultiplayerSessionSubsystem* SessionSubsystem = 
        GetGameInstance()->GetSubsystem<UMultiplayerSessionSubsystem>();
    
    if (SessionSubsystem)
    {
        SessionSubsystem->OnCreateSessionComplete.AddDynamic(
            this, &AMultiplayerGameMode::OnSessionCreated);
        
        SessionSubsystem->CreateSession(
            4,      // Max 4 players
            false,  // Not LAN
            TEXT("HorrorGameSession")
        );
    }
}

void AMultiplayerGameMode::OnSessionCreated(bool bSuccess)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Session created successfully"));
        
        UMultiplayerSessionSubsystem* SessionSubsystem = 
            GetGameInstance()->GetSubsystem<UMultiplayerSessionSubsystem>();
        
        if (SessionSubsystem)
        {
            SessionSubsystem->StartSession();
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create session"));
    }
}
```

---

## Example 2: Session Browser

```cpp
// SessionBrowser.h
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Network/MultiplayerSessionSubsystem.h"
#include "SessionBrowser.generated.h"

UCLASS()
class HORRORPROJECT_API USessionBrowser : public UUserWidget
{
    GENERATED_BODY()

public:
    void RefreshSessionList();
    void JoinSessionByIndex(int32 Index);
    
    UFUNCTION()
    void OnSessionsFound(bool bSuccess);
    
    UFUNCTION()
    void OnSessionJoined(bool bSuccess);

private:
    UMultiplayerSessionSubsystem* SessionSubsystem;
};

// SessionBrowser.cpp
#include "SessionBrowser.h"

void USessionBrowser::RefreshSessionList()
{
    SessionSubsystem = GetGameInstance()->GetSubsystem<UMultiplayerSessionSubsystem>();
    
    if (SessionSubsystem)
    {
        SessionSubsystem->OnFindSessionsComplete.AddDynamic(
            this, &USessionBrowser::OnSessionsFound);
        
        SessionSubsystem->FindSessions(10, false);
    }
}

void USessionBrowser::OnSessionsFound(bool bSuccess)
{
    if (bSuccess && SessionSubsystem)
    {
        int32 Count = SessionSubsystem->GetFoundSessionsCount();
        
        UE_LOG(LogTemp, Log, TEXT("Found %d sessions"), Count);
        
        for (int32 i = 0; i < Count; i++)
        {
            FString Name = SessionSubsystem->GetSessionName(i);
            int32 Ping = SessionSubsystem->GetSessionPing(i);
            int32 CurrentPlayers = SessionSubsystem->GetSessionCurrentPlayers(i);
            int32 MaxPlayers = SessionSubsystem->GetSessionMaxPlayers(i);
            
            UE_LOG(LogTemp, Log, TEXT("Session %d: %s (%d/%d) Ping: %dms"), 
                i, *Name, CurrentPlayers, MaxPlayers, Ping);
            
            // Update UI with session info
        }
    }
}

void USessionBrowser::JoinSessionByIndex(int32 Index)
{
    if (SessionSubsystem)
    {
        SessionSubsystem->OnJoinSessionComplete.AddDynamic(
            this, &USessionBrowser::OnSessionJoined);
        
        SessionSubsystem->JoinSession(Index);
    }
}

void USessionBrowser::OnSessionJoined(bool bSuccess)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully joined session"));
        // Client will automatically travel to host's map
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to join session"));
    }
}
```

---

## Example 3: Replicated Game State

```cpp
// HorrorGameState.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "HorrorGameState.generated.h"

UCLASS()
class HORRORPROJECT_API AHorrorGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AHorrorGameState();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    int32 RemainingTime;
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    TArray<FString> CollectedEvidence;
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game")
    bool bGameInProgress;
    
    UFUNCTION(BlueprintCallable, Category = "Game")
    void AddEvidence(const FString& EvidenceID);
};

// HorrorGameState.cpp
#include "HorrorGameState.h"

AHorrorGameState::AHorrorGameState()
{
    RemainingTime = 600; // 10 minutes
    bGameInProgress = false;
}

void AHorrorGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AHorrorGameState, RemainingTime);
    DOREPLIFETIME(AHorrorGameState, CollectedEvidence);
    DOREPLIFETIME(AHorrorGameState, bGameInProgress);
}

void AHorrorGameState::AddEvidence(const FString& EvidenceID)
{
    if (HasAuthority())
    {
        CollectedEvidence.Add(EvidenceID);
        UE_LOG(LogTemp, Log, TEXT("Evidence collected: %s (Total: %d)"), 
            *EvidenceID, CollectedEvidence.Num());
    }
}
```

---

## Example 4: Replicated Player State

```cpp
// HorrorPlayerState.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "HorrorPlayerState.generated.h"

UCLASS()
class HORRORPROJECT_API AHorrorPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    AHorrorPlayerState();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
    int32 EvidenceCollected;
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
    bool bIsAlive;
    
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
    float Health;
    
    UFUNCTION(BlueprintCallable, Category = "Player")
    void AddScore(int32 Points);
    
    UFUNCTION(BlueprintCallable, Category = "Player")
    void TakeDamage(float Damage);
};

// HorrorPlayerState.cpp
#include "HorrorPlayerState.h"

AHorrorPlayerState::AHorrorPlayerState()
{
    EvidenceCollected = 0;
    bIsAlive = true;
    Health = 100.0f;
}

void AHorrorPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AHorrorPlayerState, EvidenceCollected);
    DOREPLIFETIME(AHorrorPlayerState, bIsAlive);
    DOREPLIFETIME(AHorrorPlayerState, Health);
}

void AHorrorPlayerState::AddScore(int32 Points)
{
    if (HasAuthority())
    {
        SetScore(GetScore() + Points);
        EvidenceCollected++;
    }
}

void AHorrorPlayerState::TakeDamage(float Damage)
{
    if (HasAuthority())
    {
        Health = FMath::Max(0.0f, Health - Damage);
        
        if (Health <= 0.0f)
        {
            bIsAlive = false;
        }
    }
}
```

---

## Example 5: RPCs (Remote Procedure Calls)

```cpp
// NetworkedCharacter.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetworkedCharacter.generated.h"

UCLASS()
class HORRORPROJECT_API ANetworkedCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    // Server RPC - Client calls, executes on server
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerCollectItem(const FString& ItemID);
    
    // Multicast RPC - Server calls, executes on all clients
    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlaySound(USoundBase* Sound, FVector Location);
    
    // Client RPC - Server calls, executes on owning client
    UFUNCTION(Client, Reliable)
    void ClientShowNotification(const FString& Message);
};

// NetworkedCharacter.cpp
#include "NetworkedCharacter.h"
#include "Kismet/GameplayStatics.h"

void ANetworkedCharacter::ServerCollectItem_Implementation(const FString& ItemID)
{
    // Validate on server
    if (ItemID.IsEmpty())
    {
        return;
    }
    
    // Server-side logic
    UE_LOG(LogTemp, Log, TEXT("Server: Item collected: %s"), *ItemID);
    
    // Update game state
    AHorrorGameState* GameState = GetWorld()->GetGameState<AHorrorGameState>();
    if (GameState)
    {
        GameState->AddEvidence(ItemID);
    }
    
    // Update player state
    AHorrorPlayerState* PlayerState = GetPlayerState<AHorrorPlayerState>();
    if (PlayerState)
    {
        PlayerState->AddScore(10);
    }
    
    // Notify all clients
    MulticastPlaySound(CollectSound, GetActorLocation());
    
    // Notify owning client
    ClientShowNotification(FString::Printf(TEXT("Collected: %s"), *ItemID));
}

bool ANetworkedCharacter::ServerCollectItem_Validate(const FString& ItemID)
{
    // Validate input
    return !ItemID.IsEmpty() && ItemID.Len() < 100;
}

void ANetworkedCharacter::MulticastPlaySound_Implementation(USoundBase* Sound, FVector Location)
{
    if (Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Sound, Location);
    }
}

void ANetworkedCharacter::ClientShowNotification_Implementation(const FString& Message)
{
    // Show UI notification on client
    UE_LOG(LogTemp, Log, TEXT("Client notification: %s"), *Message);
}
```

---

## Example 6: Network Replication Component

```cpp
// NetworkSyncComponent.h
#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "NetworkSyncComponent.generated.h"

UCLASS()
class HORRORPROJECT_API UNetworkSyncComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNetworkSyncComponent();
    
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Sync")
    FVector SyncedLocation;
    
    UPROPERTY(Replicated, BlueprintReadWrite, Category = "Sync")
    FRotator SyncedRotation;
    
    UPROPERTY(ReplicatedUsing = OnRep_SyncedState, BlueprintReadWrite, Category = "Sync")
    bool bSyncedState;
    
    UFUNCTION()
    void OnRep_SyncedState();
    
    UFUNCTION(BlueprintCallable, Category = "Sync")
    void UpdateSyncedTransform(FVector NewLocation, FRotator NewRotation);
};

// NetworkSyncComponent.cpp
#include "NetworkSyncComponent.h"

UNetworkSyncComponent::UNetworkSyncComponent()
{
    SetIsReplicatedByDefault(true);
    bSyncedState = false;
}

void UNetworkSyncComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(UNetworkSyncComponent, SyncedLocation);
    DOREPLIFETIME(UNetworkSyncComponent, SyncedRotation);
    DOREPLIFETIME(UNetworkSyncComponent, bSyncedState);
}

void UNetworkSyncComponent::OnRep_SyncedState()
{
    // Called automatically when bSyncedState changes
    UE_LOG(LogTemp, Log, TEXT("Synced state changed to: %d"), bSyncedState);
    
    // Update visual representation
    AActor* Owner = GetOwner();
    if (Owner)
    {
        // Apply state change
    }
}

void UNetworkSyncComponent::UpdateSyncedTransform(FVector NewLocation, FRotator NewRotation)
{
    if (GetOwner()->HasAuthority())
    {
        SyncedLocation = NewLocation;
        SyncedRotation = NewRotation;
    }
}
```

---

## Blueprint Examples

### Create Session
```
On Host Button Clicked
  → Get Multiplayer Session Subsystem
  → Bind Event to OnCreateSessionComplete
  → Create Session (NumPlayers: 4, IsLAN: false, Name: "MyGame")
  
On Session Created
  → Branch (Success?)
    → True: Start Session
    → False: Show Error Message
```

### Find and Join Session
```
On Find Button Clicked
  → Get Multiplayer Session Subsystem
  → Bind Event to OnFindSessionsComplete
  → Find Sessions (MaxResults: 10, IsLAN: false)
  
On Sessions Found
  → Get Found Sessions Count
  → For Each Index (0 to Count)
    → Get Session Name
    → Get Session Ping
    → Add to List Widget
    
On Join Button Clicked
  → Get Multiplayer Session Subsystem
  → Join Session (Index: Selected Index)
```

### Replicate Variable
```
In Actor Blueprint:
  → Add Variable: Health (Float)
  → Set Replication: Replicated
  
On Take Damage (Server Event):
  → Health = Health - Damage
  → (Automatically replicates to clients)
```
