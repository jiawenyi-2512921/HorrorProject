# Runtime Errors

Solutions for crashes and runtime errors in HorrorProject.

## Startup Crashes

### Editor crashes on launch

**Symptoms**:
- Editor crashes immediately after splash screen
- "Fatal error" dialog appears
- No editor window opens

**Solutions**:

1. **Delete intermediate files**:
```bash
rm -rf Saved/ Intermediate/ Binaries/
```

2. **Regenerate project files**:
```bash
# Right-click HorrorProject.uproject
# Select "Generate Visual Studio project files"
```

3. **Rebuild solution**:
```bash
# Open in Visual Studio
# Build → Rebuild Solution
```

4. **Check log files**:
```bash
# View: Saved/Logs/HorrorProject.log
# Look for error messages before crash
```

5. **Verify engine installation**:
```bash
# Epic Games Launcher → Library → UE 5.6 → Verify
```

---

## Null Pointer Exceptions

### Access violation reading location

```
Exception thrown: read access violation.
this->EvidenceComponent was nullptr.
```

**Causes**:
- Component not initialized
- Object destroyed
- Invalid reference

**Solutions**:

1. **Check for null before use**:
```cpp
// Bad
EvidenceComponent->CollectEvidence(Evidence);

// Good
if (EvidenceComponent && EvidenceComponent->IsValidLowLevel())
{
    EvidenceComponent->CollectEvidence(Evidence);
}
```

2. **Initialize in constructor**:
```cpp
AMyCharacter::AMyCharacter()
{
    EvidenceComponent = CreateDefaultSubobject<UEvidenceCollectionComponent>(TEXT("Evidence"));
}
```

3. **Use ensure for debugging**:
```cpp
if (!ensure(EvidenceComponent))
{
    UE_LOG(LogTemp, Error, TEXT("EvidenceComponent is null!"));
    return;
}
```

4. **Check object validity**:
```cpp
if (IsValid(EvidenceComponent))
{
    // Safe to use
}
```

---

## Blueprint Errors

### Blueprint compilation failed

**Symptoms**:
- Red error icon on Blueprint
- "Blueprint could not be compiled"
- Missing nodes or connections

**Solutions**:

1. **Check for circular dependencies**:
- Blueprint A references Blueprint B
- Blueprint B references Blueprint A
- Solution: Use interfaces or event dispatchers

2. **Verify parent class**:
```cpp
// Ensure parent class is compiled
// Rebuild C++ project if parent is C++ class
```

3. **Clear Blueprint cache**:
```bash
rm -rf Saved/
# Reopen editor
```

4. **Fix broken references**:
- Open Blueprint
- Look for "?" icons
- Reconnect or replace broken nodes

5. **Reparent Blueprint**:
- File → Reparent Blueprint
- Select correct parent class
- Recompile

---

## Packaging Errors

### Package game failed

**Symptoms**:
- Packaging process fails
- Error in output log
- Incomplete package

**Solutions**:

1. **Check for compilation errors**:
```bash
# Ensure project compiles in Development configuration
# Build → Build Solution
```

2. **Verify content references**:
- All referenced assets exist
- No missing textures/sounds
- Blueprint references valid

3. **Check packaging settings**:
- Project Settings → Packaging
- Verify maps to include
- Check platform-specific settings

4. **Review packaging log**:
```bash
# Saved/Logs/Cook.txt
# Look for specific error messages
```

5. **Test in Standalone mode first**:
```bash
# Play → Standalone Game
# Verify game works before packaging
```

---

## Memory Errors

### Out of memory crash

```
Ran out of memory allocating [size] bytes
```

**Solutions**:

1. **Check for memory leaks**:
```cpp
// Use smart pointers
TSharedPtr<FData> Data = MakeShared<FData>();

// Use UPROPERTY for UObject references
UPROPERTY()
UMyObject* MyObject;
```

2. **Reduce texture sizes**:
- Lower texture resolution
- Enable texture streaming
- Use texture compression

3. **Optimize asset loading**:
```cpp
// Use async loading
FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
Streamable.RequestAsyncLoad(AssetPath, FStreamableDelegate::CreateUObject(this, &AMyActor::OnAssetLoaded));
```

4. **Profile memory usage**:
```bash
stat memory
stat memoryplatform
```

---

## Asset Loading Errors

### Failed to load asset

```
LogStreaming: Error: Failed to read file '../../../Content/Audio/Ambient.uasset'
```

**Solutions**:

1. **Verify asset exists**:
- Check Content Browser
- Ensure file not deleted
- Check file path is correct

2. **Fix asset references**:
```cpp
// Use soft references for optional assets
UPROPERTY(EditAnywhere, Category = "Audio")
TSoftObjectPtr<USoundBase> AmbientSound;

// Load when needed
if (AmbientSound.IsValid())
{
    USoundBase* Sound = AmbientSound.LoadSynchronous();
}
```

3. **Regenerate asset registry**:
- Tools → Refresh All Nodes
- File → Refresh All

4. **Check asset redirectors**:
- Content Browser → Filters → Show Redirectors
- Right-click → Fix Up Redirectors

---

## Physics Errors

### Physics constraint errors

```
LogPhysics: Warning: Constraint is invalid
```

**Solutions**:

1. **Verify constraint setup**:
- Both actors have physics enabled
- Constraint properly configured
- Actors not destroyed

2. **Check collision settings**:
- Collision enabled
- Correct collision channels
- Physics asset valid

3. **Validate physics bodies**:
```cpp
if (Mesh && Mesh->IsSimulatingPhysics())
{
    // Safe to apply physics
}
```

---

## Network Errors

### Replication errors

```
LogNet: Warning: Failed to replicate property
```

**Solutions**:

1. **Mark properties for replication**:
```cpp
UPROPERTY(Replicated)
int32 EvidenceCount;

void AMyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMyActor, EvidenceCount);
}
```

2. **Enable replication on actor**:
```cpp
AMyActor::AMyActor()
{
    bReplicates = true;
}
```

3. **Check network authority**:
```cpp
if (HasAuthority())
{
    // Server-only code
}
```

---

## Audio Errors

### Audio not playing

**Solutions**:

1. **Check audio subsystem**:
```cpp
UHorrorAudioSubsystem* AudioSys = GetWorld()->GetSubsystem<UHorrorAudioSubsystem>();
if (!AudioSys)
{
    UE_LOG(LogAudio, Error, TEXT("Audio subsystem not initialized"));
    return;
}
```

2. **Verify sound asset**:
```cpp
if (!Sound || !Sound->IsValidLowLevel())
{
    UE_LOG(LogAudio, Warning, TEXT("Invalid sound asset"));
    return;
}
```

3. **Check audio settings**:
- Audio device enabled
- Volume not muted
- Attenuation settings correct

---

## Animation Errors

### Animation not playing

**Solutions**:

1. **Verify animation asset**:
- Animation exists
- Skeleton matches
- Animation blueprint compiled

2. **Check animation instance**:
```cpp
UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
if (AnimInstance)
{
    AnimInstance->Montage_Play(Animation);
}
```

3. **Debug animation blueprint**:
- Enable animation debug
- Check state machine transitions
- Verify blend spaces

---

## Save/Load Errors

### Failed to save game

```
LogSaveGame: Error: Failed to save game to slot
```

**Solutions**:

1. **Check save directory permissions**:
```cpp
// Ensure save directory is writable
FString SaveDir = FPaths::ProjectSavedDir();
```

2. **Validate save data**:
```cpp
if (SaveGame && SaveGame->IsValidLowLevel())
{
    UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);
}
```

3. **Handle save failures**:
```cpp
bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);
if (!bSuccess)
{
    UE_LOG(LogSave, Error, TEXT("Failed to save game"));
    // Show error to player
}
```

---

## Debugging Tools

### Visual Studio Debugger
```cpp
// Set breakpoints
// Press F5 to launch with debugger
// Inspect variables when breakpoint hits
```

### Unreal Console Commands
```bash
stat fps          # Show FPS
stat unit         # Frame time breakdown
stat memory       # Memory usage
showdebug         # Debug overlay
```

### Logging
```cpp
UE_LOG(LogTemp, Warning, TEXT("Value: %d"), Value);
UE_LOG(LogTemp, Error, TEXT("Error: %s"), *ErrorMessage);
```

### Crash Dumps
```bash
# Located in: Saved/Crashes/
# Open .dmp file in Visual Studio
# Debug → Open Crash Dump
```

---

## Getting Help

When reporting runtime errors, include:
- Full error message
- Steps to reproduce
- Log files (Saved/Logs/)
- Crash dumps if available
- System specifications

See [FAQ](FAQ.md) for more solutions.

## Related Documentation
- [Compilation Errors](CompilationErrors.md)
- [Performance Issues](PerformanceIssues.md)
- [Debugging Tutorial](../Tutorials/DebuggingTechniques.md)
