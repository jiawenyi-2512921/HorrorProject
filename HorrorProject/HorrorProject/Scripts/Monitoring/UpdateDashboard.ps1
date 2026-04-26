# UpdateDashboard.ps1
# Real-time dashboard update script
# Updates DASHBOARD.md with current project status

param(
    [switch]$Verbose = $false
)

$ErrorActionPreference = "Stop"
$ProjectRoot = "D:\gptzuo\HorrorProject\HorrorProject"
$VaultRoot = "D:\gptzuo\ContextVault"
$DashboardFile = "$VaultRoot\DASHBOARD.md"

Write-Host "=== Dashboard Update Script ===" -ForegroundColor Cyan
Write-Host "Time: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray

# Function to count files
function Get-FileCount {
    param([string]$Path, [string]$Pattern)
    if (Test-Path $Path) {
        return (Get-ChildItem -Path $Path -Filter $Pattern -Recurse -File -ErrorAction SilentlyContinue | Measure-Object).Count
    }
    return 0
}

# Function to get git status
function Get-GitStatus {
    Push-Location $ProjectRoot
    try {
        $branch = git rev-parse --abbrev-ref HEAD 2>$null
        $lastCommit = git log -1 --oneline 2>$null
        $uncommitted = (git status --porcelain 2>$null | Measure-Object).Count
        return @{
            Branch = $branch
            LastCommit = $lastCommit
            Uncommitted = $uncommitted
        }
    } finally {
        Pop-Location
    }
}

# Function to count completed agents
function Get-CompletedAgents {
    Push-Location $ProjectRoot
    try {
        $commits = git log --oneline --all 2>$null
        $agentCommits = $commits | Select-String -Pattern "P\d+" | Measure-Object
        return $agentCommits.Count
    } finally {
        Pop-Location
    }
}

# Gather metrics
Write-Host "Gathering metrics..." -ForegroundColor Yellow

$gitStatus = Get-GitStatus
$totalFiles = Get-FileCount -Path $ProjectRoot -Pattern "*.*"
$cppFiles = Get-FileCount -Path "$ProjectRoot\Source" -Pattern "*.cpp"
$headerFiles = Get-FileCount -Path "$ProjectRoot\Source" -Pattern "*.h"
$completedAgents = Get-CompletedAgents
$remainingAgents = 99 - $completedAgents

Write-Host "  Total Files: $totalFiles" -ForegroundColor Gray
Write-Host "  C++ Files: $cppFiles" -ForegroundColor Gray
Write-Host "  Header Files: $headerFiles" -ForegroundColor Gray
Write-Host "  Completed Agents: $completedAgents" -ForegroundColor Gray
Write-Host "  Remaining Agents: $remainingAgents" -ForegroundColor Gray

# Calculate velocity
$hoursElapsed = 23 # Update based on actual time
$velocity = if ($hoursElapsed -gt 0) { [math]::Round($completedAgents / $hoursElapsed, 2) } else { 0 }
$estimatedHours = if ($velocity -gt 0) { [math]::Round($remainingAgents / $velocity, 1) } else { 0 }

Write-Host "  Velocity: $velocity agents/hour" -ForegroundColor Gray
Write-Host "  Estimated Remaining: $estimatedHours hours" -ForegroundColor Gray

# Update timestamp in dashboard
if (Test-Path $DashboardFile) {
    $content = Get-Content $DashboardFile -Raw
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $content = $content -replace '\*\*Last Updated:\*\* \d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}', "**Last Updated:** $timestamp"

    # Update agent counts
    $content = $content -replace '\*\*Active Agents:\*\* \d+', "**Active Agents:** $completedAgents"
    $content = $content -replace '\*\*Queued Agents:\*\* \d+', "**Queued Agents:** $remainingAgents"

    # Update file counts
    $content = $content -replace '\*\*Total Files:\*\* \d+', "**Total Files:** $totalFiles"

    # Update git status
    $content = $content -replace '\*\*Last Commit:\*\* `[^`]+`', "**Last Commit:** ``$($gitStatus.LastCommit)``"
    $content = $content -replace '\*\*Uncommitted Changes:\*\* \d+', "**Uncommitted Changes:** $($gitStatus.Uncommitted)"

    Set-Content -Path $DashboardFile -Value $content -NoNewline
    Write-Host "Dashboard updated successfully!" -ForegroundColor Green
} else {
    Write-Host "Dashboard file not found: $DashboardFile" -ForegroundColor Red
    exit 1
}

# Update PROGRESS_TRACKER.md
$trackerFile = "$VaultRoot\PROGRESS_TRACKER.md"
if (Test-Path $trackerFile) {
    $trackerContent = Get-Content $trackerFile -Raw
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $trackerContent = $trackerContent -replace '\*\*Last Updated:\*\* \d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}', "**Last Updated:** $timestamp"
    Set-Content -Path $trackerFile -Value $trackerContent -NoNewline
    Write-Host "Progress tracker updated!" -ForegroundColor Green
}

# Update ALERTS.md
$alertsFile = "$VaultRoot\ALERTS.md"
if (Test-Path $alertsFile) {
    $alertsContent = Get-Content $alertsFile -Raw
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $alertsContent = $alertsContent -replace '\*\*Last Updated:\*\* \d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}', "**Last Updated:** $timestamp"
    Set-Content -Path $alertsFile -Value $alertsContent -NoNewline
    Write-Host "Alerts updated!" -ForegroundColor Green
}

Write-Host "`n=== Update Complete ===" -ForegroundColor Cyan
Write-Host "Next update in 5 minutes" -ForegroundColor Gray
