# CollectMetrics.ps1
# Collect comprehensive project metrics
# Performance, quality, code statistics

param(
    [switch]$Export = $false,
    [string]$OutputPath = ""
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "MonitoringCommon.ps1")

$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
$WorkspaceRoot = Split-Path -Parent (Split-Path -Parent $ProjectRoot)
$VaultRoot = if ($env:CONTEXTVAULT_ROOT) { $env:CONTEXTVAULT_ROOT } else { Join-Path $WorkspaceRoot "ContextVault" }
if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $VaultRoot "metrics.json"
}

Write-Host "=== Metrics Collection ===" -ForegroundColor Cyan
Write-Host "Time: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray

function Get-LatestFile {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path,
        [Parameter(Mandatory = $true)]
        [string]$Filter
    )

    if (-not (Test-Path -LiteralPath $Path)) {
        return $null
    }

    return Get-ChildItem -LiteralPath $Path -Filter $Filter -File -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1
}

function Get-LogIssueCount {
    param(
        [string[]]$Lines,
        [string]$Pattern
    )

    if (-not $Lines) {
        return 0
    }

    return @($Lines | Select-String -Pattern $Pattern -CaseSensitive:$false).Count
}

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
$latestCompilationLog = Get-LatestFile -Path (Join-Path $ProjectRoot "Saved\Logs") -Filter "CompilationValidation_*.log"
$latestPackageLog = Get-LatestFile -Path (Join-Path $ProjectRoot "Build\Logs\Package") -Filter "Package_*.log"
$securityReportPath = Join-Path $ProjectRoot "Docs\Security\SecurityAnalysisProbe.json"

$buildWarnings = 0
$buildErrors = 0
$buildStatus = "Unknown"

if ($latestCompilationLog) {
    $compilationLines = Get-Content -LiteralPath $latestCompilationLog.FullName -ErrorAction SilentlyContinue
    $buildWarnings += Get-LogIssueCount -Lines $compilationLines -Pattern '(^|\s)(warning|warning C\d+)[:\s]'
    $buildErrors += Get-LogIssueCount -Lines $compilationLines -Pattern '(^|\s)(error|error C\d+)[:\s]'

    if ($compilationLines -match 'Result:\s+Succeeded') {
        $buildStatus = "Success"
    } elseif ($compilationLines -match 'Result:\s+Failed|BUILD FAILED') {
        $buildStatus = "Failed"
    }
}

if ($latestPackageLog) {
    $packageLines = Get-Content -LiteralPath $latestPackageLog.FullName -ErrorAction SilentlyContinue
    $buildWarnings += Get-LogIssueCount -Lines $packageLines -Pattern '\[(WARN|WARNING)\]|:\s*Warning:'
    $buildErrors += Get-LogIssueCount -Lines $packageLines -Pattern '\[ERROR\]|:\s*Error:|PACKAGE FAILED'

    if ($packageLines -match 'PACKAGE FAILED') {
        $buildStatus = "Failed"
    } elseif ($buildStatus -eq "Unknown" -and $packageLines -match 'PACKAGE COMPLETE') {
        $buildStatus = "Success"
    }
}

$securitySummary = $null
if (Test-Path -LiteralPath $securityReportPath) {
    try {
        $securitySummary = (Get-Content -LiteralPath $securityReportPath -Raw | ConvertFrom-Json).Summary
    } catch {
        Write-Warning "Unable to parse security report: $securityReportPath"
    }
}

$criticalIssues = if ($securitySummary) { [int]$securitySummary.Critical } else { 0 }
$highIssues = if ($securitySummary) { [int]$securitySummary.High } else { 0 }
$mediumIssues = if ($securitySummary) { [int]$securitySummary.Medium } else { 0 }
$lowIssues = if ($securitySummary) { [int]$securitySummary.Low } else { 0 }
$qualityPenalty = ($criticalIssues * 25) + ($highIssues * 15) + ($mediumIssues * 5) + [math]::Min(10, $lowIssues * 0.02) + ($buildErrors * 10) + ($buildWarnings * 2)

$metrics.Quality.BuildStatus = $buildStatus
$metrics.Quality.BuildWarnings = $buildWarnings
$metrics.Quality.BuildErrors = $buildErrors
$metrics.Quality.LatestCompilationLog = if ($latestCompilationLog) { $latestCompilationLog.FullName } else { "" }
$metrics.Quality.LatestPackageLog = if ($latestPackageLog) { $latestPackageLog.FullName } else { "" }
$metrics.Quality.SecurityCritical = $criticalIssues
$metrics.Quality.SecurityHigh = $highIssues
$metrics.Quality.SecurityMedium = $mediumIssues
$metrics.Quality.SecurityLow = $lowIssues
$metrics.Quality.CodeQualityScore = [math]::Max(0, [math]::Round(100 - $qualityPenalty, 1))

# Check for common issues
$issueCount = 0
if ($metrics.Git.UncommittedFiles -gt 50) { $issueCount++ }
if ($metrics.Performance.CPU -gt 90) { $issueCount++ }
if ($metrics.Performance.MemoryPercent -gt 90) { $issueCount++ }
if ($metrics.Performance.DiskFreeGB -lt 100) { $issueCount++ }
if ($metrics.Quality.BuildStatus -ne "Success") { $issueCount++ }
if ($metrics.Quality.BuildWarnings -gt 0) { $issueCount++ }
if (($criticalIssues + $highIssues + $mediumIssues) -gt 0) { $issueCount++ }

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
$estimatedCompletion = if ($estimatedHoursRemaining -gt 0) { (Get-Date).AddHours($estimatedHoursRemaining) } else { Get-Date }

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
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $OutputPath) | Out-Null
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
Write-Host "Estimated Completion: $($estimatedCompletion.ToString('yyyy-MM-dd HH:mm'))" -ForegroundColor Cyan

Write-Host "`n=== Collection Complete ===" -ForegroundColor Cyan
