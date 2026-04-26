# Performance Issues

Diagnostic and solutions for performance problems in HorrorProject.

## Low FPS

### Symptoms
- Frame rate below 60 FPS
- Stuttering gameplay
- Slow response time

### Diagnosis

```bash
# Check frame rate
stat fps

# Check frame time breakdown
stat unit

# Check GPU performance
stat gpu

# Check rendering stats
stat scenerendering
```

### Solutions

#### CPU Optimization

1. **Reduce tick frequency**:
```cpp
// Disable tick if not needed
PrimaryActorTick.bCanEverTick = false;

// Or reduce tick interval
PrimaryActorTick.TickInterval = 0.1f;  // Every 0.1 seconds
```

2. **Use timers instead of tick**:
```cpp
void AMyActor::BeginPlay()
{
    Super::BeginPlay();
    
    GetWorldTimerManager().SetTimer(
        UpdateTimerHandle,
        this,
        &AMyActor::UpdateLogic,
        0.5f,  // Every 0.5 seconds
        true   // Loop
    );
}
```

3. **Cache expensive lookups**:
```cpp
// Bad: Every frame
void Tick(float DeltaTime)
{
    UEvidenceCollectionComponent* Evidence = FindComponentByClass<UEvidenceCollectionComponent>();
}

// Good: Cache in BeginPlay
void BeginPlay()
{
    CachedEvidence = FindComponentByClass<UEvidenceCollectionComponent>();
}
```

4. **Optimize AI**:
```cpp
// Reduce perception update frequency
PerceptionComponent->SetSenseConfig(
    UAISenseConfig_Sight::StaticClass(),
    SightConfig
);
SightConfig->SetMaxAge(5.0f);  // Update every 5 seconds
```

#### GPU Optimization

1. **Reduce draw calls**:
- Use instanced static meshes
- Merge static meshes
- Use LODs (Level of Detail)

2. **Optimize materials**:
- Reduce material complexity
- Use material instances
- Disable features not needed

3. **Optimize lighting**:
- Use static lighting where possible
- Reduce dynamic light count
- Use lightmaps

4. **Optimize shadows**:
```cpp
// Reduce shadow distance
r.Shadow.MaxResolution 1024
r.Shadow.DistanceScale 0.5
```

---

## Stuttering

### Symptoms
- Periodic frame drops
- Hitches during gameplay
- Irregular frame times

### Diagnosis

```bash
# Check for hitches
stat unitgraph

# Check garbage collection
stat memory

# Profile with Unreal Insights
UnrealInsights
```

### Solutions

#### Garbage Collection Hitches

1. **Reduce GC frequency**:
```cpp
// In DefaultEngine.ini
[/Script/Engine.GarbageCollectionSettings]
gc.TimeBetweenPurgingPendingKillObjects=60.0
gc.MaxObjectsInGame=200000
```

2. **Use object pooling**:
```cpp
class UObjectPool : public UObject
{
    TArray<UMyObject*> Pool;
    
public:
    UMyObject* GetObject()
    {
        if (Pool.Num() > 0)
        {
            return Pool.Pop();
        }
        return NewObject<UMyObject>();
    }
    
    void ReturnObject(UMyObject* Object)
    {
        Pool.Add(Object);
    }
};
```

#### Asset Loading Hitches

1. **Use async loading**:
```cpp
FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
Streamable.RequestAsyncLoad(
    AssetPath,
    FStreamableDelegate::CreateUObject(this, &AMyActor::OnAssetLoaded)
);
```

2. **Preload critical assets**:
```cpp
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Preload assets
    TArray<FSoftObjectPath> AssetsToLoad;
    AssetsToLoad.Add(FSoftObjectPath(TEXT("/Game/Audio/Ambient.Ambient")));
    
    UAssetManager::GetStreamableManager().RequestAsyncLoad(AssetsToLoad);
}
```

3. **Use texture streaming**:
```cpp
// In DefaultEngine.ini
[/Script/Engine.RendererSettings]
r.Streaming.PoolSize=2000
r.Streaming.UseFixedPoolSize=False
```

---

## High Memory Usage

### Symptoms
- Memory usage over 8GB
- Out of memory crashes
- Slow performance

### Diagnosis

```bash
# Check memory usage
stat memory

# Check memory by category
stat memoryplatform

# Detailed memory report
obj list
```

### Solutions

1. **Reduce texture memory**:
```cpp
// In DefaultEngine.ini
[/Script/Engine.RendererSettings]
r.Streaming.PoolSize=1000
r.Streaming.MaxEffectiveScreenSize=0
```

2. **Use texture streaming**:
```cpp
// Enable on static meshes
StaticMeshComponent->bUseStreamingTextures = true;
```

3. **Optimize audio**:
- Use compressed audio formats
- Stream large audio files
- Reduce sample rate where possible

4. **Fix memory leaks**:
```cpp
// Use UPROPERTY for UObject references
UPROPERTY()
UMyObject* MyObject;

// Use smart pointers for non-UObject
TSharedPtr<FMyData> MyData;
```

---

## Long Load Times

### Symptoms
- Slow level loading
- Long startup time
- Delayed asset loading

### Solutions

1. **Use level streaming**:
```cpp
// Load level async
FLatentActionInfo LatentInfo;
LatentInfo.CallbackTarget = this;
UGameplayStatics::LoadStreamLevel(
    GetWorld(),
    LevelName,
    true,
    false,
    LatentInfo
);
```

2. **Optimize asset references**:
```cpp
// Use soft references
UPROPERTY(EditAnywhere, Category = "Assets")
TSoftObjectPtr<UStaticMesh> MeshAsset;

// Load when needed
UStaticMesh* Mesh = MeshAsset.LoadSynchronous();
```

3. **Reduce asset count**:
- Remove unused assets
- Combine similar assets
- Use asset bundles

4. **Enable pak file compression**:
```cpp
// In DefaultGame.ini
[/Script/UnrealEd.ProjectPackagingSettings]
bCompressed=True
```

---

## Audio Performance

### Issues
- Audio crackling
- Delayed sound playback
- High CPU usage from audio

### Solutions

1. **Limit simultaneous sounds**:
```cpp
// In HorrorAudioSubsystem
UPROPERTY(EditDefaultsOnly, Category = "Settings")
int32 MaxAmbientSounds = 3;
```

2. **Use audio zones**:
```cpp
// Only play sounds in active zones
if (AudioZone && AudioZone->IsPlayerInZone())
{
    PlayAmbientSound(Sound);
}
```

3. **Optimize audio settings**:
```cpp
// In DefaultEngine.ini
[Audio]
AudioMaxChannels=32
AudioNumSourceWorkers=4
```

---

## Blueprint Performance

### Issues
- Slow Blueprint execution
- High CPU usage from Blueprints
- Tick performance problems

### Solutions

1. **Minimize Blueprint tick**:
- Disable tick when not needed
- Use events instead of tick
- Use timers for periodic updates

2. **Optimize Blueprint logic**:
- Reduce node count
- Cache references
- Avoid complex math in tick

3. **Use C++ for performance-critical code**:
```cpp
// Move expensive logic to C++
UFUNCTION(BlueprintCallable, Category = "Performance")
void OptimizedFunction();
```

4. **Enable Blueprint nativization** (shipping builds):
```cpp
// Project Settings → Packaging
// Blueprint Nativization Method: Inclusive
```

---

## Profiling Tools

### Stat Commands
```bash
stat fps          # Frame rate
stat unit         # Frame time breakdown
stat game         # Game thread time
stat gpu          # GPU time
stat memory       # Memory usage
stat audio        # Audio statistics
```

### Unreal Insights
```bash
# Launch Unreal Insights
UnrealInsights

# Start trace
Trace.Start

# Stop trace
Trace.Stop

# Analyze in Unreal Insights
```

### GPU Profiler
```bash
# Profile GPU
profilegpu

# Detailed GPU stats
stat gpu
```

### Memory Profiler
```bash
# Memory report
memreport -full

# Object list
obj list

# Garbage collection stats
obj gc
```

---

## Performance Targets

### Minimum Spec (GTX 1060, 16GB RAM)
- **FPS**: 60 FPS minimum
- **Frame Time**: < 16.67ms
- **Memory**: < 8GB
- **Load Time**: < 30 seconds

### Recommended Spec (RTX 3060, 32GB RAM)
- **FPS**: 120 FPS
- **Frame Time**: < 8.33ms
- **Memory**: < 12GB
- **Load Time**: < 15 seconds

---

## Optimization Checklist

### CPU
- [ ] Minimize tick functions
- [ ] Use timers instead of tick
- [ ] Cache component references
- [ ] Optimize AI perception
- [ ] Reduce actor count

### GPU
- [ ] Use LODs
- [ ] Optimize materials
- [ ] Reduce draw calls
- [ ] Use static lighting
- [ ] Optimize shadows

### Memory
- [ ] Enable texture streaming
- [ ] Use compressed textures
- [ ] Fix memory leaks
- [ ] Use object pooling
- [ ] Reduce asset sizes

### Loading
- [ ] Use async loading
- [ ] Enable level streaming
- [ ] Preload critical assets
- [ ] Use soft references
- [ ] Compress pak files

---

## Related Documentation
- [Performance Best Practices](../BestPractices/PerformanceOptimization.md)
- [Profiling Guide](../Performance/ProfilingGuide.md)
- [Optimization Tutorial](../Tutorials/PerformanceOptimization.md)
