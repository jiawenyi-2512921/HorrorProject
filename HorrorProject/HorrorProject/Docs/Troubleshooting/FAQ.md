# Frequently Asked Questions (FAQ)

Answers to 50+ common questions about HorrorProject.

## General Questions

### What is HorrorProject?
HorrorProject is a comprehensive horror game framework built on Unreal Engine 5.6. It provides systems for audio, evidence collection, AI, interactions, and more.

### What are the system requirements?
- **Unreal Engine**: 5.6 or later
- **IDE**: Visual Studio 2022
- **RAM**: 16GB minimum, 32GB recommended
- **GPU**: NVIDIA GTX 1060 or equivalent
- **Storage**: 50GB free space

### Is this project open source?
Check the LICENSE file in the repository root for licensing information.

### Can I use this for commercial projects?
Refer to the LICENSE file for commercial usage terms.

---

## Setup & Installation

### How do I set up the project?
Follow the [Getting Started Guide](../Tutorials/GettingStarted.md).

### Build fails with "Cannot open include file"
**Solution**: 
1. Regenerate Visual Studio project files
2. Clean and rebuild solution
3. Check that all required modules are listed in `HorrorProject.Build.cs`

See [Compilation Errors](CompilationErrors.md#missing-includes) for details.

### Editor crashes on first launch
**Solution**:
1. Delete `Saved/` and `Intermediate/` folders
2. Regenerate project files
3. Rebuild in Visual Studio
4. Launch editor

See [Runtime Errors](RuntimeErrors.md#startup-crashes) for details.

### How do I update to the latest version?
```bash
git pull origin main
# Regenerate project files
# Rebuild solution
```

---

## Audio System

### How do I play ambient sounds?
```cpp
UHorrorAudioSubsystem* AudioSys = GetWorld()->GetSubsystem<UHorrorAudioSubsystem>();
AudioSys->PlayAmbientSound(MySound, 2.0f);
```

See [Audio System Tutorial](../Tutorials/AudioSystem.md).

### Audio zones not working
**Solution**:
1. Ensure audio zone is registered with subsystem
2. Check zone bounds overlap player
3. Verify zone priority settings

See [Audio Troubleshooting](AssetProblems.md#audio-issues).

### How do I add footstep sounds?
Add `UFootstepAudioComponent` to your character:
```cpp
FootstepComponent = CreateDefaultSubobject<UFootstepAudioComponent>(TEXT("Footsteps"));
```

See [Audio Blueprint Guide](../Blueprint/Audio_Blueprint_Guide.md).

### Music intensity not changing
**Solution**:
1. Check that music system is initialized
2. Verify intensity value is between 0.0 and 1.0
3. Ensure music asset has intensity layers

### How do I optimize audio performance?
- Limit simultaneous sounds
- Use audio zones
- Enable sound occlusion
- Use appropriate attenuation settings

See [Audio Optimization](../Performance/AudioOptimization.md).

---

## Evidence System

### How do I create evidence items?
1. Create Blueprint based on `AEvidenceActor`
2. Set evidence data (ID, name, description)
3. Add collision for interaction
4. Place in level

See [Evidence Tutorial](../Tutorials/EvidenceCollection.md).

### Evidence not collecting
**Solution**:
1. Check player has `UEvidenceCollectionComponent`
2. Verify evidence actor is valid
3. Check collection capacity not exceeded
4. Ensure evidence ID is unique

### How do I save evidence collection?
Enable auto-save:
```cpp
EvidenceComponent->bAutoSave = true;
```

Or manually:
```cpp
EvidenceComponent->SaveToArchive();
```

### Archive viewer not showing evidence
**Solution**:
1. Verify evidence is actually collected
2. Check archive subsystem is initialized
3. Ensure widget is bound to correct component

### How do I create custom evidence types?
1. Add new category to `EEvidenceCategory` enum
2. Update `FEvidenceData` struct if needed
3. Create Blueprint with custom logic

See [Evidence Types](../API/Evidence/EvidenceTypes.md).

---

## AI System

### How do I create threat AI?
1. Create Blueprint based on `AHorrorThreatCharacter`
2. Set up AI controller (`AHorrorThreatAIController`)
3. Create behavior tree
4. Configure perception system

See [AI Tutorial](../Tutorials/AISystem.md).

### AI not detecting player
**Solution**:
1. Check AI perception component is configured
2. Verify sight/hearing settings
3. Ensure player has stimulus source
4. Check line of sight not blocked

### AI stuck in place
**Solution**:
1. Verify nav mesh exists and is built
2. Check AI controller is assigned
3. Ensure behavior tree is running
4. Check for movement component issues

### How do I adjust AI difficulty?
Modify perception settings:
```cpp
SightConfig->SightRadius = 2000.0f;  // Detection range
SightConfig->LoseSightRadius = 2500.0f;
SightConfig->PeripheralVisionAngleDegrees = 90.0f;
```

### AI performance issues
**Solution**:
- Reduce perception update frequency
- Limit active AI count
- Use LOD for distant AI
- Optimize behavior tree complexity

See [Performance Optimization](../Performance/OptimizationGuide.md).

---

## Blueprint Questions

### How do I access C++ classes in Blueprint?
C++ classes marked with `UCLASS(Blueprintable)` can be:
1. Extended in Blueprint (Create child Blueprint)
2. Used as variables
3. Called via Blueprint nodes

### Blueprint compilation failed
**Solution**:
1. Check for circular dependencies
2. Verify all referenced assets exist
3. Clear Blueprint cache: Delete `Saved/` folder
4. Reparent Blueprint if parent changed

See [Runtime Errors](RuntimeErrors.md#blueprint-errors).

### How do I call C++ functions from Blueprint?
Mark functions with `UFUNCTION(BlueprintCallable)`:
```cpp
UFUNCTION(BlueprintCallable, Category = "MyCategory")
void MyFunction();
```

### Blueprint variables not showing
**Solution**:
1. Ensure marked with `UPROPERTY(BlueprintReadWrite)`
2. Check category is set
3. Verify class is compiled
4. Refresh Blueprint editor

### How do I optimize Blueprints?
- Minimize tick events
- Use event-driven logic
- Cache component references
- Avoid complex math in tick
- Use Blueprint nativization for shipping

See [Blueprint Best Practices](../BestPractices/BlueprintDesign.md).

---

## Performance Questions

### Low frame rate in editor
**Solution**:
1. Disable real-time rendering when not needed
2. Reduce viewport quality settings
3. Close unnecessary editor windows
4. Use stat commands to identify bottleneck

### Game stutters during play
**Solution**:
1. Check for synchronous asset loading
2. Reduce garbage collection hitches
3. Optimize tick functions
4. Use async loading for large assets

See [Performance Issues](PerformanceIssues.md#stuttering).

### High memory usage
**Solution**:
1. Check for memory leaks
2. Reduce texture sizes
3. Use texture streaming
4. Limit loaded assets
5. Profile with Memory Insights

See [Performance Issues](PerformanceIssues.md#memory-usage).

### Long load times
**Solution**:
1. Use async loading
2. Reduce level size
3. Optimize asset references
4. Enable pak file compression
5. Use level streaming

### How do I profile performance?
Use built-in tools:
- `stat fps` - Frame rate
- `stat unit` - Frame time breakdown
- `stat memory` - Memory usage
- Unreal Insights - Detailed profiling

See [Profiling Guide](../Performance/ProfilingGuide.md).

---

## Packaging & Deployment

### Package game failed
**Solution**:
1. Check all content is saved
2. Verify no compilation errors
3. Check packaging settings
4. Review packaging log for errors

See [Runtime Errors](RuntimeErrors.md#packaging-errors).

### Packaged game crashes on startup
**Solution**:
1. Test in Standalone mode first
2. Check for editor-only code
3. Verify all assets are included
4. Check platform-specific issues

### How do I reduce package size?
1. Remove unused assets
2. Compress textures
3. Use pak file compression
4. Exclude editor content
5. Use shared material instances

### Shipping build optimization
1. Enable Blueprint nativization
2. Use shipping configuration
3. Strip debug symbols
4. Enable code optimization
5. Compress pak files

---

## Development Workflow

### How do I debug C++ code?
1. Set breakpoints in Visual Studio
2. Press F5 to launch with debugger
3. Trigger breakpoint in game
4. Inspect variables and call stack

See [Debugging Tutorial](../Tutorials/DebuggingTechniques.md).

### How do I add new modules?
1. Create module folder in `Source/`
2. Add `.Build.cs` file
3. Update `.uproject` file
4. Regenerate project files

### Version control best practices
- Commit often with clear messages
- Use `.gitignore` for generated files
- Don't commit `Binaries/`, `Intermediate/`, `Saved/`
- Use Git LFS for large assets

### How do I contribute?
See [Contributing Guide](../Developer/ContributingGuide.md).

---

## Still Need Help?

### Documentation
- [API Reference](../API/README.md)
- [Tutorials](../Tutorials/README.md)
- [Best Practices](../BestPractices/README.md)

### Community
- [Discord Server](https://discord.gg/horrorproject)
- [GitHub Issues](https://github.com/your-org/HorrorProject/issues)
- [GitHub Discussions](https://github.com/your-org/HorrorProject/discussions)

### Reporting Bugs
Include:
- Unreal Engine version
- Steps to reproduce
- Error messages
- Log files
- Screenshots/videos

[Report a Bug](https://github.com/your-org/HorrorProject/issues/new?template=bug_report.md)
