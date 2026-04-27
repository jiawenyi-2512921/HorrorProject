# CheckAgentStatus.ps1
# Monitor agent execution status and health
# Detects failures, bottlenecks, and performance issues

param(
    [switch]$Detailed = $false
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "MonitoringCommon.ps1")

$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
$WorkspaceRoot = Split-Path -Parent (Split-Path -Parent $ProjectRoot)
$VaultRoot = if ($env:CONTEXTVAULT_ROOT) { $env:CONTEXTVAULT_ROOT } else { Join-Path $WorkspaceRoot "ContextVault" }

Write-Host "=== Agent Status Check ===" -ForegroundColor Cyan
Write-Host "Time: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray

# Check git log for agent commits
Push-Location $ProjectRoot
try {
    Write-Host "`nRecent Agent Completions:" -ForegroundColor Yellow
    $recentCommits = git log --oneline -20 2>$null
    $agentCommits = $recentCommits | Select-String -Pattern "P\d+"

    if ($agentCommits) {
        $agentCommits | ForEach-Object {
            $line = $_.Line
            if ($line -match '(P\d+)') {
                $agentNum = $Matches[1]
                Write-Host "  ✅ $agentNum - $line" -ForegroundColor Green
            }
        }
    } else {
        Write-Host "  No agent commits found" -ForegroundColor Gray
    }

    # Count total completed
    $totalCompleted = ($agentCommits | Measure-Object).Count
    $remaining = 99 - $totalCompleted

    Write-Host "`nAgent Statistics:" -ForegroundColor Yellow
    Write-Host "  Completed: $totalCompleted / 99" -ForegroundColor Green
    Write-Host "  Remaining: $remaining" -ForegroundColor Cyan
    Write-Host "  Progress: $([math]::Round($totalCompleted / 99 * 100, 1))%" -ForegroundColor Cyan

    # Check for failures (commits with "fix", "revert", "error")
    Write-Host "`nFailure Detection:" -ForegroundColor Yellow
    $failureCommits = $recentCommits | Select-String -Pattern "(fix|revert|error|failed)" -CaseSensitive:$false
    if ($failureCommits) {
        Write-Host "  ⚠️  Potential issues detected:" -ForegroundColor Yellow
        $failureCommits | ForEach-Object {
            Write-Host "    - $($_.Line)" -ForegroundColor Yellow
        }
    } else {
        Write-Host "  ✅ No failures detected" -ForegroundColor Green
    }

    # Check build status
    Write-Host "`nBuild Status:" -ForegroundColor Yellow
    $binariesPath = "$ProjectRoot\Binaries"
    if (Test-Path $binariesPath) {
        $latestBinary = Get-ChildItem -Path $binariesPath -Recurse -File | Sort-Object LastWriteTime -Descending | Select-Object -First 1
        if ($latestBinary) {
            $buildAge = (Get-Date) - $latestBinary.LastWriteTime
            Write-Host "  Last Build: $($buildAge.Hours)h $($buildAge.Minutes)m ago" -ForegroundColor Gray
            if ($buildAge.TotalHours -lt 2) {
                Write-Host "  ✅ Build is recent" -ForegroundColor Green
            } else {
                Write-Host "  ⚠️  Build is old (>2h)" -ForegroundColor Yellow
            }
        }
    } else {
        Write-Host "  ⚠️  No binaries found" -ForegroundColor Yellow
    }

    # Check for uncommitted changes
    Write-Host "`nGit Status:" -ForegroundColor Yellow
    $status = git status --porcelain 2>$null
    if ($status) {
        $fileCount = ($status | Measure-Object).Count
        Write-Host "  ⚠️  $fileCount uncommitted files" -ForegroundColor Yellow
        if ($Detailed) {
            $status | ForEach-Object { Write-Host "    $_" -ForegroundColor Gray }
        }
    } else {
        Write-Host "  ✅ Working tree clean" -ForegroundColor Green
    }

} finally {
    Pop-Location
}

# Check system resources
Write-Host "`nSystem Resources:" -ForegroundColor Yellow

# CPU
$cpu = Get-WmiObject Win32_Processor | Measure-Object -Property LoadPercentage -Average
Write-Host "  CPU: $([math]::Round($cpu.Average, 1))%" -ForegroundColor Gray

# Memory
$os = Get-WmiObject Win32_OperatingSystem
$memUsed = ($os.TotalVisibleMemorySize - $os.FreePhysicalMemory) / 1MB
$memTotal = $os.TotalVisibleMemorySize / 1MB
$memPercent = [math]::Round($memUsed / $memTotal * 100, 1)
Write-Host "  Memory: $([math]::Round($memUsed, 1))GB / $([math]::Round($memTotal, 1))GB ($memPercent%)" -ForegroundColor Gray

# Disk
$disk = Get-WmiObject Win32_LogicalDisk -Filter "DeviceID='D:'"
$diskFree = $disk.FreeSpace / 1GB
$diskTotal = $disk.Size / 1GB
$diskPercent = [math]::Round(($diskTotal - $diskFree) / $diskTotal * 100, 1)
Write-Host "  Disk D: $([math]::Round($diskFree, 1))GB free / $([math]::Round($diskTotal, 1))GB ($diskPercent% used)" -ForegroundColor Gray

# Resource alerts
Write-Host "`nResource Alerts:" -ForegroundColor Yellow
$alerts = @()
if ($cpu.Average -gt 90) { $alerts += "🔴 CPU usage critical (>90%)" }
elseif ($cpu.Average -gt 80) { $alerts += "🟡 CPU usage high (>80%)" }

if ($memPercent -gt 90) { $alerts += "🔴 Memory usage critical (>90%)" }
elseif ($memPercent -gt 80) { $alerts += "🟡 Memory usage high (>80%)" }

if ($diskFree -lt 50) { $alerts += "🔴 Disk space low (<50GB)" }
elseif ($diskFree -lt 100) { $alerts += "🟡 Disk space warning (<100GB)" }

if ($alerts.Count -gt 0) {
    $alerts | ForEach-Object { Write-Host "  $_" -ForegroundColor Yellow }
} else {
    Write-Host "  ✅ All resources normal" -ForegroundColor Green
}

# Agent velocity calculation
Write-Host "`nVelocity Analysis:" -ForegroundColor Yellow
$hoursElapsed = 23 # Update based on project start time
$velocity = if ($hoursElapsed -gt 0) { [math]::Round($totalCompleted / $hoursElapsed, 2) } else { 0 }
$targetVelocity = 2.5
Write-Host "  Current: $velocity agents/hour" -ForegroundColor Gray
Write-Host "  Target: $targetVelocity agents/hour" -ForegroundColor Gray
if ($velocity -ge $targetVelocity) {
    Write-Host "  ✅ On track" -ForegroundColor Green
} elseif ($velocity -ge $targetVelocity * 0.8) {
    Write-Host "  🟡 Slightly behind" -ForegroundColor Yellow
} else {
    Write-Host "  🔴 Behind schedule" -ForegroundColor Red
}

Write-Host "`n=== Status Check Complete ===" -ForegroundColor Cyan
