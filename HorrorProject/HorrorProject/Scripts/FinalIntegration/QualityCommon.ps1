# Shared helpers for final integration quality scripts.

$ValidationCommon = Join-Path (Split-Path -Parent $PSScriptRoot) "Validation\Common.ps1"
. $ValidationCommon

function New-HorrorQualityReport {
    param(
        [string]$Name
    )

    [ordered]@{
        Name = $Name
        GeneratedAt = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        ProjectRoot = Get-HorrorProjectRoot
        Checks = New-Object System.Collections.Generic.List[object]
    }
}

function Add-HorrorQualityCheck {
    param(
        [Parameter(Mandatory = $true)]
        [hashtable]$Report,

        [Parameter(Mandatory = $true)]
        [string]$Name,

        [Parameter(Mandatory = $true)]
        [bool]$Passed,

        [string]$Detail = "",

        [ValidateSet("Info", "Minor", "Major", "Critical")]
        [string]$Severity = "Major"
    )

    $Report.Checks.Add([PSCustomObject]@{
        Name = $Name
        Passed = $Passed
        Severity = $Severity
        Detail = $Detail
    }) | Out-Null
}

function Write-HorrorQualitySummary {
    param(
        [Parameter(Mandatory = $true)]
        [hashtable]$Report
    )

    $Failed = @($Report.Checks | Where-Object { -not $_.Passed })
    $Passed = @($Report.Checks | Where-Object { $_.Passed })

    Write-Host "=== $($Report.Name) ===" -ForegroundColor Cyan
    Write-Host "Passed: $($Passed.Count)" -ForegroundColor Green
    Write-Host "Failed: $($Failed.Count)" -ForegroundColor $(if ($Failed.Count -gt 0) { "Red" } else { "Green" })

    foreach ($Check in $Report.Checks) {
        $Color = if ($Check.Passed) { "Green" } elseif ($Check.Severity -eq "Critical") { "Red" } else { "Yellow" }
        $Status = if ($Check.Passed) { "PASS" } else { "FAIL" }
        Write-Host "[$Status] $($Check.Name) - $($Check.Detail)" -ForegroundColor $Color
    }

    return $Failed.Count
}

function Save-HorrorQualityReport {
    param(
        [Parameter(Mandatory = $true)]
        [hashtable]$Report,

        [string]$ReportPath = ""
    )

    if ([string]::IsNullOrWhiteSpace($ReportPath)) {
        $ProjectRoot = Get-HorrorProjectRoot
        $ReportDir = Join-Path $ProjectRoot "Saved\FinalIntegration"
        New-Item -ItemType Directory -Force -Path $ReportDir | Out-Null
        $SafeName = $Report.Name -replace "[^a-zA-Z0-9_-]", "_"
        $ReportPath = Join-Path $ReportDir "$SafeName.json"
    } else {
        $Parent = Split-Path -Parent $ReportPath
        if ($Parent) {
            New-Item -ItemType Directory -Force -Path $Parent | Out-Null
        }
    }

    $Report | ConvertTo-Json -Depth 6 | Set-Content -Path $ReportPath -Encoding UTF8
    Write-Host "Report: $ReportPath" -ForegroundColor Gray
}

function Test-HorrorRequiredPath {
    param(
        [hashtable]$Report,
        [string]$Name,
        [string]$Path,
        [string]$Severity = "Critical"
    )

    $Exists = Test-Path -LiteralPath $Path
    Add-HorrorQualityCheck -Report $Report -Name $Name -Passed $Exists -Detail $Path -Severity $Severity
}

function Get-HorrorPowerShellSyntaxIssueCount {
    param(
        [string[]]$ScanPath
    )

    $ProjectRoot = Get-HorrorProjectRoot
    $Count = 0
    foreach ($Path in $ScanPath) {
        $AbsolutePath = if ([System.IO.Path]::IsPathRooted($Path)) { $Path } else { Join-Path $ProjectRoot $Path }
        if (-not (Test-Path -LiteralPath $AbsolutePath)) {
            continue
        }

        foreach ($File in Get-ChildItem -LiteralPath $AbsolutePath -Filter "*.ps1" -File -Recurse) {
            $Tokens = $null
            $Errors = $null
            [System.Management.Automation.Language.Parser]::ParseFile($File.FullName, [ref]$Tokens, [ref]$Errors) | Out-Null
            $Count += $Errors.Count
        }
    }

    return $Count
}
