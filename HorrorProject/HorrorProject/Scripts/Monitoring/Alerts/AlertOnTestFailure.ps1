# AlertOnTestFailure.ps1 - 测试失败告警
param(
    [int]$CheckInterval = 60,
    [double]$FailureRateThreshold = 5.0
)

$ErrorActionPreference = "Stop"
$MonitoringRoot = Split-Path -Parent $PSScriptRoot
$DataDir = Join-Path $MonitoringRoot "Data\Test"
$AlertScript = Join-Path $PSScriptRoot "AlertSystem.ps1"

function Check-TestFailures {
    $latestFile = Join-Path $DataDir "test-status-latest.json"

    if (-not (Test-Path $latestFile)) {
        return $null
    }

    $status = Get-Content $latestFile | ConvertFrom-Json

    # 检查测试失败
    if ($status.Failed -gt 0) {
        return @{
            Type = "TestFailure"
            Severity = if ($status.FailureRate -gt 10) { "Critical" } else { "Warning" }
            Message = "$($status.Failed) tests failed (Failure Rate: $($status.FailureRate)%)"
            Details = @{
                Total = $status.Total
                Passed = $status.Passed
                Failed = $status.Failed
                Skipped = $status.Skipped
                FailureRate = $status.FailureRate
                FailedTests = $status.FailedTests
            }
        }
    }

    # 检查失败率趋势
    if ($status.FailureRate -gt $FailureRateThreshold) {
        return @{
            Type = "TestFailureRate"
            Severity = "Warning"
            Message = "Test failure rate ($($status.FailureRate)%) exceeds threshold ($FailureRateThreshold%)"
            Details = @{
                FailureRate = $status.FailureRate
                Threshold = $FailureRateThreshold
                Failed = $status.Failed
                Total = $status.Total
            }
        }
    }

    # 检查代码覆盖率下降
    if ($status.Coverage.Line -lt 70) {
        return @{
            Type = "CodeCoverage"
            Severity = "Warning"
            Message = "Code coverage dropped below 70%: $($status.Coverage.Line)%"
            Details = @{
                LineCoverage = $status.Coverage.Line
                BranchCoverage = $status.Coverage.Branch
                FunctionCoverage = $status.Coverage.Function
            }
        }
    }

    return $null
}

Write-Host "Starting Test Failure Alert Monitor..." -ForegroundColor Green
Write-Host "Check Interval: $CheckInterval seconds"
Write-Host "Failure Rate Threshold: $FailureRateThreshold%"
Write-Host ""

while ($true) {
    try {
        $alert = Check-TestFailures

        if ($alert) {
            & $AlertScript -Type $alert.Type -Severity $alert.Severity -Message $alert.Message -Details $alert.Details
        }

    } catch {
        Write-Host "Error: $_" -ForegroundColor Red
    }

    Start-Sleep -Seconds $CheckInterval
}
