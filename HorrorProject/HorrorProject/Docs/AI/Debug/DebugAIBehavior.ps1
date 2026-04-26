# AI Behavior Debug Script

# Debug AI behavior in Horror Project
# Usage: .\DebugAIBehavior.ps1 [-AIName <name>] [-ShowAll] [-Continuous]

param(
    [string]$AIName = "",
    [switch]$ShowAll = $false,
    [switch]$Continuous = $false,
    [int]$RefreshInterval = 1
)

$ProjectPath = "D:\gptzuo\HorrorProject\HorrorProject"
$UE5Root = if ($env:UE5_ROOT) { $env:UE5_ROOT } elseif ($env:UE_5_6_ROOT) { $env:UE_5_6_ROOT } elseif (Test-Path 'D:\UnrealEngine\UE_5.6') { 'D:\UnrealEngine\UE_5.6' } else { 'C:\Program Files\Epic Games\UE_5.6' }
$UEEditorPath = Join-Path $UE5Root "Engine\Binaries\Win64\UnrealEditor.exe"
$LogPath = "$ProjectPath\Saved\Logs\HorrorProject.log"

Write-Host "=== AI Behavior Debug Tool ===" -ForegroundColor Cyan
Write-Host ""

function Get-AIStatus {
    param([string]$Name)

    if (-not (Test-Path $LogPath)) {
        Write-Host "Log file not found: $LogPath" -ForegroundColor Red
        return
    }

    $recentLines = Get-Content $LogPath -Tail 1000

    if ($Name) {
        $aiLines = $recentLines | Where-Object { $_ -match $Name }
    } else {
        $aiLines = $recentLines | Where-Object { $_ -match "AI|Behavior|Perception|Golem|Threat" }
    }

    return $aiLines
}

function Show-AIBehaviorState {
    param([string]$Name)

    Write-Host "AI Behavior State:" -ForegroundColor Yellow
    Write-Host "==================" -ForegroundColor Yellow

    $status = Get-AIStatus -Name $Name

    if ($status) {
        # Parse behavior states
        $stateLines = $status | Where-Object { $_ -match "State:|CurrentState|Transition" }

        if ($stateLines) {
            Write-Host "`nState Transitions:" -ForegroundColor Green
            $stateLines | ForEach-Object {
                if ($_ -match "(\d{2}:\d{2}:\d{2}).*State.*:(.*)") {
                    $time = $matches[1]
                    $state = $matches[2].Trim()
                    Write-Host "  [$time] $state"
                }
            }
        }

        # Parse distances
        $distanceLines = $status | Where-Object { $_ -match "Distance|Dist:" }

        if ($distanceLines) {
            Write-Host "`nDistance Information:" -ForegroundColor Green
            $distanceLines | Select-Object -Last 5 | ForEach-Object {
                if ($_ -match "(\d{2}:\d{2}:\d{2}).*Dist.*:(.*)") {
                    $time = $matches[1]
                    $dist = $matches[2].Trim()
                    Write-Host "  [$time] $dist"
                }
            }
        }

        # Parse perception events
        $perceptionLines = $status | Where-Object { $_ -match "Sight:|Hearing:|Damage:" }

        if ($perceptionLines) {
            Write-Host "`nPerception Events:" -ForegroundColor Green
            $perceptionLines | Select-Object -Last 10 | ForEach-Object {
                if ($_ -match "(\d{2}:\d{2}:\d{2}).*(Sight|Hearing|Damage):(.*)") {
                    $time = $matches[1]
                    $sense = $matches[2]
                    $event = $matches[3].Trim()

                    $color = switch ($sense) {
                        "Sight" { "Cyan" }
                        "Hearing" { "Yellow" }
                        "Damage" { "Red" }
                        default { "White" }
                    }

                    Write-Host "  [$time] " -NoNewline
                    Write-Host "$sense" -ForegroundColor $color -NoNewline
                    Write-Host ": $event"
                }
            }
        }

        # Parse errors/warnings
        $errorLines = $status | Where-Object { $_ -match "Error|Warning" }

        if ($errorLines) {
            Write-Host "`nErrors/Warnings:" -ForegroundColor Red
            $errorLines | Select-Object -Last 5 | ForEach-Object {
                Write-Host "  $_" -ForegroundColor Red
            }
        }
    } else {
        Write-Host "No AI behavior data found" -ForegroundColor Yellow
    }
}

function Show-AllAI {
    Write-Host "All Active AI:" -ForegroundColor Yellow
    Write-Host "==============" -ForegroundColor Yellow

    $status = Get-AIStatus

    # Extract unique AI names
    $aiNames = $status | Where-Object { $_ -match "AI|Golem|Threat" } | ForEach-Object {
        if ($_ -match "(Golem|Threat)_\w+") {
            $matches[0]
        }
    } | Select-Object -Unique

    if ($aiNames) {
        $aiNames | ForEach-Object {
            Write-Host "  - $_" -ForegroundColor Cyan
        }
    } else {
        Write-Host "No active AI found" -ForegroundColor Yellow
    }
}

function Show-BehaviorTreeStatus {
    param([string]$Name)

    Write-Host "`nBehavior Tree Status:" -ForegroundColor Yellow
    Write-Host "=====================" -ForegroundColor Yellow

    $status = Get-AIStatus -Name $Name
    $btLines = $status | Where-Object { $_ -match "BehaviorTree|Task|Decorator|Service" }

    if ($btLines) {
        $btLines | Select-Object -Last 10 | ForEach-Object {
            if ($_ -match "(\d{2}:\d{2}:\d{2}).*(Task|Decorator|Service):(.*)") {
                $time = $matches[1]
                $type = $matches[2]
                $info = $matches[3].Trim()

                Write-Host "  [$time] ${type}: $info"
            }
        }
    } else {
        Write-Host "No behavior tree data found" -ForegroundColor Yellow
    }
}

function Show-BlackboardValues {
    param([string]$Name)

    Write-Host "`nBlackboard Values:" -ForegroundColor Yellow
    Write-Host "==================" -ForegroundColor Yellow

    $status = Get-AIStatus -Name $Name
    $bbLines = $status | Where-Object { $_ -match "Blackboard|Key:" }

    if ($bbLines) {
        $bbLines | Select-Object -Last 15 | ForEach-Object {
            if ($_ -match "Key:\s*(\w+)\s*=\s*(.*)") {
                $key = $matches[1]
                $value = $matches[2].Trim()
                Write-Host "  $key = $value" -ForegroundColor Cyan
            }
        }
    } else {
        Write-Host "No blackboard data found" -ForegroundColor Yellow
    }
}

function Show-PerformanceMetrics {
    param([string]$Name)

    Write-Host "`nPerformance Metrics:" -ForegroundColor Yellow
    Write-Host "====================" -ForegroundColor Yellow

    $status = Get-AIStatus -Name $Name
    $perfLines = $status | Where-Object { $_ -match "ms|FPS|Performance" }

    if ($perfLines) {
        $perfLines | Select-Object -Last 5 | ForEach-Object {
            Write-Host "  $_"
        }
    } else {
        Write-Host "No performance data found" -ForegroundColor Yellow
    }
}

# Main execution
if ($ShowAll) {
    Show-AllAI
} else {
    do {
        Clear-Host
        Write-Host "=== AI Behavior Debug Tool ===" -ForegroundColor Cyan
        Write-Host "Time: $(Get-Date -Format 'HH:mm:ss')" -ForegroundColor Gray
        Write-Host ""

        if ($AIName) {
            Write-Host "Debugging AI: $AIName" -ForegroundColor Green
        } else {
            Write-Host "Debugging: All AI" -ForegroundColor Green
        }
        Write-Host ""

        Show-AIBehaviorState -Name $AIName
        Show-BehaviorTreeStatus -Name $AIName
        Show-BlackboardValues -Name $AIName
        Show-PerformanceMetrics -Name $AIName

        if ($Continuous) {
            Write-Host "`nRefreshing in $RefreshInterval seconds... (Press Ctrl+C to stop)" -ForegroundColor Gray
            Start-Sleep -Seconds $RefreshInterval
        }
    } while ($Continuous)
}

Write-Host "`nDebug session complete." -ForegroundColor Green

# Usage examples:
# .\DebugAIBehavior.ps1 -AIName "Golem_1"
# .\DebugAIBehavior.ps1 -ShowAll
# .\DebugAIBehavior.ps1 -AIName "Golem_1" -Continuous
# .\DebugAIBehavior.ps1 -Continuous -RefreshInterval 2
