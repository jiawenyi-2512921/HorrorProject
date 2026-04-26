# Input Validation Analysis for HorrorProject
# Checks for missing input validation and sanitization

param(
    [string]$ProjectRoot = "D:\gptzuo\HorrorProject\HorrorProject",
    [string]$OutputFile = "InputValidationReport.json"
)

$ErrorActionPreference = "Stop"

Write-Host "Starting Input Validation Analysis..." -ForegroundColor Cyan

$results = @{
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Issues = @()
    Summary = @{
        MissingValidation = 0
        UnsafeInput = 0
        NoSanitization = 0
    }
}

# Input validation patterns
$validationPatterns = @(
    @{
        Name = "FName Parameter Without None Check"
        Pattern = 'UFUNCTION[^}]+FName\s+\w+[^}]+\{'
        Category = "MissingValidation"
        Risk = "Medium"
        Check = '\.IsNone\(\)'
    },
    @{
        Name = "Array Parameter Without Size Check"
        Pattern = 'UFUNCTION[^}]+TArray<[^>]+>&?\s+\w+[^}]+\{'
        Category = "MissingValidation"
        Risk = "Medium"
        Check = '\.Num\(\)'
    },
    @{
        Name = "Pointer Parameter Without Null Check"
        Pattern = 'UFUNCTION[^}]+\w+\*\s+\w+[^}]+\{'
        Category = "MissingValidation"
        Risk = "High"
        Check = 'if\s*\(.*\)'
    },
    @{
        Name = "String Input Without Validation"
        Pattern = 'FString\s+\w+\s*=.*Input|User|External'
        Category = "UnsafeInput"
        Risk = "Medium"
    }
)

$sourceFiles = Get-ChildItem -Path "$ProjectRoot\Source" -Recurse -Include *.cpp,*.h | Where-Object { $_.FullName -notmatch "\\Intermediate\\" }

Write-Host "Analyzing $($sourceFiles.Count) files for input validation..." -ForegroundColor Yellow

foreach ($file in $sourceFiles) {
    $content = Get-Content $file.FullName -Raw

    foreach ($pattern in $validationPatterns) {
        $matches = [regex]::Matches($content, $pattern.Pattern, [System.Text.RegularExpressions.RegexOptions]::Singleline)

        foreach ($match in $matches) {
            # Check if validation exists nearby
            $hasValidation = $false
            if ($pattern.Check) {
                $contextStart = [Math]::Max(0, $match.Index)
                $contextEnd = [Math]::Min($content.Length, $match.Index + $match.Length + 500)
                $context = $content.Substring($contextStart, $contextEnd - $contextStart)
                $hasValidation = $context -match $pattern.Check
            }

            if (-not $hasValidation) {
                $issue = @{
                    File = $file.FullName.Replace($ProjectRoot, "")
                    Issue = $pattern.Name
                    Category = $pattern.Category
                    Risk = $pattern.Risk
                    Recommendation = "Add validation check: $($pattern.Check)"
                }

                $results.Issues += $issue
                $results.Summary[$pattern.Category]++
            }
        }
    }
}

Write-Host "`nInput Validation Analysis Complete!" -ForegroundColor Green
Write-Host "Missing Validation: $($results.Summary.MissingValidation)" -ForegroundColor Yellow
Write-Host "Unsafe Input: $($results.Summary.UnsafeInput)" -ForegroundColor Magenta
Write-Host "No Sanitization: $($results.Summary.NoSanitization)" -ForegroundColor Cyan

$outputPath = Join-Path $ProjectRoot "Docs\Security\$OutputFile"
$outputDir = Split-Path $outputPath -Parent
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

$results | ConvertTo-Json -Depth 10 | Out-File $outputPath -Encoding UTF8
Write-Host "`nResults saved to: $outputPath" -ForegroundColor Green
