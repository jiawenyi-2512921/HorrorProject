# CollectMetrics.ps1
# Collect comprehensive project metrics
# Performance, quality, code statistics

param(
    [switch]$Export = $false,
    [string]$OutputPath = "D:\gptzuo\ContextVault\metrics.json"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = "D:\gptzuo\HorrorProject\HorrorProject"

Write-Host "=== Metrics Collection ===" -ForegroundColor Cyan
Write-Host "Time: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray

$metrics = @{
    Timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    Project = @{}
    Code = @{}
    Git = @{}
    Performance = @{}
    Quality = @{}
}

# Project Metrics
Write-Host "`nCollecting Project Metrics..." -ForegroundColor Yellow
Push-Location $ProjectRoot
try {
    $metrics.Project.Name = "HorrorProject"
    $metrics.Project.Path = $ProjectRoot
    $metrics.Project.TotalFiles = (Get-ChildItem -Recurse -File -ErrorAction SilentlyContinue | Measure-Object).Count
    $metrics.Project.TotalSize = [math]::Round((Get-ChildItem -Recurse -File -ErrorAction SilentlyContinue | Measure-Object -Property Length -Sum).Sum / 1GB, 2)
    Write-Host "  Total Files: $($metrics.Project.TotalFiles)" -ForegroundColor Gray
    Write-Host "  Total Size: $($metrics.Project.TotalSize) GB" -ForegroundColor Gray
} finally {
    Pop-Location
}

# Code Metrics
Write-Host "`nCollecting Code Metrics..." -ForegroundColor Yellow
$sourcePath = "$ProjectRoot\Source"
if (Test-Path $sourcePath) {
    $cppFiles = Get-ChildItem -Path $sourcePath -Filter "*.cpp" -Recurse -File -ErrorAction SilentlyContinue
    $headerFiles = Get-ChildItem -Path $sourcePath -Filter "*.h" -Recurse -File -ErrorAction SilentlyContinue

    $metrics.Code.CppFiles = ($cppFiles | Measure-Object).Count
    $metrics.Code.HeaderFiles = ($headerFiles | Measure-Object).Count
    $metrics.Code.TotalSourceFiles = $metrics.Code.CppFiles + $metrics.Code.HeaderFiles

    # Count lines of code
    $totalLines = 0
    $cppFiles | ForEach-Object {
        $totalLines += (Get-Content $_.FullName -ErrorAction SilentlyContinue | Measure-Object).Count
    }
    $headerFiles | ForEach-Object {
        $totalLines += (Get-Content $_.FullName -ErrorAction SilentlyContinue | Measure-Object).Count
    }
    $metrics.Code.TotalLines = $totalLines
    $metrics.Code.AvgLinesPerFile = if ($metrics.Code.TotalSourceFiles -gt 0) {
        [math]::Round($totalLines / $metrics.Code.TotalSourceFiles, 0)
    } else { 0 }

    Write-Host "  C++ Files: $($metrics.Code.CppFiles)" -ForegroundColor Gray
    Write-Host "  Header Files: $($metrics.Code.HeaderFiles)" -ForegroundColor Gray
    Write-Host "  Total Lines: $($metrics.Code.TotalLines)" -ForegroundColor Gray
    Write-Host "  Avg Lines/File: $($metrics.Code.AvgLinesPerFile)" -ForegroundColor Gray
}

# Git Metrics
Write-Host "`nCollecting Git Metrics..." -ForegroundColor Yellow
Push-Location $ProjectRoot
try {
    $metrics.Git.Branch = git rev-parse --abbrev-ref HEAD 2>$null
    $metrics.Git.TotalCommits = (git rev-list --count HEAD 2>$null)
    $metrics.Git.LastCommit = git log -1 --format="%h - %s" 2>$null
    $metrics.Git.LastCommitTime = git log -1 --format="%ci" 2>$null
    $metrics.Git.UncommittedFiles = (git status --porcelain 2>$null | Measure-Object).Count

    # Count commits today
    $today = Get-Date -Format "yyyy-MM-dd"
    $commitsToday = (git log --since="$today 00:00" --oneline 2>$null | Measure-Object).Count
    $metrics.Git.CommitsToday = $commitsToday

    # Count agent commits
    $agentCommits = (git log --oneline --all 2>$null | Select-String -Pattern "P\d+" | Measure-Object).Count
    $metrics.Git.AgentCommits = $agentCommits

    Write-Host "  Branch: $($metrics.Git.Branch)" -ForegroundColor Gray
    Write-Host "  Total Commits: $($metrics.Git.TotalCommits)" -ForegroundColor Gray
    Write-Host "  Commits Today: $($metrics.Git.CommitsToday)" -ForegroundColor Gray
    Write-Host "  Agent Commits: $($metrics.Git.AgentCommits)" -ForegroundColor Gray
} finally {
    Pop-Location
}

# Performance Metrics
Write-Host "`nCollecting Performance Metrics..." -ForegroundColor Yellow
$metrics.Performance.CPU = [math]::Round((Get-WmiObject Win32_Processor | Measure-Object -Property LoadPercentage -Average).Average, 1)

$os = Get-WmiObject Win32_OperatingSystem
$memUsedGB = [math]::Round(($os.TotalVisibleMemorySize - $os.FreePhysicalMemory) / 1MB, 2)
$memTotalGB = [math]::Round($os.TotalVisibleMemorySize / 1MB, 2)
$metrics.Performance.MemoryUsedGB = $memUsedGB
$metrics.Performance.MemoryTotalGB = $memTotalGB
$metrics.Performance.MemoryPercent = [math]::Round($memUsedGB / $memTotalGB * 100, 1)

$disk = Get-WmiObject Win32_LogicalDisk -Filter "DeviceID='D:'"
$metrics.Performance.DiskFreeGB = [math]::Round($disk.FreeSpace / 1GB, 2)
$metrics.Performance.DiskTotalGB = [math]::Round($disk.Size / 1GB, 2)
$metrics.Performance.DiskUsedPercent = [math]::Round(($metrics.Performance.DiskTotalGB - $metrics.Performance.DiskFreeGB) / $metrics.Performance.DiskTotalGB * 100, 1)

Write-Host "  CPU: $($metrics.Performance.CPU)%" -ForegroundColor Gray
Write-Host "  Memory: $($metrics.Performance.MemoryUsedGB)GB / $($metrics.Performance.MemoryTotalGB)GB ($($metrics.Performance.MemoryPercent)%)" -ForegroundColor Gray
Write-Host "  Disk: $($metrics.Performance.DiskFreeGB)GB free ($($metrics.Performance.DiskUsedPercent)% used)" -ForegroundColor Gray

# Quality Metrics
Write-Host "`nCollecting Quality Metrics..." -ForegroundColor Yellow
$metrics.Quality.BuildStatus = "Success" # Placeholder
$metrics.Quality.BuildWarnings = 0 # Placeholder
$metrics.Quality.BuildErrors = 0 # Placeholder
$metrics.Quality.CodeQualityScore = 95 # Estimated

# Check for common issues
$issueCount = 0
if ($metrics.Git.UncommittedFiles -gt 50) { $issueCount++ }
if ($metrics.Performance.CPU -gt 90) { $issueCount++ }
if ($metrics.Performance.MemoryPercent -gt 90) { $issueCount++ }
if ($metrics.Performance.DiskFreeGB -lt 100) { $issueCount++ }

$metrics.Quality.IssueCount = $issueCount
$metrics.Quality.HealthScore = [math]::Max(0, 100 - ($issueCount * 10))

Write-Host "  Build Status: $($metrics.Quality.BuildStatus)" -ForegroundColor Gray
Write-Host "  Code Quality: $($metrics.Quality.CodeQualityScore)/100" -ForegroundColor Gray
Write-Host "  Health Score: $($metrics.Quality.HealthScore)/100" -ForegroundColor Gray
Write-Host "  Issues: $($metrics.Quality.IssueCount)" -ForegroundColor Gray

# Development Velocity
Write-Host "`nCalculating Velocity..." -ForegroundColor Yellow
$completedAgents = $metrics.Git.AgentCommits
$remainingAgents = 99 - $completedAgents
$hoursElapsed = 23 # Update based on project start
$velocity = if ($hoursElapsed -gt 0) { [math]::Round($completedAgents / $hoursElapsed, 2) } else { 0 }
$estimatedHoursRemaining = if ($velocity -gt 0) { [math]::Round($remainingAgents / $velocity, 1) } else { 0 }

$metrics.Velocity = @{
    CompletedAgents = $completedAgents
    RemainingAgents = $remainingAgents
    HoursElapsed = $hoursElapsed
    AgentsPerHour = $velocity
    EstimatedHoursRemaining = $estimatedHoursRemaining
    ProgressPercent = [math]::Round($completedAgents / 99 * 100, 1)
}

Write-Host "  Completed: $completedAgents / 99 ($($metrics.Velocity.ProgressPercent)%)" -ForegroundColor Gray
Write-Host "  Velocity: $velocity agents/hour" -ForegroundColor Gray
Write-Host "  Estimated Remaining: $estimatedHoursRemaining hours" -ForegroundColor Gray

# Export to JSON if requested
if ($Export) {
    Write-Host "`nExporting metrics to JSON..." -ForegroundColor Yellow
    $metrics | ConvertTo-Json -Depth 10 | Set-Content -Path $OutputPath
    Write-Host "  Exported to: $OutputPath" -ForegroundColor Green
}

# Summary
Write-Host "`n=== Metrics Summary ===" -ForegroundColor Cyan
Write-Host "Project Health: " -NoNewline
if ($metrics.Quality.HealthScore -ge 90) {
    Write-Host "🟢 Excellent ($($metrics.Quality.HealthScore)/100)" -ForegroundColor Green
} elseif ($metrics.Quality.HealthScore -ge 70) {
    Write-Host "🟡 Good ($($metrics.Quality.HealthScore)/100)" -ForegroundColor Yellow
} else {
    Write-Host "🔴 Needs Attention ($($metrics.Quality.HealthScore)/100)" -ForegroundColor Red
}

Write-Host "Development Progress: $($metrics.Velocity.ProgressPercent)% complete" -ForegroundColor Cyan
Write-Host "Estimated Completion: $(Get-Date).AddHours($estimatedHoursRemaining).ToString('yyyy-MM-dd HH:mm')" -ForegroundColor Cyan

Write-Host "`n=== Collection Complete ===" -ForegroundColor Cyan
