# Build Guide

## Build Configurations

### Editor Configurations

**Development Editor**
- Optimized for development
- Includes debug symbols
- Assertions enabled
- Logging enabled
- **Use for:** Daily development

**Debug Editor**
- No optimization
- Full debug symbols
- All checks enabled
- Verbose logging
- **Use for:** Debugging crashes

**DebugGame Editor**
- Game code not optimized
- Engine code optimized
- Debug symbols for game code
- **Use for:** Debugging game logic with good performance

### Shipping Configurations

**Shipping**
- Full optimization
- No debug symbols
- Assertions disabled
- Minimal logging
- **Use for:** Final release builds

**Test**
- Optimized
- Some debug features
- Profiling enabled
- **Use for:** QA testing

## Building from Visual Studio

### Quick Build

1. Open `HorrorProject.sln`
2. Select configuration (Development Editor)
3. Build → Build Solution (Ctrl+Shift+B)

### Clean Build

1. Build → Clean Solution
2. Delete `Binaries/` and `Intermediate/` folders
3. Build → Rebuild Solution

### Build Output

**Success:**
```
Build succeeded.
    0 Warning(s)
    0 Error(s)
```

**Location:**
- Binaries: `Binaries/Win64/`
- Intermediate: `Intermediate/Build/Win64/`

## Building from Command Line

### Using UnrealBuildTool

```powershell
# Development Editor build
& "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" `
    HorrorProjectEditor Win64 Development `
    -Project="D:\gptzuo\HorrorProject\HorrorProject\HorrorProject.uproject"

# Shipping build
& "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" `
    HorrorProject Win64 Shipping `
    -Project="D:\gptzuo\HorrorProject\HorrorProject\HorrorProject.uproject"
```

### Using Build Script

```powershell
# Build editor
.\Scripts\BuildProject.ps1 -Configuration Development

# Build shipping
.\Scripts\BuildProject.ps1 -Configuration Shipping -Clean
```

## Packaging

### Package from Editor

1. File → Package Project → Windows → Windows (64-bit)
2. Select output directory
3. Wait for packaging to complete

### Package from Command Line

```powershell
& "C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\RunUAT.bat" `
    BuildCookRun `
    -project="D:\gptzuo\HorrorProject\HorrorProject\HorrorProject.uproject" `
    -platform=Win64 `
    -clientconfig=Shipping `
    -cook -stage -pak -archive `
    -archivedirectory="D:\gptzuo\HorrorProject\Packaged"
```

### Package Settings

**Project Settings → Packaging:**
- Build Configuration: Shipping
- Use Pak File: Enabled
- Compress Pak File: Enabled
- Include Prerequisites: Enabled
- Include Crash Reporter: Enabled

## Build Optimization

### Incremental Builds

UnrealBuildTool automatically performs incremental builds:
- Only changed files are recompiled
- Dependency tracking
- Precompiled headers

**Force full rebuild:**
```powershell
# Delete intermediate files
Remove-Item -Recurse -Force Intermediate\
Remove-Item -Recurse -Force Binaries\

# Rebuild
.\Scripts\BuildProject.ps1 -Clean
```

### Parallel Compilation

**Visual Studio:**
- Tools → Options → Projects and Solutions → Build and Run
- Maximum number of parallel project builds: 8

**UnrealBuildTool:**
- Automatically uses all CPU cores
- Configure in `BuildConfiguration.xml`

### Precompiled Headers

**Enabled by default:**
```csharp
// HorrorProject.Build.cs
PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
```

**Benefits:**
- Faster compilation
- Reduced memory usage
- Better incremental builds

## Build Troubleshooting

### Common Build Errors

**Error: Cannot open include file**
```
Solution: Add module to Build.cs
PublicDependencyModuleNames.Add("ModuleName");
```

**Error: Unresolved external symbol**
```
Solution: Check module dependencies, ensure proper linking
```

**Error: PCH out of date**
```
Solution: Clean and rebuild
Remove-Item -Recurse -Force Intermediate\
```

**Error: Access denied to file**
```
Solution: Close editor, kill UE processes
Get-Process | Where-Object {$_.Name -like "*Unreal*"} | Stop-Process
```

### Build Performance Issues

**Slow compilation:**
1. Check antivirus exclusions
2. Use SSD for project files
3. Increase virtual memory
4. Close unnecessary applications

**Out of memory:**
1. Reduce parallel builds
2. Increase system RAM
3. Close other applications
4. Use 64-bit compiler

### Clean Build Steps

```powershell
# 1. Close editor and Visual Studio

# 2. Delete generated files
Remove-Item -Recurse -Force Binaries\
Remove-Item -Recurse -Force Intermediate\
Remove-Item -Recurse -Force Saved\
Remove-Item -Force *.sln

# 3. Regenerate project files
Right-click HorrorProject.uproject → Generate Visual Studio project files

# 4. Open solution and rebuild
```

## Build Automation

### Continuous Integration

**Build Pipeline:**
```yaml
# Example CI configuration
build:
  steps:
    - checkout
    - generate-project-files
    - build-editor
    - run-tests
    - package-shipping
    - upload-artifacts
```

### Automated Testing

```powershell
# Build and test script
.\Scripts\BuildProject.ps1 -Configuration Development
.\Scripts\RunTests.ps1
if ($LASTEXITCODE -ne 0) {
    Write-Error "Tests failed"
    exit 1
}
```

### Nightly Builds

```powershell
# Scheduled task for nightly builds
$trigger = New-ScheduledTaskTrigger -Daily -At 2am
$action = New-ScheduledTaskAction -Execute "PowerShell.exe" `
    -Argument "-File D:\gptzuo\HorrorProject\HorrorProject\Scripts\BuildProject.ps1 -Configuration Shipping"
Register-ScheduledTask -TaskName "HorrorProject Nightly Build" -Trigger $trigger -Action $action
```

## Build Configuration Files

### BuildConfiguration.xml

Location: `%APPDATA%\Unreal Engine\UnrealBuildTool\BuildConfiguration.xml`

```xml
<?xml version="1.0" encoding="utf-8" ?>
<Configuration xmlns="https://www.unrealengine.com/BuildConfiguration">
    <BuildConfiguration>
        <MaxParallelActions>8</MaxParallelActions>
        <bUseUnityBuild>true</bUseUnityBuild>
    </BuildConfiguration>
</Configuration>
```

### HorrorProject.Build.cs

```csharp
public class HorrorProject : ModuleRules
{
    public HorrorProject(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "GameplayTags",
            "UMG",
            "Slate",
            "SlateCore"
        });
        
        PrivateDependencyModuleNames.AddRange(new string[] {
            "RenderCore",
            "RHI"
        });
        
        // Optimization settings
        if (Target.Configuration == UnrealTargetConfiguration.Shipping)
        {
            bUseUnity = true;
            MinFilesUsingPrecompiledHeaderOverride = 1;
        }
    }
}
```

## Platform-Specific Builds

### Windows

**Requirements:**
- Windows 10/11 SDK
- Visual Studio 2022
- DirectX 12

**Build command:**
```powershell
.\Scripts\BuildProject.ps1 -Platform Win64 -Configuration Shipping
```

### Future Platforms

**Console (PS5/Xbox):**
- Requires platform SDK
- Requires platform access
- Special build configurations

**Linux:**
- Cross-compilation from Windows
- Linux toolchain required

## Build Metrics

### Tracking Build Times

```powershell
# Measure build time
$startTime = Get-Date
.\Scripts\BuildProject.ps1
$endTime = Get-Date
$duration = $endTime - $startTime
Write-Host "Build completed in $($duration.TotalMinutes) minutes"
```

### Build Statistics

**Typical build times (Development Editor):**
- Clean build: 15-20 minutes
- Incremental build: 1-3 minutes
- Hot reload: 10-30 seconds

**Factors affecting build time:**
- CPU cores (more is better)
- Storage speed (SSD vs HDD)
- RAM amount (16GB minimum)
- Antivirus settings

## Build Best Practices

### Do's
- Use incremental builds when possible
- Keep intermediate files on SSD
- Exclude project from antivirus scanning
- Use precompiled headers
- Build in Development Editor for daily work

### Don'ts
- Don't commit Binaries/ or Intermediate/
- Don't build in Debug unless debugging
- Don't modify engine source without good reason
- Don't ignore build warnings
- Don't package in Development configuration

## Build Checklist

**Before committing:**
- [ ] Clean build succeeds
- [ ] No compiler warnings
- [ ] Unit tests pass
- [ ] Editor launches successfully
- [ ] No runtime errors in logs

**Before release:**
- [ ] Shipping build succeeds
- [ ] Package completes successfully
- [ ] Game runs on clean machine
- [ ] Performance targets met
- [ ] No critical bugs

---
Last Updated: 2026-04-26
