# UpdateDashboard.ps1 - 更新仪表板数据
param(
    [int]$UpdateInterval = 60,
    [string]$DashboardPath = "dashboard.html"
)

$ErrorActionPreference = "Stop"
$GenerateScript = Join-Path $PSScriptRoot "GenerateDashboard.ps1"

Write-Host "Starting Dashboard Auto-Update..." -ForegroundColor Green
Write-Host "Update Interval: $UpdateInterval seconds"
Write-Host "Dashboard: $DashboardPath"
Write-Host "Press Ctrl+C to stop"
Write-Host ""

$iteration = 0

while ($true) {
    $iteration++
    Write-Host "[$(Get-Date -Format 'HH:mm:ss')] Updating dashboard (iteration #$iteration)..." -ForegroundColor Cyan

    try {
        & $GenerateScript -OutputPath $DashboardPath
        Write-Host "Dashboard updated successfully" -ForegroundColor Green
    } catch {
        Write-Host "Error updating dashboard: $_" -ForegroundColor Red
    }

    Start-Sleep -Seconds $UpdateInterval
}
