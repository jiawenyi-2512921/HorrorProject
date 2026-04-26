# ParseTestLogs.ps1 - 测试日志解析
param(
    [Parameter(Mandatory=$true)]
    [string]$LogPath
)

$ErrorActionPreference = "Stop"

function Parse-TestLog {
    param([string]$Path)

    if (-not (Test-Path $Path)) {
        throw "Log file not found: $Path"
    }

    $content = Get-Content $Path -Raw
    $lines = $content -split "`n"

    $result = @{
        Status = "Unknown"
        StartTime = $null
        EndTime = $null
        Duration = 0
        Statistics = @{
            Total = 0
            Passed = 0
            Failed = 0
            Skipped = 0
            FailureRate = 0
        }
        FailedTests = @()
        PassedTests = @()
        SkippedTests = @()
        Coverage = @{
            Line = 0
            Branch = 0
            Function = 0
        }
    }

    # 解析测试统计
    if ($content -match "Total Tests:\s*(\d+)") {
        $result.Statistics.Total = [int]$matches[1]
    }
    if ($content -match "Passed:\s*(\d+)") {
        $result.Statistics.Passed = [int]$matches[1]
    }
    if ($content -match "Failed:\s*(\d+)") {
        $result.Statistics.Failed = [int]$matches[1]
    }
    if ($content -match "Skipped:\s*(\d+)") {
        $result.Statistics.Skipped = [int]$matches[1]
    }
    if ($content -match "Duration:\s*(\d+\.?\d*)\s*seconds") {
        $result.Duration = [double]$matches[1]
    }

    # 计算失败率
    if ($result.Statistics.Total -gt 0) {
        $result.Statistics.FailureRate = [math]::Round($result.Statistics.Failed / $result.Statistics.Total * 100, 2)
    }

    # 确定状态
    $result.Status = if ($result.Statistics.Failed -eq 0) { "Passed" } else { "Failed" }

    # 提取失败的测试
    foreach ($line in $lines) {
        if ($line -match "FAILED:\s*(.+)") {
            $testName = $matches[1].Trim()
            $result.FailedTests += @{
                Name = $testName
                Line = $line.Trim()
            }
        }
    }

    # 提取通过的测试
    foreach ($line in $lines) {
        if ($line -match "PASSED:\s*(.+)") {
            $testName = $matches[1].Trim()
            $result.PassedTests += @{
                Name = $testName
            }
        }
    }

    # 提取跳过的测试
    foreach ($line in $lines) {
        if ($line -match "SKIPPED:\s*(.+)") {
            $testName = $matches[1].Trim()
            $result.SkippedTests += @{
                Name = $testName
            }
        }
    }

    # 解析覆盖率
    if ($content -match "Line Coverage:\s*(\d+\.?\d*)%") {
        $result.Coverage.Line = [double]$matches[1]
    }
    if ($content -match "Branch Coverage:\s*(\d+\.?\d*)%") {
        $result.Coverage.Branch = [double]$matches[1]
    }
    if ($content -match "Function Coverage:\s*(\d+\.?\d*)%") {
        $result.Coverage.Function = [double]$matches[1]
    }

    return $result
}

# 主执行
$parsed = Parse-TestLog -Path $LogPath
$parsed | ConvertTo-Json -Depth 10
