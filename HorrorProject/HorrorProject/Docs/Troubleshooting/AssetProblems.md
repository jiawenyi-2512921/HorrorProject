# Asset Problems

Solutions for asset loading, management, and usage issues.

## Loading Failures

### Failed to load asset

```
LogStreaming: Error: Failed to read file '../../../Content/Audio/Ambient.uasset'
```

**Causes**:
- Asset deleted or moved
- Incorrect file path
- Corrupted asset file
- Missing dependencies

**Solutions**:

1. **Verify asset exists**:
- Open Content Browser
- Search for asset by name
- Check file exists on disk

2. **Fix asset path**:
```cpp
// Use correct path format
FSoftObjectPath AssetPath(TEXT("/Game/Audio/Ambient.Ambient"));

// Or use soft reference
UPROPERTY(EditAnywhere, Category = "Audio")
TSoftObjectPtr<USoundBase> AmbientSound;
```

3. **Regenerate asset registry**:
- Tools → Refresh All Nodes
- File → Refresh All
- Restart editor

4. **Check asset dependencies**:
- Right-click asset → Reference Viewer
- Ensure all dependencies exist

---

## Missing Textures

### Pink/checkerboard textures

**Symptoms**:
- Pink or checkerboard pattern on meshes
- "Missing texture" warnings in log
- Broken material references

**Solutions**:

1. **Find missing textures**:
```bash
# Check log for missing assets
# Saved/Logs/HorrorProject.log
# Search for "Failed to load"
```

2. **Fix material references**:
- Open material in editor
- Find broken texture nodes (red X)
- Reassign correct textures

3. **Fix redirectors**:
- Content Browser → Filters → Show Redirectors
- Right-click redirectors → Fix Up Redirectors

4. **Reimport textures**:
- Right-click texture → Reimport
- Or reimport from source file

---

## Audio Issues

### Audio not playing

**Symptoms**:
- No sound output
- Audio component not working
- Silent gameplay

**Solutions**:

1. **Check audio asset**:
```cpp
if (!Sound || !Sound->IsValidLowLevel())
{
    UE_LOG(LogAudio, Warning, TEXT("Invalid sound asset"));
    return;
}
```

2. **Verify audio subsystem**:
```cpp
UHorrorAudioSubsystem* AudioSys = GetWorld()->GetSubsystem<UHorrorAudioSubsystem>();
if (!AudioSys)
{
    UE_LOG(LogAudio, Error, TEXT("Audio subsystem not initialized"));
    return;
}
```

3. **Check audio settings**:
- Volume not muted
- Audio device enabled
- Attenuation settings correct
- Audio component activated

4. **Test audio asset**:
- Double-click sound in Content Browser
- Verify it plays in editor
- Check sample rate and format

### Audio crackling/popping

**Solutions**:

1. **Increase audio buffer size**:
```cpp
// In DefaultEngine.ini
[Audio]
AudioCallbackBufferFrameSize=1024
AudioNumBuffersToEnqueue=2
```

2. **Reduce simultaneous sounds**:
```cpp
// Limit active sounds
UPROPERTY(EditDefaultsOnly, Category = "Audio")
int32 MaxAmbientSounds = 3;
```

3. **Use compressed audio**:
- Convert to OGG Vorbis
- Reduce sample rate if possible
- Use streaming for large files

---

## Broken References

### Blueprint references broken

**Symptoms**:
- "?" icons in Blueprint
- Missing nodes
- Compilation errors
- Broken variable types

**Solutions**:

1. **Find broken references**:
- Open Blueprint
- Look for nodes with "?" icon
- Check variables with broken types

2. **Fix asset references**:
```cpp
// Use soft references to prevent hard dependencies
UPROPERTY(EditAnywhere, Category = "Assets")
TSoftObjectPtr<UStaticMesh> MeshAsset;
```

3. **Reparent Blueprint**:
- File → Reparent Blueprint
- Select correct parent class
- Recompile Blueprint

4. **Replace broken nodes**:
- Delete broken nodes
- Add correct nodes
- Reconnect execution pins

### C++ class references broken

**Solutions**:

1. **Rebuild C++ project**:
```bash
# In Visual Studio
Build → Rebuild Solution
```

2. **Regenerate project files**:
```bash
# Right-click .uproject
# Generate Visual Studio project files
```

3. **Update Blueprint parent**:
- Compile C++ class first
- Reopen Blueprint
- Reparent if needed

---

## Asset Corruption

### Corrupted asset file

**Symptoms**:
- Asset won't open
- Crash when loading asset
- "Asset is corrupt" error

**Solutions**:

1. **Restore from backup**:
```bash
# Check version control
git checkout path/to/asset.uasset

# Or restore from backup folder
```

2. **Reimport from source**:
- Right-click asset → Reimport
- Select source file
- Reimport with correct settings

3. **Recreate asset**:
- Delete corrupted asset
- Create new asset
- Reconfigure settings
- Update references

---

## Asset Organization

### Disorganized content

**Best Practices**:

1. **Use consistent naming**:
```
T_AssetName_Suffix    # Textures
M_AssetName           # Materials
MI_AssetName          # Material Instances
SM_AssetName          # Static Meshes
SK_AssetName          # Skeletal Meshes
BP_AssetName          # Blueprints
```

2. **Organize by type**:
```
Content/
  Audio/
    Ambient/
    Music/
    SFX/
  Blueprints/
    Characters/
    Interactables/
  Materials/
  Meshes/
  Textures/
```

3. **Use folders effectively**:
- Group related assets
- Separate by feature
- Use subfolders for variants

See [Asset Management Best Practices](../BestPractices/AssetManagement.md)

---

## Asset Loading Performance

### Slow asset loading

**Solutions**:

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
    
    TArray<FSoftObjectPath> AssetsToLoad;
    AssetsToLoad.Add(FSoftObjectPath(TEXT("/Game/Audio/Ambient.Ambient")));
    
    UAssetManager::GetStreamableManager().RequestAsyncLoad(AssetsToLoad);
}
```

3. **Use soft references**:
```cpp
// Soft reference (loaded on demand)
UPROPERTY(EditAnywhere, Category = "Assets")
TSoftObjectPtr<UStaticMesh> OptionalMesh;

// Hard reference (always loaded)
UPROPERTY(EditAnywhere, Category = "Assets")
UStaticMesh* RequiredMesh;
```

---

## Asset Size Issues

### Large asset sizes

**Solutions**:

1. **Compress textures**:
- Use DXT compression
- Reduce texture resolution
- Use texture groups

2. **Optimize meshes**:
- Reduce polygon count
- Use LODs
- Remove unnecessary vertices

3. **Compress audio**:
- Use OGG Vorbis format
- Reduce sample rate
- Use mono for non-spatial audio

4. **Check asset sizes**:
```bash
# In Content Browser
# View Options → Show Asset Size
# Sort by size to find large assets
```

---

## Version Control Issues

### Asset conflicts

**Solutions**:

1. **Use binary merge tool**:
```bash
# Configure in .gitattributes
*.uasset binary
*.umap binary
```

2. **Avoid simultaneous edits**:
- Communicate with team
- Lock assets when editing
- Use source control properly

3. **Resolve conflicts**:
```bash
# Keep local version
git checkout --ours path/to/asset.uasset

# Keep remote version
git checkout --theirs path/to/asset.uasset
```

---

## Asset Migration

### Migrating assets between projects

**Steps**:

1. **Use migration tool**:
- Right-click asset → Asset Actions → Migrate
- Select destination project
- Verify dependencies included

2. **Manual migration**:
```bash
# Copy asset files
cp Source/Content/MyAsset.uasset Dest/Content/

# Copy dependencies
# Check Reference Viewer for dependencies
```

3. **Fix references after migration**:
- Update asset paths
- Reconnect broken references
- Test thoroughly

---

## Asset Validation

### Validate assets before use

```cpp
// Check asset validity
if (!Asset || !Asset->IsValidLowLevel())
{
    UE_LOG(LogTemp, Error, TEXT("Invalid asset"));
    return;
}

// Check asset loaded
if (!Asset->IsAsyncLoading())
{
    // Safe to use
}

// Use IsValid helper
if (IsValid(Asset))
{
    // Safe to use
}
```

---

## Common Asset Errors

### "Asset is being garbage collected"

**Solution**: Use UPROPERTY to prevent GC:
```cpp
UPROPERTY()
UMyAsset* Asset;
```

### "Asset not found in package"

**Solution**: Verify asset path and regenerate asset registry

### "Failed to import asset"

**Solution**: Check source file format and import settings

---

## Asset Auditing

### Find unused assets

```bash
# In Content Browser
# Filters → Show Only Assets Not Used In Levels
```

### Find missing assets

```bash
# Check log for "Failed to load" messages
# Use Reference Viewer to find broken references
```

### Asset size report

```bash
# Content Browser → View Options → Show Asset Size
# Sort by size to identify large assets
```

---

## Related Documentation
- [Asset Management Best Practices](../BestPractices/AssetManagement.md)
- [Asset Optimization](../Performance/AssetOptimization.md)
- [Asset Naming Convention](../Assets/NamingConvention.md)
