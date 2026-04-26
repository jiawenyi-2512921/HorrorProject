# ImproveNaming.ps1 - Naming Improvement Helper
# Generates rename commands and suggestions for naming convention violations

param(
    [string]$SourcePath = "Source",
    [string]$OutputFile = "Docs/Quality/Improvement/NamingImprovementPlan.md"
)

$RenameCommands = @()

function Generate-RenameCommand {
    param($File, $Type, $OldName, $NewName, $Severity)

    return [PSCustomObject]@{
        File = $File
        Type = $Type
        OldName = $OldName
        NewName = $NewName
        Severity = $Severity
        VSCommand = "Rename '$OldName' to '$NewName'"
        SearchReplace = "Find: \b$OldName\b, Replace: $NewName"
    }
}

# Main execution
Write-Host "Generating Naming Improvement Plan..." -ForegroundColor Cyan

$report = @"
# Naming Improvement Plan
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## Overview
This document provides systematic guidance for improving naming conventions across the codebase.

## Unreal Engine Naming Quick Reference

| Element | Convention | Example |
|---------|------------|---------|
| UObject Class | U prefix + PascalCase | UMyComponent |
| Actor Class | A prefix + PascalCase | AMyCharacter |
| Struct | F prefix + PascalCase | FMyStruct |
| Interface | I prefix + PascalCase | IMyInterface |
| Enum | E prefix + PascalCase | EMyEnum |
| Boolean | b prefix + PascalCase | bIsActive |
| Function | PascalCase + Verb | GetPlayerHealth |
| Variable | PascalCase | PlayerName |
| Constant | UPPER_SNAKE_CASE | MAX_PLAYERS |
| Template | T prefix + PascalCase | TArray |

## Automated Renaming Process

### Using Visual Studio

#### Method 1: Rename Symbol (Recommended)
1. Place cursor on symbol
2. Press **Ctrl+R, R** (or right-click → Rename)
3. Enter new name
4. Press **Enter** or click **Apply**
5. Review changes in preview window
6. Click **Apply** to confirm

#### Method 2: Find and Replace
1. Press **Ctrl+H** (Find and Replace)
2. Enter old name in "Find what"
3. Enter new name in "Replace with"
4. Check "Match whole word"
5. Check "Match case"
6. Click "Replace All" or review each occurrence

### Using ReSharper C++

1. Place cursor on symbol
2. Press **Ctrl+R, R**
3. Enter new name
4. ReSharper automatically:
   - Updates all references
   - Updates header and source files
   - Updates comments (optional)
5. Review and apply changes

### Using Command Line (PowerShell)

```powershell
# Rename across all files
Get-ChildItem -Path Source -Recurse -Include *.cpp,*.h | ForEach-Object {
    (Get-Content $_.FullName) -replace '\bOldName\b', 'NewName' |
    Set-Content $_.FullName
}
```

## Common Naming Fixes

### Fix 1: Add Class Prefix

**Issue**: Class missing U/A/F/I prefix

```cpp
// Before
class MyComponent : public UActorComponent {
    // ...
};

// After
class UMyComponent : public UActorComponent {
    // ...
};
```

**Rename Steps**:
1. Rename class in header file
2. Update constructor/destructor names
3. Update all references in source files
4. Update forward declarations
5. Rebuild project

### Fix 2: Boolean Prefix

**Issue**: Boolean variable without 'b' prefix

```cpp
// Before
UPROPERTY()
bool IsActive;

// After
UPROPERTY()
bool bIsActive;
```

**Rename Steps**:
1. Use Visual Studio Rename (Ctrl+R, R)
2. Verify UPROPERTY references updated
3. Check Blueprint compatibility
4. Test in editor

### Fix 3: PascalCase Functions

**Issue**: Function using camelCase or snake_case

```cpp
// Before
void updatePlayerState() { }
void update_player_state() { }

// After
void UpdatePlayerState() { }
```

**Rename Steps**:
1. Rename function declaration in header
2. Rename function definition in source
3. Update all call sites
4. Update any UFUNCTION macros

### Fix 4: Remove Hungarian Notation

**Issue**: Using type prefixes (except 'b' for bool)

```cpp
// Before
int32 iPlayerCount;
float fDamageAmount;
FString strPlayerName;

// After
int32 PlayerCount;
float DamageAmount;
FString PlayerName;
```

### Fix 5: Enum Prefix

**Issue**: Enum without 'E' prefix

```cpp
// Before
enum class GameState {
    Menu,
    Playing,
    Paused
};

// After
enum class EGameState {
    Menu,
    Playing,
    Paused
};
```

## Batch Renaming Script

Save this as `BatchRename.ps1`:

```powershell
param(
    [string]$SourcePath = "Source",
    [hashtable]$Renames = @{}
)

function Rename-InFiles {
    param($OldName, $NewName)

    $files = Get-ChildItem -Path $SourcePath -Recurse -Include *.cpp,*.h
    $count = 0

    foreach ($file in $files) {
        $content = Get-Content $file.FullName -Raw
        $newContent = $content -replace "\b$OldName\b", $NewName

        if ($content -ne $newContent) {
            Set-Content $file.FullName -Value $newContent -NoNewline
            $count++
            Write-Host "Updated: $($file.Name)" -ForegroundColor Green
        }
    }

    Write-Host "Renamed '$OldName' to '$NewName' in $count files" -ForegroundColor Cyan
}

# Execute renames
foreach ($rename in $Renames.GetEnumerator()) {
    Rename-InFiles $rename.Key $rename.Value
}
```

**Usage**:
```powershell
.\BatchRename.ps1 -Renames @{
    "OldClassName" = "UNewClassName"
    "oldFunction" = "NewFunction"
    "IsActive" = "bIsActive"
}
```

## Naming Improvement Checklist

### Pre-Rename Checklist
- [ ] Backup code or commit to version control
- [ ] Identify all occurrences of the name
- [ ] Check for Blueprint references
- [ ] Note any serialized data dependencies
- [ ] Plan rename order (dependencies first)

### During Rename
- [ ] Use IDE refactoring tools when possible
- [ ] Update header files first
- [ ] Update source files second
- [ ] Update comments and documentation
- [ ] Update any related Blueprint assets

### Post-Rename Checklist
- [ ] Full project rebuild (Ctrl+Shift+B)
- [ ] Fix any compilation errors
- [ ] Run all unit tests
- [ ] Test in editor
- [ ] Verify Blueprint connections
- [ ] Check serialized data loads correctly
- [ ] Commit changes with descriptive message

## Handling Special Cases

### Serialized Properties

When renaming UPROPERTY variables that are serialized:

```cpp
// Use UPROPERTY meta to maintain compatibility
UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Use bNewName instead"))
bool OldName;

UPROPERTY()
bool bNewName;

// In PostLoad or constructor
void UMyClass::PostLoad() {
    Super::PostLoad();
    if (OldName) {
        bNewName = OldName;
        OldName = false;
    }
}
```

### Blueprint-Exposed Functions

When renaming UFUNCTION:

```cpp
// Add DisplayName to maintain Blueprint compatibility
UFUNCTION(BlueprintCallable, meta = (DisplayName = "Old Function Name"))
void NewFunctionName();
```

### Reflected Types

For classes, structs, enums used in reflection:

1. Rename in C++
2. Update .uasset files if needed
3. Use redirectors for backward compatibility
4. Test thoroughly in editor

## Git Workflow for Renaming

### Small Renames (Single Symbol)
```bash
git checkout -b refactor/rename-symbol-name
# Perform rename
git add -A
git commit -m "Refactor: Rename Symbol to NewSymbol for clarity"
git push origin refactor/rename-symbol-name
# Create PR
```

### Large Renames (Multiple Symbols)
```bash
git checkout -b refactor/naming-improvements
# Rename one symbol at a time
git add -A
git commit -m "Refactor: Rename FirstSymbol to NewFirstSymbol"
# Rename next symbol
git add -A
git commit -m "Refactor: Rename SecondSymbol to NewSecondSymbol"
# Continue...
git push origin refactor/naming-improvements
# Create PR
```

## Commit Message Templates

### For Class Renames
```
Refactor: Rename MyComponent to UMyComponent

- Add U prefix to follow Unreal naming convention
- Update all references in source files
- Update forward declarations
- No functional changes
```

### For Function Renames
```
Refactor: Rename updateState to UpdateState

- Convert to PascalCase per coding standard
- Update all call sites
- Update unit tests
- No functional changes
```

### For Variable Renames
```
Refactor: Rename IsActive to bIsActive

- Add b prefix for boolean variable
- Update all references
- Update UPROPERTY declarations
- No functional changes
```

## Naming Patterns Library

### Common Verb Prefixes

| Verb | Usage | Example |
|------|-------|---------|
| Get | Retrieve value | GetPlayerHealth() |
| Set | Assign value | SetPlayerHealth(int32) |
| Is | Boolean query | IsAlive() |
| Has | Possession check | HasWeapon() |
| Can | Capability check | CanJump() |
| Should | Conditional check | ShouldRespawn() |
| Initialize | Setup | InitializeComponents() |
| Update | Modify state | UpdatePosition() |
| Process | Handle data | ProcessInput() |
| Handle | Event response | HandleDamage() |
| On | Event callback | OnPlayerDeath() |
| Begin | Start action | BeginPlay() |
| End | Finish action | EndPlay() |

### Common Noun Patterns

| Pattern | Usage | Example |
|---------|-------|---------|
| Manager | Coordinates | UInventoryManager |
| Controller | Controls | APlayerController |
| Component | Modular part | UHealthComponent |
| System | Subsystem | UQuestSystem |
| Handler | Processes events | UInputHandler |
| Provider | Supplies data | UDataProvider |
| Factory | Creates objects | UActorFactory |
| Builder | Constructs | ULevelBuilder |
| Validator | Checks validity | UDataValidator |
| Processor | Transforms | UImageProcessor |

### Unreal-Specific Patterns

| Pattern | Usage | Example |
|---------|-------|---------|
| Subsystem | Game subsystem | UMyGameSubsystem |
| GameMode | Game rules | AMyGameMode |
| GameState | Game state | AMyGameState |
| PlayerState | Player data | AMyPlayerState |
| HUD | UI overlay | AMyHUD |
| Widget | UI element | UMyWidget |
| AnimInstance | Animation | UMyAnimInstance |
| AnimNotify | Anim event | UMyAnimNotify |

## Quality Metrics

Track naming convention compliance:

| Metric | Target | Measurement |
|--------|--------|-------------|
| Class Prefix Compliance | 100% | Classes with correct U/A/F/I prefix |
| Boolean Prefix Compliance | 100% | Booleans with 'b' prefix |
| Function PascalCase | 100% | Functions in PascalCase |
| Variable PascalCase | 95% | Variables in PascalCase |
| No Hungarian Notation | 100% | Except 'b' for bool |

## Training Resources

### Team Training
1. **Naming Convention Workshop** (1 hour)
   - Review Unreal conventions
   - Practice renaming exercises
   - Q&A session

2. **Tool Training** (30 minutes)
   - Visual Studio rename features
   - ReSharper C++ capabilities
   - Batch renaming scripts

3. **Code Review Focus** (ongoing)
   - Check naming in every review
   - Provide constructive feedback
   - Share good examples

### Documentation
- Maintain naming convention guide
- Update with team decisions
- Include examples from codebase

## Continuous Improvement

### Weekly
- Run NamingConventionChecker.ps1
- Review new violations
- Address high-severity issues

### Monthly
- Review naming patterns
- Update conventions if needed
- Team retrospective on naming

### Quarterly
- Full naming audit
- Refactoring sprint
- Update training materials

## Success Stories

Document successful renames to motivate team:

```markdown
### Example: Health Component Rename
**Before**: HealthComp (unclear, abbreviated)
**After**: UHealthComponent (clear, follows convention)
**Impact**:
- Improved code searchability
- Better IntelliSense suggestions
- Clearer Blueprint integration
**Effort**: 2 hours
**Files Changed**: 15
```

## Common Pitfalls

### Pitfall 1: Incomplete Renames
**Problem**: Missing some references
**Solution**: Use IDE refactoring tools, not manual find/replace

### Pitfall 2: Breaking Serialization
**Problem**: Renamed serialized properties lose data
**Solution**: Use property redirectors or migration code

### Pitfall 3: Blueprint Breakage
**Problem**: Blueprint references break after rename
**Solution**: Use DisplayName meta or update Blueprints

### Pitfall 4: Merge Conflicts
**Problem**: Large renames cause merge conflicts
**Solution**: Coordinate with team, rename in small batches

### Pitfall 5: Inconsistent Naming
**Problem**: Different naming styles in same file
**Solution**: Establish and enforce team conventions

## Next Steps

1. **Review this plan** with development team
2. **Prioritize renames** based on severity
3. **Schedule refactoring time** (2-3 hours/week)
4. **Use automated tools** where possible
5. **Track progress** using metrics
6. **Update guidelines** based on learnings

---
*Generated by ImproveNaming.ps1*
"@

$outputDir = Split-Path $OutputFile -Parent
if (!(Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

$report | Out-File $OutputFile -Encoding UTF8

Write-Host "`nNaming Improvement Plan Generated!" -ForegroundColor Green
Write-Host "Report saved to: $OutputFile" -ForegroundColor Cyan
