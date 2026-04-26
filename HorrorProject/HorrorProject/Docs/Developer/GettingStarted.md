# Getting Started

## Prerequisites

### Required Software

**Unreal Engine 5.6**
- Download from Epic Games Launcher
- Install to: `C:\Program Files\Epic Games\UE_5.6`

**Visual Studio 2022**
- Install location: `C:\Program Files\Microsoft Visual Studio\2022\Community`
- Required workloads:
  - Game development with C++
  - .NET desktop development

**Git**
- Version 2.40 or later
- Configure user name and email

### System Requirements

**Minimum:**
- Windows 10 64-bit
- 16 GB RAM
- 50 GB free disk space
- DirectX 12 compatible GPU

**Recommended:**
- Windows 11 64-bit
- 32 GB RAM
- 100 GB free SSD space
- NVIDIA RTX 3060 or better

## Project Setup

### 1. Clone Repository

```bash
git clone <repository-url>
cd HorrorProject
```

### 2. Generate Project Files

Right-click `HorrorProject.uproject` and select:
- "Generate Visual Studio project files"

Or use command line:
```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="D:\gptzuo\HorrorProject\HorrorProject\HorrorProject.uproject" -game -engine
```

### 3. Open in Visual Studio

Open `HorrorProject.sln` in Visual Studio 2022.

**Build Configuration:**
- Development Editor (for development)
- Debug Editor (for debugging)
- Shipping (for release builds)

### 4. Build Project

In Visual Studio:
1. Set startup project to "HorrorProject"
2. Select "Development Editor" configuration
3. Build → Build Solution (Ctrl+Shift+B)

### 5. Launch Editor

**From Visual Studio:**
- Debug → Start Debugging (F5)

**From Unreal:**
- Double-click `HorrorProject.uproject`

## Project Structure

```
HorrorProject/
├── Binaries/           # Compiled binaries
├── Build/              # Build configuration
├── Config/             # Game configuration files
├── Content/            # Game assets (blueprints, materials, etc.)
├── Docs/               # Documentation
├── Intermediate/       # Temporary build files
├── Plugins/            # Third-party plugins
├── Saved/              # Saved data, logs, screenshots
├── Scripts/            # Automation scripts
└── Source/             # C++ source code
    └── HorrorProject/
        ├── AI/         # AI systems
        ├── Audio/      # Audio systems
        ├── Evidence/   # Evidence collection
        ├── Game/       # Core game logic
        ├── Interaction/# Interaction system
        ├── Performance/# Performance monitoring
        ├── Player/     # Player systems
        ├── Save/       # Save system
        ├── Tests/      # Unit tests
        ├── UI/         # User interface
        ├── VFX/        # Visual effects
        └── Variant_Horror/ # Horror-specific variants
```

## First Steps

### 1. Explore the Main Level

Open `Content/Maps/MainLevel.umap` in the editor.

**Key Actors:**
- Player Start
- Encounter Directors
- Audio Zones
- Evidence Actors
- Lighting

### 2. Run the Game

Click "Play" (Alt+P) in the editor toolbar.

**Controls:**
- WASD - Move
- Mouse - Look
- E - Interact
- Shift - Sprint
- Tab - Open Archive
- Esc - Pause

### 3. Examine Core Systems

**Event Bus:**
- Open `Source/HorrorProject/Game/HorrorEventBusSubsystem.h`
- Review event publishing and subscription

**Encounter Director:**
- Open `Source/HorrorProject/Game/HorrorEncounterDirector.h`
- Understand encounter state machine

**Player Character:**
- Open `Source/HorrorProject/HorrorProjectCharacter.h`
- Review player capabilities

### 4. Create Your First Feature

**Example: Add a new interactable object**

1. Create new C++ class derived from `AActor`
2. Implement `IHorrorInteractable` interface
3. Override `Interact_Implementation()`
4. Publish event to Event Bus
5. Test in editor

```cpp
// MyInteractable.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/HorrorInteractable.h"
#include "MyInteractable.generated.h"

UCLASS()
class HORRORPROJECT_API AMyInteractable : public AActor, public IHorrorInteractable
{
    GENERATED_BODY()

public:
    virtual void Interact_Implementation(AActor* Interactor) override;
};

// MyInteractable.cpp
#include "MyInteractable.h"
#include "Game/HorrorEventBusSubsystem.h"

void AMyInteractable::Interact_Implementation(AActor* Interactor)
{
    if (UHorrorEventBusSubsystem* EventBus = GetWorld()->GetSubsystem<UHorrorEventBusSubsystem>())
    {
        EventBus->Publish(
            FGameplayTag::RequestGameplayTag("Event.Interaction.Custom"),
            TEXT("MyInteractable"),
            FGameplayTag::RequestGameplayTag("State.Active"),
            this
        );
    }
}
```

## Development Workflow

### Daily Workflow

1. **Pull latest changes**
   ```bash
   git pull origin main
   ```

2. **Build project**
   - Visual Studio: Ctrl+Shift+B

3. **Run tests**
   ```bash
   Scripts\RunTests.ps1
   ```

4. **Make changes**
   - Write code
   - Test in editor
   - Write unit tests

5. **Commit changes**
   ```bash
   git add .
   git commit -m "Description"
   git push
   ```

### Hot Reload

Unreal supports hot reload for C++ changes:
1. Make code changes in Visual Studio
2. Save files
3. In Unreal Editor: Tools → Compile
4. Changes applied without restarting editor

**Limitations:**
- Cannot add new UPROPERTY
- Cannot change class hierarchy
- Cannot add new UFUNCTION signatures

For major changes, restart the editor.

### Live Coding

Enable Live Coding for faster iteration:
1. Editor Preferences → General → Live Coding
2. Check "Enable Live Coding"
3. Use Ctrl+Alt+F11 to compile

## Debugging

### Visual Studio Debugging

1. Set breakpoints in code (F9)
2. Start debugging (F5)
3. Trigger code path in game
4. Inspect variables, call stack

**Useful Watch Expressions:**
- `GEngine->GetWorld()`
- `GetWorld()->GetSubsystem<UHorrorEventBusSubsystem>()`
- `Cast<AHorrorProjectCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn())`

### Unreal Logging

```cpp
UE_LOG(LogHorrorProject, Log, TEXT("Message: %s"), *StringValue);
UE_LOG(LogHorrorProject, Warning, TEXT("Warning!"));
UE_LOG(LogHorrorProject, Error, TEXT("Error!"));
```

View logs:
- Window → Developer Tools → Output Log
- Or: `Saved/Logs/HorrorProject.log`

### Console Commands

Open console with ` (backtick) key:

```
stat fps              # Show FPS
stat unit             # Show frame time breakdown
stat memory           # Show memory usage
showdebug             # Show debug info
HorrorEventBus.ShowHistory  # Show event history
```

## Testing

### Run Unit Tests

**From Editor:**
- Window → Test Automation
- Select tests to run
- Click "Start Tests"

**From Command Line:**
```powershell
Scripts\RunTests.ps1
```

### Write Unit Tests

```cpp
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyTest, "HorrorProject.MyModule.MyTest", 
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMyTest::RunTest(const FString& Parameters)
{
    // Test code here
    TestTrue("Condition", bCondition);
    return true;
}
```

## Common Issues

### Build Errors

**"Cannot open include file"**
- Solution: Add module to Build.cs dependencies

**"Unresolved external symbol"**
- Solution: Check module dependencies, rebuild

**"Hot reload failed"**
- Solution: Restart editor for major changes

### Editor Crashes

**Check logs:**
- `Saved/Logs/HorrorProject.log`
- Look for crash callstack

**Common causes:**
- Null pointer dereference
- Invalid object reference
- Infinite loop
- Stack overflow

### Performance Issues

**Profile with Unreal Insights:**
1. Launch with `-trace=cpu,frame,log`
2. Open Unreal Insights
3. Analyze frame data

**Check:**
- Draw calls (stat rhi)
- Tick time (stat game)
- Render time (stat gpu)

## Next Steps

1. Read [Coding Standards](CodingStandards.md)
2. Review [Build Guide](BuildGuide.md)
3. Study [Testing Guide](TestingGuide.md)
4. Explore [System Architecture](../Architecture/SystemOverview.md)
5. Join team communication channels

## Resources

**Documentation:**
- [Unreal Engine Documentation](https://docs.unrealengine.com/)
- [C++ API Reference](https://docs.unrealengine.com/en-US/API/)
- Project Wiki (internal)

**Community:**
- Unreal Slackers Discord
- Unreal Engine Forums
- Stack Overflow

**Tools:**
- [RenderDoc](https://renderdoc.org/) - Graphics debugging
- [Visual Assist](https://www.wholetomato.com/) - C++ productivity
- [Unreal Insights](https://docs.unrealengine.com/en-US/TestingAndOptimization/PerformanceAndProfiling/UnrealInsights/) - Profiling

---
Last Updated: 2026-04-26
