# MonitorAssetStatus.ps1 - 资产状态监控
param(
    [string]$ProjectPath = "D:\gptzuo\HorrorProject",
    [int]$CheckInterval = 60
)

$ErrorActionPreference = "Stop"
$MonitoringRoot = Split-Path -Parent $PSScriptRoot
$DataDir = Join-Path $MonitoringRoot "Data\Assets"
$LogDir = Join-Path $MonitoringRoot "Logs"

New-Item -ItemType Directory -Force -Path $DataDir, $LogDir | Out-Null

function Get-AssetMetrics {
    $contentDir = Join-Path $ProjectPath "Content"
    if (-not (Test-Path $contentDir)) {
        return @{
            Status = "NoData"
            Message = "Content directory not found"
        }
    }

    # 统计资产文件
    $assets = @{
        Textures = @(Get-ChildItem $contentDir -Recurse -Include "*.uasset" | Where-Object { $_.Directory.Name -match "Texture" }).Count
        Materials = @(Get-ChildItem $contentDir -Recurse -Include "*.uasset" | Where-Object { $_.Directory.Name -match "Material" }).Count
        Meshes = @(Get-ChildItem $contentDir -Recurse -Include "*.uasset" | Where-Object { $_.Directory.Name -match "Mesh" }).Count
        Blueprints = @(Get-ChildItem $contentDir -Recurse -Include "*.uasset" | Where-Object { $_.Directory.Name -match "Blueprint" }).Count
        Sounds = @(Get-ChildItem $contentDir -Recurse -Include "*.uasset" | Where-Object { $_.Directory.Name -match "Sound|Audio" }).Count
        Animations = @(Get-ChildItem $contentDir -Recurse -Include "*.uasset" | Where-Object { $_.Directory.Name -match "Animation" }).Count
    }

    $totalAssets = ($assets.Values | Measure-Object -Sum).Sum

    # 检查资产验证日志
    $validationLog = Join-Path $LogDir "asset-validation.log"
    $errors = 0
    $warnings = 0

    if (Test-Path $validationLog) {
        $content = Get-Content $validationLog -Raw
        $errors = ([regex]::Matches($content, "(?i)error")).Count
        $warnings = ([regex]::Matches($content, "(?i)warning")).Count
    }

    # 计算资产大小
    $totalSize = (Get-ChildItem $contentDir -Recurse -File | Measure-Object -Property Length -Sum).Sum
    $totalSizeGB = [math]::Round($totalSize / 1GB, 2)

    return @{
        Status = if ($errors -eq 0) { "Healthy" } else { "Issues" }
        TotalAssets = $totalAssets
        AssetsByType = $assets
        TotalSizeGB = $totalSizeGB
        Errors = $errors
        Warnings = $warnings
        Timestamp = Get-Date
    }
}

function Get-AssetTrend {
    param([int]$Days = 7)

    $cutoff = (Get-Date).AddDays(-$Days)
    $assetFiles = Get-ChildItem $DataDir -Filter "asset-status-*.json" |
        Where-Object { $_.LastWriteTime -gt $cutoff } |
        Sort-Object LastWriteTime

    $trend = @{
        AssetGrowth = 0
        SizeGrowth = 0
        ErrorTrend = @()
        WarningTrend = @()
    }

    if ($assetFiles.Count -lt 2) { return $trend }

    $first = Get-Content $assetFiles[0].FullName | ConvertFrom-Json
    $last = Get-Content $assetFiles[-1].FullName | ConvertFrom-Json

    $trend.AssetGrowth = $last.TotalAssets - $first.TotalAssets
    $trend.SizeGrowth = [math]::Round($last.TotalSizeGB - $first.TotalSizeGB, 2)

    foreach ($file in $assetFiles) {
        $data = Get-Content $file.FullName | ConvertFrom-Json
        $trend.ErrorTrend += $data.Errors
        $trend.WarningTrend += $data.Warnings
    }

    return $trend
}

function Save-AssetStatus {
    param([hashtable]$Status)

    $timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
    $statusFile = Join-Path $DataDir "asset-status-$timestamp.json"

    $Status | ConvertTo-Json -Depth 10 | Set-Content $statusFile

    $latestFile = Join-Path $DataDir "asset-status-latest.json"
    $Status | ConvertTo-Json -Depth 10 | Set-Content $latestFile
}

function Show-AssetDashboard {
    param([hashtable]$Status, [hashtable]$Trend)

    Clear-Host
    Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
    Write-Host "║           ASSET STATUS MONITORING DASHBOARD               ║" -ForegroundColor Cyan
    Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
    Write-Host ""

    Write-Host "Asset Status:" -ForegroundColor Yellow
    $statusColor = if ($Status.Status -eq "Healthy") { "Green" } else { "Red" }
    Write-Host "  Status: $($Status.Status)" -ForegroundColor $statusColor
    Write-Host "  Total Assets: $($Status.TotalAssets)"
    Write-Host "  Total Size: $($Status.TotalSizeGB) GB"
    Write-Host "  Errors: $($Status.Errors)" -ForegroundColor $(if ($Status.Errors -gt 0) { "Red" } else { "Green" })
    Write-Host "  Warnings: $($Status.Warnings)" -ForegroundColor $(if ($Status.Warnings -gt 0) { "Yellow" } else { "Green" })
    Write-Host ""

    Write-Host "Assets by Type:" -ForegroundColor Yellow
    foreach ($type in $Status.AssetsByType.Keys | Sort-Object) {
        Write-Host "  $type: $($Status.AssetsByType[$type])"
    }
    Write-Host ""

    Write-Host "7-Day Trend:" -ForegroundColor Yellow
    Write-Host "  Asset Growth: $($Trend.AssetGrowth)" -ForegroundColor $(if ($Trend.AssetGrowth -gt 0) { "Green" } else { "Gray" })
    Write-Host "  Size Growth: $($Trend.SizeGrowth) GB"
    Write-Host ""

    Write-Host "Last updated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray
}

function Start-AssetMonitoring {
    Write-Host "Starting Asset Status Monitoring..." -ForegroundColor Green
    Write-Host "Check Interval: $CheckInterval seconds"
    Write-Host "Press Ctrl+C to stop"
    Write-Host ""

    while ($true) {
        try {
            $status = Get-AssetMetrics
            $trend = Get-AssetTrend -Days 7

            Save-AssetStatus -Status $status
            Show-AssetDashboard -Status $status -Trend $trend

            $cutoff = (Get-Date).AddDays(-30)
            Get-ChildItem $DataDir -Filter "asset-status-*.json" |
                Where-Object { $_.LastWriteTime -lt $cutoff } |
                Remove-Item -Force

        } catch {
            Write-Host "Error: $_" -ForegroundColor Red
        }

        Start-Sleep -Seconds $CheckInterval
    }
}

Start-AssetMonitoring
