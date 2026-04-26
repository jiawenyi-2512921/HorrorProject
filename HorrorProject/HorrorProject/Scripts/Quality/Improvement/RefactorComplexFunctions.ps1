# RefactorComplexFunctions.ps1 - Automated Refactoring Suggestions
# Generates specific refactoring recommendations for complex functions

param(
    [string]$SourcePath = "Source",
    [string]$OutputFile = "Docs/Quality/Improvement/RefactoringPlan.md",
    [int]$ComplexityThreshold = 15
)

$RefactoringTasks = @()

function Generate-RefactoringTask {
    param($File, $Function, $Complexity, $Lines, $Content)

    $task = [PSCustomObject]@{
        File = $File
        Function = $Function
        Complexity = $Complexity
        Lines = $Lines
        Priority = if ($Complexity -gt 25) { "P0" } elseif ($Complexity -gt 20) { "P1" } elseif ($Complexity -gt 15) { "P2" } else { "P3" }
        EstimatedHours = if ($Complexity -gt 25) { 6 } elseif ($Complexity -gt 20) { 4 } elseif ($Complexity -gt 15) { 2 } else { 1 }
        Techniques = @()
        CodeExample = ""
    }

    # Analyze and suggest techniques
    if ($Content -match 'for.*for') {
        $task.Techniques += "Extract nested loops into separate methods"
    }

    if (([regex]::Matches($Content, 'else\s+if')).Count -gt 3) {
        $task.Techniques += "Replace if-else chain with switch or strategy pattern"
    }

    if (([regex]::Matches($Content, '\breturn\b')).Count -gt 3) {
        $task.Techniques += "Consolidate return statements using result variable"
    }

    if ($Lines -gt 80) {
        $task.Techniques += "Split into smaller, focused functions"
    }

    # Calculate nesting depth
    $maxNesting = 0
    $currentNesting = 0
    foreach ($char in $Content.ToCharArray()) {
        if ($char -eq '{') { $currentNesting++ }
        elseif ($char -eq '}') { $currentNesting-- }
        $maxNesting = [Math]::Max($maxNesting, $currentNesting)
    }

    if ($maxNesting -gt 4) {
        $task.Techniques += "Reduce nesting with guard clauses and early returns"
    }

    return $task
}

# Main execution
Write-Host "Generating Refactoring Plan..." -ForegroundColor Cyan

$files = Get-ChildItem -Path $SourcePath -Recurse -Include *.cpp,*.h |
    Where-Object { $_.FullName -notmatch '\\Intermediate\\' }

foreach ($file in $files) {
    $content = Get-Content $file.FullName -Raw
    $functionMatches = [regex]::Matches($content, '(?m)^[\w\s\*&:<>,]+\s+(\w+::\w+|\w+)\s*\([^)]*\)\s*(?:const)?\s*\{')

    foreach ($match in $functionMatches) {
        $functionName = $match.Groups[1].Value
        $startPos = $match.Index

        # Extract function body
        $braceCount = 0
        $functionContent = ""
        for ($i = $startPos; $i -lt $content.Length; $i++) {
            $char = $content[$i]
            $functionContent += $char
            if ($char -eq '{') { $braceCount++ }
            elseif ($char -eq '}') {
                $braceCount--
                if ($braceCount -eq 0) { break }
            }
        }

        $lines = ($functionContent -split "`n").Count
        $complexity = 1 + ([regex]::Matches($functionContent, '\bif\s*\(')).Count +
                          ([regex]::Matches($functionContent, '\bfor\s*\(')).Count +
                          ([regex]::Matches($functionContent, '\bwhile\s*\(')).Count +
                          ([regex]::Matches($functionContent, '\b&&\b|\|\|\b')).Count

        if ($complexity -gt $ComplexityThreshold) {
            $RefactoringTasks += Generate-RefactoringTask $file.Name $functionName $complexity $lines $functionContent
        }
    }
}

# Generate report
$report = @"
# Refactoring Plan
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## Overview
This document provides a prioritized refactoring plan for complex functions in the codebase.

**Total Functions to Refactor**: $($RefactoringTasks.Count)
**Estimated Total Effort**: $(($RefactoringTasks | Measure-Object -Property EstimatedHours -Sum).Sum) hours

## Priority Breakdown

| Priority | Count | Total Hours | Description |
|----------|-------|-------------|-------------|
| P0 | $(($RefactoringTasks | Where-Object { $_.Priority -eq "P0" }).Count) | $(($RefactoringTasks | Where-Object { $_.Priority -eq "P0" } | Measure-Object -Property EstimatedHours -Sum).Sum) | Critical - Complexity > 25 |
| P1 | $(($RefactoringTasks | Where-Object { $_.Priority -eq "P1" }).Count) | $(($RefactoringTasks | Where-Object { $_.Priority -eq "P1" } | Measure-Object -Property EstimatedHours -Sum).Sum) | High - Complexity 20-25 |
| P2 | $(($RefactoringTasks | Where-Object { $_.Priority -eq "P2" }).Count) | $(($RefactoringTasks | Where-Object { $_.Priority -eq "P2" } | Measure-Object -Property EstimatedHours -Sum).Sum) | Medium - Complexity 15-20 |
| P3 | $(($RefactoringTasks | Where-Object { $_.Priority -eq "P3" }).Count) | $(($RefactoringTasks | Where-Object { $_.Priority -eq "P3" } | Measure-Object -Property EstimatedHours -Sum).Sum) | Low - Complexity < 15 |

## Refactoring Tasks

"@

foreach ($task in ($RefactoringTasks | Sort-Object Priority, Complexity -Descending)) {
    $report += @"

### [$($task.Priority)] $($task.Function)
**File**: ``$($task.File)``
**Complexity**: $($task.Complexity) | **Lines**: $($task.Lines) | **Estimated**: $($task.EstimatedHours)h

**Refactoring Techniques**:
"@
    foreach ($technique in $task.Techniques) {
        $report += "`n- $technique"
    }

    $report += @"


**Steps**:
1. Write unit tests for current behavior
2. Apply refactoring techniques
3. Verify tests still pass
4. Review and commit

---
"@
}

$report += @"

## Refactoring Workflow

### 1. Preparation
- Create feature branch: ``git checkout -b refactor/function-name``
- Ensure all tests pass
- Review function behavior

### 2. Write Tests
```cpp
// Add tests before refactoring
TEST_F(MyComponentTest, TestOriginalBehavior) {
    // Test current functionality
}
```

### 3. Refactor
- Apply one technique at a time
- Run tests after each change
- Commit frequently

### 4. Review
- Self-review changes
- Check complexity reduction
- Ensure readability improved

### 5. Merge
- Create pull request
- Request code review
- Merge after approval

## Sprint Planning

### Sprint 1 (Week 1-2): P0 Tasks
$(($RefactoringTasks | Where-Object { $_.Priority -eq "P0" } | ForEach-Object { "- [ ] $($_.Function) ($($_.File)) - $($_.EstimatedHours)h" }) -join "`n")

**Total**: $(($RefactoringTasks | Where-Object { $_.Priority -eq "P0" } | Measure-Object -Property EstimatedHours -Sum).Sum) hours

### Sprint 2 (Week 3-4): P1 Tasks
$(($RefactoringTasks | Where-Object { $_.Priority -eq "P1" } | Select-Object -First 10 | ForEach-Object { "- [ ] $($_.Function) ($($_.File)) - $($_.EstimatedHours)h" }) -join "`n")

**Total**: $(($RefactoringTasks | Where-Object { $_.Priority -eq "P1" } | Measure-Object -Property EstimatedHours -Sum).Sum) hours

### Sprint 3 (Week 5-6): P2 Tasks
$(($RefactoringTasks | Where-Object { $_.Priority -eq "P2" } | Select-Object -First 10 | ForEach-Object { "- [ ] $($_.Function) ($($_.File)) - $($_.EstimatedHours)h" }) -join "`n")

**Total**: $(($RefactoringTasks | Where-Object { $_.Priority -eq "P2" } | Measure-Object -Property EstimatedHours -Sum).Sum) hours

## Success Metrics

- [ ] Average complexity reduced to < 10
- [ ] No functions with complexity > 20
- [ ] Average function length < 50 lines
- [ ] All refactored functions have tests
- [ ] Code review approval for all changes

---
*Generated by RefactorComplexFunctions.ps1*
"@

$outputDir = Split-Path $OutputFile -Parent
if (!(Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

$report | Out-File $OutputFile -Encoding UTF8

Write-Host "`nRefactoring Plan Generated!" -ForegroundColor Green
Write-Host "Total tasks: $($RefactoringTasks.Count)" -ForegroundColor Cyan
Write-Host "Estimated effort: $(($RefactoringTasks | Measure-Object -Property EstimatedHours -Sum).Sum) hours" -ForegroundColor Cyan
Write-Host "Report saved to: $OutputFile" -ForegroundColor Cyan
