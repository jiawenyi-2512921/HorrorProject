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
$DocsRootFullPath = [System.IO.Path]::GetFullPath($DocsPath).TrimEnd('\')

function Get-DocsRelativePath {
    param(
        [string]$Path
    )

    $fullPath = [System.IO.Path]::GetFullPath($Path)
    $rootWithSeparator = $DocsRootFullPath + [System.IO.Path]::DirectorySeparatorChar
    if ($fullPath.StartsWith($rootWithSeparator, [System.StringComparison]::OrdinalIgnoreCase)) {
        return $fullPath.Substring($rootWithSeparator.Length)
    }

    return $fullPath
}

function Test-ShouldValidateMarkdownLink {
    param(
        [string]$LinkPath
    )

    if ([string]::IsNullOrWhiteSpace($LinkPath)) {
        return $false
    }

    $candidate = $LinkPath.Trim()

    if ($candidate.StartsWith("#")) {
        return $false
    }

    if ($candidate -match '^[a-zA-Z][a-zA-Z0-9+.-]*:') {
        return $false
    }

    if ($candidate.StartsWith("<") -and $candidate.EndsWith(">")) {
        $candidate = $candidate.Trim("<", ">")
    }

    $candidateWithoutAnchor = ($candidate -replace "#.*$", "").Trim()
    if ([string]::IsNullOrWhiteSpace($candidateWithoutAnchor)) {
        return $false
    }

    # The documentation contains C++ signatures that look like markdown links,
    # for example [Name](const FThing& Value). Those are not file links.
    if ($candidateWithoutAnchor -match '[<>&]|\bconst\b|::|^\w+\s+\w+') {
        return $false
    }

    if ($candidateWithoutAnchor -match '\s' -and $candidateWithoutAnchor -notmatch '[\\/]') {
        return $false
    }

    return $true
}

function Resolve-MarkdownLinkPath {
    param(
        [System.IO.FileInfo]$MarkdownFile,
        [string]$LinkPath
    )

    $candidate = $LinkPath.Trim()
    if ($candidate.StartsWith("<") -and $candidate.EndsWith(">")) {
        $candidate = $candidate.Trim("<", ">")
    }

    $candidate = ($candidate -replace "#.*$", "").Trim()
    try {
        $candidate = [System.Uri]::UnescapeDataString($candidate)
    } catch {
        # Keep the original candidate if it is not valid URI-escaped text.
    }

    return Join-Path (Split-Path $MarkdownFile.FullName) $candidate
}

function Test-HasImmediateDocComment {
    param(
        [string[]]$Lines,
        [int]$Index
    )

    for ($lineIndex = $Index - 1; $lineIndex -ge 0; $lineIndex--) {
        $trimmed = $Lines[$lineIndex].Trim()
        if ([string]::IsNullOrWhiteSpace($trimmed)) {
            continue
        }

        return ($trimmed -eq "*/" -or $trimmed.StartsWith("///"))
    }

    return $false
}

function Get-UClassDeclarations {
    param(
        [string]$Content
    )

    $lines = $Content -split '\r?\n'
    $declarations = @()

    for ($index = 0; $index -lt $lines.Count; $index++) {
        if ($lines[$index] -notmatch '^\s*UCLASS\b') {
            continue
        }

        $uclassStart = $index
        $uclassText = $lines[$index]
        $parenDepth = ([regex]::Matches($uclassText, '\(').Count - [regex]::Matches($uclassText, '\)').Count)
        while ($parenDepth -gt 0 -and $index + 1 -lt $lines.Count) {
            $index++
            $uclassText += "`n" + $lines[$index]
            $parenDepth += ([regex]::Matches($lines[$index], '\(').Count - [regex]::Matches($lines[$index], '\)').Count)
        }

        $scanLimit = [Math]::Min($lines.Count - 1, $index + 12)
        for ($scan = $index + 1; $scan -le $scanLimit; $scan++) {
            if ($lines[$scan] -match '^\s*class\s+(?:(?:\w+_API|MinimalAPI)\s+)?(?<Name>\w+)\b') {
                $className = $matches['Name']
                if ($uclassText -match '\bHidden\b' -and $className -match '(DelegateProbe|Probe)$') {
                    break
                }

                $declarations += [PSCustomObject]@{
                    Name = $className
                    UClassLine = $uclassStart
                    HasDocComment = Test-HasImmediateDocComment -Lines $lines -Index $uclassStart
                }
                break
            }
        }
    }

    return $declarations
}

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

    foreach ($declaration in (Get-UClassDeclarations -Content $content)) {
        if (-not $declaration.HasDocComment) {
            $undocumentedClasses += "$($declaration.Name) in $($header.Name)"
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

        if (-not (Test-ShouldValidateMarkdownLink -LinkPath $linkPath)) { continue }

        $targetPath = Resolve-MarkdownLinkPath -MarkdownFile $mdFile -LinkPath $linkPath

        if (-not (Test-Path $targetPath)) {
            $brokenLinks += [PSCustomObject]@{
                Source = Get-DocsRelativePath -Path $mdFile.FullName
                Target = $linkPath
                ResolvedPath = $targetPath
            }
        }
    }
}

if ($brokenLinks.Count -gt 0) {
    $issues += "Found $($brokenLinks.Count) broken links"
    Write-Host "  Found $($brokenLinks.Count) broken links" -ForegroundColor Red
    $brokenLinks | ForEach-Object { Write-Host "    - $($_.Source): $($_.Target)" -ForegroundColor Gray }
} else {
    Write-Host "  All links are valid" -ForegroundColor Green
}

# Check for incomplete-section markers in documentation
Write-Host "`nChecking for incomplete sections..." -ForegroundColor Cyan
$incompleteDocs = @()
$IncompleteMarkerPattern = 'TO' + 'DO|FIX' + 'ME|TBD|\[WIP\]'

foreach ($mdFile in $mdFiles) {
    $content = Get-Content $mdFile.FullName -Raw
    if ($content -match $IncompleteMarkerPattern) {
        $incompleteDocs += $mdFile.Name
    }
}

if ($incompleteDocs.Count -gt 0) {
    $warnings += "Found $($incompleteDocs.Count) documents with incomplete-section markers"
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

Broken Links:
$($brokenLinks | ForEach-Object { "- $($_.Source): $($_.Target) -> $($_.ResolvedPath)" } | Out-String)

Undocumented Classes:
$($undocumentedClasses | ForEach-Object { "- $_" } | Out-String)

Incomplete Documents:
$($incompleteDocs | ForEach-Object { "- $_" } | Out-String)
"@

Set-Content -Path $reportPath -Value $report
Write-Host "`nReport saved to: $reportPath" -ForegroundColor Green

if ($issues.Count -gt 0) {
    exit 1
} else {
    exit 0
}
