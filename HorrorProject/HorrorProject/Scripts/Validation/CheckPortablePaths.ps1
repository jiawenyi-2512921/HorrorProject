# Checks executable PowerShell scripts for project-root-specific absolute paths.

$ErrorActionPreference = "Stop"

. (Join-Path $PSScriptRoot "Common.ps1")

$ProjectRoot = Get-HorrorProjectRoot
$OuterProjectRoot = Split-Path -Parent $ProjectRoot
$ScanRoots = @(
    (Join-Path $ProjectRoot "Scripts"),
    (Join-Path $ProjectRoot "Source")
) | Where-Object { Test-Path -LiteralPath $_ }

$ForbiddenPaths = @(
    $ProjectRoot,
    $OuterProjectRoot
) | Where-Object { -not [string]::IsNullOrWhiteSpace($_) } | Select-Object -Unique

Write-Host "=== Portable Path Check ===" -ForegroundColor Cyan
Write-Host "Project: $ProjectRoot" -ForegroundColor Yellow

$Issues = New-Object System.Collections.Generic.List[object]

foreach ($ScriptFile in Get-ChildItem -LiteralPath $ScanRoots -Recurse -File -Filter "*.ps1") {
    $Matches = Select-String -LiteralPath $ScriptFile.FullName -Pattern $ForbiddenPaths -SimpleMatch -ErrorAction SilentlyContinue
    foreach ($Match in $Matches) {
        $Issues.Add([pscustomobject]@{
            Path = $Match.Path
            LineNumber = $Match.LineNumber
            Line = $Match.Line.Trim()
        })
    }
}

if ($Issues.Count -gt 0) {
    Write-Host "[ERROR] Found project-root-specific absolute paths in PowerShell scripts" -ForegroundColor Red
    $Issues | Select-Object -First 40 | ForEach-Object {
        Write-Host ("  {0}:{1}: {2}" -f $_.Path, $_.LineNumber, $_.Line) -ForegroundColor Red
    }

    if ($Issues.Count -gt 40) {
        Write-Host ("  ... and {0} more" -f ($Issues.Count - 40)) -ForegroundColor Red
    }

    exit 1
}

Write-Host "[OK] No project-root-specific absolute paths detected in PowerShell scripts" -ForegroundColor Green
