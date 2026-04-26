# Compilation Errors

Solutions for build and compilation errors in HorrorProject.

## Missing Includes

### Error: Cannot open include file

```
fatal error C1083: Cannot open include file: 'HorrorAudioSubsystem.h': No such file or directory
```

**Causes**:
- Incorrect include path
- Missing module dependency
- File not in correct location

**Solutions**:

1. **Check include path**:
```cpp
// Wrong
#include "HorrorAudioSubsystem.h"

// Correct
#include "Audio/HorrorAudioSubsystem.h"
```

2. **Add module dependency** in `HorrorProject.Build.cs`:
```csharp
PublicDependencyModuleNames.AddRange(new string[] 
{ 
    "Core", 
    "CoreUObject", 
    "Engine",
    "AudioMixer"  // Add missing module
});
```

3. **Regenerate project files**:
```bash
# Right-click HorrorProject.uproject
# Select "Generate Visual Studio project files"
```

4. **Clean and rebuild**:
```bash
# In Visual Studio
Build → Clean Solution
Build → Rebuild Solution
```

---

## Linker Errors

### Error: Unresolved external symbol

```
error LNK2019: unresolved external symbol "public: void __cdecl UHorrorAudioSubsystem::PlayAmbientSound(class USoundBase *,float)"
```

**Causes**:
- Function declared but not defined
- Missing HORRORPROJECT_API macro
- Incorrect module linkage

**Solutions**:

1. **Implement the function**:
```cpp
// Header
UFUNCTION(BlueprintCallable)
void PlayAmbientSound(USoundBase* Sound, float FadeInTime);

// CPP - Add implementation
void UHorrorAudioSubsystem::PlayAmbientSound(USoundBase* Sound, float FadeInTime)
{
    // Implementation here
}
```

2. **Add API macro**:
```cpp
// Wrong
class UHorrorAudioSubsystem : public UWorldSubsystem

// Correct
class HORRORPROJECT_API UHorrorAudioSubsystem : public UWorldSubsystem
```

3. **Check module dependencies**:
```csharp
// In .Build.cs
PrivateDependencyModuleNames.AddRange(new string[] 
{
    "AudioMixer",
    "AudioExtensions"
});
```

---

## Generated Code Errors

### Error: Syntax error in generated.h

```
error C2059: syntax error: '}'
```

**Causes**:
- Missing GENERATED_BODY() macro
- Incorrect UCLASS/USTRUCT syntax
- Malformed UPROPERTY/UFUNCTION

**Solutions**:

1. **Add GENERATED_BODY()**:
```cpp
UCLASS()
class HORRORPROJECT_API UMyClass : public UObject
{
    GENERATED_BODY()  // Required!
    
public:
    // Class content
};
```

2. **Fix UCLASS syntax**:
```cpp
// Wrong
UCLASS(BlueprintType, Blueprintable)

// Correct
UCLASS(BlueprintType, Blueprintable)
class HORRORPROJECT_API UMyClass : public UObject
{
    GENERATED_BODY()
};
```

3. **Fix UPROPERTY syntax**:
```cpp
// Wrong
UPROPERTY(EditAnywhere BlueprintReadWrite)

// Correct
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
int32 MyProperty;
```

4. **Regenerate code**:
- Save all files
- Close Visual Studio
- Delete `Intermediate/` folder
- Regenerate project files
- Reopen and rebuild

---

## Memory Errors

### Error: Out of memory during compilation

```
fatal error C1060: compiler is out of heap space
```

**Solutions**:

1. **Increase Visual Studio memory**:
```xml
<!-- In .vcxproj file -->
<PropertyGroup>
    <PreferredToolArchitecture>x64</PreferredToolArchitecture>
</PropertyGroup>
```

2. **Reduce parallel builds**:
- Tools → Options → Projects and Solutions → Build and Run
- Set "maximum number of parallel project builds" to 1 or 2

3. **Close other applications**:
- Close browser, Discord, etc.
- Free up system RAM

4. **Use incremental builds**:
- Build → Build Solution (not Rebuild)

---

## Module Errors

### Error: Module not found

```
ERROR: Unable to instantiate module 'HorrorProject': Unable to instantiate UnrealBuildTool.RulesCompiler+RulesException
```

**Solutions**:

1. **Check .Build.cs file**:
```csharp
public class HorrorProject : ModuleRules
{
    public HorrorProject(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core", 
            "CoreUObject", 
            "Engine" 
        });
    }
}
```

2. **Verify module in .uproject**:
```json
{
    "Modules": [
        {
            "Name": "HorrorProject",
            "Type": "Runtime",
            "LoadingPhase": "Default"
        }
    ]
}
```

3. **Check file locations**:
```
Source/
  HorrorProject/
    HorrorProject.Build.cs
    HorrorProject.h
    HorrorProject.cpp
```

---

## PCH Errors

### Error: Precompiled header issues

```
fatal error C1853: precompiled header file is from a previous version of the compiler
```

**Solutions**:

1. **Clean intermediate files**:
```bash
rm -rf Intermediate/ Binaries/
```

2. **Rebuild solution**:
```bash
Build → Rebuild Solution
```

3. **Check PCH settings** in .Build.cs:
```csharp
PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
```

---

## Template Errors

### Error: Template instantiation issues

```
error C2440: cannot convert from 'UObject *' to 'UHorrorAudioSubsystem *'
```

**Solutions**:

1. **Use correct cast**:
```cpp
// Wrong
UHorrorAudioSubsystem* AudioSys = GetWorld()->GetSubsystem();

// Correct
UHorrorAudioSubsystem* AudioSys = GetWorld()->GetSubsystem<UHorrorAudioSubsystem>();
```

2. **Check template parameters**:
```cpp
// Correct template usage
TArray<FEvidenceData> Evidence;
TSubclassOf<AEvidenceActor> EvidenceClass;
```

---

## Macro Errors

### Error: UFUNCTION/UPROPERTY errors

```
error: 'BlueprintCallable' is not a valid specifier
```

**Solutions**:

1. **Check macro syntax**:
```cpp
// Wrong
UFUNCTION(BlueprintCallable Category = "Audio")

// Correct
UFUNCTION(BlueprintCallable, Category = "Audio")
```

2. **Verify function signature**:
```cpp
// BlueprintCallable functions must be public
public:
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySound();
```

3. **Check property specifiers**:
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
int32 MaxCapacity;
```

---

## Common Build Warnings

### Warning: Deprecated function

```
warning C4996: 'UGameplayStatics::PlaySound2D': Use PlaySound2D with new signature
```

**Solution**: Update to new API:
```cpp
// Old
UGameplayStatics::PlaySound2D(GetWorld(), Sound);

// New
UGameplayStatics::PlaySound2D(GetWorld(), Sound, 1.0f, 1.0f, 0.0f);
```

---

## Build Configuration Issues

### Wrong build configuration

**Solution**: Set correct configuration:
1. Configuration: **Development Editor**
2. Platform: **Win64**

![Build Configuration](../_images/troubleshooting/build_config.png)

---

## Advanced Troubleshooting

### Nuclear Option: Complete Clean

```bash
# 1. Close Visual Studio and Unreal Editor
# 2. Delete generated folders
rm -rf Binaries/ Intermediate/ Saved/ .vs/

# 3. Delete Visual Studio files
rm *.sln *.suo *.sdf

# 4. Regenerate project files
# Right-click .uproject → Generate Visual Studio project files

# 5. Open solution and rebuild
```

### Verify Engine Installation

```bash
# Epic Games Launcher
# Library → Unreal Engine 5.6 → Verify
```

---

## Getting Help

If issues persist:
1. Check [FAQ](FAQ.md)
2. Search [GitHub Issues](https://github.com/your-org/HorrorProject/issues)
3. Ask on [Discord](https://discord.gg/horrorproject)

When reporting:
- Include full error message
- Specify Unreal Engine version
- Specify Visual Studio version
- Include relevant code snippets
- Attach build log if possible

## Related Documentation
- [Getting Started](../Tutorials/GettingStarted.md)
- [Runtime Errors](RuntimeErrors.md)
- [Best Practices](../BestPractices/CppCoding.md)
