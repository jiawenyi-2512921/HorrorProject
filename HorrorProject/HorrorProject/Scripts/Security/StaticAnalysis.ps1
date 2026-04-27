# Static Code Analysis for HorrorProject
# Scans C++ code for common security vulnerabilities

param(
    [string]$ProjectRoot = "",
    [string]$OutputFile = "SecurityAnalysis.json",
    [switch]$IncludeTestIssues
)

$ErrorActionPreference = "Stop"

. (Join-Path (Split-Path -Parent $PSScriptRoot) "Validation\Common.ps1")

if ([string]::IsNullOrWhiteSpace($ProjectRoot)) {
    $ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
} else {
    $ProjectRoot = (Resolve-Path -LiteralPath $ProjectRoot).Path
}

Write-Host "Starting Static Security Analysis..." -ForegroundColor Cyan

$results = @{
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    TotalFiles = 0
    Issues = @()
    Summary = @{
        Critical = 0
        High = 0
        Medium = 0
        Low = 0
        SuppressedTestIssues = 0
        SuppressedLiteralPrintf = 0
    }
}

# Security patterns to check
$patterns = @(
    @{
        Name = "Unchecked Pointer Dereference"
        Pattern = '(GetWorld|GetOwner|FindComponentByClass|GetComponent)\s*\([^)]*\)->'
        Severity = "High"
        Description = "Potential null pointer dereference without validation"
    },
    @{
        Name = "Unsafe Cast Without Validation"
        Pattern = 'Cast<[^>]+>\([^)]+\)->'
        Severity = "Medium"
        Description = "Cast result used without null check"
    },
    @{
        Name = "Array Access Without Bounds Check"
        Pattern = '\b[A-Za-z_]\w*\s*\[[^\]]+\](?!\s*=)'
        Severity = "Medium"
        Description = "Array access without explicit bounds validation"
    },
    @{
        Name = "ProcessEvent Call"
        Pattern = '->ProcessEvent\s*\('
        Severity = "Medium"
        Description = "Dynamic function invocation - ensure parameters are validated"
    },
    @{
        Name = "FString::Printf Usage"
        Pattern = 'FString::Printf\s*\('
        Severity = "Low"
        Description = "Format string usage - ensure format specifiers match arguments"
    }
)

function Get-SanitizedCodeLine {
    param([string]$Line)

    $sanitized = [regex]::Replace($Line, '"(?:\\.|[^"\\])*"', '""')
    $sanitized = [regex]::Replace($sanitized, "'(?:\\.|[^'\\])+'", "''")
    $sanitized = [regex]::Replace($sanitized, '/\*.*?\*/', '')
    $sanitized = [regex]::Replace($sanitized, '//.*$', '')
    return $sanitized
}

function Test-IsTestSourceFile {
    param([string]$Path)

    return $Path -match '\\Tests\\' -or
        $Path -match '\\Tests?\\' -or
        $Path -match '\\Test[^\\]*\.cpp$' -or
        $Path -match 'Tests\.cpp$'
}

function Get-AnalysisContext {
    param(
        [string[]]$Lines,
        [int]$LineIndex,
        [int]$Before = 10
    )

    $start = [Math]::Max(0, $LineIndex - $Before)
    $length = $LineIndex - $start + 1
    return ($Lines[$start..($start + $length - 1)] -join "`n")
}

function Test-IsLiteralPrintfCall {
    param(
        [string[]]$SanitizedLines,
        [int]$LineIndex
    )

    $end = [Math]::Min($SanitizedLines.Count - 1, $LineIndex + 3)
    $context = ($SanitizedLines[$LineIndex..$end] -join " ")
    return $context -match 'FString::Printf\s*\(\s*TEXT\s*\(\s*""\s*\)' -or
        $context -match 'FString::Printf\s*\(\s*""'
}

function Get-AutomationTestLineMask {
    param([string[]]$Lines)

    $mask = @()
    $automationDepth = 0
    foreach ($line in $Lines) {
        if ($line -match '^\s*#\s*if.*WITH_DEV_AUTOMATION_TESTS') {
            $automationDepth++
        }

        $mask += ($automationDepth -gt 0)

        if ($line -match '^\s*#\s*endif' -and $automationDepth -gt 0) {
            $automationDepth--
        }
    }

    return $mask
}

function Test-ShouldSkipIssue {
    param(
        [string]$PatternName,
        [string]$SanitizedLine,
        [string]$MatchValue,
        [string[]]$SanitizedLines,
        [int]$LineIndex
    )

    if ([string]::IsNullOrWhiteSpace($SanitizedLine)) {
        return $true
    }

    if ($PatternName -eq "Array Access Without Bounds Check") {
        if ($SanitizedLine -match '^\s*(static\s+)?(const\s+)?[\w:<>,\s\*&]+[A-Za-z_]\w*\s*\[\]\s*[=;]') {
            return $true
        }

        if ($SanitizedLine -match '\.IsValidIndex\s*\(') {
            return $true
        }

        if ($MatchValue -match '\.M\s*\[\s*[0-3]\s*\]') {
            return $true
        }
    }

    if ($PatternName -eq "ProcessEvent Call") {
        $context = Get-AnalysisContext -Lines $SanitizedLines -LineIndex $LineIndex -Before 16
        if ($context -match 'CanInvokeActivationFunction\s*\(' -or
            $context -match 'FindValidatedLegacyInteractionFunction\s*\(') {
            return $true
        }
    }

    return $false
}

# Scan source files
$sourceFiles = Get-ChildItem -Path "$ProjectRoot\Source" -Recurse -Include *.cpp,*.h | Where-Object { $_.FullName -notmatch "\\Intermediate\\" }
$results.TotalFiles = $sourceFiles.Count

Write-Host "Scanning $($sourceFiles.Count) files..." -ForegroundColor Yellow

foreach ($file in $sourceFiles) {
    $lines = Get-Content $file.FullName
    $sanitizedLines = @($lines | ForEach-Object { Get-SanitizedCodeLine -Line $_ })
    $automationTestLineMask = @(Get-AutomationTestLineMask -Lines $sanitizedLines)
    $isTestFile = Test-IsTestSourceFile -Path $file.FullName

    foreach ($pattern in $patterns) {
        for ($lineIndex = 0; $lineIndex -lt $sanitizedLines.Count; $lineIndex++) {
            $line = $sanitizedLines[$lineIndex]
            $matches = [regex]::Matches($line, $pattern.Pattern)

            foreach ($match in $matches) {
                if (Test-ShouldSkipIssue -PatternName $pattern.Name -SanitizedLine $line -MatchValue $match.Value -SanitizedLines $sanitizedLines -LineIndex $lineIndex) {
                    continue
                }

                $severity = $pattern.Severity
                $description = $pattern.Description
                $isTestOnlyLine = $isTestFile -or ($lineIndex -lt $automationTestLineMask.Count -and $automationTestLineMask[$lineIndex])
                if ($isTestOnlyLine -and -not $IncludeTestIssues) {
                    $results.Summary.SuppressedTestIssues++
                    continue
                }

                if ($pattern.Name -eq "FString::Printf Usage" -and
                    (Test-IsLiteralPrintfCall -SanitizedLines $sanitizedLines -LineIndex $lineIndex)) {
                    $results.Summary.SuppressedLiteralPrintf++
                    continue
                }

                if ($pattern.Name -eq "Array Access Without Bounds Check" -and $isTestOnlyLine) {
                    $severity = "Low"
                    $description = "Test-only array access without explicit bounds validation"
                }

                $issue = @{
                    File = $file.FullName.Replace($ProjectRoot, "")
                    Line = $lineIndex + 1
                    Pattern = $pattern.Name
                    Severity = $severity
                    Description = $description
                    Code = $match.Value
                }

                $results.Issues += $issue
                $results.Summary[$severity]++
            }
        }
    }
}

Write-Host "`nAnalysis Complete!" -ForegroundColor Green
Write-Host "Critical: $($results.Summary.Critical)" -ForegroundColor Red
Write-Host "High: $($results.Summary.High)" -ForegroundColor Magenta
Write-Host "Medium: $($results.Summary.Medium)" -ForegroundColor Yellow
Write-Host "Low: $($results.Summary.Low)" -ForegroundColor Cyan
Write-Host "Suppressed test issues: $($results.Summary.SuppressedTestIssues)" -ForegroundColor DarkGray
Write-Host "Suppressed literal FString::Printf checks: $($results.Summary.SuppressedLiteralPrintf)" -ForegroundColor DarkGray

# Save results
$outputPath = Join-Path $ProjectRoot "Docs\Security\$OutputFile"
$outputDir = Split-Path $outputPath -Parent
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

$results | ConvertTo-Json -Depth 10 | Out-File $outputPath -Encoding UTF8
Write-Host "`nResults saved to: $outputPath" -ForegroundColor Green
