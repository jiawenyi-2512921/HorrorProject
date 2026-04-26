# AlertOnBuildFailure.ps1 - 构建失败告警
param(
    [int]$CheckInterval = 60
)

$ErrorActionPreference = "Stop"
$MonitoringRoot = Split-Path -Parent $PSScriptRoot
$DataDir = Join-Path $MonitoringRoot "Data\Build"
$AlertScript = Join-Path $PSScriptRoot "AlertSystem.ps1"

function Check-BuildFailures {
    $latestFile = Join-Path $DataDir "build-status-latest.json"

    if (-not (Test-Path $latestFile)) {
        return $null
    }

    $status = Get-Content $latestFile | ConvertFrom-Json

    if ($status.Status -eq "Failed") {
        return @{
            Type = "BuildFailure"
            Severity = "Critical"
            Message = "Build failed with $($status.Errors) errors"
            Details = @{
                Configuration = $status.Configuration
                Platform = $status.Platform
                Duration = $status.Duration
                Errors = $status.Errors
                Warnings = $status.Warnings
                Timestamp = $status.Timestamp
            }
        }
    }

    # 检查构建时间过长
    if ($status.Duration -gt 600) {
        return @{
            Type = "BuildPerformance"
            Severity = "Warning"
            Message = "Build time exceeded 10 minutes: $($status.Duration)s"
            Details = @{
                Configuration = $status.Configuration
                Platform = $status.Platform
                Duration = $status.Duration
            }
        }
    }

    # 检查警告数量
    if ($status.Warnings -gt 50) {
        return @{
            Type = "BuildWarnings"
            Severity = "Warning"
            Message = "Build has $($status.Warnings) warnings"
            Details = @{
                Configuration = $status.Configuration
                Platform = $status.Platform
                Warnings = $status.Warnings
            }
        }
    }

    return $null
}

Write-Host "Starting Build Failure Alert Monitor..." -ForegroundColor Green
Write-Host "Check Interval: $CheckInterval seconds"
Write-Host ""

while ($true) {
    try {
        $alert = Check-BuildFailures

        if ($alert) {
            & $AlertScript -Type $alert.Type -Severity $alert.Severity -Message $alert.Message -Details $alert.Details
        }

    } catch {
        Write-Host "Error: $_" -ForegroundColor Red
    }

    Start-Sleep -Seconds $CheckInterval
}
