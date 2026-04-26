# Static Code Analysis for HorrorProject
# Scans C++ code for common security vulnerabilities

param(
    [string]$ProjectRoot = "D:\gptzuo\HorrorProject\HorrorProject",
    [string]$OutputFile = "SecurityAnalysis.json"
)

$ErrorActionPreference = "Stop"

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
        Pattern = '\[[^\]]+\](?!\s*=)'
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

# Scan source files
$sourceFiles = Get-ChildItem -Path "$ProjectRoot\Source" -Recurse -Include *.cpp,*.h | Where-Object { $_.FullName -notmatch "\\Intermediate\\" }
$results.TotalFiles = $sourceFiles.Count

Write-Host "Scanning $($sourceFiles.Count) files..." -ForegroundColor Yellow

foreach ($file in $sourceFiles) {
    $content = Get-Content $file.FullName -Raw
    $lines = Get-Content $file.FullName

    foreach ($pattern in $patterns) {
        $matches = [regex]::Matches($content, $pattern.Pattern)

        foreach ($match in $matches) {
            # Find line number
            $lineNum = 1
            $charCount = 0
            foreach ($line in $lines) {
                $charCount += $line.Length + 2
                if ($charCount -ge $match.Index) {
                    break
                }
                $lineNum++
            }

            $issue = @{
                File = $file.FullName.Replace($ProjectRoot, "")
                Line = $lineNum
                Pattern = $pattern.Name
                Severity = $pattern.Severity
                Description = $pattern.Description
                Code = $match.Value
            }

            $results.Issues += $issue
            $results.Summary[$pattern.Severity]++
        }
    }
}

Write-Host "`nAnalysis Complete!" -ForegroundColor Green
Write-Host "Critical: $($results.Summary.Critical)" -ForegroundColor Red
Write-Host "High: $($results.Summary.High)" -ForegroundColor Magenta
Write-Host "Medium: $($results.Summary.Medium)" -ForegroundColor Yellow
Write-Host "Low: $($results.Summary.Low)" -ForegroundColor Cyan

# Save results
$outputPath = Join-Path $ProjectRoot "Docs\Security\$OutputFile"
$outputDir = Split-Path $outputPath -Parent
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

$results | ConvertTo-Json -Depth 10 | Out-File $outputPath -Encoding UTF8
Write-Host "`nResults saved to: $outputPath" -ForegroundColor Green
