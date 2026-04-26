# StartMonitoring.ps1 - 监控系统启动脚本
param(
    [switch]$Dashboard,
    [switch]$Alerts,
    [switch]$All
)

$ErrorActionPreference = "Stop"
$MonitoringRoot = $PSScriptRoot

Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║        HorrorProject Monitoring System Launcher           ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

$jobs = @()

if ($All -or $Dashboard) {
    Write-Host "Starting Dashboard Server..." -ForegroundColor Green
    $dashboardScript = Join-Path $MonitoringRoot "Dashboard\DashboardServer.ps1"
    $job = Start-Job -ScriptBlock {
        param($Script)
        & $Script -Port 8080
    } -ArgumentList $dashboardScript
    $jobs += @{ Name = "Dashboard"; Job = $job }
    Write-Host "  Dashboard Server started (Job ID: $($job.Id))" -ForegroundColor Cyan
    Write-Host "  URL: http://localhost:8080/" -ForegroundColor Yellow
    Write-Host ""
}

if ($All) {
    Write-Host "Starting Real-time Monitors..." -ForegroundColor Green

    # 实时监控
    $realtimeScript = Join-Path $MonitoringRoot "RealTime\RealtimeMonitor.ps1"
    $job = Start-Job -ScriptBlock {
        param($Script)
        & $Script -UpdateInterval 60 -EnableAlerts
    } -ArgumentList $realtimeScript
    $jobs += @{ Name = "RealtimeMonitor"; Job = $job }
    Write-Host "  Realtime Monitor started (Job ID: $($job.Id))" -ForegroundColor Cyan
    Write-Host ""
}

if ($All -or $Alerts) {
    Write-Host "Starting Alert Monitors..." -ForegroundColor Green

    # 构建失败告警
    $buildAlertScript = Join-Path $MonitoringRoot "Alerts\AlertOnBuildFailure.ps1"
    $job = Start-Job -ScriptBlock {
        param($Script)
        & $Script -CheckInterval 60
    } -ArgumentList $buildAlertScript
    $jobs += @{ Name = "BuildAlert"; Job = $job }
    Write-Host "  Build Alert Monitor started (Job ID: $($job.Id))" -ForegroundColor Cyan

    # 测试失败告警
    $testAlertScript = Join-Path $MonitoringRoot "Alerts\AlertOnTestFailure.ps1"
    $job = Start-Job -ScriptBlock {
        param($Script)
        & $Script -CheckInterval 60
    } -ArgumentList $testAlertScript
    $jobs += @{ Name = "TestAlert"; Job = $job }
    Write-Host "  Test Alert Monitor started (Job ID: $($job.Id))" -ForegroundColor Cyan

    # 性能告警
    $perfAlertScript = Join-Path $MonitoringRoot "Alerts\AlertOnPerformanceDrop.ps1"
    $job = Start-Job -ScriptBlock {
        param($Script)
        & $Script -CheckInterval 60
    } -ArgumentList $perfAlertScript
    $jobs += @{ Name = "PerformanceAlert"; Job = $job }
    Write-Host "  Performance Alert Monitor started (Job ID: $($job.Id))" -ForegroundColor Cyan

    # 内存泄漏告警
    $memAlertScript = Join-Path $MonitoringRoot "Alerts\AlertOnMemoryLeak.ps1"
    $job = Start-Job -ScriptBlock {
        param($Script)
        & $Script -CheckInterval 300
    } -ArgumentList $memAlertScript
    $jobs += @{ Name = "MemoryLeakAlert"; Job = $job }
    Write-Host "  Memory Leak Alert Monitor started (Job ID: $($job.Id))" -ForegroundColor Cyan
    Write-Host ""
}

if ($jobs.Count -eq 0) {
    Write-Host "No monitors started. Use -Dashboard, -Alerts, or -All" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Usage:" -ForegroundColor Cyan
    Write-Host "  .\StartMonitoring.ps1 -Dashboard    # Start dashboard only"
    Write-Host "  .\StartMonitoring.ps1 -Alerts       # Start alert monitors only"
    Write-Host "  .\StartMonitoring.ps1 -All          # Start all monitors"
    exit
}

Write-Host "All monitors started successfully!" -ForegroundColor Green
Write-Host ""
Write-Host "Active Jobs:" -ForegroundColor Yellow
foreach ($item in $jobs) {
    Write-Host "  - $($item.Name): Job ID $($item.Job.Id)" -ForegroundColor Gray
}
Write-Host ""
Write-Host "Press Ctrl+C to stop all monitors" -ForegroundColor Yellow
Write-Host ""

try {
    while ($true) {
        Start-Sleep -Seconds 5

        # 检查作业状态
        foreach ($item in $jobs) {
            if ($item.Job.State -ne "Running") {
                Write-Host "[$($item.Name)] Job stopped with state: $($item.Job.State)" -ForegroundColor Red
                if ($item.Job.State -eq "Failed") {
                    Write-Host "Error: $($item.Job.ChildJobs[0].JobStateInfo.Reason)" -ForegroundColor Red
                }
            }
        }
    }
} finally {
    Write-Host "`nStopping all monitors..." -ForegroundColor Yellow
    foreach ($item in $jobs) {
        Stop-Job -Job $item.Job -ErrorAction SilentlyContinue
        Remove-Job -Job $item.Job -Force -ErrorAction SilentlyContinue
        Write-Host "  Stopped: $($item.Name)" -ForegroundColor Gray
    }
    Write-Host "All monitors stopped" -ForegroundColor Green
}
