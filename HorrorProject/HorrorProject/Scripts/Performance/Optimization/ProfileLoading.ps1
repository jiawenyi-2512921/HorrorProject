# ProfileLoading.ps1
# Loading Time Profiling Tool
# Analyzes level loading and asset streaming performance

param(
    [string]$ProjectPath = "D:\gptzuo\HorrorProject\HorrorProject",
    [string]$OutputDir = "D:\gptzuo\HorrorProject\HorrorProject\Saved\Profiling\Loading",
    [string]$Map = "MainLevel",
    [int]$Iterations = 5
)

$ErrorActionPreference = "Stop"

Write-Host "=== Loading Time Profiler ===" -ForegroundColor Cyan
Write-Host "Iterations: $Iterations"
Write-Host ""

# Create output directory
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reportFile = Join-Path $OutputDir "loading_profile_$timestamp.json"

# UE5 Editor path
$UE5Editor = "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

if (-not (Test-Path $UE5Editor)) {
    Write-Error "UE5 Editor not found at: $UE5Editor"
    exit 1
}

$profileData = @{
    Timestamp = $timestamp
    Map = $Map
    Iterations = $Iterations
    LoadingTimes = @()
    AverageLoadTime = 0
    MinLoadTime = 0
    MaxLoadTime = 0
    AssetLoadingBreakdown = @{}
}

Write-Host "Starting loading time analysis..." -ForegroundColor Yellow

for ($i = 1; $i -le $Iterations; $i++) {
    Write-Host ""
    Write-Host "Iteration $i of $Iterations" -ForegroundColor Green

    # Clear log file
    $logPath = Join-Path $ProjectPath "Saved\Logs\HorrorProject.log"
    if (Test-Path $logPath) {
        Remove-Item $logPath -Force
    }

    # Build command line arguments
    $args = @(
        "`"$ProjectPath\HorrorProject.uproject`"",
        "$Map",
        "-game",
        "-windowed",
        "-ResX=1920",
        "-ResY=1080",
        "-log",
        "-unattended",
        "-nosplash",
        "-ExecCmds=`"stat levels;stat streaming`""
    )

    # Measure loading time
    $startTime = Get-Date
    $process = Start-Process -FilePath $UE5Editor -ArgumentList $args -PassThru -NoNewWindow

    # Wait for level to load (look for specific log entry)
    $loaded = $false
    $timeout = 300 # 5 minutes timeout
    $elapsed = 0

    while (-not $loaded -and $elapsed -lt $timeout -and -not $process.HasExited) {
        Start-Sleep -Seconds 2
        $elapsed += 2

        if (Test-Path $logPath) {
            $logContent = Get-Content $logPath -Tail 100 -ErrorAction SilentlyContinue
            if ($logContent -match "LoadMap.*took.*seconds" -or $logContent -match "Game Engine Initialized") {
                $loaded = $true
            }
        }
    }

    $endTime = Get-Date
    $loadTime = ($endTime - $startTime).TotalSeconds

    # Stop process
    if (-not $process.HasExited) {
        $process.Kill()
        $process.WaitForExit(5000)
    }

    Write-Host "  Load time: $([math]::Round($loadTime, 2)) seconds" -ForegroundColor Cyan

    # Parse detailed loading information
    if (Test-Path $logPath) {
        $logContent = Get-Content $logPath

        # Extract asset loading times
        $assetLoadTimes = @{}
        $assetLines = $logContent | Select-String "LoadPackage:\s*(\S+)\s+took\s+(\d+\.?\d*)\s*ms"
        foreach ($line in $assetLines) {
            $assetName = $line.Matches.Groups[1].Value
            $time = [float]$line.Matches.Groups[2].Value

            $assetType = "Other"
            if ($assetName -match "\.uasset$") {
                if ($assetName -match "Texture") { $assetType = "Textures" }
                elseif ($assetName -match "Material") { $assetType = "Materials" }
                elseif ($assetName -match "StaticMesh") { $assetType = "Meshes" }
                elseif ($assetName -match "Sound") { $assetType = "Audio" }
                elseif ($assetName -match "Blueprint") { $assetType = "Blueprints" }
            }

            if (-not $assetLoadTimes.ContainsKey($assetType)) {
                $assetLoadTimes[$assetType] = @()
            }
            $assetLoadTimes[$assetType] += $time
        }

        # Store iteration data
        $iterationData = @{
            Iteration = $i
            TotalLoadTime = $loadTime
            AssetBreakdown = @{}
        }

        foreach ($assetType in $assetLoadTimes.Keys) {
            $times = $assetLoadTimes[$assetType]
            $iterationData.AssetBreakdown[$assetType] = @{
                Count = $times.Count
                TotalTime = ($times | Measure-Object -Sum).Sum
                AverageTime = ($times | Measure-Object -Average).Average
            }
        }

        $profileData.LoadingTimes += $iterationData
    }
}

# Calculate statistics
$loadTimes = $profileData.LoadingTimes | ForEach-Object { $_.TotalLoadTime }
$profileData.AverageLoadTime = ($loadTimes | Measure-Object -Average).Average
$profileData.MinLoadTime = ($loadTimes | Measure-Object -Minimum).Minimum
$profileData.MaxLoadTime = ($loadTimes | Measure-Object -Maximum).Maximum

# Aggregate asset loading breakdown
$allAssetTypes = $profileData.LoadingTimes | ForEach-Object { $_.AssetBreakdown.Keys } | Select-Object -Unique
foreach ($assetType in $allAssetTypes) {
    $typeCounts = @()
    $typeTimes = @()

    foreach ($iteration in $profileData.LoadingTimes) {
        if ($iteration.AssetBreakdown.ContainsKey($assetType)) {
            $typeCounts += $iteration.AssetBreakdown[$assetType].Count
            $typeTimes += $iteration.AssetBreakdown[$assetType].TotalTime
        }
    }

    if ($typeCounts.Count -gt 0) {
        $profileData.AssetLoadingBreakdown[$assetType] = @{
            AverageCount = ($typeCounts | Measure-Object -Average).Average
            AverageTotalTime = ($typeTimes | Measure-Object -Average).Average
        }
    }
}

# Save report
$profileData | ConvertTo-Json -Depth 10 | Out-File $reportFile -Encoding UTF8

Write-Host ""
Write-Host "=== Loading Time Analysis Results ===" -ForegroundColor Cyan
Write-Host ""

Write-Host "Overall Loading Times:" -ForegroundColor Yellow
Write-Host "  Average: $([math]::Round($profileData.AverageLoadTime, 2)) seconds"
Write-Host "  Min: $([math]::Round($profileData.MinLoadTime, 2)) seconds"
Write-Host "  Max: $([math]::Round($profileData.MaxLoadTime, 2)) seconds"
Write-Host ""

if ($profileData.AssetLoadingBreakdown.Count -gt 0) {
    Write-Host "Asset Loading Breakdown:" -ForegroundColor Yellow
    $sortedAssets = $profileData.AssetLoadingBreakdown.GetEnumerator() |
        Sort-Object { $_.Value.AverageTotalTime } -Descending

    foreach ($asset in $sortedAssets) {
        $avgTime = [math]::Round($asset.Value.AverageTotalTime, 2)
        $avgCount = [math]::Round($asset.Value.AverageCount, 0)
        Write-Host "  $($asset.Key): $avgTime ms (avg $avgCount assets)"
    }
    Write-Host ""
}

Write-Host "Report saved to: $reportFile" -ForegroundColor Green
Write-Host ""

# Loading optimization recommendations
Write-Host "=== Loading Optimization Recommendations ===" -ForegroundColor Cyan

if ($profileData.AverageLoadTime -gt 30) {
    Write-Host "- Loading time is high (>30s). Consider:" -ForegroundColor Red
    Write-Host "  * Implement level streaming" -ForegroundColor Yellow
    Write-Host "  * Reduce initial asset loading" -ForegroundColor Yellow
    Write-Host "  * Use async loading for non-critical assets" -ForegroundColor Yellow
}

if ($profileData.AssetLoadingBreakdown.Textures -and $profileData.AssetLoadingBreakdown.Textures.AverageTotalTime -gt 5000) {
    Write-Host "- Texture loading is slow. Use texture streaming" -ForegroundColor Yellow
}

if ($profileData.AssetLoadingBreakdown.Meshes -and $profileData.AssetLoadingBreakdown.Meshes.AverageTotalTime -gt 3000) {
    Write-Host "- Mesh loading is slow. Optimize mesh complexity" -ForegroundColor Yellow
}

if ($profileData.AssetLoadingBreakdown.Blueprints -and $profileData.AssetLoadingBreakdown.Blueprints.AverageTotalTime -gt 2000) {
    Write-Host "- Blueprint loading is slow. Consider nativizing or simplifying" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Loading profiling complete!" -ForegroundColor Green
