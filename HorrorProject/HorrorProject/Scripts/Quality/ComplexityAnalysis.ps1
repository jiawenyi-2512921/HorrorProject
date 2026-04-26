# ComplexityAnalysis.ps1 - Cyclomatic Complexity and Code Metrics
# Analyzes code complexity and maintainability

param(
    [string]$SourcePath = "Source",
    [string]$OutputFile = "Docs/Quality/ComplexityReport.md"
)

$Metrics = @()

function Calculate-CyclomaticComplexity {
    param($Content)

    $complexity = 1  # Base complexity

    # Count decision points
    $complexity += ([regex]::Matches($Content, '\bif\s*\(')).Count
    $complexity += ([regex]::Matches($Content, '\belse\s+if\s*\(')).Count
    $complexity += ([regex]::Matches($Content, '\bfor\s*\(')).Count
    $complexity += ([regex]::Matches($Content, '\bwhile\s*\(')).Count
    $complexity += ([regex]::Matches($Content, '\bcase\s+')).Count
    $complexity += ([regex]::Matches($Content, '\bcatch\s*\(')).Count
    $complexity += ([regex]::Matches($Content, '\b&&\b')).Count
    $complexity += ([regex]::Matches($Content, '\b\|\|\b')).Count
    $complexity += ([regex]::Matches($Content, '\?.*:')).Count

    return $complexity
}

function Analyze-Function {
    param($FilePath, $Content, $FunctionName, $StartLine, $EndLine)

    $functionContent = ($Content -split "`n")[$StartLine..$EndLine] -join "`n"
    $lineCount = $EndLine - $StartLine + 1

    $complexity = Calculate-CyclomaticComplexity $functionContent

    # Count parameters
    $paramMatch = [regex]::Match($functionContent, '\(([^)]*)\)')
    $paramCount = if ($paramMatch.Success) {
        ($paramMatch.Groups[1].Value -split ',').Count
    } else { 0 }

    # Count local variables
    $varCount = ([regex]::Matches($functionContent, '^\s+(int|float|bool|F\w+|U\w+|A\w+|T\w+)\s+\w+\s*[=;]', [System.Text.RegularExpressions.RegexOptions]::Multiline)).Count

    # Calculate maintainability index (simplified)
    $maintainability = [Math]::Max(0, 100 - ($complexity * 2) - ($lineCount / 10) - ($paramCount * 5))

    $script:Metrics += [PSCustomObject]@{
        File = $FilePath
        Function = $FunctionName
        StartLine = $StartLine + 1
        Lines = $lineCount
        Complexity = $complexity
        Parameters = $paramCount
        Variables = $varCount
        Maintainability = [Math]::Round($maintainability, 1)
        Risk = if ($complexity -gt 20) { "High" } elseif ($complexity -gt 10) { "Medium" } else { "Low" }
    }
}

function Parse-Functions {
    param($FilePath, $Content)

    $lines = $Content -split "`n"
    $inFunction = $false
    $functionName = ""
    $functionStart = 0
    $braceCount = 0

    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]

        # Match function definition
        if ($line -match '^\s*(\w+(?:::\w+)?)\s*\([^)]*\)\s*(?:const)?\s*$' -or
            $line -match '^\s*(\w+(?:::\w+)?)\s*\([^)]*\)\s*(?:const)?\s*\{') {

            if (-not $inFunction) {
                $functionName = $Matches[1]
                $functionStart = $i
                $inFunction = $true
                $braceCount = 0
            }
        }

        if ($inFunction) {
            $braceCount += ($line.ToCharArray() | Where-Object { $_ -eq '{' }).Count
            $braceCount -= ($line.ToCharArray() | Where-Object { $_ -eq '}' }).Count

            if ($braceCount -eq 0 -and $line -match '}') {
                Analyze-Function $FilePath $Content $functionName $functionStart $i
                $inFunction = $false
            }
        }
    }
}

function Calculate-FileMetrics {
    param($FilePath, $Content)

    $lines = $Content -split "`n"
    $codeLines = ($lines | Where-Object { $_ -match '\S' -and $_ -notmatch '^\s*//' }).Count
    $commentLines = ($lines | Where-Object { $_ -match '^\s*//' }).Count
    $blankLines = ($lines | Where-Object { $_ -notmatch '\S' }).Count

    $classCount = ([regex]::Matches($Content, '^\s*class\s+\w+', [System.Text.RegularExpressions.RegexOptions]::Multiline)).Count
    $functionCount = ([regex]::Matches($Content, '^\s*\w+(?:::\w+)?\s*\([^)]*\)\s*(?:const)?\s*\{?', [System.Text.RegularExpressions.RegexOptions]::Multiline)).Count

    return [PSCustomObject]@{
        File = $FilePath
        TotalLines = $lines.Count
        CodeLines = $codeLines
        CommentLines = $commentLines
        BlankLines = $blankLines
        Classes = $classCount
        Functions = $functionCount
        CommentRatio = if ($codeLines -gt 0) { [Math]::Round(($commentLines / $codeLines) * 100, 1) } else { 0 }
    }
}

# Main execution
Write-Host "Starting complexity analysis..." -ForegroundColor Cyan

$files = Get-ChildItem -Path $SourcePath -Recurse -Include *.cpp,*.h |
    Where-Object { $_.FullName -notmatch '\\Intermediate\\' -and $_.FullName -notmatch '\\Tests\\' }

$fileMetrics = @()

foreach ($file in $files) {
    Write-Host "Analyzing: $($file.Name)" -ForegroundColor Gray
    $content = Get-Content $file.FullName -Raw

    Parse-Functions $file.FullName $content
    $fileMetrics += Calculate-FileMetrics $file.FullName $content
}

# Generate report
$highRisk = ($Metrics | Where-Object { $_.Risk -eq "High" }).Count
$mediumRisk = ($Metrics | Where-Object { $_.Risk -eq "Medium" }).Count
$lowRisk = ($Metrics | Where-Object { $_.Risk -eq "Low" }).Count

$avgComplexity = if ($Metrics.Count -gt 0) { [Math]::Round(($Metrics | Measure-Object -Property Complexity -Average).Average, 1) } else { 0 }
$avgLines = if ($Metrics.Count -gt 0) { [Math]::Round(($Metrics | Measure-Object -Property Lines -Average).Average, 1) } else { 0 }
$avgMaintainability = if ($Metrics.Count -gt 0) { [Math]::Round(($Metrics | Measure-Object -Property Maintainability -Average).Average, 1) } else { 0 }

$totalCode = ($fileMetrics | Measure-Object -Property CodeLines -Sum).Sum
$totalComments = ($fileMetrics | Measure-Object -Property CommentLines -Sum).Sum
$overallCommentRatio = if ($totalCode -gt 0) { [Math]::Round(($totalComments / $totalCode) * 100, 1) } else { 0 }

$report = @"
# Complexity Analysis Report
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

## Summary
- Files Analyzed: $($files.Count)
- Functions Analyzed: $($Metrics.Count)
- Average Complexity: $avgComplexity
- Average Function Length: $avgLines lines
- Average Maintainability: $avgMaintainability/100
- Overall Comment Ratio: $overallCommentRatio%

## Risk Distribution
- High Risk (Complexity > 20): $highRisk functions
- Medium Risk (Complexity 10-20): $mediumRisk functions
- Low Risk (Complexity < 10): $lowRisk functions

## High Risk Functions (Top 20)
These functions require immediate refactoring:

| Function | File | Lines | Complexity | Maintainability | Risk |
|----------|------|-------|------------|-----------------|------|
"@

$topComplex = $Metrics | Sort-Object Complexity -Descending | Select-Object -First 20
foreach ($metric in $topComplex) {
    $shortFile = Split-Path $metric.File -Leaf
    $report += "| $($metric.Function) | $shortFile | $($metric.Lines) | $($metric.Complexity) | $($metric.Maintainability) | $($metric.Risk) |`n"
}

$report += @"

## Longest Functions (Top 20)
Functions exceeding 50 lines should be refactored:

| Function | File | Lines | Complexity | Maintainability |
|----------|------|-------|------------|-----------------|
"@

$topLong = $Metrics | Sort-Object Lines -Descending | Select-Object -First 20
foreach ($metric in $topLong) {
    $shortFile = Split-Path $metric.File -Leaf
    $report += "| $($metric.Function) | $shortFile | $($metric.Lines) | $($metric.Complexity) | $($metric.Maintainability) |`n"
}

$report += @"

## File Metrics
Overview of code distribution:

| File | Lines | Code | Comments | Ratio | Functions |
|------|-------|------|----------|-------|-----------|
"@

foreach ($fm in $fileMetrics | Sort-Object CodeLines -Descending | Select-Object -First 30) {
    $shortFile = Split-Path $fm.File -Leaf
    $report += "| $shortFile | $($fm.TotalLines) | $($fm.CodeLines) | $($fm.CommentLines) | $($fm.CommentRatio)% | $($fm.Functions) |`n"
}

$report += @"

## Recommendations

### Immediate Actions (High Priority)
1. Refactor functions with complexity > 20
2. Break down functions exceeding 100 lines
3. Add documentation to functions with maintainability < 40

### Medium Priority
1. Improve comment ratio in files below 10%
2. Reduce parameter count in functions with > 5 parameters
3. Extract complex conditional logic into helper functions

### Long Term
1. Maintain average complexity below 10
2. Keep functions under 50 lines
3. Target maintainability index above 70
4. Maintain comment ratio above 20%

## Complexity Guidelines
- **1-10**: Simple, easy to maintain
- **11-20**: Moderate complexity, acceptable
- **21-50**: High complexity, refactor recommended
- **50+**: Very high complexity, refactor required

## Maintainability Index
- **85-100**: Excellent maintainability
- **65-84**: Good maintainability
- **40-64**: Moderate maintainability
- **0-39**: Difficult to maintain
"@

$report | Out-File $OutputFile -Encoding UTF8

Write-Host "`nComplexity analysis complete!" -ForegroundColor Green
Write-Host "High Risk: $highRisk, Medium Risk: $mediumRisk, Low Risk: $lowRisk" -ForegroundColor $(if($highRisk -gt 0){"Red"}elseif($mediumRisk -gt 10){"Yellow"}else{"Green"})
Write-Host "Average Complexity: $avgComplexity, Average Maintainability: $avgMaintainability" -ForegroundColor Cyan
Write-Host "Report saved to: $OutputFile" -ForegroundColor Cyan
