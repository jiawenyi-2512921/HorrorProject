# Shared helpers for safe monitoring scripts.

$ValidationCommon = Join-Path (Split-Path -Parent $PSScriptRoot) "Validation\Common.ps1"
if (-not (Test-Path -LiteralPath $ValidationCommon)) {
    $ValidationCommon = Join-Path (Split-Path -Parent (Split-Path -Parent $PSScriptRoot)) "Validation\Common.ps1"
}
. $ValidationCommon

function Invoke-HorrorMonitoringTask {
    param(
        [Parameter(Mandatory = $true)]
        [string]$TaskName,

        [string]$OutputPath = "",
        [switch]$Continuous
    )

    $ProjectRoot = Get-HorrorProjectRoot
    $SavedRoot = Join-Path $ProjectRoot "Saved\Monitoring"
    New-Item -ItemType Directory -Force -Path $SavedRoot | Out-Null

    if ([string]::IsNullOrWhiteSpace($OutputPath)) {
        $OutputPath = Join-Path $SavedRoot "$TaskName.json"
    }

    $Logs = @(Get-ChildItem -Path (Join-Path $ProjectRoot "Saved\Logs") -File -Filter "*.log" -ErrorAction SilentlyContinue)
    $Reports = @(Get-ChildItem -Path (Join-Path $ProjectRoot "Saved") -File -Recurse -Include *.json,*.xml,*.html -ErrorAction SilentlyContinue)
    $RecentErrors = @()

    foreach ($Log in $Logs | Sort-Object LastWriteTime -Descending | Select-Object -First 5) {
        $Matches = @(Select-String -Path $Log.FullName -Pattern "error|fatal|ensure" -SimpleMatch -ErrorAction SilentlyContinue | Select-Object -First 20)
        foreach ($Match in $Matches) {
            $RecentErrors += [PSCustomObject]@{
                Log = $Log.Name
                Line = $Match.LineNumber
                Text = $Match.Line
            }
        }
    }

    $Payload = [ordered]@{
        Task = $TaskName
        GeneratedAt = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
        ProjectRoot = $ProjectRoot
        Continuous = [bool]$Continuous
        LogCount = $Logs.Count
        ReportCount = $Reports.Count
        RecentErrorCount = $RecentErrors.Count
        RecentErrors = $RecentErrors
    }

    $Payload | ConvertTo-Json -Depth 6 | Set-Content -Path $OutputPath -Encoding UTF8

    Write-Host "=== $TaskName ===" -ForegroundColor Cyan
    Write-Host "Logs: $($Payload.LogCount)" -ForegroundColor Gray
    Write-Host "Reports: $($Payload.ReportCount)" -ForegroundColor Gray
    Write-Host "Recent log errors: $($Payload.RecentErrorCount)" -ForegroundColor $(if ($Payload.RecentErrorCount -eq 0) { "Green" } else { "Yellow" })
    Write-Host "Output: $OutputPath" -ForegroundColor Green
}
