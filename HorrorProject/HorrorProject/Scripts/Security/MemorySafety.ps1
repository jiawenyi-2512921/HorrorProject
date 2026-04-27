# Memory Safety Analysis for HorrorProject
# Checks for memory leaks, buffer overflows, and unsafe memory operations

param(
    [string]$ProjectRoot = "",
    [string]$OutputFile = "MemorySafetyReport.json"
)

$ErrorActionPreference = "Stop"

. (Join-Path (Split-Path -Parent $PSScriptRoot) "Validation\Common.ps1")

if ([string]::IsNullOrWhiteSpace($ProjectRoot)) {
    $ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
} else {
    $ProjectRoot = (Resolve-Path -LiteralPath $ProjectRoot).Path
}

Write-Host "Starting Memory Safety Analysis..." -ForegroundColor Cyan

$results = @{
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Issues = @()
    Summary = @{
        BufferOverflow = 0
        MemoryLeak = 0
        UseAfterFree = 0
        NullPointer = 0
    }
}

# Memory safety patterns
$memoryPatterns = @(
    @{
        Name = "Potential Buffer Overflow"
        Pattern = '(memcpy|strcpy|strcat|sprintf)\s*\('
        Category = "BufferOverflow"
        Risk = "Critical"
    },
    @{
        Name = "Raw Pointer Without Smart Pointer"
        Pattern = '\bnew\s+\w+(?!\s*\(.*TSharedPtr|TUniquePtr|TWeakPtr)'
        Category = "MemoryLeak"
        Risk = "High"
    },
    @{
        Name = "Unchecked GetWorld() Dereference"
        Pattern = 'GetWorld\(\)->'
        Category = "NullPointer"
        Risk = "High"
    },
    @{
        Name = "Unchecked GetOwner() Dereference"
        Pattern = 'GetOwner\(\)->'
        Category = "NullPointer"
        Risk = "High"
    },
    @{
        Name = "Array Access in Loop"
        Pattern = 'for\s*\([^)]+\)\s*\{[^}]*\[[^\]]+\]'
        Category = "BufferOverflow"
        Risk = "Medium"
    }
)

$sourceFiles = Get-ChildItem -Path "$ProjectRoot\Source" -Recurse -Include *.cpp,*.h | Where-Object { $_.FullName -notmatch "\\Intermediate\\" }

Write-Host "Analyzing $($sourceFiles.Count) files for memory safety issues..." -ForegroundColor Yellow

foreach ($file in $sourceFiles) {
    $content = Get-Content $file.FullName -Raw
    $lines = Get-Content $file.FullName

    foreach ($pattern in $memoryPatterns) {
        $matches = [regex]::Matches($content, $pattern.Pattern)

        foreach ($match in $matches) {
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
                Issue = $pattern.Name
                Category = $pattern.Category
                Risk = $pattern.Risk
                Code = $match.Value.Substring(0, [Math]::Min(100, $match.Value.Length))
            }

            $results.Issues += $issue
            $results.Summary[$pattern.Category]++
        }
    }
}

Write-Host "`nMemory Safety Analysis Complete!" -ForegroundColor Green
Write-Host "Buffer Overflow Risks: $($results.Summary.BufferOverflow)" -ForegroundColor Red
Write-Host "Memory Leak Risks: $($results.Summary.MemoryLeak)" -ForegroundColor Magenta
Write-Host "Use After Free Risks: $($results.Summary.UseAfterFree)" -ForegroundColor Yellow
Write-Host "Null Pointer Risks: $($results.Summary.NullPointer)" -ForegroundColor Cyan

$outputPath = Join-Path $ProjectRoot "Docs\Security\$OutputFile"
$outputDir = Split-Path $outputPath -Parent
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir -Force | Out-Null
}

$results | ConvertTo-Json -Depth 10 | Out-File $outputPath -Encoding UTF8
Write-Host "`nResults saved to: $outputPath" -ForegroundColor Green
