# AggregateLogs.ps1 - 日志聚合
param(
    [string]$ProjectPath = "",
    [int]$DaysToKeep = 30
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "..\..\Validation\Common.ps1")
$ProjectRoot = Get-HorrorProjectRoot -StartPath $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($ProjectPath)) { $ProjectPath = $ProjectRoot }

$MonitoringRoot = Split-Path -Parent $PSScriptRoot
$LogDir = Join-Path $MonitoringRoot "Logs"
$AggregatedDir = Join-Path $MonitoringRoot "Data\Aggregated"

New-Item -ItemType Directory -Force -Path $AggregatedDir | Out-Null

function Get-LogSources {
    return @(
        @{
            Name = "Build"
            Pattern = "$LogDir\build-*.log"
            Parser = "ParseBuildLogs.ps1"
        }
        @{
            Name = "Test"
            Pattern = "$LogDir\test-*.log"
            Parser = "ParseTestLogs.ps1"
        }
        @{
            Name = "Editor"
            Pattern = "$ProjectPath\Saved\Logs\*.log"
            Parser = $null
        }
        @{
            Name = "Performance"
            Pattern = "$LogDir\performance-*.log"
            Parser = $null
        }
    )
}

function Aggregate-LogsByType {
    param([string]$Type, [string]$Pattern, [string]$Parser)

    Write-Host "Aggregating $Type logs..." -ForegroundColor Cyan

    $logs = Get-ChildItem $Pattern -ErrorAction SilentlyContinue
    if (-not $logs) {
        Write-Host "  No logs found" -ForegroundColor Gray
        return
    }

    $aggregated = @{
        Type = $Type
        Timestamp = Get-Date
        TotalLogs = $logs.Count
        TotalSizeMB = [math]::Round(($logs | Measure-Object -Property Length -Sum).Sum / 1MB, 2)
        Logs = @()
    }

    foreach ($log in $logs) {
        $logInfo = @{
            FileName = $log.Name
            Path = $log.FullName
            SizeMB = [math]::Round($log.Length / 1MB, 2)
            Created = $log.CreationTime
            Modified = $log.LastWriteTime
        }

        # 如果有解析器，使用解析器提取关键信息
        if ($Parser) {
            $parserScript = Join-Path $PSScriptRoot $Parser
            if (Test-Path $parserScript) {
                try {
                    $parsed = & $parserScript -LogPath $log.FullName
                    $logInfo.Parsed = $parsed
                } catch {
                    Write-Host "  Failed to parse $($log.Name): $_" -ForegroundColor Yellow
                }
            }
        }

        $aggregated.Logs += $logInfo
    }

    # 保存聚合结果
    $timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
    $outputFile = Join-Path $AggregatedDir "$Type-aggregated-$timestamp.json"
    $aggregated | ConvertTo-Json -Depth 10 | Set-Content $outputFile

    Write-Host "  Aggregated $($aggregated.TotalLogs) logs ($($aggregated.TotalSizeMB) MB)" -ForegroundColor Green
    Write-Host "  Output: $outputFile" -ForegroundColor Gray
}

function Clean-OldLogs {
    param([int]$Days)

    Write-Host "`nCleaning old logs..." -ForegroundColor Cyan

    $cutoff = (Get-Date).AddDays(-$Days)
    $sources = Get-LogSources

    $totalCleaned = 0
    $totalSizeMB = 0

    foreach ($source in $sources) {
        $oldLogs = Get-ChildItem $source.Pattern -ErrorAction SilentlyContinue |
            Where-Object { $_.LastWriteTime -lt $cutoff }

        if ($oldLogs) {
            $sizeMB = [math]::Round(($oldLogs | Measure-Object -Property Length -Sum).Sum / 1MB, 2)
            $oldLogs | Remove-Item -Force
            $totalCleaned += $oldLogs.Count
            $totalSizeMB += $sizeMB
            Write-Host "  Cleaned $($oldLogs.Count) $($source.Name) logs ($sizeMB MB)" -ForegroundColor Yellow
        }
    }

    Write-Host "  Total cleaned: $totalCleaned logs ($totalSizeMB MB)" -ForegroundColor Green
}

function Generate-LogSummary {
    $sources = Get-LogSources
    $summary = @{
        Timestamp = Get-Date
        Sources = @()
        TotalLogs = 0
        TotalSizeMB = 0
    }

    foreach ($source in $sources) {
        $logs = Get-ChildItem $source.Pattern -ErrorAction SilentlyContinue
        if ($logs) {
            $sizeMB = [math]::Round(($logs | Measure-Object -Property Length -Sum).Sum / 1MB, 2)
            $summary.Sources += @{
                Name = $source.Name
                Count = $logs.Count
                SizeMB = $sizeMB
            }
            $summary.TotalLogs += $logs.Count
            $summary.TotalSizeMB += $sizeMB
        }
    }

    $summaryFile = Join-Path $AggregatedDir "log-summary.json"
    $summary | ConvertTo-Json -Depth 10 | Set-Content $summaryFile

    return $summary
}

# 主执行
Write-Host "=== Log Aggregation Started ===" -ForegroundColor Green
Write-Host ""

$sources = Get-LogSources

foreach ($source in $sources) {
    Aggregate-LogsByType -Type $source.Name -Pattern $source.Pattern -Parser $source.Parser
}

Write-Host ""
Clean-OldLogs -Days $DaysToKeep

Write-Host ""
$summary = Generate-LogSummary

Write-Host "`n=== Summary ===" -ForegroundColor Green
Write-Host "Total Logs: $($summary.TotalLogs)"
Write-Host "Total Size: $($summary.TotalSizeMB) MB"
Write-Host ""

foreach ($source in $summary.Sources) {
    Write-Host "  $($source.Name): $($source.Count) logs ($($source.SizeMB) MB)"
}

Write-Host "`nLog aggregation completed" -ForegroundColor Green
