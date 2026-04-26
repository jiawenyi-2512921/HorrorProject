# CodeDuplicationDetector.ps1 - Duplicate Code Detection
# Identifies code duplication and suggests refactoring opportunities

param(
    [string]$SourcePath = "Source",
    [string]$OutputFile = "Docs/Quality/Improvement/DuplicationReport.md",
    [int]$MinTokens = 50,
    [int]$MinLines = 6
)

$Duplicates = @()
$FileHashes = @{}

function Get-CodeTokens {
    param($Content)

    # Normalize code by removing whitespace and comments
    $normalized = $Content -replace '//.*$', '' -replace '/\*[\s\S]*?\*/', ''
    $normalized = $normalized -replace '\s+', ' ' -replace '^\s+|\s+$', ''

    return $normalized
}

function Get-CodeBlocks {
    param($Content, $MinLines)

    $lines = $Content -split "`n"
    $blocks = @()

    for ($i = 0; $i -lt $lines.Count - $MinLines; $i++) {
        $block = $lines[$i..($i + $MinLines - 1)] -join "`n"
        $normalized = Get-CodeTokens $block

        if ($normalized.Length -gt $MinTokens) {
            $blocks += [PSCustomObject]@{
                StartLine = $i + 1
                EndLine = $i + $MinLines
                Content = $block
                Normalized = $normalized
                Hash = (Get-StringHash $normalized)
            }
        }
    }

    return $blocks
}

function Get-StringHash {
    param($String)

    $md5 = [System.Security.Cryptography.MD5]::Create()
    $hash = $md5.ComputeHash([System.Text.Encoding]::UTF8.GetBytes($String))
    return [System.BitConverter]::ToString($hash) -replace '-', ''
}

function Find-Duplicates {
    param($Files)

    $blockMap = @{}

    foreach ($file in $Files) {
        Write-Host "Scanning: $($file.Name)" -ForegroundColor Gray
        $content = Get-Content $file.FullName -Raw

        $blocks = Get-CodeBlocks $content $MinLines

        foreach ($block in $blocks) {
            if ($blockMap.ContainsKey($block.Hash)) {
                $blockMap[$block.Hash] += @{
                    File = $file.FullName
                    StartLine = $block.StartLine
                    EndLine = $block.EndLine
                    Content = $block.Content
                }
            }
            else {
                $blockMap[$block.Hash] = @(
                    @{
                        File = $file.FullName
                        StartLine = $block.StartLine
                        EndLine = $block.EndLine
                        Content = $block.Content
                    }
                )
            }
        }
    }

    # Find actual duplicates (appearing in 2+ locations)
    foreach ($hash in $blockMap.Keys) {
        $locations = $blockMap[$hash]
        if ($locations.Count -gt 1) {
            $script:Duplicates += [PSCustomObject]@{
                Hash = $hash
                Occurrences = $locations.Count
                Locations = $locations
                Lines = ($locations[0].EndLine - $locations[0].StartLine + 1)
                Severity = if ($locations.Count -gt 3) { "High" }
                          elseif ($locations.Count -gt 2) { "Medium" }
                          else { "Low" }
            }
        }
    }
}

function Generate-RefactoringAdvice {
    param($Duplicate)

    $advice = @"

### Duplication Group ($(($Duplicate.Occurrences)) occurrences, $($Duplicate.Lines) lines)
**Severity**: $($Duplicate.Severity)

**Locations**:
"@

    foreach ($loc in $Duplicate.Locations) {
        $shortFile = Split-Path $loc.File -Leaf
        $advice += "`n- ``$shortFile`` lines $($loc.StartLine)-$($loc.EndLine)"
    }

    $advice += @"


**Refactoring Strategy**:

1. **Extract Method**: Create a shared utility function
   ```cpp
   // Before: Duplicated in multiple files
   void ClassA::DoSomething() {
       // Duplicated code block
       if (Condition) {
           // Complex logic
       }
   }

   void ClassB::DoSomething() {
       // Same duplicated code block
       if (Condition) {
           // Complex logic
       }
   }

   // After: Extracted to utility
   class SharedUtility {
   public:
       static void ExtractedMethod() {
           if (Condition) {
               // Complex logic
           }
       }
   };

   void ClassA::DoSomething() {
       SharedUtility::ExtractedMethod();
   }

   void ClassB::DoSomething() {
       SharedUtility::ExtractedMethod();
   }
   ```

2. **Consider Inheritance**: If duplication is in related classes
   - Create a base class with the common functionality
   - Use template method pattern for variations

3. **Use Composition**: If classes are unrelated
   - Create a component/helper class
   - Inject or reference the component

**Estimated Effort**: $($Duplicate.Occurrences * 0.5) hours
**Expected Benefit**: Reduced maintenance burden, improved consistency

---
"@

    return $advice
}

# Main execution
Write-Host "Code Duplication Detection Starting..." -ForegroundColor Cyan
Write-Host "Min Tokens: $MinTokens, Min Lines: $MinLines" -ForegroundColor Gray

$files = Get-ChildItem -Path $SourcePath -Recurse -Include *.cpp,*.h |
    Where-Object { $_.FullName -notmatch '\\Intermediate\\' -and $_.FullName -notmatch '\\Tests\\' }

Write-Host "Analyzing $($files.Count) files for duplication..." -ForegroundColor Yellow

Find-Duplicates $files

# Sort by severity and occurrences
$Duplicates = $Duplicates | Sort-Object Severity, Occurrences -Descending

# Generate report
$totalDuplicatedLines = ($Duplicates | ForEach-Object { $_.Lines * ($_.Occurrences - 1) } | Measure-Object -Sum).Sum
$highSeverity = ($Duplicates | Where-Object { $_.Severity -eq "High" }).Count
$mediumSeverity = ($Duplicates | Where-Object { $_.Severity -eq "Medium" }).Count
$lowSeverity = ($Duplicates | Where-Object { $_.Severity -eq "Low" }).Count

$report = @"
# Code Duplication Analysis Report
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## Executive Summary
- **Files Analyzed**: $($files.Count)
- **Duplication Groups Found**: $($Duplicates.Count)
- **Total Duplicated Lines**: $totalDuplicatedLines (excluding first occurrence)
- **High Severity**: $highSeverity groups
- **Medium Severity**: $mediumSeverity groups
- **Low Severity**: $lowSeverity groups

## Impact Analysis

| Metric | Value | Impact |
|--------|-------|--------|
| Duplication Groups | $($Duplicates.Count) | Maintenance burden |
| Duplicated Lines | $totalDuplicatedLines | Code bloat |
| Avg Occurrences | $([Math]::Round(($Duplicates | Measure-Object -Property Occurrences -Average).Average, 1)) | Consistency risk |
| Estimated Refactoring | $([Math]::Round(($Duplicates | ForEach-Object { $_.Occurrences * 0.5 } | Measure-Object -Sum).Sum, 1))h | Time investment |

## Severity Distribution

| Severity | Count | Avg Occurrences | Total Lines | Priority |
|----------|-------|-----------------|-------------|----------|
| High | $highSeverity | $([Math]::Round((($Duplicates | Where-Object { $_.Severity -eq "High" } | Measure-Object -Property Occurrences -Average).Average), 1)) | $(($Duplicates | Where-Object { $_.Severity -eq "High" } | ForEach-Object { $_.Lines * ($_.Occurrences - 1) } | Measure-Object -Sum).Sum) | P0 |
| Medium | $mediumSeverity | $([Math]::Round((($Duplicates | Where-Object { $_.Severity -eq "Medium" } | Measure-Object -Property Occurrences -Average).Average), 1)) | $(($Duplicates | Where-Object { $_.Severity -eq "Medium" } | ForEach-Object { $_.Lines * ($_.Occurrences - 1) } | Measure-Object -Sum).Sum) | P1 |
| Low | $lowSeverity | $([Math]::Round((($Duplicates | Where-Object { $_.Severity -eq "Low" } | Measure-Object -Property Occurrences -Average).Average), 1)) | $(($Duplicates | Where-Object { $_.Severity -eq "Low" } | ForEach-Object { $_.Lines * ($_.Occurrences - 1) } | Measure-Object -Sum).Sum) | P2 |

## Duplication Summary

| Group | Occurrences | Lines | Severity | Estimated Effort |
|-------|-------------|-------|----------|------------------|
"@

$groupId = 1
foreach ($dup in $Duplicates) {
    $effort = "$($dup.Occurrences * 0.5)h"
    $report += "| Group $groupId | $($dup.Occurrences) | $($dup.Lines) | $($dup.Severity) | $effort |`n"
    $groupId++
}

$report += @"

## Detailed Duplication Analysis

"@

# Add detailed analysis for top duplicates
$topDuplicates = $Duplicates | Select-Object -First 15
foreach ($dup in $topDuplicates) {
    $report += Generate-RefactoringAdvice $dup
}

$report += @"

## Refactoring Patterns for Duplication

### 1. Extract Method
**When**: Same code block in multiple methods
**How**: Create a new method and call it from all locations
**Benefit**: Single source of truth, easier to maintain

### 2. Extract Class
**When**: Related duplicated methods across classes
**How**: Create a new utility/helper class
**Benefit**: Better organization, reusability

### 3. Template Method Pattern
**When**: Similar algorithms with variations
**How**: Define skeleton in base class, override specific steps
**Benefit**: Code reuse while allowing customization

### 4. Strategy Pattern
**When**: Different implementations of same behavior
**How**: Encapsulate algorithms in separate classes
**Benefit**: Easy to add new strategies, testable

### 5. Pull Up Method
**When**: Identical methods in sibling classes
**How**: Move method to parent class
**Benefit**: Eliminates duplication through inheritance

### 6. Form Template Method
**When**: Similar methods with different details
**How**: Extract common parts to template method
**Benefit**: Reduces duplication while preserving differences

## Best Practices to Prevent Duplication

### 1. Code Reviews
- Check for similar code during reviews
- Use duplication detection tools
- Encourage refactoring before merging

### 2. Design Patterns
- Learn and apply common patterns
- Use composition over inheritance
- Favor small, focused functions

### 3. Utility Libraries
- Create project-specific utilities
- Document common operations
- Make utilities discoverable

### 4. Automated Detection
- Run duplication analysis in CI/CD
- Set thresholds for acceptable duplication
- Block PRs with excessive duplication

### 5. Team Communication
- Share refactoring opportunities
- Discuss common patterns
- Maintain coding standards

## Implementation Roadmap

### Week 1: High Severity (P0)
- Focus on duplications with 4+ occurrences
- Extract to utility classes
- Estimated effort: $(($Duplicates | Where-Object { $_.Severity -eq "High" } | ForEach-Object { $_.Occurrences * 0.5 } | Measure-Object -Sum).Sum) hours
- Expected improvement: +1.5 quality points

### Week 2-3: Medium Severity (P1)
- Address duplications with 3 occurrences
- Consider design patterns
- Estimated effort: $(($Duplicates | Where-Object { $_.Severity -eq "Medium" } | ForEach-Object { $_.Occurrences * 0.5 } | Measure-Object -Sum).Sum) hours
- Expected improvement: +1 quality point

### Week 4: Low Severity (P2)
- Clean up remaining duplications
- Document patterns
- Estimated effort: $(($Duplicates | Where-Object { $_.Severity -eq "Low" } | ForEach-Object { $_.Occurrences * 0.5 } | Measure-Object -Sum).Sum) hours
- Expected improvement: +0.5 quality points

### Total Expected Improvement: +3 Quality Points

## Quality Metrics

| Metric | Current | Target | Status |
|--------|---------|--------|--------|
| Duplication Groups | $($Duplicates.Count) | < 10 | $(if ($Duplicates.Count -lt 10) { "✅ Good" } else { "⚠️ Needs Work" }) |
| Duplicated Lines | $totalDuplicatedLines | < 500 | $(if ($totalDuplicatedLines -lt 500) { "✅ Good" } else { "⚠️ Needs Work" }) |
| High Severity | $highSeverity | 0 | $(if ($highSeverity -eq 0) { "✅ Good" } else { "⚠️ Needs Work" }) |
| Code Reuse Score | $(100 - [Math]::Min(100, ($totalDuplicatedLines / 100))) | > 95 | $(if ((100 - [Math]::Min(100, ($totalDuplicatedLines / 100))) -gt 95) { "✅ Good" } else { "⚠️ Needs Work" }) |

## Tools and Automation

### Pre-commit Hook
```powershell
# Check for new duplication
.\Scripts\Quality\Improvement\CodeDuplicationDetector.ps1 -MinLines 6
if ($LASTEXITCODE -ne 0) {
    Write-Warning "Code duplication detected. Consider refactoring."
}
```

### CI/CD Integration
```yaml
quality-check:
  script:
    - pwsh Scripts/Quality/Improvement/CodeDuplicationDetector.ps1
  artifacts:
    reports:
      - Docs/Quality/Improvement/DuplicationReport.md
```

## Next Steps

1. **Review high-severity duplications** with team
2. **Create refactoring tasks** in project tracker
3. **Schedule refactoring sessions** (2-3 hours/week)
4. **Implement automated checks** in CI/CD
5. **Track progress** using this tool weekly
6. **Update coding guidelines** with duplication prevention strategies

---
*Generated by CodeDuplicationDetector.ps1*
*Detection Parameters: MinTokens=$MinTokens, MinLines=$MinLines*
"@

# Ensure output directory exists
$outputDir = Split-Path $OutputFile -Parent
if (!(Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

$report | Out-File $OutputFile -Encoding UTF8

Write-Host "`nDuplication Detection Complete!" -ForegroundColor Green
Write-Host "Found $($Duplicates.Count) duplication groups" -ForegroundColor Yellow
Write-Host "Total duplicated lines: $totalDuplicatedLines" -ForegroundColor Cyan
Write-Host "High: $highSeverity, Medium: $mediumSeverity, Low: $lowSeverity" -ForegroundColor Cyan
Write-Host "Report saved to: $OutputFile" -ForegroundColor Cyan
