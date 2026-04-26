# BenchmarkFramerate.ps1
# Framerate Benchmark Tool
# Measures FPS performance across different quality settings

param(
    [string]$ProjectPath = "D:\gptzuo\HorrorProject\HorrorProject",
    [string]$OutputDir = "D:\gptzuo\HorrorProject\HorrorProject\Saved\Benchmarks",
    [string]$Map = "MainLevel",
    [int]$DurationSeconds = 120
)

$ErrorActionPreference = "Stop"

Write-Host "=== Framerate Benchmark Tool ===" -ForegroundColor Cyan
Write-Host ""

# Create output directory
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$reportFile = Join-Path $OutputDir "framerate_benchmark_$timestamp.json"

# Quality presets to test
$qualityPresets = @("Low", "Medium", "High", "Epic")

# UE5 Editor path
$UE5Root = if ($env:UE5_ROOT) { $env:UE5_ROOT } elseif ($env:UE_5_6_ROOT) { $env:UE_5_6_ROOT } elseif (Test-Path 'D:\UnrealEngine\UE_5.6') { 'D:\UnrealEngine\UE_5.6' } else { 'C:\Program Files\Epic Games\UE_5.6' }
$UE5Editor = Join-Path $UE5Root "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if (-not (Test-Path $UE5Editor)) {
    Write-Error "UE5 Editor not found at: $UE5Editor"
    exit 1
}

$benchmarkData = @{
    Timestamp = $timestamp
    Map = $Map
    Duration = $DurationSeconds
    Results = @{}
}

foreach ($quality in $qualityPresets) {
    Write-Host ""
    Write-Host "=== Testing Quality: $quality ===" -ForegroundColor Yellow

    # Clear log
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
        "-ExecCmds=`"sg.ViewDistanceQuality $(switch($quality){'Low'{0}'Medium'{1}'High'{2}'Epic'{3}});sg.AntiAliasingQuality $(switch($quality){'Low'{0}'Medium'{1}'High'{2}'Epic'{3}});sg.ShadowQuality $(switch($quality){'Low'{0}'Medium'{1}'High'{2}'Epic'{3}});sg.PostProcessQuality $(switch($quality){'Low'{0}'Medium'{1}'High'{2}'Epic'{3}});sg.TextureQuality $(switch($quality){'Low'{0}'Medium'{1}'High'{2}'Epic'{3}});sg.EffectsQuality $(switch($quality){'Low'{0}'Medium'{1}'High'{2}'Epic'{3}});sg.FoliageQuality $(switch($quality){'Low'{0}'Medium'{1}'High'{2}'Epic'{3}});stat fps;stat unit`"",
        "-log",
        "-unattended",
        "-nosplash"
    )

    # Start game
    $process = Start-Process -FilePath $UE5Editor -ArgumentList $args -PassThru -NoNewWindow

    Write-Host "Running benchmark for $DurationSeconds seconds..." -ForegroundColor Green

    # Wait for benchmark duration
    $elapsed = 0
    while ($elapsed -lt $DurationSeconds -and -not $process.HasExited) {
        Start-Sleep -Seconds 10
        $elapsed += 10
        $progress = [math]::Min(100, ($elapsed / $DurationSeconds) * 100)
        Write-Progress -Activity "Benchmarking $quality" -Status "$elapsed / $DurationSeconds seconds" -PercentComplete $progress
    }

    Write-Progress -Activity "Benchmarking $quality" -Completed

    # Stop game
    if (-not $process.HasExited) {
        $process.Kill()
        $process.WaitForExit(5000)
    }

    Write-Host "Analyzing results..." -ForegroundColor Cyan

    # Parse log file
    if (Test-Path $logPath) {
        $logContent = Get-Content $logPath -Tail 5000

        # Extract FPS data
        $fpsData = $logContent | Select-String "FPS:\s*(\d+\.?\d*)" | ForEach-Object {
            [float]$_.Matches.Groups[1].Value
        }

        # Extract frame time data
        $frameTimeData = $logContent | Select-String "Frame:\s*(\d+\.?\d*)\s*ms" | ForEach-Object {
            [float]$_.Matches.Groups[1].Value
        }

        # Extract game thread time
        $gameThreadData = $logContent | Select-String "Game:\s*(\d+\.?\d*)\s*ms" | ForEach-Object {
            [float]$_.Matches.Groups[1].Value
        }

        # Extract render thread time
        $renderThreadData = $logContent | Select-String "Render:\s*(\d+\.?\d*)\s*ms" | ForEach-Object {
            [float]$_.Matches.Groups[1].Value
        }

        # Extract GPU time
        $gpuTimeData = $logContent | Select-String "GPU:\s*(\d+\.?\d*)\s*ms" | ForEach-Object {
            [float]$_.Matches.Groups[1].Value
        }

        # Calculate statistics
        $qualityResults = @{
            Quality = $quality
            FPS = @{}
            FrameTime = @{}
            GameThread = @{}
            RenderThread = @{}
            GPU = @{}
        }

        if ($fpsData.Count -gt 0) {
            $sortedFPS = $fpsData | Sort-Object
            $qualityResults.FPS = @{
                Average = ($fpsData | Measure-Object -Average).Average
                Min = ($fpsData | Measure-Object -Minimum).Minimum
                Max = ($fpsData | Measure-Object -Maximum).Maximum
                P1 = $sortedFPS[[math]::Floor($sortedFPS.Count * 0.01)]
                P5 = $sortedFPS[[math]::Floor($sortedFPS.Count * 0.05)]
                Median = $sortedFPS[[math]::Floor($sortedFPS.Count * 0.5)]
                P95 = $sortedFPS[[math]::Floor($sortedFPS.Count * 0.95)]
                P99 = $sortedFPS[[math]::Floor($sortedFPS.Count * 0.99)]
            }
        }

        if ($frameTimeData.Count -gt 0) {
            $sortedFrameTime = $frameTimeData | Sort-Object
            $qualityResults.FrameTime = @{
                Average = ($frameTimeData | Measure-Object -Average).Average
                Min = ($frameTimeData | Measure-Object -Minimum).Minimum
                Max = ($frameTimeData | Measure-Object -Maximum).Maximum
                P95 = $sortedFrameTime[[math]::Floor($sortedFrameTime.Count * 0.95)]
                P99 = $sortedFrameTime[[math]::Floor($sortedFrameTime.Count * 0.99)]
            }
        }

        if ($gameThreadData.Count -gt 0) {
            $qualityResults.GameThread = @{
                Average = ($gameThreadData | Measure-Object -Average).Average
                Max = ($gameThreadData | Measure-Object -Maximum).Maximum
            }
        }

        if ($renderThreadData.Count -gt 0) {
            $qualityResults.RenderThread = @{
                Average = ($renderThreadData | Measure-Object -Average).Average
                Max = ($renderThreadData | Measure-Object -Maximum).Maximum
            }
        }

        if ($gpuTimeData.Count -gt 0) {
            $qualityResults.GPU = @{
                Average = ($gpuTimeData | Measure-Object -Average).Average
                Max = ($gpuTimeData | Measure-Object -Maximum).Maximum
            }
        }

        $benchmarkData.Results[$quality] = $qualityResults

        # Display results
        Write-Host ""
        Write-Host "Results for ${quality}:" -ForegroundColor Green
        if ($qualityResults.FPS.Average) {
            Write-Host "  Average FPS: $([math]::Round($qualityResults.FPS.Average, 2))"
            Write-Host "  Min FPS: $([math]::Round($qualityResults.FPS.Min, 2))"
            Write-Host "  1% Low: $([math]::Round($qualityResults.FPS.P1, 2))"
            Write-Host "  5% Low: $([math]::Round($qualityResults.FPS.P5, 2))"
        }
        if ($qualityResults.FrameTime.Average) {
            Write-Host "  Avg Frame Time: $([math]::Round($qualityResults.FrameTime.Average, 2)) ms"
        }
    }
}

# Save report
$benchmarkData | ConvertTo-Json -Depth 10 | Out-File $reportFile -Encoding UTF8

Write-Host ""
Write-Host "=== Benchmark Summary ===" -ForegroundColor Cyan
Write-Host ""

# Compare quality settings
Write-Host "Quality Comparison:" -ForegroundColor Yellow
Write-Host ""
Write-Host "Quality  | Avg FPS | Min FPS | 1% Low | Frame Time" -ForegroundColor White
Write-Host "---------|---------|---------|--------|------------" -ForegroundColor White

foreach ($quality in $qualityPresets) {
    if ($benchmarkData.Results.ContainsKey($quality)) {
        $result = $benchmarkData.Results[$quality]
        $avgFPS = if ($result.FPS.Average) { [math]::Round($result.FPS.Average, 1) } else { "N/A" }
        $minFPS = if ($result.FPS.Min) { [math]::Round($result.FPS.Min, 1) } else { "N/A" }
        $p1FPS = if ($result.FPS.P1) { [math]::Round($result.FPS.P1, 1) } else { "N/A" }
        $frameTime = if ($result.FrameTime.Average) { [math]::Round($result.FrameTime.Average, 1) } else { "N/A" }

        $color = "White"
        if ($avgFPS -ne "N/A" -and $avgFPS -ge 60) { $color = "Green" }
        elseif ($avgFPS -ne "N/A" -and $avgFPS -ge 30) { $color = "Yellow" }
        elseif ($avgFPS -ne "N/A") { $color = "Red" }

        Write-Host ("{0,-8} | {1,7} | {2,7} | {3,6} | {4,7} ms" -f $quality, $avgFPS, $minFPS, $p1FPS, $frameTime) -ForegroundColor $color
    }
}

Write-Host ""
Write-Host "Report saved to: $reportFile" -ForegroundColor Green
Write-Host ""

# Recommendations
Write-Host "=== Recommendations ===" -ForegroundColor Cyan

$epicResult = $benchmarkData.Results["Epic"]
if ($epicResult -and $epicResult.FPS.Average) {
    if ($epicResult.FPS.Average -ge 60) {
        Write-Host "Epic quality achieves 60 FPS target!" -ForegroundColor Green
    } elseif ($epicResult.FPS.Average -ge 30) {
        Write-Host "Epic quality achieves 30+ FPS but below 60 FPS target" -ForegroundColor Yellow
        Write-Host "Consider optimizations or use High quality preset" -ForegroundColor Yellow
    } else {
        Write-Host "Epic quality below 30 FPS - significant optimization needed" -ForegroundColor Red
    }
}

Write-Host ""
Write-Host "Framerate benchmark complete!" -ForegroundColor Green
