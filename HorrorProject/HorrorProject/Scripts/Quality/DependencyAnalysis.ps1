# DependencyAnalysis.ps1 - Analyze code dependencies and coupling
# Detects circular dependencies, tight coupling, and dependency issues

param(
    [string]$SourcePath = "Source",
    [string]$OutputFile = "Docs/Quality/DependencyReport.md"
)

$Dependencies = @{}
$Issues = @()

function Add-Issue {
    param($File, $Severity, $Category, $Message)
    $script:Issues += [PSCustomObject]@{
        File = $File
        Severity = $Severity
        Category = $Category
        Message = $Message
    }
}

function Extract-Includes {
    param($FilePath, $Content)

    $includes = @()
    $lines = $Content -split "`n"

    foreach ($line in $lines) {
        if ($line -match '#include\s+"([^"]+)"') {
            $includes += $Matches[1]
        }
    }

    return $includes
}

function Calculate-Coupling {
    param($FilePath)

    $includes = $Dependencies[$FilePath]
    if (-not $includes) { return 0 }

    # Efferent coupling (outgoing dependencies)
    return $includes.Count
}

function Detect-CircularDependencies {
    $visited = @{}
    $recursionStack = @{}
    $cycles = @()

    function DFS {
        param($node, $path)

        if ($recursionStack[$node]) {
            $cycleStart = $path.IndexOf($node)
            if ($cycleStart -ge 0) {
                $cycle = $path[$cycleStart..($path.Count-1)] + @($node)
                $cycles += ,@($cycle)
            }
            return
        }

        if ($visited[$node]) { return }

        $visited[$node] = $true
        $recursionStack[$node] = $true

        if ($Dependencies[$node]) {
            foreach ($dep in $Dependencies[$node]) {
                $depPath = Join-Path (Split-Path $node) $dep
                if ($Dependencies.ContainsKey($depPath)) {
                    DFS $depPath ($path + @($node))
                }
            }
        }

        $recursionStack[$node] = $false
    }

    foreach ($file in $Dependencies.Keys) {
        if (-not $visited[$file]) {
            DFS $file @()
        }
    }

    return $cycles
}

function Analyze-ModuleCoupling {
    $modules = @{}

    foreach ($file in $Dependencies.Keys) {
        # Extract module name (e.g., AI, Player, Game)
        if ($file -match 'Source[\\/]HorrorProject[\\/]([^\\/]+)') {
            $module = $Matches[1]

            if (-not $modules[$module]) {
                $modules[$module] = @{
                    Files = @()
                    InternalDeps = 0
                    ExternalDeps = 0
                    DependsOn = @{}
                }
            }

            $modules[$module].Files += $file

            foreach ($dep in $Dependencies[$file]) {
                if ($dep -match '([^\\/]+)[\\/]') {
                    $depModule = $Matches[1]

                    if ($depModule -eq $module) {
                        $modules[$module].InternalDeps++
                    } else {
                        $modules[$module].ExternalDeps++
                        if (-not $modules[$module].DependsOn[$depModule]) {
                            $modules[$module].DependsOn[$depModule] = 0
                        }
                        $modules[$module].DependsOn[$depModule]++
                    }
                }
            }
        }
    }

    return $modules
}

function Check-DependencyIssues {
    param($FilePath, $Includes)

    # Check for excessive dependencies
    if ($Includes.Count -gt 15) {
        Add-Issue $FilePath "Warning" "Coupling" "Excessive dependencies ($($Includes.Count) includes)"
    }

    # Check for cross-module dependencies
    $fileModule = ""
    if ($FilePath -match 'Source[\\/]HorrorProject[\\/]([^\\/]+)') {
        $fileModule = $Matches[1]
    }

    foreach ($inc in $Includes) {
        # Check for including implementation files
        if ($inc -match '\.cpp$') {
            Add-Issue $FilePath "Error" "Dependency" "Including .cpp file: $inc"
        }

        # Check for cross-module dependencies
        if ($inc -match '([^\\/]+)[\\/]' -and $Matches[1] -ne $fileModule) {
            $depModule = $Matches[1]
            if ($depModule -notin @('Public', 'Private', 'Classes')) {
                # This is acceptable for cross-module dependencies
            }
        }

        # Check for platform-specific includes
        if ($inc -match '(Windows|Linux|Mac|Android|IOS)\.h') {
            Add-Issue $FilePath "Warning" "Portability" "Platform-specific include: $inc"
        }
    }
}

# Main execution
Write-Host "Starting dependency analysis..." -ForegroundColor Cyan

$files = Get-ChildItem -Path $SourcePath -Recurse -Include *.cpp,*.h |
    Where-Object { $_.FullName -notmatch '\\Intermediate\\' }

# Build dependency graph
foreach ($file in $files) {
    $content = Get-Content $file.FullName -Raw
    $includes = Extract-Includes $file.FullName $content
    $Dependencies[$file.FullName] = $includes

    Check-DependencyIssues $file.FullName $includes
}

# Detect circular dependencies
Write-Host "Detecting circular dependencies..." -ForegroundColor Gray
$cycles = Detect-CircularDependencies

foreach ($cycle in $cycles) {
    $cycleStr = ($cycle | ForEach-Object { Split-Path $_ -Leaf }) -join " -> "
    Add-Issue $cycle[0] "Error" "CircularDependency" "Circular dependency: $cycleStr"
}

# Analyze module coupling
Write-Host "Analyzing module coupling..." -ForegroundColor Gray
$modules = Analyze-ModuleCoupling

# Calculate metrics
$totalFiles = $files.Count
$avgCoupling = if ($Dependencies.Count -gt 0) {
    [Math]::Round(($Dependencies.Values | ForEach-Object { $_.Count } | Measure-Object -Average).Average, 1)
} else { 0 }

$highCoupling = ($Dependencies.GetEnumerator() | Where-Object { $_.Value.Count -gt 15 }).Count

# Generate report
$report = @"
# Dependency Analysis Report
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## Summary
- Files Analyzed: $totalFiles
- Average Coupling: $avgCoupling dependencies per file
- High Coupling Files: $highCoupling (>15 dependencies)
- Circular Dependencies: $($cycles.Count)
- Issues Found: $($Issues.Count)

## Module Coupling Analysis

| Module | Files | Internal Deps | External Deps | Coupling Ratio |
|--------|-------|---------------|---------------|----------------|
"@

foreach ($module in $modules.GetEnumerator() | Sort-Object Name) {
    $name = $module.Key
    $data = $module.Value
    $total = $data.InternalDeps + $data.ExternalDeps
    $ratio = if ($total -gt 0) { [Math]::Round(($data.ExternalDeps / $total) * 100, 1) } else { 0 }

    $report += "| $name | $($data.Files.Count) | $($data.InternalDeps) | $($data.ExternalDeps) | $ratio% |`n"
}

$report += @"

## Module Dependencies

"@

foreach ($module in $modules.GetEnumerator() | Sort-Object Name) {
    $name = $module.Key
    $data = $module.Value

    if ($data.DependsOn.Count -gt 0) {
        $report += "### $name depends on:`n"
        foreach ($dep in $data.DependsOn.GetEnumerator() | Sort-Object Value -Descending) {
            $report += "- $($dep.Key): $($dep.Value) references`n"
        }
        $report += "`n"
    }
}

$report += @"

## Circular Dependencies
"@

if ($cycles.Count -gt 0) {
    $report += "`nCircular dependencies detected (CRITICAL):`n`n"
    foreach ($cycle in $cycles) {
        $cycleStr = ($cycle | ForEach-Object { Split-Path $_ -Leaf }) -join " -> "
        $report += "- $cycleStr`n"
    }
} else {
    $report += "`nNo circular dependencies detected.`n"
}

$report += @"

## High Coupling Files (Top 20)
Files with excessive dependencies:

| File | Dependencies | Risk |
|------|--------------|------|
"@

$topCoupled = $Dependencies.GetEnumerator() | Sort-Object { $_.Value.Count } -Descending | Select-Object -First 20
foreach ($entry in $topCoupled) {
    $shortFile = Split-Path $entry.Key -Leaf
    $count = $entry.Value.Count
    $risk = if ($count -gt 20) { "High" } elseif ($count -gt 15) { "Medium" } else { "Low" }
    $report += "| $shortFile | $count | $risk |`n"
}

$report += @"

## Issues by Category

"@

$byCategory = $Issues | Group-Object Category | Sort-Object Count -Descending
foreach ($group in $byCategory) {
    $report += "### $($group.Name) ($($group.Count) issues)`n`n"
    foreach ($issue in $group.Group | Select-Object -First 10) {
        $shortFile = Split-Path $issue.File -Leaf
        $report += "- **$($issue.Severity)** [$shortFile] $($issue.Message)`n"
    }
    if ($group.Count -gt 10) {
        $report += "- ... and $($group.Count - 10) more`n"
    }
    $report += "`n"
}

$report += @"

## Recommendations

### Critical (Immediate Action Required)
1. **Resolve circular dependencies** - Break cycles by introducing interfaces or dependency inversion
2. **Fix .cpp includes** - Never include implementation files

### High Priority
1. **Reduce high coupling files** - Files with >15 dependencies should be refactored
2. **Improve module boundaries** - Modules with >50% external coupling need better encapsulation
3. **Remove platform-specific dependencies** - Use abstraction layers

### Medium Priority
1. **Balance module dependencies** - Avoid one-way dependencies where possible
2. **Extract common interfaces** - Reduce direct dependencies between modules
3. **Apply dependency inversion** - High-level modules should not depend on low-level modules

### Best Practices
1. **Keep coupling low** - Target <10 dependencies per file
2. **Favor composition over inheritance** - Reduces tight coupling
3. **Use interfaces** - Define clear contracts between modules
4. **Apply SOLID principles** - Especially Dependency Inversion Principle
5. **Regular refactoring** - Continuously improve module boundaries

## Coupling Guidelines
- **0-5**: Excellent, loosely coupled
- **6-10**: Good, acceptable coupling
- **11-15**: Moderate, monitor for growth
- **16-20**: High, refactoring recommended
- **20+**: Very high, refactoring required
"@

$report | Out-File $OutputFile -Encoding UTF8

Write-Host "`nDependency analysis complete!" -ForegroundColor Green
Write-Host "Circular Dependencies: $($cycles.Count), High Coupling: $highCoupling" -ForegroundColor $(if($cycles.Count -gt 0){"Red"}elseif($highCoupling -gt 5){"Yellow"}else{"Green"})
Write-Host "Average Coupling: $avgCoupling dependencies per file" -ForegroundColor Cyan
Write-Host "Report saved to: $OutputFile" -ForegroundColor Cyan
