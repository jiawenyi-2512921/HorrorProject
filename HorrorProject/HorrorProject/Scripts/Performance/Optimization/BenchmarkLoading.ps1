# BenchmarkLoading.ps1
# Loading Time Benchmark Tool
# Measures level loading performance

param(
    [string]$ProjectPath = "D:\gptzuo\HorrorProject\HorrorProject",
    [string]$OutputDir = "D:\gptzuo\HorrorProject\HorrorProject\Saved\Benchmarks",
    [string[]]$Maps = @("MainLevel"),
    [int]$Iterations = 3
)

$ErrorActionPreference = "Stop"

Write-Host "=== Loading Time Benchmark Tool ===" -ForegroundColor Cyan
Write-Host "Iterations per map: $Iterations"
Write-Host ""

# Create output directory
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reportFile = Join-Path $OutputDir "loading_benchmark_$timestamp.json"

# UE5 Editor path
$UE5Root = if ($env:UE5_ROOT) { $env:UE5_ROOT } elseif ($env:UE_5_6_ROOT) { $env:UE_5_6_ROOT } elseif (Test-Path 'D:\UnrealEngine\UE_5.6') { 'D:\UnrealEngine\UE_5.6' } else { 'C:\Program Files\Epic Games\UE_5.6' }
$UE5Editor = Join-Path $UE5Root "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if (-not (Test-Path $UE5Editor)) {
    Write-Error "UE5 Editor not found at: $UE5Editor"
    exit 1
}

$benchmarkData = @{
    Timestamp = $timestamp
    Iterations = $Iterations
    Maps = @{}
}

foreach ($map in $Maps) {
    Write-Host ""
    Write-Host "=== Benchmarking Map: $map ===" -ForegroundColor Yellow

    $mapResults = @{
        MapName = $map
        LoadTimes = @()
        Statistics = @{}
    }

    for ($i = 1; $i -le $Iterations; $i++) {
        Write-Host ""
        Write-Host "Iteration $i of $Iterations" -ForegroundColor Cyan

        # Clear log
        $logPath = Join-Path $ProjectPath "Saved\Logs\HorrorProject.log"
        if (Test-Path $logPath) {
            Remove-Item $logPath -Force
        }

        # Build command line arguments
        $args = @(
            "`"$ProjectPath\HorrorProject.uproject`"",
            "$map",
            "-game",
            "-windowed",
            "-ResX=1920",
            "-ResY=1080",
            "-log",
            "-unattended",
            "-nosplash"
        )

        # Measure loading time
        $startTime = Get-Date
        $process = Start-Process -FilePath $UE5Editor -ArgumentList $args -PassThru -NoNewWindow

        # Wait for level to load
        $loaded = $false
        $timeout = 300
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

        Write-Host "  Load time: $([math]::Round($loadTime, 2)) seconds" -ForegroundColor Green

        $mapResults.LoadTimes += $loadTime
    }

    # Calculate statistics
    $loadTimes = $mapResults.LoadTimes
    $mapResults.Statistics = @{
        Average = ($loadTimes | Measure-Object -Average).Average
        Min = ($loadTimes | Measure-Object -Minimum).Minimum
        Max = ($loadTimes | Measure-Object -Maximum).Maximum
        StdDev = if ($loadTimes.Count -gt 1) {
            $avg = ($loadTimes | Measure-Object -Average).Average
            $variance = ($loadTimes | ForEach-Object { [math]::Pow($_ - $avg, 2) } | Measure-Object -Average).Average
            [math]::Sqrt($variance)
        } else { 0 }
    }

    $benchmarkData.Maps[$map] = $mapResults

    Write-Host ""
    Write-Host "Results for ${map}:" -ForegroundColor Yellow
    Write-Host "  Average: $([math]::Round($mapResults.Statistics.Average, 2)) seconds"
    Write-Host "  Min: $([math]::Round($mapResults.Statistics.Min, 2)) seconds"
    Write-Host "  Max: $([math]::Round($mapResults.Statistics.Max, 2)) seconds"
    Write-Host "  Std Dev: $([math]::Round($mapResults.Statistics.StdDev, 2)) seconds"
}

# Save report
$benchmarkData | ConvertTo-Json -Depth 10 | Out-File $reportFile -Encoding UTF8

Write-Host ""
Write-Host "=== Loading Benchmark Summary ===" -ForegroundColor Cyan
Write-Host ""

Write-Host "Map Loading Times:" -ForegroundColor Yellow
Write-Host ""
Write-Host "Map          | Average | Min     | Max     | Target" -ForegroundColor White
Write-Host "-------------|---------|---------|---------|--------" -ForegroundColor White

foreach ($map in $Maps) {
    if ($benchmarkData.Maps.ContainsKey($map)) {
        $result = $benchmarkData.Maps[$map]
        $avg = [math]::Round($result.Statistics.Average, 1)
        $min = [math]::Round($result.Statistics.Min, 1)
        $max = [math]::Round($result.Statistics.Max, 1)

        $color = "White"
        if ($avg -lt 15) { $color = "Green" }
        elseif ($avg -lt 30) { $color = "Yellow" }
        else { $color = "Red" }

        $target = if ($avg -lt 15) { "✓ Pass" } elseif ($avg -lt 30) { "~ Warn" } else { "✗ Fail" }

        Write-Host ("{0,-12} | {1,7}s | {2,7}s | {3,7}s | {4}" -f $map, $avg, $min, $max, $target) -ForegroundColor $color
    }
}

Write-Host ""
Write-Host "Report saved to: $reportFile" -ForegroundColor Green
Write-Host ""

# Recommendations
Write-Host "=== Recommendations ===" -ForegroundColor Cyan

$avgLoadTime = ($benchmarkData.Maps.Values | ForEach-Object { $_.Statistics.Average } | Measure-Object -Average).Average

if ($avgLoadTime -gt 30) {
    Write-Host "Loading times are high (>30s). Consider:" -ForegroundColor Red
    Write-Host "  - Implement level streaming" -ForegroundColor Yellow
    Write-Host "  - Reduce initial asset count" -ForegroundColor Yellow
    Write-Host "  - Use async loading" -ForegroundColor Yellow
    Write-Host "  - Optimize asset sizes" -ForegroundColor Yellow
} elseif ($avgLoadTime -gt 15) {
    Write-Host "Loading times are acceptable but could be improved" -ForegroundColor Yellow
    Write-Host "  - Review asset loading priorities" -ForegroundColor Cyan
    Write-Host "  - Consider texture streaming" -ForegroundColor Cyan
} else {
    Write-Host "Loading times are good (<15s)" -ForegroundColor Green
}

Write-Host ""
Write-Host "Loading benchmark complete!" -ForegroundColor Green
