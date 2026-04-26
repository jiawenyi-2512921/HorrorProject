# AlertOnMemoryLeak.ps1 - 内存泄漏告警
param(
    [int]$CheckInterval = 300,
    [double]$GrowthThresholdMB = 100
)

$ErrorActionPreference = "Stop"
$MonitoringRoot = Split-Path -Parent $PSScriptRoot
$DataDir = Join-Path $MonitoringRoot "Data\Performance"
$AlertScript = Join-Path $PSScriptRoot "AlertSystem.ps1"

function Detect-MemoryLeak {
    $cutoff = (Get-Date).AddHours(-1)
    $perfFiles = Get-ChildItem $DataDir -Filter "perf-*.json" |
        Where-Object { $_.LastWriteTime -gt $cutoff } |
        Sort-Object LastWriteTime

    if ($perfFiles.Count -lt 10) {
        return $null
    }

    $memoryValues = @()
    foreach ($file in $perfFiles) {
        $data = Get-Content $file.FullName | ConvertFrom-Json
        if ($data.Process.Status -eq "Running") {
            $memoryValues += $data.Process.MemoryMB
        }
    }

    if ($memoryValues.Count -lt 10) {
        return $null
    }

    # 计算内存增长趋势
    $firstHalf = $memoryValues[0..([math]::Floor($memoryValues.Count / 2) - 1)]
    $secondHalf = $memoryValues[([math]::Floor($memoryValues.Count / 2))..($memoryValues.Count - 1)]

    $avgFirst = ($firstHalf | Measure-Object -Average).Average
    $avgSecond = ($secondHalf | Measure-Object -Average).Average
    $growth = $avgSecond - $avgFirst

    if ($growth -gt $GrowthThresholdMB) {
        return @{
            Type = "MemoryLeak"
            Severity = "Critical"
            Message = "Potential memory leak detected: $([math]::Round($growth, 2)) MB growth in 1 hour"
            Details = @{
                GrowthMB = [math]::Round($growth, 2)
                Threshold = $GrowthThresholdMB
                InitialMemoryMB = [math]::Round($avgFirst, 2)
                CurrentMemoryMB = [math]::Round($avgSecond, 2)
                Samples = $memoryValues.Count
            }
        }
    }

    return $null
}

Write-Host "Starting Memory Leak Alert Monitor..." -ForegroundColor Green
Write-Host "Check Interval: $CheckInterval seconds"
Write-Host "Growth Threshold: $GrowthThresholdMB MB/hour"
Write-Host ""

while ($true) {
    try {
        $alert = Detect-MemoryLeak

        if ($alert) {
            & $AlertScript -Type $alert.Type -Severity $alert.Severity -Message $alert.Message -Details $alert.Details
        }

    } catch {
        Write-Host "Error: $_" -ForegroundColor Red
    }

    Start-Sleep -Seconds $CheckInterval
}
