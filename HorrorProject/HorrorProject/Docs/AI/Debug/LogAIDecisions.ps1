# Log AI Decisions Script

# Log and analyze AI decision-making in Horror Project
# Usage: .\LogAIDecisions.ps1 [-Start] [-Stop] [-Analyze] [-Export]

param(
    [switch]$Start = $false,
    [switch]$Stop = $false,
    [switch]$Analyze = $false,
    [switch]$Export = $false,
    [string]$OutputPath = ".\AI_Decisions_Log.txt",
    [int]$TailLines = 100
)

$ProjectPath = "D:\gptzuo\HorrorProject\HorrorProject"
$LogPath = "$ProjectPath\Saved\Logs\HorrorProject.log"
$DecisionLogPath = "$ProjectPath\Saved\Logs\AI_Decisions.log"

Write-Host "=== AI Decision Logging Tool ===" -ForegroundColor Cyan
Write-Host ""

function Start-DecisionLogging {
    Write-Host "Starting AI decision logging..." -ForegroundColor Yellow

    # Create logging configuration
    $loggingConfig = @"
[Core.Log]
LogAIBehavior=Verbose
LogAIPerception=Verbose
LogBehaviorTree=Verbose
LogAINavigation=Log
LogTemp=Log
"@

    $configPath = "$ProjectPath\Saved\Config\WindowsEditor\Engine.ini"
    $configDir = Split-Path $configPath -Parent

    if (-not (Test-Path $configDir)) {
        New-Item -ItemType Directory -Path $configDir -Force | Out-Null
    }

    $loggingConfig | Out-File -FilePath $configPath -Encoding UTF8 -Append

    Write-Host "Decision logging enabled" -ForegroundColor Green
    Write-Host "Log file: $DecisionLogPath" -ForegroundColor Cyan
    Write-Host "Restart the editor to apply changes" -ForegroundColor Yellow
}

function Stop-DecisionLogging {
    Write-Host "Stopping AI decision logging..." -ForegroundColor Yellow

    $loggingConfig = @"
[Core.Log]
LogAIBehavior=Log
LogAIPerception=Log
LogBehaviorTree=Log
LogAINavigation=Warning
LogTemp=Warning
"@

    $configPath = "$ProjectPath\Saved\Config\WindowsEditor\Engine.ini"
    $loggingConfig | Out-File -FilePath $configPath -Encoding UTF8 -Append

    Write-Host "Decision logging disabled" -ForegroundColor Green
    Write-Host "Restart the editor to apply changes" -ForegroundColor Yellow
}

function Analyze-Decisions {
    Write-Host "Analyzing AI decisions..." -ForegroundColor Yellow
    Write-Host ""

    if (-not (Test-Path $LogPath)) {
        Write-Host "Log file not found: $LogPath" -ForegroundColor Red
        return
    }

    $logContent = Get-Content $LogPath -Tail $TailLines

    # Analyze state transitions
    Write-Host "State Transitions:" -ForegroundColor Cyan
    Write-Host "==================" -ForegroundColor Cyan
    $stateTransitions = $logContent | Where-Object { $_ -match "Transition|State.*Changed" }
    $stateCount = ($stateTransitions | Measure-Object).Count
    Write-Host "Total transitions: $stateCount"

    $stateTransitions | Group-Object {
        if ($_ -match "to\s+(\w+)") { $matches[1] }
    } | Sort-Object Count -Descending | ForEach-Object {
        Write-Host "  $($_.Name): $($_.Count) times" -ForegroundColor Yellow
    }
    Write-Host ""

    # Analyze perception events
    Write-Host "Perception Events:" -ForegroundColor Cyan
    Write-Host "==================" -ForegroundColor Cyan
    $perceptionEvents = $logContent | Where-Object { $_ -match "Sight:|Hearing:|Damage:" }
    $perceptionCount = ($perceptionEvents | Measure-Object).Count
    Write-Host "Total events: $perceptionCount"

    $sightEvents = ($perceptionEvents | Where-Object { $_ -match "Sight:" } | Measure-Object).Count
    $hearingEvents = ($perceptionEvents | Where-Object { $_ -match "Hearing:" } | Measure-Object).Count
    $damageEvents = ($perceptionEvents | Where-Object { $_ -match "Damage:" } | Measure-Object).Count

    Write-Host "  Sight events: $sightEvents" -ForegroundColor Blue
    Write-Host "  Hearing events: $hearingEvents" -ForegroundColor Yellow
    Write-Host "  Damage events: $damageEvents" -ForegroundColor Red
    Write-Host ""

    # Analyze behavior tree execution
    Write-Host "Behavior Tree Execution:" -ForegroundColor Cyan
    Write-Host "========================" -ForegroundColor Cyan
    $btEvents = $logContent | Where-Object { $_ -match "Task|Decorator|Service" }
    $btCount = ($btEvents | Measure-Object).Count
    Write-Host "Total BT events: $btCount"

    $taskEvents = ($btEvents | Where-Object { $_ -match "Task" } | Measure-Object).Count
    $decoratorEvents = ($btEvents | Where-Object { $_ -match "Decorator" } | Measure-Object).Count
    $serviceEvents = ($btEvents | Where-Object { $_ -match "Service" } | Measure-Object).Count

    Write-Host "  Task executions: $taskEvents" -ForegroundColor Green
    Write-Host "  Decorator checks: $decoratorEvents" -ForegroundColor Cyan
    Write-Host "  Service updates: $serviceEvents" -ForegroundColor Magenta
    Write-Host ""

    # Analyze decision patterns
    Write-Host "Decision Patterns:" -ForegroundColor Cyan
    Write-Host "==================" -ForegroundColor Cyan

    # Find common decision sequences
    $sequences = @{}
    for ($i = 0; $i -lt $stateTransitions.Count - 1; $i++) {
        $current = $stateTransitions[$i]
        $next = $stateTransitions[$i + 1]

        if ($current -match "to\s+(\w+)" -and $next -match "to\s+(\w+)") {
            $sequence = "$($matches[1]) -> $($matches[1])"
            if ($sequences.ContainsKey($sequence)) {
                $sequences[$sequence]++
            } else {
                $sequences[$sequence] = 1
            }
        }
    }

    $sequences.GetEnumerator() | Sort-Object Value -Descending | Select-Object -First 5 | ForEach-Object {
        Write-Host "  $($_.Key): $($_.Value) times" -ForegroundColor Yellow
    }
    Write-Host ""

    # Analyze errors and warnings
    Write-Host "Errors and Warnings:" -ForegroundColor Cyan
    Write-Host "====================" -ForegroundColor Cyan
    $errors = $logContent | Where-Object { $_ -match "Error" }
    $warnings = $logContent | Where-Object { $_ -match "Warning" }

    $errorCount = ($errors | Measure-Object).Count
    $warningCount = ($warnings | Measure-Object).Count

    Write-Host "  Errors: $errorCount" -ForegroundColor Red
    Write-Host "  Warnings: $warningCount" -ForegroundColor Yellow

    if ($errorCount -gt 0) {
        Write-Host "`nRecent Errors:" -ForegroundColor Red
        $errors | Select-Object -Last 3 | ForEach-Object {
            Write-Host "  $_" -ForegroundColor Red
        }
    }
    Write-Host ""

    # Performance metrics
    Write-Host "Performance Metrics:" -ForegroundColor Cyan
    Write-Host "====================" -ForegroundColor Cyan
    $perfLines = $logContent | Where-Object { $_ -match "ms|Performance" }

    if ($perfLines) {
        Write-Host "Recent performance data:"
        $perfLines | Select-Object -Last 5 | ForEach-Object {
            Write-Host "  $_"
        }
    } else {
        Write-Host "No performance data found"
    }
    Write-Host ""
}

function Export-DecisionLog {
    Write-Host "Exporting AI decision log..." -ForegroundColor Yellow

    if (-not (Test-Path $LogPath)) {
        Write-Host "Log file not found: $LogPath" -ForegroundColor Red
        return
    }

    $logContent = Get-Content $LogPath
    $aiDecisions = $logContent | Where-Object {
        $_ -match "AI|Behavior|Perception|State|Task|Decorator|Service"
    }

    # Create structured export
    $export = @"
AI Decision Log Export
======================
Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
Project: HorrorProject
Total Entries: $($aiDecisions.Count)

======================
Decision Log:
======================

"@

    $export += ($aiDecisions -join "`n")

    $export | Out-File -FilePath $OutputPath -Encoding UTF8

    Write-Host "Decision log exported to: $OutputPath" -ForegroundColor Green
    Write-Host "Total entries: $($aiDecisions.Count)" -ForegroundColor Cyan
}

function Show-DecisionTimeline {
    Write-Host "AI Decision Timeline:" -ForegroundColor Yellow
    Write-Host "=====================" -ForegroundColor Yellow
    Write-Host ""

    if (-not (Test-Path $LogPath)) {
        Write-Host "Log file not found: $LogPath" -ForegroundColor Red
        return
    }

    $logContent = Get-Content $LogPath -Tail 50
    $decisions = $logContent | Where-Object {
        $_ -match "(State|Perception|Task).*:"
    }

    $decisions | ForEach-Object {
        if ($_ -match "(\d{2}:\d{2}:\d{2}).*") {
            $time = $matches[1]

            if ($_ -match "State") {
                Write-Host "[$time] " -NoNewline
                Write-Host "STATE" -ForegroundColor Cyan -NoNewline
                Write-Host " $_"
            }
            elseif ($_ -match "Sight") {
                Write-Host "[$time] " -NoNewline
                Write-Host "SIGHT" -ForegroundColor Blue -NoNewline
                Write-Host " $_"
            }
            elseif ($_ -match "Hearing") {
                Write-Host "[$time] " -NoNewline
                Write-Host "HEAR " -ForegroundColor Yellow -NoNewline
                Write-Host " $_"
            }
            elseif ($_ -match "Damage") {
                Write-Host "[$time] " -NoNewline
                Write-Host "DAMAGE" -ForegroundColor Red -NoNewline
                Write-Host " $_"
            }
            elseif ($_ -match "Task") {
                Write-Host "[$time] " -NoNewline
                Write-Host "TASK" -ForegroundColor Green -NoNewline
                Write-Host " $_"
            }
        }
    }
    Write-Host ""
}

function Generate-DecisionReport {
    Write-Host "Generating decision report..." -ForegroundColor Yellow

    $reportPath = "$ProjectPath\Docs\AI\Debug\AI_Decision_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').html"

    $report = @"
<!DOCTYPE html>
<html>
<head>
    <title>AI Decision Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #1e1e1e; color: #d4d4d4; }
        h1 { color: #4ec9b0; }
        h2 { color: #569cd6; border-bottom: 2px solid #569cd6; padding-bottom: 5px; }
        .metric { background: #2d2d30; padding: 15px; margin: 10px 0; border-radius: 5px; }
        .metric-value { font-size: 24px; font-weight: bold; color: #4ec9b0; }
        .metric-label { color: #9cdcfe; }
        .state { color: #ce9178; }
        .perception { color: #dcdcaa; }
        .error { color: #f48771; }
        .warning { color: #d7ba7d; }
        table { width: 100%; border-collapse: collapse; margin: 10px 0; }
        th { background: #2d2d30; padding: 10px; text-align: left; color: #569cd6; }
        td { padding: 8px; border-bottom: 1px solid #3e3e42; }
        tr:hover { background: #2d2d30; }
    </style>
</head>
<body>
    <h1>AI Decision Report</h1>
    <p>Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')</p>

    <h2>Summary</h2>
    <div class="metric">
        <div class="metric-label">Total Decisions</div>
        <div class="metric-value">--</div>
    </div>

    <h2>State Transitions</h2>
    <table>
        <tr><th>State</th><th>Count</th><th>Percentage</th></tr>
        <!-- Data will be filled by analysis -->
    </table>

    <h2>Perception Events</h2>
    <table>
        <tr><th>Type</th><th>Count</th><th>Percentage</th></tr>
        <!-- Data will be filled by analysis -->
    </table>

    <h2>Performance</h2>
    <div class="metric">
        <div class="metric-label">Average Decision Time</div>
        <div class="metric-value">-- ms</div>
    </div>

    <h2>Issues</h2>
    <div class="error">Errors: --</div>
    <div class="warning">Warnings: --</div>
</body>
</html>
"@

    $report | Out-File -FilePath $reportPath -Encoding UTF8

    Write-Host "Report generated: $reportPath" -ForegroundColor Green
}

# Main execution
if ($Start) {
    Start-DecisionLogging
} elseif ($Stop) {
    Stop-DecisionLogging
} elseif ($Analyze) {
    Analyze-Decisions
    Show-DecisionTimeline
} elseif ($Export) {
    Export-DecisionLog
} else {
    Write-Host "AI Decision Logging Tool" -ForegroundColor Cyan
    Write-Host "========================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Usage:" -ForegroundColor Yellow
    Write-Host "  .\LogAIDecisions.ps1 -Start      # Enable decision logging"
    Write-Host "  .\LogAIDecisions.ps1 -Stop       # Disable decision logging"
    Write-Host "  .\LogAIDecisions.ps1 -Analyze    # Analyze recent decisions"
    Write-Host "  .\LogAIDecisions.ps1 -Export     # Export decision log"
    Write-Host ""
    Write-Host "Options:" -ForegroundColor Yellow
    Write-Host "  -OutputPath <path>   # Specify export file path"
    Write-Host "  -TailLines <number>  # Number of log lines to analyze (default: 100)"
    Write-Host ""
    Write-Host "Examples:" -ForegroundColor Cyan
    Write-Host "  .\LogAIDecisions.ps1 -Analyze -TailLines 500"
    Write-Host "  .\LogAIDecisions.ps1 -Export -OutputPath 'C:\Logs\AI_Log.txt'"
    Write-Host ""

    Generate-DecisionReport
}

Write-Host "`nDecision logging tool complete." -ForegroundColor Green
