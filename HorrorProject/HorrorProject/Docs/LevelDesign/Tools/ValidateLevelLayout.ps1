# Validate Level Layout Tool
# Validates level design against specifications

param(
    [Parameter(Mandatory=$false)]
    [string]$LevelName = "SM13",

    [Parameter(Mandatory=$false)]
    [string]$DataPath = "D:/gptzuo/HorrorProject/HorrorProject/Content/Maps",

    [Parameter(Mandatory=$false)]
    [switch]$CheckObjectives,

    [Parameter(Mandatory=$false)]
    [switch]$CheckEvidence,

    [Parameter(Mandatory=$false)]
    [switch]$CheckSpacing,

    [Parameter(Mandatory=$false)]
    [switch]$CheckTiming,

    [Parameter(Mandatory=$false)]
    [switch]$CheckPerformance,

    [Parameter(Mandatory=$false)]
    [switch]$GenerateReport
)

$ErrorActionPreference = "Stop"

Write-Host "=== Validate Level Layout ===" -ForegroundColor Cyan
Write-Host "Level: $LevelName"
Write-Host ""

$validationResults = @{
    LevelName = $LevelName
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Passed = $true
    Warnings = @()
    Errors = @()
    Tests = @()
}

# Load data files
$blockoutFile = Join-Path $DataPath "${LevelName}_Blockout.json"
$objectivesFile = Join-Path $DataPath "${LevelName}_Objectives.json"
$evidenceFile = Join-Path $DataPath "${LevelName}_Evidence.json"

$blockoutData = $null
$objectivesData = $null
$evidenceData = $null

if (Test-Path $blockoutFile) {
    $blockoutData = Get-Content $blockoutFile -Raw | ConvertFrom-Json
    Write-Host "Loaded blockout data" -ForegroundColor Gray
}

if (Test-Path $objectivesFile) {
    $objectivesData = Get-Content $objectivesFile -Raw | ConvertFrom-Json
    Write-Host "Loaded objectives data" -ForegroundColor Gray
}

if (Test-Path $evidenceFile) {
    $evidenceData = Get-Content $evidenceFile -Raw | ConvertFrom-Json
    Write-Host "Loaded evidence data" -ForegroundColor Gray
}

Write-Host ""

# Validate Objectives
if ($CheckObjectives -and $objectivesData) {
    Write-Host "Validating objectives..." -ForegroundColor Yellow

    $test = @{
        Name = "Objectives Validation"
        Passed = $true
        Issues = @()
    }

    # Check count
    if ($objectivesData.ObjectiveCount -ne 8) {
        $test.Issues += "Expected 8 objectives, found $($objectivesData.ObjectiveCount)"
        $test.Passed = $false
    }

    # Check required objectives
    $required = $objectivesData.Objectives | Where-Object { -not $_.IsOptional }
    if ($required.Count -lt 6) {
        $test.Issues += "Expected at least 6 required objectives, found $($required.Count)"
        $test.Passed = $false
    }

    # Check spacing
    for ($i = 0; $i -lt $objectivesData.Objectives.Count - 1; $i++) {
        $obj1 = $objectivesData.Objectives[$i]
        $obj2 = $objectivesData.Objectives[$i + 1]

        $dx = $obj2.Position.X - $obj1.Position.X
        $dy = $obj2.Position.Y - $obj1.Position.Y
        $dz = $obj2.Position.Z - $obj1.Position.Z

        $distance = [Math]::Sqrt($dx * $dx + $dy * $dy + $dz * $dz) / 100

        if ($distance -lt 30) {
            $test.Issues += "Objectives $($obj1.ID) and $($obj2.ID) too close: $([Math]::Round($distance, 1))m (min 30m)"
            $validationResults.Warnings += "Objective spacing warning: Obj $($obj1.ID)-$($obj2.ID)"
        }
    }

    $validationResults.Tests += $test

    if ($test.Passed) {
        Write-Host "  ✓ Objectives validation passed" -ForegroundColor Green
    } else {
        Write-Host "  ✗ Objectives validation failed" -ForegroundColor Red
        $validationResults.Passed = $false
        foreach ($issue in $test.Issues) {
            Write-Host "    - $issue" -ForegroundColor Red
            $validationResults.Errors += $issue
        }
    }
}

# Validate Evidence
if ($CheckEvidence -and $evidenceData) {
    Write-Host "Validating evidence..." -ForegroundColor Yellow

    $test = @{
        Name = "Evidence Validation"
        Passed = $true
        Issues = @()
    }

    # Check counts
    $required = $evidenceData.Evidence | Where-Object { $_.Category -eq "Required" }
    $optional = $evidenceData.Evidence | Where-Object { $_.Category -eq "Optional" }
    $secret = $evidenceData.Evidence | Where-Object { $_.Category -eq "Secret" }

    if ($required.Count -ne 8) {
        $test.Issues += "Expected 8 required evidence, found $($required.Count)"
        $test.Passed = $false
    }

    if ($optional.Count -lt 12) {
        $test.Issues += "Expected at least 12 optional evidence, found $($optional.Count)"
        $validationResults.Warnings += "Low optional evidence count"
    }

    if ($secret.Count -lt 3) {
        $test.Issues += "Expected at least 3 secret evidence, found $($secret.Count)"
        $validationResults.Warnings += "Low secret evidence count"
    }

    # Check zone distribution
    $zones = $evidenceData.Evidence | Group-Object -Property Zone
    foreach ($zone in $zones) {
        if ($zone.Count -lt 2) {
            $test.Issues += "Zone $($zone.Name) has only $($zone.Count) evidence (min 2 recommended)"
            $validationResults.Warnings += "Low evidence density in $($zone.Name)"
        }
    }

    $validationResults.Tests += $test

    if ($test.Passed) {
        Write-Host "  ✓ Evidence validation passed" -ForegroundColor Green
    } else {
        Write-Host "  ✗ Evidence validation failed" -ForegroundColor Red
        foreach ($issue in $test.Issues) {
            Write-Host "    - $issue" -ForegroundColor Red
            $validationResults.Errors += $issue
        }
    }
}

# Validate Timing
if ($CheckTiming -and $objectivesData) {
    Write-Host "Validating timing..." -ForegroundColor Yellow

    $test = @{
        Name = "Timing Validation"
        Passed = $true
        Issues = @()
    }

    $totalTime = ($objectivesData.Objectives | Where-Object { -not $_.IsOptional } | Measure-Object -Property TargetTime -Maximum).Maximum

    if ($totalTime -lt 30) {
        $test.Issues += "Total time too short: ${totalTime}m (target 35m)"
        $validationResults.Warnings += "Level may be too short"
    }

    if ($totalTime -gt 40) {
        $test.Issues += "Total time too long: ${totalTime}m (target 35m)"
        $validationResults.Warnings += "Level may be too long"
    }

    $validationResults.Tests += $test

    if ($test.Passed) {
        Write-Host "  ✓ Timing validation passed (${totalTime}m)" -ForegroundColor Green
    } else {
        Write-Host "  ✗ Timing validation issues" -ForegroundColor Yellow
        foreach ($issue in $test.Issues) {
            Write-Host "    - $issue" -ForegroundColor Yellow
        }
    }
}

# Validate Performance Budget
if ($CheckPerformance -and $blockoutData) {
    Write-Host "Validating performance budget..." -ForegroundColor Yellow

    $test = @{
        Name = "Performance Budget"
        Passed = $true
        Issues = @()
    }

    # Check area
    if ($blockoutData.TotalArea -gt 1500) {
        $test.Issues += "Total area exceeds budget: $($blockoutData.TotalArea)m² (max 1500m²)"
        $validationResults.Warnings += "Area budget exceeded"
    }

    # Estimate draw calls (rough)
    $estimatedDrawCalls = $blockoutData.Zones.Count * 300
    if ($estimatedDrawCalls -gt 2500) {
        $test.Issues += "Estimated draw calls: $estimatedDrawCalls (max 2500)"
        $validationResults.Warnings += "Draw call budget may be exceeded"
    }

    $validationResults.Tests += $test

    if ($test.Passed) {
        Write-Host "  ✓ Performance budget validation passed" -ForegroundColor Green
    } else {
        Write-Host "  ✗ Performance budget issues" -ForegroundColor Yellow
        foreach ($issue in $test.Issues) {
            Write-Host "    - $issue" -ForegroundColor Yellow
        }
    }
}

# Generate report
if ($GenerateReport) {
    Write-Host ""
    Write-Host "Generating validation report..." -ForegroundColor Yellow

    $reportPath = Join-Path $DataPath "${LevelName}_Validation_Report.md"

    $status = if ($validationResults.Passed) { "✓ PASSED" } else { "✗ FAILED" }
    $statusColor = if ($validationResults.Passed) { "green" } else { "red" }

    $report = @"
# $LevelName Validation Report

**Status**: $status
**Generated**: $($validationResults.Timestamp)

## Summary

- **Tests Run**: $($validationResults.Tests.Count)
- **Errors**: $($validationResults.Errors.Count)
- **Warnings**: $($validationResults.Warnings.Count)

"@

    if ($validationResults.Errors.Count -gt 0) {
        $report += "`n## Errors`n`n"
        foreach ($error in $validationResults.Errors) {
            $report += "- ❌ $error`n"
        }
    }

    if ($validationResults.Warnings.Count -gt 0) {
        $report += "`n## Warnings`n`n"
        foreach ($warning in $validationResults.Warnings) {
            $report += "- ⚠️ $warning`n"
        }
    }

    $report += "`n## Test Results`n`n"

    foreach ($test in $validationResults.Tests) {
        $testStatus = if ($test.Passed) { "✓" } else { "✗" }
        $report += "### $testStatus $($test.Name)`n`n"

        if ($test.Issues.Count -gt 0) {
            foreach ($issue in $test.Issues) {
                $report += "- $issue`n"
            }
        } else {
            $report += "No issues found.`n"
        }

        $report += "`n"
    }

    $report | Set-Content -Path $reportPath -Encoding UTF8

    Write-Host "  - Report saved: $reportPath" -ForegroundColor Green
}

# Summary
Write-Host ""
Write-Host "=== Validation Complete ===" -ForegroundColor Cyan

if ($validationResults.Passed) {
    Write-Host "Status: PASSED ✓" -ForegroundColor Green
} else {
    Write-Host "Status: FAILED ✗" -ForegroundColor Red
}

Write-Host "Errors: $($validationResults.Errors.Count)" -ForegroundColor $(if ($validationResults.Errors.Count -eq 0) { "Green" } else { "Red" })
Write-Host "Warnings: $($validationResults.Warnings.Count)" -ForegroundColor $(if ($validationResults.Warnings.Count -eq 0) { "Green" } else { "Yellow" })
