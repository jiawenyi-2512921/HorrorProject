# ValidateDocs.ps1 - Documentation Validator
# Validates documentation completeness and quality

param(
    [string]$DocsPath = "$PSScriptRoot\..\..\Docs",
    [string]$SourcePath = "$PSScriptRoot\..\..\Source\HorrorProject",
    [switch]$Strict
)

$ErrorActionPreference = "Stop"

Write-Host "=== Documentation Validator ===" -ForegroundColor Cyan

$issues = @()
$warnings = @()

# Check required documentation files
$requiredDocs = @(
    "Architecture\SystemOverview.md",
    "Architecture\ComponentDiagram.md",
    "Architecture\DataFlow.md",
    "Architecture\EventBus.md",
    "Developer\GettingStarted.md",
    "Developer\CodingStandards.md",
    "Developer\BuildGuide.md",
    "Technical\PerformanceSpec.md"
)

Write-Host "`nChecking required documentation files..." -ForegroundColor Cyan
foreach ($doc in $requiredDocs) {
    $fullPath = Join-Path $DocsPath $doc
    if (-not (Test-Path $fullPath)) {
        $issues += "Missing required document: $doc"
        Write-Host "  [MISSING] $doc" -ForegroundColor Red
    } else {
        Write-Host "  [OK] $doc" -ForegroundColor Green
    }
}

# Check for undocumented public classes
Write-Host "`nChecking for undocumented classes..." -ForegroundColor Cyan
$headerFiles = Get-ChildItem -Path $SourcePath -Filter "*.h" -Recurse | Where-Object { $_.FullName -notmatch "Intermediate|Tests" }

$undocumentedClasses = @()
foreach ($header in $headerFiles) {
    $content = Get-Content $header.FullName -Raw

    # Find UCLASS declarations
    $classMatches = [regex]::Matches($content, "UCLASS\([^\)]*\)[\s\S]*?class\s+\w+\s+(\w+)")

    foreach ($match in $classMatches) {
        $className = $match.Groups[1].Value

        # Check if there's a comment block before the class
        $classPos = $match.Index
        $beforeClass = $content.Substring(0, $classPos)

        if ($beforeClass -notmatch "/\*\*[\s\S]*?\*/" -and $beforeClass -notmatch "///") {
            $undocumentedClasses += "$className in $($header.Name)"
        }
    }
}

if ($undocumentedClasses.Count -gt 0) {
    $warnings += "Found $($undocumentedClasses.Count) undocumented classes"
    Write-Host "  Found $($undocumentedClasses.Count) undocumented classes" -ForegroundColor Yellow
    if ($Strict) {
        $undocumentedClasses | ForEach-Object { Write-Host "    - $_" -ForegroundColor Gray }
    }
} else {
    Write-Host "  All classes have documentation" -ForegroundColor Green
}

# Check markdown link validity
Write-Host "`nValidating markdown links..." -ForegroundColor Cyan
$mdFiles = Get-ChildItem -Path $DocsPath -Filter "*.md" -Recurse
$brokenLinks = @()

foreach ($mdFile in $mdFiles) {
    $content = Get-Content $mdFile.FullName -Raw
    $links = [regex]::Matches($content, "\[([^\]]+)\]\(([^\)]+)\)")

    foreach ($link in $links) {
        $linkPath = $link.Groups[2].Value

        # Skip external links
        if ($linkPath -match "^https?://") { continue }

        # Resolve relative path
        $targetPath = Join-Path (Split-Path $mdFile.FullName) $linkPath
        $targetPath = $targetPath -replace "#.*$", ""  # Remove anchors

        if (-not (Test-Path $targetPath)) {
            $brokenLinks += "$($mdFile.Name): $linkPath"
        }
    }
}

if ($brokenLinks.Count -gt 0) {
    $issues += "Found $($brokenLinks.Count) broken links"
    Write-Host "  Found $($brokenLinks.Count) broken links" -ForegroundColor Red
    $brokenLinks | ForEach-Object { Write-Host "    - $_" -ForegroundColor Gray }
} else {
    Write-Host "  All links are valid" -ForegroundColor Green
}

# Check for TODO/FIXME in documentation
Write-Host "`nChecking for incomplete sections..." -ForegroundColor Cyan
$incompleteDocs = @()

foreach ($mdFile in $mdFiles) {
    $content = Get-Content $mdFile.FullName -Raw
    if ($content -match "TODO|FIXME|TBD|\[WIP\]") {
        $incompleteDocs += $mdFile.Name
    }
}

if ($incompleteDocs.Count -gt 0) {
    $warnings += "Found $($incompleteDocs.Count) documents with TODO/FIXME markers"
    Write-Host "  Found $($incompleteDocs.Count) documents with incomplete sections" -ForegroundColor Yellow
    $incompleteDocs | ForEach-Object { Write-Host "    - $_" -ForegroundColor Gray }
} else {
    Write-Host "  No incomplete sections found" -ForegroundColor Green
}

# Generate report
Write-Host "`n=== Validation Summary ===" -ForegroundColor Cyan
Write-Host "Issues: $($issues.Count)" -ForegroundColor $(if ($issues.Count -eq 0) { "Green" } else { "Red" })
Write-Host "Warnings: $($warnings.Count)" -ForegroundColor $(if ($warnings.Count -eq 0) { "Green" } else { "Yellow" })

if ($issues.Count -gt 0) {
    Write-Host "`nCritical Issues:" -ForegroundColor Red
    $issues | ForEach-Object { Write-Host "  - $_" -ForegroundColor Red }
}

if ($warnings.Count -gt 0 -and $Strict) {
    Write-Host "`nWarnings:" -ForegroundColor Yellow
    $warnings | ForEach-Object { Write-Host "  - $_" -ForegroundColor Yellow }
}

# Save report
$reportPath = Join-Path $DocsPath "validation_report.txt"
$report = @"
Documentation Validation Report
Generated: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")

Issues: $($issues.Count)
Warnings: $($warnings.Count)

Critical Issues:
$($issues | ForEach-Object { "- $_" } | Out-String)

Warnings:
$($warnings | ForEach-Object { "- $_" } | Out-String)
"@

Set-Content -Path $reportPath -Value $report
Write-Host "`nReport saved to: $reportPath" -ForegroundColor Green

if ($issues.Count -gt 0) {
    exit 1
} else {
    exit 0
}
