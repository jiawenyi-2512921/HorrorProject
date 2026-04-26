# NamingConventionChecker.ps1 - Naming Convention Validation
# Checks adherence to Unreal Engine naming conventions

param(
    [string]$SourcePath = "Source",
    [string]$OutputFile = "Docs/Quality/Improvement/NamingReport.md"
)

$Issues = @()

# Unreal Engine naming conventions
$Conventions = @{
    Class = @{
        Pattern = '^[UAFI][A-Z][a-zA-Z0-9]*$'
        Prefix = @('U', 'A', 'F', 'I')
        Description = 'Classes should start with U (UObject), A (AActor), F (struct), or I (interface)'
    }
    Function = @{
        Pattern = '^[A-Z][a-zA-Z0-9]*$'
        Description = 'Functions should be PascalCase'
    }
    Variable = @{
        Pattern = '^[A-Z][a-zA-Z0-9]*$|^b[A-Z][a-zA-Z0-9]*$'
        Description = 'Variables should be PascalCase, booleans prefixed with b'
    }
    Constant = @{
        Pattern = '^[A-Z][A-Z0-9_]*$'
        Description = 'Constants should be UPPER_SNAKE_CASE'
    }
    Parameter = @{
        Pattern = '^In[A-Z][a-zA-Z0-9]*$|^Out[A-Z][a-zA-Z0-9]*$|^[A-Z][a-zA-Z0-9]*$'
        Description = 'Parameters should be PascalCase, optionally prefixed with In/Out'
    }
}

function Check-ClassName {
    param($FileName, $Content)

    $classMatches = [regex]::Matches($Content, '(?m)^\s*class\s+(\w+)')

    foreach ($match in $classMatches) {
        $className = $match.Groups[1].Value

        # Check prefix
        $firstChar = $className[0]
        if ($firstChar -notin @('U', 'A', 'F', 'I', 'E', 'T')) {
            $script:Issues += [PSCustomObject]@{
                File = $FileName
                Type = "Class"
                Name = $className
                Issue = "Missing or incorrect prefix (should be U/A/F/I/E/T)"
                Severity = "High"
                Suggestion = "Rename to match Unreal convention (e.g., U$className for UObject-derived)"
                Line = ($Content.Substring(0, $match.Index) -split "`n").Count
            }
        }

        # Check PascalCase
        if ($className -notmatch '^[UAFIET][A-Z][a-zA-Z0-9]*$') {
            $script:Issues += [PSCustomObject]@{
                File = $FileName
                Type = "Class"
                Name = $className
                Issue = "Not in PascalCase format"
                Severity = "Medium"
                Suggestion = "Use PascalCase: $($className -replace '_', '')"
                Line = ($Content.Substring(0, $match.Index) -split "`n").Count
            }
        }

        # Check for abbreviations
        if ($className -match '[A-Z]{3,}') {
            $script:Issues += [PSCustomObject]@{
                File = $FileName
                Type = "Class"
                Name = $className
                Issue = "Contains consecutive uppercase letters (abbreviation)"
                Severity = "Low"
                Suggestion = "Avoid abbreviations or use PascalCase (e.g., HttpRequest not HTTPRequest)"
                Line = ($Content.Substring(0, $match.Index) -split "`n").Count
            }
        }
    }
}

function Check-FunctionNames {
    param($FileName, $Content)

    $functionMatches = [regex]::Matches($Content, '(?m)^\s*(?:virtual\s+)?(?:static\s+)?[\w\s\*&:<>,]+\s+(\w+)\s*\([^)]*\)')

    foreach ($match in $functionMatches) {
        $functionName = $match.Groups[1].Value

        # Skip constructors, destructors, operators
        if ($functionName -match '^~|^operator') {
            continue
        }

        # Check PascalCase
        if ($functionName -notmatch '^[A-Z][a-zA-Z0-9]*$') {
            $severity = "Medium"
            $suggestion = "Use PascalCase"

            # Check for snake_case
            if ($functionName -match '_') {
                $severity = "High"
                $suggestion = "Convert from snake_case to PascalCase: $($functionName -replace '_(\w)', { $_.Groups[1].Value.ToUpper() })"
            }

            # Check for camelCase
            if ($functionName -match '^[a-z]') {
                $severity = "Medium"
                $suggestion = "Capitalize first letter: $($functionName.Substring(0,1).ToUpper() + $functionName.Substring(1))"
            }

            $script:Issues += [PSCustomObject]@{
                File = $FileName
                Type = "Function"
                Name = $functionName
                Issue = "Not in PascalCase format"
                Severity = $severity
                Suggestion = $suggestion
                Line = ($Content.Substring(0, $match.Index) -split "`n").Count
            }
        }

        # Check for verb prefixes
        $verbs = @('Get', 'Set', 'Is', 'Has', 'Can', 'Should', 'Initialize', 'Update', 'Process', 'Handle', 'On', 'Begin', 'End')
        $hasVerb = $false
        foreach ($verb in $verbs) {
            if ($functionName -match "^$verb") {
                $hasVerb = $true
                break
            }
        }

        if (-not $hasVerb -and $functionName -notmatch '^[A-Z][a-z]+[A-Z]') {
            $script:Issues += [PSCustomObject]@{
                File = $FileName
                Type = "Function"
                Name = $functionName
                Issue = "Function name doesn't start with a verb"
                Severity = "Low"
                Suggestion = "Consider using verb prefix (Get, Set, Is, Has, etc.)"
                Line = ($Content.Substring(0, $match.Index) -split "`n").Count
            }
        }
    }
}

function Check-VariableNames {
    param($FileName, $Content)

    # Member variables
    $memberMatches = [regex]::Matches($Content, '(?m)^\s+(?:UPROPERTY[^)]*\)\s+)?(?:mutable\s+)?(?:const\s+)?(bool|int32|float|double|FString|FName|FVector|FRotator|TArray|TMap|TSet|[\w:]+[\*&]?)\s+(\w+)\s*[;=]')

    foreach ($match in $memberMatches) {
        $varType = $match.Groups[1].Value
        $varName = $match.Groups[2].Value

        # Check boolean prefix
        if ($varType -eq 'bool' -and $varName -notmatch '^b[A-Z]') {
            $script:Issues += [PSCustomObject]@{
                File = $FileName
                Type = "Variable"
                Name = $varName
                Issue = "Boolean variable should be prefixed with 'b'"
                Severity = "High"
                Suggestion = "Rename to b$($varName.Substring(0,1).ToUpper() + $varName.Substring(1))"
                Line = ($Content.Substring(0, $match.Index) -split "`n").Count
            }
        }

        # Check PascalCase
        if ($varName -notmatch '^[A-Z][a-zA-Z0-9]*$' -and $varName -notmatch '^b[A-Z][a-zA-Z0-9]*$') {
            $severity = "Medium"
            $suggestion = "Use PascalCase"

            if ($varName -match '_') {
                $severity = "High"
                $suggestion = "Convert from snake_case to PascalCase"
            }

            $script:Issues += [PSCustomObject]@{
                File = $FileName
                Type = "Variable"
                Name = $varName
                Issue = "Not in PascalCase format"
                Severity = $severity
                Suggestion = $suggestion
                Line = ($Content.Substring(0, $match.Index) -split "`n").Count
            }
        }

        # Check for Hungarian notation (except b for bool)
        if ($varName -match '^[a-z]{1,3}[A-Z]' -and $varName -notmatch '^b[A-Z]') {
            $script:Issues += [PSCustomObject]@{
                File = $FileName
                Type = "Variable"
                Name = $varName
                Issue = "Uses Hungarian notation (discouraged in modern C++)"
                Severity = "Low"
                Suggestion = "Remove type prefix, use descriptive names instead"
                Line = ($Content.Substring(0, $match.Index) -split "`n").Count
            }
        }
    }
}

function Check-ConstantNames {
    param($FileName, $Content)

    $constMatches = [regex]::Matches($Content, '(?m)^\s*(?:static\s+)?const\s+[\w:]+\s+(\w+)\s*=')

    foreach ($match in $constMatches) {
        $constName = $match.Groups[1].Value

        # Constants can be either UPPER_SNAKE_CASE or PascalCase in UE
        if ($constName -notmatch '^[A-Z][A-Z0-9_]*$' -and $constName -notmatch '^[A-Z][a-zA-Z0-9]*$') {
            $script:Issues += [PSCustomObject]@{
                File = $FileName
                Type = "Constant"
                Name = $constName
                Issue = "Not in UPPER_SNAKE_CASE or PascalCase"
                Severity = "Medium"
                Suggestion = "Use UPPER_SNAKE_CASE for constants or PascalCase for const variables"
                Line = ($Content.Substring(0, $match.Index) -split "`n").Count
            }
        }
    }
}

function Check-EnumNames {
    param($FileName, $Content)

    $enumMatches = [regex]::Matches($Content, '(?m)^\s*enum\s+(?:class\s+)?(\w+)')

    foreach ($match in $enumMatches) {
        $enumName = $match.Groups[1].Value

        # Enums should start with E
        if ($enumName -notmatch '^E[A-Z]') {
            $script:Issues += [PSCustomObject]@{
                File = $FileName
                Type = "Enum"
                Name = $enumName
                Issue = "Enum should be prefixed with 'E'"
                Severity = "High"
                Suggestion = "Rename to E$enumName"
                Line = ($Content.Substring(0, $match.Index) -split "`n").Count
            }
        }
    }
}

# Main execution
Write-Host "Naming Convention Check Starting..." -ForegroundColor Cyan

$files = Get-ChildItem -Path $SourcePath -Recurse -Include *.cpp,*.h |
    Where-Object { $_.FullName -notmatch '\\Intermediate\\' -and $_.FullName -notmatch '\\Tests\\' }

Write-Host "Analyzing $($files.Count) files..." -ForegroundColor Gray

foreach ($file in $files) {
    Write-Host "Checking: $($file.Name)" -ForegroundColor Gray
    $content = Get-Content $file.FullName -Raw

    Check-ClassName $file.Name $content
    Check-FunctionNames $file.Name $content
    Check-VariableNames $file.Name $content
    Check-ConstantNames $file.Name $content
    Check-EnumNames $file.Name $content
}

# Generate report
$highSeverity = ($Issues | Where-Object { $_.Severity -eq "High" }).Count
$mediumSeverity = ($Issues | Where-Object { $_.Severity -eq "Medium" }).Count
$lowSeverity = ($Issues | Where-Object { $_.Severity -eq "Low" }).Count

$report = @"
# Naming Convention Analysis Report
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## Executive Summary
- **Files Analyzed**: $($files.Count)
- **Naming Issues Found**: $($Issues.Count)
- **High Severity**: $highSeverity
- **Medium Severity**: $mediumSeverity
- **Low Severity**: $lowSeverity

## Unreal Engine Naming Conventions

### Class Prefixes
- **U**: UObject-derived classes (e.g., UActorComponent, UGameplayStatics)
- **A**: AActor-derived classes (e.g., ACharacter, APlayerController)
- **F**: Structs and non-UObject classes (e.g., FVector, FString)
- **I**: Interfaces (e.g., ITargetable)
- **E**: Enums (e.g., ECollisionChannel)
- **T**: Template classes (e.g., TArray, TMap)

### Naming Rules
- **Classes**: PascalCase with appropriate prefix
- **Functions**: PascalCase, start with verb
- **Variables**: PascalCase, booleans prefixed with 'b'
- **Constants**: UPPER_SNAKE_CASE or PascalCase
- **Enums**: PascalCase with 'E' prefix
- **Parameters**: PascalCase, optionally In/Out prefix

## Severity Distribution

| Severity | Count | Percentage | Priority |
|----------|-------|------------|----------|
| High | $highSeverity | $([Math]::Round(($highSeverity / [Math]::Max(1, $Issues.Count)) * 100, 1))% | P0 - Fix immediately |
| Medium | $mediumSeverity | $([Math]::Round(($mediumSeverity / [Math]::Max(1, $Issues.Count)) * 100, 1))% | P1 - Fix soon |
| Low | $lowSeverity | $([Math]::Round(($lowSeverity / [Math]::Max(1, $Issues.Count)) * 100, 1))% | P2 - Consider fixing |

## Issues by Type

| Type | Count | High | Medium | Low |
|------|-------|------|--------|-----|
| Class | $(($Issues | Where-Object { $_.Type -eq "Class" }).Count) | $(($Issues | Where-Object { $_.Type -eq "Class" -and $_.Severity -eq "High" }).Count) | $(($Issues | Where-Object { $_.Type -eq "Class" -and $_.Severity -eq "Medium" }).Count) | $(($Issues | Where-Object { $_.Type -eq "Class" -and $_.Severity -eq "Low" }).Count) |
| Function | $(($Issues | Where-Object { $_.Type -eq "Function" }).Count) | $(($Issues | Where-Object { $_.Type -eq "Function" -and $_.Severity -eq "High" }).Count) | $(($Issues | Where-Object { $_.Type -eq "Function" -and $_.Severity -eq "Medium" }).Count) | $(($Issues | Where-Object { $_.Type -eq "Function" -and $_.Severity -eq "Low" }).Count) |
| Variable | $(($Issues | Where-Object { $_.Type -eq "Variable" }).Count) | $(($Issues | Where-Object { $_.Type -eq "Variable" -and $_.Severity -eq "High" }).Count) | $(($Issues | Where-Object { $_.Type -eq "Variable" -and $_.Severity -eq "Medium" }).Count) | $(($Issues | Where-Object { $_.Type -eq "Variable" -and $_.Severity -eq "Low" }).Count) |
| Constant | $(($Issues | Where-Object { $_.Type -eq "Constant" }).Count) | $(($Issues | Where-Object { $_.Type -eq "Constant" -and $_.Severity -eq "High" }).Count) | $(($Issues | Where-Object { $_.Type -eq "Constant" -and $_.Severity -eq "Medium" }).Count) | $(($Issues | Where-Object { $_.Type -eq "Constant" -and $_.Severity -eq "Low" }).Count) |
| Enum | $(($Issues | Where-Object { $_.Type -eq "Enum" }).Count) | $(($Issues | Where-Object { $_.Type -eq "Enum" -and $_.Severity -eq "High" }).Count) | $(($Issues | Where-Object { $_.Type -eq "Enum" -and $_.Severity -eq "Medium" }).Count) | $(($Issues | Where-Object { $_.Type -eq "Enum" -and $_.Severity -eq "Low" }).Count) |

## Detailed Issues

### High Severity Issues (Fix Immediately)

| File | Type | Name | Issue | Suggestion | Line |
|------|------|------|-------|------------|------|
"@

foreach ($issue in ($Issues | Where-Object { $_.Severity -eq "High" } | Sort-Object File, Line)) {
    $report += "| $($issue.File) | $($issue.Type) | ``$($issue.Name)`` | $($issue.Issue) | $($issue.Suggestion) | $($issue.Line) |`n"
}

$report += @"

### Medium Severity Issues (Fix Soon)

| File | Type | Name | Issue | Suggestion | Line |
|------|------|------|-------|------------|------|
"@

foreach ($issue in ($Issues | Where-Object { $_.Severity -eq "Medium" } | Sort-Object File, Line | Select-Object -First 30)) {
    $report += "| $($issue.File) | $($issue.Type) | ``$($issue.Name)`` | $($issue.Issue) | $($issue.Suggestion) | $($issue.Line) |`n"
}

$report += @"

### Low Severity Issues (Consider Fixing)

| File | Type | Name | Issue | Suggestion | Line |
|------|------|------|-------|------------|------|
"@

foreach ($issue in ($Issues | Where-Object { $_.Severity -eq "Low" } | Sort-Object File, Line | Select-Object -First 20)) {
    $report += "| $($issue.File) | $($issue.Type) | ``$($issue.Name)`` | $($issue.Issue) | $($issue.Suggestion) | $($issue.Line) |`n"
}

$report += @"

## Common Naming Mistakes

### 1. Missing Class Prefixes
**Bad**: `class MyComponent`
**Good**: `class UMyComponent` (if UObject-derived)

### 2. Boolean Without 'b' Prefix
**Bad**: `bool IsActive;`
**Good**: `bool bIsActive;`

### 3. snake_case Instead of PascalCase
**Bad**: `void update_player_state()`
**Good**: `void UpdatePlayerState()`

### 4. camelCase Instead of PascalCase
**Bad**: `void updatePlayerState()`
**Good**: `void UpdatePlayerState()`

### 5. Functions Without Verbs
**Bad**: `void PlayerHealth()`
**Good**: `void UpdatePlayerHealth()` or `int32 GetPlayerHealth()`

### 6. Abbreviations in Uppercase
**Bad**: `class UHTTPRequest`
**Good**: `class UHttpRequest`

## Refactoring Guide

### Using Visual Studio Refactoring Tools

1. **Rename Symbol** (Ctrl+R, R)
   - Right-click on symbol
   - Select "Rename"
   - Enter new name
   - Preview changes
   - Apply

2. **Find All References** (Shift+F12)
   - Locate all usages
   - Manually update if needed

### Using ReSharper C++

1. **Rename** (Ctrl+R, R)
   - Automatically updates all references
   - Handles header/source files
   - Updates comments

2. **Naming Style Inspection**
   - Configure naming rules
   - Automatic suggestions
   - Batch fixes

### Manual Refactoring Steps

1. **Search and Replace**
   ```
   Find: OldName
   Replace: NewName
   Match case: Yes
   Match whole word: Yes
   ```

2. **Update Header Files First**
   - Change declarations
   - Update forward declarations

3. **Update Source Files**
   - Change implementations
   - Update local references

4. **Update Comments**
   - Update documentation
   - Fix code examples

5. **Rebuild and Test**
   - Full rebuild
   - Run all tests
   - Check for compilation errors

## Automated Renaming Script

Use the companion script `ImproveNaming.ps1` to automatically generate rename commands:

```powershell
.\Scripts\Quality\Improvement\ImproveNaming.ps1 -InputReport $OutputFile
```

This will generate:
- Visual Studio rename commands
- Search/replace patterns
- Git commit messages

## Implementation Plan

### Phase 1: High Severity (Week 1)
- Fix class prefix issues
- Fix boolean naming
- Estimated effort: $($highSeverity * 0.25)h
- Expected improvement: +1 quality point

### Phase 2: Medium Severity (Week 2)
- Fix function naming
- Fix variable naming
- Estimated effort: $($mediumSeverity * 0.15)h
- Expected improvement: +0.5 quality points

### Phase 3: Low Severity (Week 3)
- Improve function verb usage
- Clean up abbreviations
- Estimated effort: $($lowSeverity * 0.1)h
- Expected improvement: +0.3 quality points

### Total Expected Improvement: +1.8 Quality Points

## Quality Metrics

| Metric | Current | Target | Status |
|--------|---------|--------|--------|
| Naming Issues | $($Issues.Count) | < 50 | $(if ($Issues.Count -lt 50) { "✅ Good" } else { "⚠️ Needs Work" }) |
| High Severity | $highSeverity | 0 | $(if ($highSeverity -eq 0) { "✅ Good" } else { "⚠️ Needs Work" }) |
| Convention Compliance | $([Math]::Round((1 - ($Issues.Count / [Math]::Max(1, $files.Count * 10))) * 100, 1))% | > 95% | $(if ((1 - ($Issues.Count / [Math]::Max(1, $files.Count * 10))) -gt 0.95) { "✅ Good" } else { "⚠️ Needs Work" }) |

## Best Practices

### 1. Use Descriptive Names
- Avoid single-letter variables (except loop counters)
- Use full words, not abbreviations
- Make intent clear from the name

### 2. Be Consistent
- Follow team conventions
- Use same patterns throughout codebase
- Document naming decisions

### 3. Consider Context
- Class members don't need class name prefix
- Local variables can be shorter
- Public API needs clear names

### 4. Use IDE Features
- Enable naming inspections
- Use auto-complete
- Leverage refactoring tools

### 5. Review Regularly
- Check naming in code reviews
- Run this tool weekly
- Update guidelines as needed

## Resources

- [Unreal Engine Coding Standard](https://docs.unrealengine.com/5.3/en-US/epic-cplusplus-coding-standard-for-unreal-engine/)
- [C++ Core Guidelines - Naming](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-naming)
- [Clean Code - Meaningful Names](https://www.oreilly.com/library/view/clean-code-a/9780136083238/)

## Next Steps

1. **Review high-severity issues** with team
2. **Create refactoring tasks** for each issue
3. **Use automated tools** where possible
4. **Update coding guidelines** with examples
5. **Add pre-commit checks** for naming conventions
6. **Train team** on Unreal naming standards

---
*Generated by NamingConventionChecker.ps1*
"@

# Ensure output directory exists
$outputDir = Split-Path $OutputFile -Parent
if (!(Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

$report | Out-File $OutputFile -Encoding UTF8

Write-Host "`nNaming Convention Check Complete!" -ForegroundColor Green
Write-Host "Found $($Issues.Count) naming issues" -ForegroundColor Yellow
Write-Host "High: $highSeverity, Medium: $mediumSeverity, Low: $lowSeverity" -ForegroundColor Cyan
Write-Host "Report saved to: $OutputFile" -ForegroundColor Cyan
