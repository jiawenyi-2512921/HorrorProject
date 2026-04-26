# CodeComplexityAnalyzer.ps1 - Enhanced Complexity Analysis
# Provides detailed complexity metrics with refactoring suggestions

param(
    [string]$SourcePath = "Source",
    [string]$OutputFile = "Docs/Quality/Improvement/ComplexityAnalysis.md",
    [int]$ComplexityThreshold = 15,
    [int]$LineThreshold = 80
)

$Issues = @()
$RefactoringSuggestions = @()

function Analyze-ComplexFunction {
    param($File, $Function, $Complexity, $Lines, $Content)

    $suggestions = @()

    # Detect nested loops
    $nestedLoops = ([regex]::Matches($Content, 'for\s*\([^)]*\)[^{]*\{[^}]*for\s*\(')).Count
    if ($nestedLoops -gt 0) {
        $suggestions += "Extract nested loops into separate methods"
    }

    # Detect long if-else chains
    $elseIfCount = ([regex]::Matches($Content, 'else\s+if')).Count
    if ($elseIfCount -gt 3) {
        $suggestions += "Replace if-else chain with switch statement or strategy pattern"
    }

    # Detect complex boolean expressions
    $complexBooleans = ([regex]::Matches($Content, '\&\&.*\&\&|\|\|.*\|\|')).Count
    if ($complexBooleans -gt 2) {
        $suggestions += "Extract complex boolean expressions into named predicates"
    }

    # Detect multiple return statements
    $returnCount = ([regex]::Matches($Content, '\breturn\b')).Count
    if ($returnCount -gt 3) {
        $suggestions += "Consolidate multiple return statements using result variables"
    }

    # Detect deep nesting
    $maxNesting = 0
    $currentNesting = 0
    foreach ($char in $Content.ToCharArray()) {
        if ($char -eq '{') {
            $currentNesting++
            $maxNesting = [Math]::Max($maxNesting, $currentNesting)
        }
        elseif ($char -eq '}') {
            $currentNesting--
        }
    }
    if ($maxNesting -gt 4) {
        $suggestions += "Reduce nesting depth (current: $maxNesting) by extracting methods or using early returns"
    }

    # Calculate refactoring priority
    $priority = "Low"
    if ($Complexity -gt 25 -or $Lines -gt 150) {
        $priority = "Critical"
    }
    elseif ($Complexity -gt 20 -or $Lines -gt 100) {
        $priority = "High"
    }
    elseif ($Complexity -gt 15 -or $Lines -gt 80) {
        $priority = "Medium"
    }

    $script:Issues += [PSCustomObject]@{
        File = $File
        Function = $Function
        Complexity = $Complexity
        Lines = $Lines
        MaxNesting = $maxNesting
        Priority = $priority
        Suggestions = $suggestions -join "; "
        EstimatedEffort = if ($priority -eq "Critical") { "4-8h" }
                         elseif ($priority -eq "High") { "2-4h" }
                         elseif ($priority -eq "Medium") { "1-2h" }
                         else { "0.5-1h" }
    }
}

function Generate-RefactoringExample {
    param($Issue)

    $example = @"

### $($Issue.Function) - $($Issue.Priority) Priority
**File**: $($Issue.File)
**Complexity**: $($Issue.Complexity) | **Lines**: $($Issue.Lines) | **Max Nesting**: $($Issue.MaxNesting)
**Estimated Effort**: $($Issue.EstimatedEffort)

**Issues Detected**:
$($Issue.Suggestions)

**Refactoring Strategy**:
"@

    if ($Issue.Complexity -gt 20) {
        $example += @"

1. **Extract Method**: Break down into smaller, focused functions
   - Identify logical blocks that can be extracted
   - Each extracted method should have a single responsibility
   - Target: Reduce complexity to < 10 per method

2. **Simplify Control Flow**: Reduce branching complexity
   - Use early returns to reduce nesting
   - Replace complex conditions with named predicates
   - Consider using polymorphism for type-based branching

"@
    }

    if ($Issue.Lines -gt 80) {
        $example += @"

3. **Split Long Function**: Divide into logical sub-functions
   - Identify distinct phases or operations
   - Extract each phase into a well-named helper method
   - Target: Keep functions under 50 lines

"@
    }

    if ($Issue.MaxNesting -gt 4) {
        $example += @"

4. **Reduce Nesting**: Flatten control structure
   - Use guard clauses (early returns) for error conditions
   - Invert if conditions to reduce else blocks
   - Extract nested blocks into separate methods
   - Target: Maximum nesting depth of 3

"@
    }

    $example += @"

**Code Example - Before**:
```cpp
// Complex function with high cyclomatic complexity
void ComplexFunction(ACharacter* Character) {
    if (Character != nullptr) {
        if (Character->IsAlive()) {
            for (auto& Item : Items) {
                if (Item.IsValid()) {
                    if (Item->CanBeUsed()) {
                        // Deep nesting...
                    }
                }
            }
        }
    }
}
```

**Code Example - After**:
```cpp
// Refactored with guard clauses and extracted methods
void RefactoredFunction(ACharacter* Character) {
    if (!IsValidCharacter(Character)) return;

    ProcessValidItems(Character);
}

bool IsValidCharacter(ACharacter* Character) const {
    return Character != nullptr && Character->IsAlive();
}

void ProcessValidItems(ACharacter* Character) {
    for (const auto& Item : Items) {
        ProcessSingleItem(Item, Character);
    }
}

void ProcessSingleItem(const FItemData& Item, ACharacter* Character) {
    if (!Item.IsValid() || !Item->CanBeUsed()) return;

    // Single responsibility logic here
}
```

---
"@

    return $example
}

# Main execution
Write-Host "Enhanced Complexity Analysis Starting..." -ForegroundColor Cyan

$files = Get-ChildItem -Path $SourcePath -Recurse -Include *.cpp,*.h |
    Where-Object { $_.FullName -notmatch '\\Intermediate\\' -and $_.FullName -notmatch '\\Tests\\' }

Write-Host "Analyzing $($files.Count) files..." -ForegroundColor Gray

foreach ($file in $files) {
    $content = Get-Content $file.FullName -Raw

    # Simple function extraction (reuse from ComplexityAnalysis.ps1)
    $functionMatches = [regex]::Matches($content, '(?m)^[\w\s\*&:<>,]+\s+(\w+::\w+|\w+)\s*\([^)]*\)\s*(?:const)?\s*\{')

    foreach ($match in $functionMatches) {
        $functionName = $match.Groups[1].Value
        $startPos = $match.Index

        # Extract function body
        $braceCount = 0
        $inFunction = $false
        $functionContent = ""

        for ($i = $startPos; $i -lt $content.Length; $i++) {
            $char = $content[$i]
            $functionContent += $char

            if ($char -eq '{') {
                $braceCount++
                $inFunction = $true
            }
            elseif ($char -eq '}') {
                $braceCount--
                if ($braceCount -eq 0 -and $inFunction) {
                    break
                }
            }
        }

        # Calculate metrics
        $lines = ($functionContent -split "`n").Count
        $complexity = 1
        $complexity += ([regex]::Matches($functionContent, '\bif\s*\(')).Count
        $complexity += ([regex]::Matches($functionContent, '\belse\s+if\s*\(')).Count
        $complexity += ([regex]::Matches($functionContent, '\bfor\s*\(')).Count
        $complexity += ([regex]::Matches($functionContent, '\bwhile\s*\(')).Count
        $complexity += ([regex]::Matches($functionContent, '\bcase\s+')).Count
        $complexity += ([regex]::Matches($functionContent, '\b&&\b')).Count
        $complexity += ([regex]::Matches($functionContent, '\b\|\|\b')).Count

        if ($complexity -gt $ComplexityThreshold -or $lines -gt $LineThreshold) {
            Analyze-ComplexFunction $file.Name $functionName $complexity $lines $functionContent
        }
    }
}

# Generate report
$report = @"
# Enhanced Complexity Analysis Report
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## Executive Summary
- **Files Analyzed**: $($files.Count)
- **Complex Functions Found**: $($Issues.Count)
- **Critical Priority**: $(($Issues | Where-Object { $_.Priority -eq "Critical" }).Count)
- **High Priority**: $(($Issues | Where-Object { $_.Priority -eq "High" }).Count)
- **Medium Priority**: $(($Issues | Where-Object { $_.Priority -eq "Medium" }).Count)
- **Total Estimated Effort**: $(($Issues | Measure-Object -Property EstimatedEffort -Sum).Sum) hours

## Complexity Threshold Settings
- **Complexity Threshold**: $ComplexityThreshold
- **Line Count Threshold**: $LineThreshold

## Priority Distribution

| Priority | Count | Avg Complexity | Avg Lines | Total Effort |
|----------|-------|----------------|-----------|--------------|
| Critical | $(($Issues | Where-Object { $_.Priority -eq "Critical" }).Count) | $([Math]::Round((($Issues | Where-Object { $_.Priority -eq "Critical" } | Measure-Object -Property Complexity -Average).Average), 1)) | $([Math]::Round((($Issues | Where-Object { $_.Priority -eq "Critical" } | Measure-Object -Property Lines -Average).Average), 1)) | $(($Issues | Where-Object { $_.Priority -eq "Critical" }).Count * 6)h |
| High | $(($Issues | Where-Object { $_.Priority -eq "High" }).Count) | $([Math]::Round((($Issues | Where-Object { $_.Priority -eq "High" } | Measure-Object -Property Complexity -Average).Average), 1)) | $([Math]::Round((($Issues | Where-Object { $_.Priority -eq "High" } | Measure-Object -Property Lines -Average).Average), 1)) | $(($Issues | Where-Object { $_.Priority -eq "High" }).Count * 3)h |
| Medium | $(($Issues | Where-Object { $_.Priority -eq "Medium" }).Count) | $([Math]::Round((($Issues | Where-Object { $_.Priority -eq "Medium" } | Measure-Object -Property Complexity -Average).Average), 1)) | $([Math]::Round((($Issues | Where-Object { $_.Priority -eq "Medium" } | Measure-Object -Property Lines -Average).Average), 1)) | $(($Issues | Where-Object { $_.Priority -eq "Medium" }).Count * 1.5)h |

## Detailed Function Analysis

| Function | File | Complexity | Lines | Nesting | Priority | Effort |
|----------|------|------------|-------|---------|----------|--------|
"@

foreach ($issue in ($Issues | Sort-Object Priority, Complexity -Descending)) {
    $report += "| $($issue.Function) | $($issue.File) | $($issue.Complexity) | $($issue.Lines) | $($issue.MaxNesting) | $($issue.Priority) | $($issue.EstimatedEffort) |`n"
}

$report += @"

## Refactoring Recommendations

"@

# Add detailed refactoring examples for top issues
$topIssues = $Issues | Sort-Object Priority, Complexity -Descending | Select-Object -First 10
foreach ($issue in $topIssues) {
    $report += Generate-RefactoringExample $issue
}

$report += @"

## Refactoring Patterns

### 1. Extract Method
Break large functions into smaller, focused methods.
- **When**: Function > 50 lines or complexity > 10
- **Benefit**: Improved readability, testability, reusability

### 2. Replace Conditional with Polymorphism
Use inheritance/interfaces instead of type checking.
- **When**: Multiple if-else checking object types
- **Benefit**: Extensibility, reduced complexity

### 3. Introduce Parameter Object
Group related parameters into a struct/class.
- **When**: Function has > 4 parameters
- **Benefit**: Clearer intent, easier to extend

### 4. Replace Nested Conditional with Guard Clauses
Use early returns for error conditions.
- **When**: Deep nesting (> 3 levels)
- **Benefit**: Flatter code, easier to read

### 5. Decompose Conditional
Extract complex conditions into named methods.
- **When**: Boolean expressions with multiple && or ||
- **Benefit**: Self-documenting code

### 6. Replace Loop with Algorithm
Use STL algorithms instead of manual loops.
- **When**: Simple transformations or searches
- **Benefit**: Less code, clearer intent

## Implementation Plan

### Phase 1: Critical Issues (Week 1)
Focus on functions with complexity > 25 or lines > 150
- Estimated effort: $(($Issues | Where-Object { $_.Priority -eq "Critical" }).Count * 6) hours
- Expected quality improvement: +2 points

### Phase 2: High Priority (Week 2-3)
Address functions with complexity 20-25 or lines 100-150
- Estimated effort: $(($Issues | Where-Object { $_.Priority -eq "High" }).Count * 3) hours
- Expected quality improvement: +1 point

### Phase 3: Medium Priority (Week 4)
Improve functions with complexity 15-20 or lines 80-100
- Estimated effort: $(($Issues | Where-Object { $_.Priority -eq "Medium" }).Count * 1.5) hours
- Expected quality improvement: +0.5 points

### Total Expected Improvement: +3.5 Quality Points

## Quality Metrics Targets

| Metric | Current | Target | Status |
|--------|---------|--------|--------|
| Avg Complexity | $([Math]::Round(($Issues | Measure-Object -Property Complexity -Average).Average, 1)) | < 10 | ⚠️ Needs Work |
| Max Complexity | $(($Issues | Measure-Object -Property Complexity -Maximum).Maximum) | < 20 | ⚠️ Needs Work |
| Avg Function Length | $([Math]::Round(($Issues | Measure-Object -Property Lines -Average).Average, 1)) | < 50 | ⚠️ Needs Work |
| Max Nesting Depth | $(($Issues | Measure-Object -Property MaxNesting -Maximum).Maximum) | < 4 | ⚠️ Needs Work |

## Next Steps

1. **Review this report** with the development team
2. **Create GitHub issues** for each critical and high-priority item
3. **Schedule refactoring sessions** - dedicate 2-3 hours per week
4. **Implement automated checks** in pre-commit hooks
5. **Track progress** weekly using this analyzer
6. **Celebrate wins** - acknowledge complexity reductions

## Automation Recommendations

Add to pre-commit hook:
```powershell
# Reject commits with functions > complexity 25
.\Scripts\Quality\Improvement\CodeComplexityAnalyzer.ps1 -ComplexityThreshold 25
if ($LASTEXITCODE -ne 0) {
    Write-Error "Code complexity too high. Please refactor before committing."
    exit 1
}
```

---
*Generated by CodeComplexityAnalyzer.ps1*
"@

# Ensure output directory exists
$outputDir = Split-Path $OutputFile -Parent
if (!(Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

$report | Out-File $OutputFile -Encoding UTF8

Write-Host "`nAnalysis Complete!" -ForegroundColor Green
Write-Host "Found $($Issues.Count) functions requiring attention" -ForegroundColor Yellow
Write-Host "Critical: $(($Issues | Where-Object { $_.Priority -eq "Critical" }).Count), High: $(($Issues | Where-Object { $_.Priority -eq "High" }).Count), Medium: $(($Issues | Where-Object { $_.Priority -eq "Medium" }).Count)" -ForegroundColor Cyan
Write-Host "Report saved to: $OutputFile" -ForegroundColor Cyan
