# RunMonitoring.ps1
# Master monitoring script - runs all monitoring tasks
# Can be scheduled to run every 5 minutes

param(
    [switch]$Continuous = $false,
    [int]$IntervalMinutes = 5
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║     HorrorProject Real-Time Monitoring System v1.0.0      ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

function Run-MonitoringCycle {
    $cycleStart = Get-Date
    Write-Host "=== Monitoring Cycle Started ===" -ForegroundColor Green
    Write-Host "Time: $($cycleStart.ToString('yyyy-MM-dd HH:mm:ss'))" -ForegroundColor Gray
    Write-Host ""

    # 1. Update Dashboard
    Write-Host "[1/4] Updating Dashboard..." -ForegroundColor Yellow
    try {
        & "$ScriptDir\UpdateDashboard.ps1"
        Write-Host "✅ Dashboard updated" -ForegroundColor Green
    } catch {
        Write-Host "❌ Dashboard update failed: $_" -ForegroundColor Red
    }
    Write-Host ""

    # 2. Check Agent Status
    Write-Host "[2/4] Checking Agent Status..." -ForegroundColor Yellow
    try {
        & "$ScriptDir\CheckAgentStatus.ps1"
        Write-Host "✅ Agent status checked" -ForegroundColor Green
    } catch {
        Write-Host "❌ Agent status check failed: $_" -ForegroundColor Red
    }
    Write-Host ""

    # 3. Collect Metrics
    Write-Host "[3/4] Collecting Metrics..." -ForegroundColor Yellow
    try {
        & "$ScriptDir\CollectMetrics.ps1" -Export
        Write-Host "✅ Metrics collected" -ForegroundColor Green
    } catch {
        Write-Host "❌ Metrics collection failed: $_" -ForegroundColor Red
    }
    Write-Host ""

    # 4. Generate Timeline
    Write-Host "[4/4] Generating Timeline..." -ForegroundColor Yellow
    try {
        & "$ScriptDir\GenerateTimeline.ps1"
        Write-Host "✅ Timeline generated" -ForegroundColor Green
    } catch {
        Write-Host "❌ Timeline generation failed: $_" -ForegroundColor Red
    }
    Write-Host ""

    $cycleEnd = Get-Date
    $duration = ($cycleEnd - $cycleStart).TotalSeconds
    Write-Host "=== Monitoring Cycle Complete ===" -ForegroundColor Green
    Write-Host "Duration: $([math]::Round($duration, 2)) seconds" -ForegroundColor Gray
    Write-Host ""
}

# Run initial cycle
Run-MonitoringCycle

# Continuous monitoring mode
if ($Continuous) {
    Write-Host "Continuous monitoring enabled (every $IntervalMinutes minutes)" -ForegroundColor Cyan
    Write-Host "Press Ctrl+C to stop" -ForegroundColor Gray
    Write-Host ""

    while ($true) {
        $nextRun = (Get-Date).AddMinutes($IntervalMinutes)
        Write-Host "Next update at: $($nextRun.ToString('HH:mm:ss'))" -ForegroundColor Gray
        Start-Sleep -Seconds ($IntervalMinutes * 60)
        Write-Host "`n" + ("=" * 60) + "`n" -ForegroundColor DarkGray
        Run-MonitoringCycle
    }
} else {
    Write-Host "Single run complete. Use -Continuous flag for continuous monitoring." -ForegroundColor Cyan
}
