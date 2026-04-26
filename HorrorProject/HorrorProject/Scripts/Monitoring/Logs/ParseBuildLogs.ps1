# ParseBuildLogs.ps1 - 构建日志解析
param(
    [Parameter(Mandatory=$true)]
    [string]$LogPath
)

$ErrorActionPreference = "Stop"

function Parse-BuildLog {
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
        Configuration = ""
        Platform = ""
        Errors = @()
        Warnings = @()
        Statistics = @{
            TotalErrors = 0
            TotalWarnings = 0
            CompiledFiles = 0
            LinkedLibraries = 0
        }
    }

    # 解析构建状态
    if ($content -match "BUILD SUCCESSFUL") {
        $result.Status = "Success"
    } elseif ($content -match "BUILD FAILED") {
        $result.Status = "Failed"
    } elseif ($content -match "BUILD CANCELLED") {
        $result.Status = "Cancelled"
    }

    # 解析时间信息
    if ($content -match "Build started at:\s*(.+)") {
        $result.StartTime = $matches[1]
    }
    if ($content -match "Build completed at:\s*(.+)") {
        $result.EndTime = $matches[1]
    }
    if ($content -match "Total build time:\s*(\d+\.?\d*)\s*seconds") {
        $result.Duration = [double]$matches[1]
    }

    # 解析配置和平台
    if ($content -match "Configuration:\s*(\w+)") {
        $result.Configuration = $matches[1]
    }
    if ($content -match "Platform:\s*(\w+)") {
        $result.Platform = $matches[1]
    }

    # 提取错误
    foreach ($line in $lines) {
        if ($line -match "(?i)error\s+([A-Z]+\d+):\s*(.+)") {
            $result.Errors += @{
                Code = $matches[1]
                Message = $matches[2].Trim()
                Line = $line.Trim()
            }
            $result.Statistics.TotalErrors++
        }
    }

    # 提取警告
    foreach ($line in $lines) {
        if ($line -match "(?i)warning\s+([A-Z]+\d+):\s*(.+)") {
            $result.Warnings += @{
                Code = $matches[1]
                Message = $matches[2].Trim()
                Line = $line.Trim()
            }
            $result.Statistics.TotalWarnings++
        }
    }

    # 解析编译统计
    if ($content -match "Compiled\s+(\d+)\s+files?") {
        $result.Statistics.CompiledFiles = [int]$matches[1]
    }
    if ($content -match "Linked\s+(\d+)\s+librar(?:y|ies)") {
        $result.Statistics.LinkedLibraries = [int]$matches[1]
    }

    return $result
}

# 主执行
$parsed = Parse-BuildLog -Path $LogPath
$parsed | ConvertTo-Json -Depth 10
