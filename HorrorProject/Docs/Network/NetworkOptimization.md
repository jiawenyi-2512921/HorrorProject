# Network Optimization Guide

## Overview
This guide covers optimization techniques for HorrorProject's multiplayer networking to achieve smooth gameplay with minimal bandwidth usage.

## Replication Optimization

### 1. Conditional Replication

#### Relevancy-Based Replication
```cpp
bool AActor::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
    // Only replicate to nearby players
    if (RealViewer)
    {
        float DistanceSquared = (GetActorLocation() - SrcLocation).SizeSquared();
        float MaxDistanceSquared = 5000.0f * 5000.0f; // 50 meters
        return DistanceSquared <= MaxDistanceSquared;
    }
    return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}
```

#### Conditional Property Replication
```cpp
void AMyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // Only replicate to owner
    DOREPLIFETIME_CONDITION(AMyActor, PrivateData, COND_OwnerOnly);
    
    // Skip owner (for other players only)
    DOREPLIFETIME_CONDITION(AMyActor, PublicData, COND_SkipOwner);
    
    // Only when changed
    DOREPLIFETIME_CONDITION(AMyActor, RarelyChangedData, COND_InitialOnly);
}
```

### 2. Replication Rate Tuning

#### Dynamic Update Frequency
```cpp
void AMyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Reduce update rate for distant players
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        float Distance = GetDistanceTo(PC->GetViewTarget());
        
        if (Distance > 3000.0f)
        {
            NetUpdateFrequency = 5.0f;  // Low priority
        }
        else if (Distance > 1000.0f)
        {
            NetUpdateFrequency = 15.0f; // Medium priority
        }
        else
        {
            NetUpdateFrequency = 30.0f; // High priority
        }
    }
}
```

#### Component-Level Control
```cpp
// In NetworkReplicationComponent
void UNetworkReplicationComponent::OptimizeReplicationRate()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    // Check if actor is moving
    FVector Velocity = Owner->GetVelocity();
    bool bIsMoving = !Velocity.IsNearlyZero();
    
    if (bIsMoving)
    {
        ReplicationRate = 30.0f; // High rate when moving
    }
    else
    {
        ReplicationRate = 5.0f;  // Low rate when stationary
    }
}
```

### 3. Delta Compression

#### Position Compression
```cpp
void UNetworkSyncComponent::SyncTransform(FVector Location, FRotator Rotation)
{
    if (bEnableDeltaCompression)
    {
        // Only sync if changed significantly
        float PosDelta = FVector::Dist(Location, LastSyncedPosition);
        float RotDelta = FMath::Abs((Rotation - LastSyncedRotation).Yaw);
        
        if (PosDelta > PositionTolerance || RotDelta > RotationTolerance)
        {
            LastSyncedPosition = Location;
            LastSyncedRotation = Rotation;
            // Replicate
        }
    }
}
```

#### Quantization
```cpp
// Compress vector to 16-bit per component
FVector CompressPosition(const FVector& Position)
{
    FVector Compressed;
    Compressed.X = FMath::RoundToFloat(Position.X / 0.1f) * 0.1f;
    Compressed.Y = FMath::RoundToFloat(Position.Y / 0.1f) * 0.1f;
    Compressed.Z = FMath::RoundToFloat(Position.Z / 0.1f) * 0.1f;
    return Compressed;
}

// Compress rotation to 8-bit per component
FRotator CompressRotation(const FRotator& Rotation)
{
    FRotator Compressed;
    Compressed.Pitch = FMath::RoundToFloat(Rotation.Pitch / 1.0f) * 1.0f;
    Compressed.Yaw = FMath::RoundToFloat(Rotation.Yaw / 1.0f) * 1.0f;
    Compressed.Roll = FMath::RoundToFloat(Rotation.Roll / 1.0f) * 1.0f;
    return Compressed;
}
```

## Bandwidth Optimization

### 1. RPC Optimization

#### Batch RPCs
```cpp
// Instead of multiple RPCs
void SendMultipleEvents()
{
    ServerEvent1();
    ServerEvent2();
    ServerEvent3();
}

// Use single batched RPC
USTRUCT()
struct FBatchedEvents
{
    GENERATED_BODY()
    TArray<FString> Events;
};

UFUNCTION(Server, Reliable)
void ServerBatchedEvents(const FBatchedEvents& Events);
```

#### Unreliable RPCs for Non-Critical Data
```cpp
// Use unreliable for frequent, non-critical updates
UFUNCTION(NetMulticast, Unreliable)
void MulticastUpdateVisualEffect(FVector Location);

// Use reliable for critical events
UFUNCTION(NetMulticast, Reliable)
void MulticastPlayerDied(APlayerState* Player);
```

### 2. Data Structure Optimization

#### Efficient Structs
```cpp
// Bad: Large struct with unnecessary data
USTRUCT()
struct FBadPlayerData
{
    GENERATED_BODY()
    FString PlayerName;        // 64+ bytes
    FVector Position;          // 12 bytes
    FRotator Rotation;         // 12 bytes
    TArray<FString> Inventory; // Variable size
};

// Good: Compact struct with essential data
USTRUCT()
struct FGoodPlayerData
{
    GENERATED_BODY()
    uint8 PlayerID;            // 1 byte
    FVector_NetQuantize Position; // 6 bytes (quantized)
    uint8 CompressedYaw;       // 1 byte (0-255 = 0-360°)
    uint32 InventoryBitMask;   // 4 bytes (32 items max)
};
```

#### Bit Packing
```cpp
USTRUCT()
struct FPackedPlayerState
{
    GENERATED_BODY()
    
    uint8 Health : 7;      // 0-127
    uint8 bIsAlive : 1;    // 1 bit
    uint8 Stamina : 7;     // 0-127
    uint8 bIsHiding : 1;   // 1 bit
    uint8 Fear : 7;        // 0-127
    uint8 bIsSprinting : 1; // 1 bit
};
```

### 3. Dormancy

#### Actor Dormancy
```cpp
void AMyActor::BeginPlay()
{
    Super::BeginPlay();
    
    // Enable dormancy for static/inactive actors
    NetDormancy = DORM_DormantAll;
}

void AMyActor::OnInteracted()
{
    // Wake up when needed
    FlushNetDormancy();
    
    // Do interaction logic
    
    // Go back to sleep after delay
    GetWorld()->GetTimerManager().SetTimer(
        DormancyTimer,
        [this]() { NetDormancy = DORM_DormantAll; },
        5.0f,
        false
    );
}
```

## Client Prediction & Interpolation

### 1. Movement Prediction

#### Client-Side Prediction
```cpp
void AMyCharacter::MoveForward(float Value)
{
    // Predict locally immediately
    FVector Movement = GetActorForwardVector() * Value * GetWorld()->GetDeltaSeconds();
    AddActorWorldOffset(Movement);
    
    // Send to server for validation
    if (GetController() && GetController()->IsLocalController())
    {
        ServerMove(Movement, GetWorld()->GetTimeSeconds());
    }
}

UFUNCTION(Server, Reliable, WithValidation)
void ServerMove(FVector Movement, float Timestamp);

void AMyCharacter::ServerMove_Implementation(FVector Movement, float Timestamp)
{
    // Server validates and applies
    float ServerTime = GetWorld()->GetTimeSeconds();
    float Latency = ServerTime - Timestamp;
    
    if (Latency < 0.5f) // Reasonable latency
    {
        AddActorWorldOffset(Movement);
    }
}
```

#### Interpolation
```cpp
void UNetworkReplicationComponent::InterpolateTransform(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner || Owner->HasAuthority()) return;
    
    // Smooth interpolation to server position
    FVector CurrentPos = Owner->GetActorLocation();
    FVector TargetPos = ReplicatedTransform.Location;
    
    FVector NewPos = FMath::VInterpTo(
        CurrentPos,
        TargetPos,
        DeltaTime,
        InterpolationSpeed
    );
    
    Owner->SetActorLocation(NewPos);
}
```

### 2. Lag Compensation

#### Server-Side Rewind
```cpp
void AHorrorPlayerControllerMultiplayer::ServerShoot_Implementation(FVector Start, FVector Direction, float ClientTime)
{
    float ServerTime = GetWorld()->GetTimeSeconds();
    float Latency = ServerTime - ClientTime;
    
    // Rewind world state by latency
    RewindWorldState(Latency);
    
    // Perform hit detection
    FHitResult Hit;
    GetWorld()->LineTraceSingleByChannel(Hit, Start, Start + Direction * 10000.0f, ECC_Visibility);
    
    // Restore world state
    RestoreWorldState();
    
    // Process hit
    if (Hit.bBlockingHit)
    {
        // Apply damage
    }
}
```

## Memory Optimization

### 1. Object Pooling

#### Actor Pooling
```cpp
class ANetworkActorPool : public AActor
{
public:
    TArray<AActor*> AvailableActors;
    TArray<AActor*> ActiveActors;
    
    AActor* GetPooledActor()
    {
        if (AvailableActors.Num() > 0)
        {
            AActor* Actor = AvailableActors.Pop();
            ActiveActors.Add(Actor);
            Actor->SetActorHiddenInGame(false);
            return Actor;
        }
        return nullptr;
    }
    
    void ReturnToPool(AActor* Actor)
    {
        ActiveActors.Remove(Actor);
        AvailableActors.Add(Actor);
        Actor->SetActorHiddenInGame(true);
    }
};
```

### 2. Connection Optimization

#### Adaptive Quality
```cpp
void AHorrorPlayerControllerMultiplayer::AdaptNetworkQuality()
{
    float Ping = GetCurrentPing();
    float PacketLoss = GetPacketLoss();
    
    if (Ping > 150.0f || PacketLoss > 5.0f)
    {
        // Reduce quality
        if (UNetworkReplicationComponent* RepComp = GetPawn()->FindComponentByClass<UNetworkReplicationComponent>())
        {
            RepComp->ReplicationRate = 15.0f;
            RepComp->InterpolationSpeed = 5.0f;
        }
    }
    else
    {
        // Restore quality
        if (UNetworkReplicationComponent* RepComp = GetPawn()->FindComponentByClass<UNetworkReplicationComponent>())
        {
            RepComp->ReplicationRate = 30.0f;
            RepComp->InterpolationSpeed = 10.0f;
        }
    }
}
```

## Performance Monitoring

### 1. Network Stats

#### Enable Stats
```cpp
// In console
stat net
stat netplayermovement
stat game
```

#### Custom Profiling
```cpp
void UNetworkSyncComponent::ProfileBandwidth()
{
    static int32 TotalBytesSent = 0;
    static float LastReportTime = 0.0f;
    
    TotalBytesSent += BytesSentThisFrame;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastReportTime >= 1.0f)
    {
        float BandwidthKBps = TotalBytesSent / 1024.0f;
        UE_LOG(LogTemp, Log, TEXT("Bandwidth: %.2f KB/s"), BandwidthKBps);
        
        TotalBytesSent = 0;
        LastReportTime = CurrentTime;
    }
}
```

### 2. Optimization Checklist

- [ ] Replication rate optimized per actor type
- [ ] Conditional replication implemented
- [ ] Delta compression enabled
- [ ] Dormancy used for static actors
- [ ] RPCs batched where possible
- [ ] Unreliable RPCs for non-critical data
- [ ] Data structures optimized
- [ ] Client prediction implemented
- [ ] Interpolation smooth
- [ ] Lag compensation working
- [ ] Bandwidth under 50 KB/s per client
- [ ] Latency under 150ms
- [ ] Packet loss under 5%

## Best Practices

1. **Profile First**: Measure before optimizing
2. **Prioritize**: Optimize high-frequency updates first
3. **Test with Latency**: Use network emulation
4. **Monitor Bandwidth**: Track usage per actor
5. **Iterate**: Optimize incrementally
6. **Document**: Note optimization decisions

## Common Pitfalls

1. **Over-replication**: Replicating too much data
2. **Reliable RPCs**: Using reliable for frequent updates
3. **Large Structs**: Replicating unnecessary data
4. **No Dormancy**: Active replication for static actors
5. **Fixed Rates**: Not adapting to network conditions
6. **No Prediction**: Poor responsiveness
7. **Synchronous Logic**: Blocking on network calls

## Tools

- **Network Profiler**: Built-in UE profiler
- **Wireshark**: Packet analysis
- **NetLimiter**: Bandwidth throttling
- **Clumsy**: Network condition simulation
- **PIE Network Emulation**: Editor testing

## Further Reading

- Unreal Engine Network Compendium
- Valve's Source Multiplayer Networking
- Gaffer on Games: Networked Physics
- GDC Talks on Network Optimization
